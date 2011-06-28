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

#ifndef VFD_VUSB_H
#define VFD_VUSB_H

#include "lcd.h"

// mandatory functions (necessary for all drivers)
MODULE_EXPORT int vfd_vusb_init(Driver *drvthis);
MODULE_EXPORT void vfd_vusb_close(Driver *drvthis);

// essential output functions (necessary for output drivers)
MODULE_EXPORT int vfd_vusb_width(Driver *drvthis);
MODULE_EXPORT int vfd_vusb_height(Driver *drvthis);
MODULE_EXPORT void vfd_vusb_clear(Driver *drvthis);
MODULE_EXPORT void vfd_vusb_flush(Driver *drvthis);
MODULE_EXPORT void vfd_vusb_string(
    Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void vfd_vusb_chr(Driver *drvthis, int x, int y, char c);

// VFD physical dimensions
#define VFD_VUSB_WIDTH         10
#define VFD_VUSB_HEIGHT        1

// VFD USB properties
#define VFD_VUSB_VENDOR_ID      0x16c0
#define VFD_VUSB_PRODUCT_ID     0x05dc
#define VFD_VUSB_DEVICE_NAME    "VFD"
#define VFD_VUSB_CUSTOM_RQ_SETTEXT    0x01

#endif
