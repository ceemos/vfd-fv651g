/** \file server/drivers/vfd_vusb.c
 * LCDd \c vfd_vusb driver for 14-Segment VFD Displays using a simple AVR µC with VUSB Stack as a controller: https://github.com/ceemos/vfd-fv651g
 */

/*
 * vfd_vusb Driver
 *
 * Copyright (C) 2011 Marcel Schneider, m.schneider.neuffen@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 *
 * Based on work from:
 *   shuttleVFD Driver
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <usb.h>

#include "lcd.h"
#include "vfd_vusb.h"
#include "report.h"

MODULE_EXPORT char *api_version = API_VERSION;
MODULE_EXPORT int stay_in_foreground = 0;
MODULE_EXPORT int supports_multiple = 0;
MODULE_EXPORT char *symbol_prefix = "vfd_vusb_";

/** private data for the \c vfd_vusb driver */
typedef struct shuttleVFD_private_data {
  usb_dev_handle *dev;		/**< device handle */
  int width;			/**< display width in characters */
  int height;			/**< display height in characters */
  char *framebuf;		/**< frame buffer */
  char *last_framebuf;		/**< old contents of frame buffer */
} PrivateData;

static void send_text(Driver *drvthis, char* packet)
{
  PrivateData *p = drvthis->private_data;

  usb_control_msg(p->dev, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, VFD_VUSB_CUSTOM_RQ_SETTEXT, 0, 0, packet, p->width, 5000);

}

// From: VUSB opendevice.c
int usbGetStringAscii(usb_dev_handle *dev, int index, char *buf, int buflen)
{
char    buffer[256];
int     rval, i;

    if((rval = usb_get_string_simple(dev, index, buf, buflen)) >= 0) /* use libusb version if it works */
        return rval;
    if((rval = usb_control_msg(dev, USB_ENDPOINT_IN, USB_REQ_GET_DESCRIPTOR, (USB_DT_STRING << 8) + index, 0x0409, buffer, sizeof(buffer), 5000)) < 0)
        return rval;
    if(buffer[1] != USB_DT_STRING){
        *buf = 0;
        return 0;
    }
    if((unsigned char)buffer[0] < rval)
        rval = (unsigned char)buffer[0];
    rval /= 2;
    /* lossy conversion to ISO Latin1: */
    for(i=1;i<rval;i++){
        if(i > buflen)              /* destination buffer overflow */
            break;
        buf[i-1] = buffer[2 * i];
        if(buffer[2 * i + 1] != 0)  /* outside of ISO Latin1 range */
            buf[i-1] = '?';
    }
    buf[i-1] = 0;
    return i-1;
}

/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval <0      Error.
 */
MODULE_EXPORT int vfd_vusb_init(Driver *drvthis)
{
  PrivateData *p;
  struct usb_bus *bus;

  // allocate and store private data
  p = (PrivateData *)calloc(1, sizeof(PrivateData));
  if (p == NULL) {
    report(RPT_ERR, "%s: unable to allocate private data", drvthis->name);
    return -1;
  }
  if (drvthis->store_private_ptr(drvthis, p) < 0) {
    report(RPT_ERR, "%s: unable to store private data", drvthis->name);
    return -1;
  }

  // initialize private data
  p->dev = NULL;
  p->width = VFD_VUSB_WIDTH;
  p->height = VFD_VUSB_HEIGHT;
  p->framebuf = (char *)malloc(p->width * p->height);
  if (p->framebuf == NULL) {
    report(RPT_ERR, "%s: unable to create framebuffer", drvthis->name);
    return -1;
  }
  memset(p->framebuf, ' ', p->width * p->height);
  p->last_framebuf = (char *)malloc(p->width * p->height);
  if (p->last_framebuf == NULL) {
    report(RPT_ERR, "%s: unable to create second framebuffer", drvthis->name);
    return -1;
  }
  memset(p->last_framebuf, 0, p->width * p->height);

  // find VFD
  usb_init();
  usb_find_busses();
  usb_find_devices();
  for (bus = usb_get_busses(); bus != NULL; bus = bus->next) {
    struct usb_device *dev;
    for (dev = bus->devices; dev != NULL; dev = dev->next) {
      if (dev->descriptor.idVendor == VFD_VUSB_VENDOR_ID &&
          dev->descriptor.idProduct == VFD_VUSB_PRODUCT_ID) {
	int len = 0;
        char product[256];
        product[0] = 0;
        usb_dev_handle handle = usb_open(dev); // Open Device to query Product Name
	if(dev->descriptor.iProduct > 0){
          len = usbGetStringAscii(handle, dev->descriptor.iProduct, product, sizeof(product));
	}
        if(len < 0){
	  if(!strcmp(product, VFD_VUSB_DEVICE_NAME)){
	    p->dev = handle; // we found the correct device
	  }
	}
      }
    }
  }
  if (p->dev == NULL) {
    report(RPT_ERR, "%s: unable to find VUSB-VFD", drvthis->name);
    return -1;
  }

  report(RPT_DEBUG, "%s: init() done", drvthis->name);
  return 0;
}

