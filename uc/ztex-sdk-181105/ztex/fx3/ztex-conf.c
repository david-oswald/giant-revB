/*%
   ZTEX Firmware Kit for EZ-USB FX3 Microcontrollers
   Copyright (C) 2009-2017 ZTEX GmbH.
   http://www.ztex.de
   
   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this file,
   You can obtain one at http://mozilla.org/MPL/2.0/.

   Alternatively, the contents of this file may be used under the terms
   of the GNU General Public License Version 3, as described below:

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 3 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, see http://www.gnu.org/licenses/.
%*/
/* 
   This file contains settings which can be overwritten in user code.
*/

#ifndef _ZTEX_CONF_H_
#define _ZTEX_CONF_H_

#define ZTEX_USB_VENDOR_ID (0x221a)
#define ZTEX_USB_PRODUCT_ID (0x100)

// please see ../fx2/ztex-descriptors.h
#define ZTEX_PRODUCT_ID_0	(0)
#define ZTEX_PRODUCT_ID_1	(0)
#define ZTEX_PRODUCT_ID_2	(0)
#define ZTEX_PRODUCT_ID_3	(0)

#define ZTEX_FWVER		(0)

#define ZTEX_MODULE_RESERVED_00	(0)
#define ZTEX_MODULE_RESERVED_01	(0)
#define ZTEX_MODULE_RESERVED_02	(0)
#define ZTEX_MODULE_RESERVED_03	(0)
#define ZTEX_MODULE_RESERVED_04	(0)
#define ZTEX_MODULE_RESERVED_05	(0)
#define ZTEX_MODULE_RESERVED_06	(0)
#define ZTEX_MODULE_RESERVED_07	(0)
#define ZTEX_MODULE_RESERVED_08	(0)
#define ZTEX_MODULE_RESERVED_09	(0)
#define ZTEX_MODULE_RESERVED_10	(0)

/* 
   This macro defines the Manufacturer string. Limited to 31 characters. 
*/
#define ZTEX_MANUFACTURER_STRING "ZTEX"

/* 
   This macro defines the Product string. Limited to 31 characters. 
*/
#define ZTEX_PRODUCT_STRING "FX3 Firmware"

#define EP_BULK(num,dir,burst,settings) EP(num,dir,BULK,1024,burst,0,settings)
#define EP_ISO(num,dir,burst,settings) EP(num,dir,ISO,1024,burst,1,settings)

/* 
   This macro defines the endpoint setup and should overwritten by user.
*/
#define EP_SETUP

/* 
   GPIO enable bitmaps for simple/complex GPIO's 0..31 and 32..63 .
   These bitmaps are reserved for firmware usage and should not be overwritten by user
*/
#define ZTEX_GPIO_SIMPLE_BITMAP0 0
#define ZTEX_GPIO_SIMPLE_BITMAP1 0
#define ZTEX_GPIO_COMPLEX_BITMAP0 0
#define ZTEX_GPIO_COMPLEX_BITMAP1 0

/* 
   GPIO enable bitmaps for simple/complex GPIO's 0..31 and 32..63 .
   These bitmaps are or'ed with ZTEX_GPIO_* bitmaps and should be used by user
*/
#define GPIO_SIMPLE_BITMAP0 0
#define GPIO_SIMPLE_BITMAP1 0
#define GPIO_COMPLEX_BITMAP0 0
#define GPIO_COMPLEX_BITMAP1 0

#define GPIO_INT_HANDLER 0

#define ZTEX_EP_CLEANUP_HANDLER ztex_ep_cleanup_default_handler

// called when USB connection is started
#define ZTEX_USB_START {}

// called when USB connection is stopped
#define ZTEX_USB_STOP {}

// stack size of application thread
#define ZTEX_APP_THREAD_STACK_SIZE 0x1000

// priority of application thread, should be 7..15
#define ZTEX_APP_THREAD_PRIO 8

// application main loop
#define ZTEX_APP_THREAD_RUN {}

/* 
   functions that are enabled by default
   undef them in order to disable features
*/   
#define ENABLE_I2C  		// enables I2C IO block
#define ENABLE_SPI		// enables SPI IO block
#define ENABLE_SPI_FLASH	// enables SPI FLASH

/*
   functions that are enabled if board supports them
   (usually should not be enabled by user)
*/
//#define ENABLE_SPORT0		// enables SPORT0 IO block

/*
   functions that can be enabled by user
*/ 
//#define ENABLE_SD_FLASH	// enables SD FLASH as secondary flash device

/*
   disables FPGA configuration from flash (if supported)
*/ 
//#define DISABLE_FLASH_CONFIG	// disables FPGA configuration from flash (if supported)


#endif // _ZTEX_CONF_H_
