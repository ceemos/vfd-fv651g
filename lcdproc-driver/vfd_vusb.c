/** \file server/drivers/vfd_vusb.c
 * LCDd \c vfd_vusb driver for 14-Segment VFD Displays using a simple AVR µC with VUSB Stack as a controller: https://github.com/ceemos/vfd-fv651g
 */
 
/*
 * vfd_vusb Driver
 *
 * Copyright (C) 2011 Marcel Schneider, m.schneider.neuffen(ät)gmail.com
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
#include <stdint.h>

#include "lcd.h"
#include "vfd_vusb.h"
#include "report.h"


uint16_t font[] = {
    0b00000000000000, //sp
    0b0001000001000000, //!
    0b0000000001001000, //""
    0b0100101101010000, //#
    0b1000111001010100, //$
    0b00000000000000, //%
    0b00000000000000, //&
    0b0000000001000000, //'
    0b0000010000100000, //(
    0b0001000010000000, //)
    0b0001111111100000, //*
    0b0000101101000000, //+
    0b0001000000000000, //,
    0b0000001100000000, //-
    0b0000010000000000, //.
    0b0001000000100000, ///
    0b1111000000111100, //0
    0b0010000000001000, //1
    0b1100001100001100, //2
    0b1010001100001100, //3
    0b0010001100011000, //4
    0b1010001100010100, //0b1000011000010100, //5
    0b1110001100010100, //6
    0b0010000100001100, //7
    0b1110001100011100, //8
    0b1010001100011100, //9
    0b0000010100000000, //:
    0b0001001000000000, //;
    0b1000010000000000, //<
    0b1000001100000000, //=
    0b1001000000000000, //>
    0b0000010000100100, //?
    0b1110011000011100, //@
    0b0110001100011100, //A
    0b1010100101001100, //B
    0b1100000000010100, //C
    0b1010100001001100, //D
    0b1100001000010100, //E
    0b0100001000010100, //F
    0b1110100100010100, //G
    0b0110001100011000, //H
    0b1000100001000100, //I
    0b1010000000001000, //J
    0b0100011000110000, //K
    0b1100000000010000, //L
    0b0110000010111000, //M
    0b0110010010011000, //N
    0b1110000000011100, //O
    0b0100001100011100, //P
    0b1110010000011100, //Q
    0b0100011100011100, //R
    0b1000011000010100, //0b1010000110000100, //S
    0b0000100001000100, //T
    0b1110000000011000, //U
    0b0101000000110000, //V
    0b0111010000011000, //W
    0b0001010010100000, //X
    0b0000100010100000, //Y
    0b1001000000100100, //Z
    0b1100000000010100, //[
    0b0000010010000000, //backslash 
    0b1010000000001100, //]
    0b0000000000000100, //^
    0b1000000000000000, //_
    0b0000000010000000, //`
    0b1100011000000000, //A
    0b1100011000010000, //B
    0b1100001100000000, //C
    0b1011000100001000, //D
    0b1101001000000000, //E
    0b0000101100100000, //F
    0b1010100100000000, //G
    0b0110001100010000, //H
    0b0000100000100000, //I
    0b1010000000000000, //J
    0b0000110101000000, //K
    0b0100000000010000, //L
    0b0110101100000000, //M
    0b0110001100000000, //N
    0b1110001100000000, //O
    0b0101001000000000, //P
    0b1110011100000000, //Q
    0b0100001000000000, //R
    0b1000010100000000, //S
    0b0000101100000000, //T
    0b1110000000000000, //U
    0b0010010000000000, //V
    0b0111010000000000, //W
    0b0001011100000000, //X
    0b1010010000000000, //Y
    0b1001001000000000, //Z
    0b0000101001000000, //{
    0b0000100001000000, //|
    0b0000100101000000, //}
    0b0000001100000000, //~
    0b1111111111111111, //(del)
};

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
  //report(RPT_ERR, "%s: sending Text %s", drvthis->name, packet);
  usb_control_msg(p->dev, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, VFD_VUSB_CUSTOM_RQ_SETTEXT, 0, 0, packet, p->width * 2, 5000);

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
	//report(RPT_ERR, "%s: got VID/PID", drvthis->name);
	int len = 0;
        char product[256];
        product[0] = 0;
        usb_dev_handle *handle = usb_open(dev); // Open Device to query Product Name
	if(dev->descriptor.iProduct > 0){
          len = usbGetStringAscii(handle, dev->descriptor.iProduct, product, sizeof(product));
	  //report(RPT_ERR, "%s: has product String %s, searching for %s, strcmp: %d", drvthis->name, product, VFD_VUSB_DEVICE_NAME, strcmp(product, VFD_VUSB_DEVICE_NAME));
	}
        if(len > 0){
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
MODULE_EXPORT void vfd_vusb_flush(Driver *drvthis)
{
  PrivateData *p = drvthis->private_data;
  // *2 for RAWMODE
  char packet[p->width * 2]; // no handling for multiple lines yet.
  int i;
  for(i = 0; i < p->width; i++){
    uint16_t d = font[p->framebuf[i] - 32];
    packet[i * 2] = (char) d;
    packet[i * 2 + 1] = (char) (d >> 8);
  }
  
  send_text(drvthis, packet);
  
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