/**
 * Flush data on screen to the VFD.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void shuttleVFD_flush(Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;
  char packet[p->width]; // no handling for multiple lines yet.
  for(int i = 0; i < p->width; i++){
    packet[i] = p->framebuf[i];
  }
  
  send_text(p->dev, packet);
  
}

/**
 * Close the driver (do necessary clean-up).
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void vfd_vusb_close(Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;

  if (p != NULL) {
    if (p->dev != NULL) {
      if (usb_close(p->dev) < 0) {
        report(RPT_ERR, "%s: unable to close device", drvthis->name);
      }
      p->dev = NULL;
    }
    if (p->framebuf != NULL) {
      free(p->framebuf);
    }
    if (p->last_framebuf != NULL) {
      free(p->last_framebuf);
    }
    free(p);
  }
  drvthis->store_private_ptr(drvthis, NULL);
}

/**
 * Return the display width in characters.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of characters the display is wide.
 */
MODULE_EXPORT int vfd_vusb_width(Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;

  return p->width;
}

/**
 * Return the display height in characters.
 * \param drvthis  Pointer to driver structure.
 * \return         Number of characters the display is high.
 */
MODULE_EXPORT int vfd_vusb_height(Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;

  return p->height;
}

/**
 * Clear the screen.
 * \param drvthis  Pointer to driver structure.
 */
MODULE_EXPORT void vfd_vusb_clear(Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;

  memset(p->framebuf, ' ', p->width * p->height);
}


/**
 * Print a string on the screen at position (x,y).
 * The upper-left corner is (1,1), the lower-right corner is (p->width, p->height).
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param string   String that gets written.
 */
MODULE_EXPORT void vfd_vusb_string(Driver *drvthis, int x, int y, const char string[])
{
  PrivateData *p = drvthis->private_data;
  int i;

  --x; --y;
  if (y < 0 || y >= p->height)
    return;

  for (i = 0; (string[i] != '\0') && (x < p->width); ++i, ++x) {
    if (x >= 0) {    // no write left of left border
      p->framebuf[(y * p->width) + x] = string[i];
    }
  }
}

/**
 * Print a character on the screen at position (x,y).
 * The upper-left corner is (1,1), the lower-right corner is (p->width, p->height).
 * \param drvthis  Pointer to driver structure.
 * \param x        Horizontal character position (column).
 * \param y        Vertical character position (row).
 * \param c        Character that gets written.
 */
MODULE_EXPORT void vfd_vusb_chr(Driver *drvthis, int x, int y, char c)
{
  PrivateData *p = drvthis->private_data;

  --x; --y;
  if (x >= 0 && x < p->width && y >= 0 && y < p->height) {
    p->framebuf[(y * p->width) + x] = c;
  }
}