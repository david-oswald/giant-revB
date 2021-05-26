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
    Descriptor definitions.
*/
#ifndef _ZTEX_CONF_H_
#error "Illegal use of `ztex-descriptors.c'. This file must be included in the main firmware source after including `ztex-conf.h' and after the configuration section"
#endif
#ifndef _ZTEX_DESCRIPTORS_C_
#define _ZTEX_DESCRIPTORS_C_

#define W2B(a) (a) & 255, (a) >> 8
#define DIR_IN 128
#define DIR_OUT 0
#define TYPE_ISO 1
#define TYPE_BULK 2
#define TYPE_INT 3

// device descriptor for USB 3.0
uint8_t ztex_usb3_device_descriptor[] __attribute__ ((aligned (32))) =
{
    18,        					// 0, descriptor size
    CY_U3P_USB_DEVICE_DESCR,        		// 1, Device descriptor type
    0x00,0x03,                      		// 2, USB 3.0
    0x00,                           		// 4, Device class
    0x00,                          		// 5, Device sub-class
    0x00,                           		// 6, Device protocol
    0x09,                           		// 7, Max packet size for EP0 : 2^9
    W2B(ZTEX_USB_VENDOR_ID),	    		// 8, Vendor ID 
    W2B(ZTEX_USB_PRODUCT_ID),			// 10, Product ID
    0x00,0x00,                      		// 12, Device release number
    0x01,                           		// 14, Manufacture string index
    0x02,                           		// 15, Product string index
    0x03,                           		// 16, Serial number string index
    0x01                            		// 17, Number of configurations
};

// device descriptor for USB 2.0 
uint8_t ztex_usb2_device_descriptor[] __attribute__ ((aligned (32))) =
{
    18,                                         // 0, Descriptor size
    CY_U3P_USB_DEVICE_DESCR,        		// 1, Device descriptor type 
    0x00,0x02,                      		// 2, USB 2.00 
    0x00,                           		// 4, Device class 
    0x00,                           		// 5, Device sub-class 
    0x00,                           		// 6, Device protocol 
    0x40,                           		// 7, Max packet size for EP0 : 64 bytes 
    W2B(ZTEX_USB_VENDOR_ID),	    		// 8, Vendor ID 
    W2B(ZTEX_USB_PRODUCT_ID),			// 10, Product ID
    0x00,0x00,                      		// 12, Device release number 
    0x01,                           		// 14, Manufacture string index 
    0x02,                           		// 15, Product string index 
    0x03,                           		// 16, Serial number string index 
    0x01                            		// 17, Number of configurations 
};

// Binary device object store descriptor 
const uint8_t ztex_bos_descriptor[] __attribute__ ((aligned (32))) =
{
    5,                           		// 0, Descriptor size 
    CY_U3P_BOS_DESCR,              		// 1, Device descriptor type 
    22,0, 	                     		// 2, Length of this descriptor and all sub descriptors 
    0x02,                           		// 4, Number of device capability descriptors 

    // USB 2.0 extension 
    7,                           		// 0, Descriptor size 
    CY_U3P_DEVICE_CAPB_DESCR,       		// 1, Device capability type descriptor 
    CY_U3P_USB2_EXTN_CAPB_TYPE,     		// 2, USB 2.0 extension capability type 
    0x02,0x00,0x00,0x00,            		// 3, Supported device level features: LPM support  

    // SuperSpeed device capability 
    10,                           		// 0, Descriptor size 
    CY_U3P_DEVICE_CAPB_DESCR,       		// 1, Device capability type descriptor 
    CY_U3P_SS_USB_CAPB_TYPE,        		// 2, SuperSpeed device capability type 
    0x00,                           		// 3, Supported device level features  
    0x0E,0x00,                      		// 4, Speeds supported by the device : SS, HS and FS 
    0x03,                           		// 6, Functionality support 
    0x00,                           		// 7, U1 Device Exit latency 
    0x00,0x00                       		// 8, U2 Device Exit latency 
};

// Standard device qualifier descriptor 
const uint8_t ztex_device_qualifier_descriptor[] __attribute__ ((aligned (32))) =
{
    10,                           		// 0, Descriptor size 
    CY_U3P_USB_DEVQUAL_DESCR,       		// 1, Device qualifier descriptor type 
    0x00,0x02,                      		// 2, USB 2.0 
    0x00,                           		// 4, Device class 
    0x00,                           		// 5, Device sub-class 
    0x00,                           		// 6, Device protocol 
    0x40,                           		// 7, Max packet size for EP0 : 64 bytes 
    0x01,                           		// 8, Number of configurations 
    0x00                            		// 9, Reserved 
};

enum interface_eps {
    interface_eps_dummy = 0
#define INTERFACE(a,b), interface_eps_##a = 0 b
#define EP(num,dir,type,size,burst,interval,settings) +1
    EP_SETUP_ALL
};
#undef INTERFACE
#undef EP

// super speed configuration descriptor 
const uint8_t ztex_usb3_config_descriptor[] __attribute__ ((aligned (32))) =
{
    // Configuration descriptor 
    9,                           		// 0, Descriptor size 
    CY_U3P_USB_CONFIG_DESCR,       		// 1, Configuration descriptor type 
#define INTERFACE(a,b) +9 b    
#define EP(num,dir,type,size,burst,interval,settings) +7+6
    W2B(9 EP_SETUP_ALL),				// 2, Length of this descriptor and all sub descriptors 
#undef INTERFACE
#undef EP
#define INTERFACE(a,b) +1    
    0 EP_SETUP_ALL,  	                     	// 4, Number of interfaces 
#undef INTERFACE
    0x01,                           		// 5, Configuration number 
    0x00,                           		// 6, Configuration string index 
    0x80,                           		// 7, attributes: bus
    25                           		// 8, Max power consumption of device (in 8mA units) : 200mA 

    // Interface descriptors
#define INTERFACE(num, body) , \
    0x09,					/* 0, Descriptor size */ \
    CY_U3P_USB_INTRFC_DESCR,        		/* 1, Interface Descriptor type */ \
    num,                           		/* 2, Interface number */ \
    0x00,                           		/* 3, Alternate setting number */ \
    interface_eps_##num,               		/* 4, Number of end points */ \
    0xFF,                           		/* 5, Interface class */ \
    0x00,                           		/* 5, Interface sub class */ \
    0x00,                           		/* 6, Interface protocol code  */ \
    num < 8 ? num+4 : 0                  		/* 7, Interface descriptor string index */ \
						/*    String can be set using ztex_interface_string[] */ \
    body

    // Endpoint descriptors
#define EP(num,dir,type,size,burst,interval,settings) , \
    0x07,                           		/* 0, Descriptor size */ \
    CY_U3P_USB_ENDPNT_DESCR,        		/* 1, Endpoint descriptor type */ \
    DIR_##dir | num,				/* 2, Endpoint number + direction */ \
    TYPE_##type,              			/* 3, endpoint type */ \
    W2B(size),                      		/* 4, Max packet size */ \
    interval,                          		/* 6, Service interval */ \
    /* Super speed endpoint companion descriptor for producer EP */ \
    0x06,                           		/* 0, Descriptor size */ \
    CY_U3P_SS_EP_COMPN_DESCR,       		/* 1, SS endpoint companion descriptor type  */ \
    burst-1,          				/* 2, bursts*/ \
    0,           				/* 3, attributes */ \
    W2B(TYPE_##type==2 ? 0 : size*burst)	/* 4, Bytes per interval: 0 for bulk, size * burst for periodic transfers */ 
    EP_SETUP_ALL
#undef INTERFACE
#undef EP
};


// high speed configuration descriptor 
const uint8_t ztex_usb2_config_descriptor[] __attribute__ ((aligned (32))) =
{
    // Configuration descriptor 
    9,                           		// 0, Descriptor size 
    CY_U3P_USB_CONFIG_DESCR,       		// 1, Configuration descriptor type 
#define INTERFACE(a,b) +9 b    
#define EP(num,dir,type,size,burst,interval,settings) +7
    W2B(9 EP_SETUP_ALL),				// 2, Length of this descriptor and all sub descriptors 
#undef INTERFACE
#undef EP
#define INTERFACE(a,b) +1    
    0 EP_SETUP_ALL,  	                     	// 4, Number of interfaces 
#undef INTERFACE
    0x01,                           		// 5, Configuration number 
    0x00,                           		// 6, Configuration string index 
    0x80,                           		// 7, attributes: bus
    50                           		// 8, Max power consumption of device (in 2mA units) : 100mA 

    // Interface descriptors
#define INTERFACE(num, body) , \
    0x09,					/* 0, Descriptor size */ \
    CY_U3P_USB_INTRFC_DESCR,        		/* 1, Interface Descriptor type */ \
    num,                           		/* 2, Interface number */ \
    0x00,                           		/* 3, Alternate setting number */ \
    interface_eps_##num,               		/* 4, Number of end points */ \
    0xFF,                           		/* 5, Interface class */ \
    0x00,                           		/* 5, Interface sub class */ \
    0x00,                           		/* 6, Interface protocol code  */ \
    num < 8 ? num+4 : 0                  		/* 7, Interface descriptor string index */ \
						/*    String can be set using ztex_interface_string[] */ \
    body

    // Endpoint descriptors
#define EP(num,dir,type,size,burst,interval,settings) , \
    0x07,                           		/* 0, Descriptor size */ \
    CY_U3P_USB_ENDPNT_DESCR,        		/* 1, Endpoint descriptor type */ \
    DIR_##dir + num,				/* 2, Endpoint number + direction */ \
    TYPE_##type,              			/* 3, endpoint type */ \
    W2B( (((TYPE_##type==2) && (size>512)) ? 512 : size) | (TYPE_##type==2 ? 0 : (((burst > 3 ? 3 : burst)-1) << 11) ) ),	/* 4, Max packet for bulk transfers limited to 512 bytes */ \
    interval                           		/* 6, Service interval */
    EP_SETUP_ALL
#undef INTERFACE
#undef EP
};

// full speed configuration descriptor 
const uint8_t ztex_usb1_config_descriptor[] __attribute__ ((aligned (32))) =
{
    // Configuration descriptor 
    9,                           		// 0, Descriptor size 
    CY_U3P_USB_CONFIG_DESCR,       		// 1, Configuration descriptor type 
#define INTERFACE(a,b) +9 b    
#define EP(num,dir,type,size,burst,interval,settings) +7
    W2B(9 EP_SETUP_ALL),				// 2, Length of this descriptor and all sub descriptors 
#undef INTERFACE
#undef EP
#define INTERFACE(a,b) +1    
    0 EP_SETUP_ALL,  	                     	// 4, Number of interfaces 
#undef INTERFACE
    0x01,                           		// 5, Configuration number 
    0x00,                           		// 6, Configuration string index 
    0x80,                           		// 7, attributes: bus
    50                           		// 8, Max power consumption of device (in 2mA units) : 100mA 

    // Interface descriptors
#define INTERFACE(num, body) , \
    0x09,					/* 0, Descriptor size */ \
    CY_U3P_USB_INTRFC_DESCR,        		/* 1, Interface Descriptor type */ \
    num,                           		/* 2, Interface number */ \
    0x00,                           		/* 3, Alternate setting number */ \
    interface_eps_##num,               		/* 4, Number of end points */ \
    0xFF,                           		/* 5, Interface class */ \
    0x00,                           		/* 5, Interface sub class */ \
    0x00,                           		/* 6, Interface protocol code  */ \
    num < 8 ? num+4 : 0                  		/* 7, Interface descriptor string index */ \
						/*    String can be set using ztex_interface_string[] */ \
    body

    // Endpoint descriptors
#define EP(num,dir,type,size,burst,interval,settings) , \
    0x07,                           		/* 0, Descriptor size */ \
    CY_U3P_USB_ENDPNT_DESCR,        		/* 1, Endpoint descriptor type */ \
    DIR_##dir + num,				/* 2, Endpoint number + direction */ \
    TYPE_##type,              			/* 3, endpoint type */ \
    W2B(64), 					/* 4, size ficed to 64 bytes */ \
    1                           		/* 6, Service interval */
    EP_SETUP_ALL
#undef INTERFACE
#undef EP
};

// language string descriptor */
const uint8_t ztex_lang_string_descriptor[] __attribute__ ((aligned (32))) =
{
    0x04,                           // Descriptor size 
    CY_U3P_USB_STRING_DESCR,        // Device descriptor type
    0x04, 0x09
};

/* Place this buffer as the last buffer so that no other variable / code shares
   the same cache line. Do not add any other variables / arrays in this file.
   This will lead to variables sharing the same cache line. */
const uint8_t descriptor_allign_buffer[32] __attribute__ ((aligned (32)));


const uint8_t ztex_descriptor[] __attribute__ ((aligned (32))) =
{
    40,                 	// Descriptor size
    0x01, 		  	// Descriptor version
    'Z','T','E','X', 		// Signature "ZTEX"

    ZTEX_PRODUCT_ID_0,	  	// product ID's
    ZTEX_PRODUCT_ID_1,
    ZTEX_PRODUCT_ID_2,
    ZTEX_PRODUCT_ID_3,
    
    ZTEX_FWVER,		 	// firmware version

    1,				// interface version
    
    // interface capabilities
    0				
#ifdef _ZTEX_FPGA_
    | 2				// FPGA configuration support
#endif
#ifdef ENABLE_SPI_FLASH
    | 4				// SPI Flash support
#endif
#ifdef ZTEX_FPGA_CONF_FAST_EP
    | 32			// fast FPGA configuration support
#endif
#ifdef ENABLE_I2C
    | 64			// MAC EEPROM support
#endif
    ,
    4 | 8			// FX3, debug2
#ifdef ENABLE_SD_FLASH
    | 2				// SD Flash support
#endif
#ifdef _ZTEX_LSI_
    | 16			// default firmware interface support
#endif
    ,
    0,
    0,
    0,
    0,

    ZTEX_MODULE_RESERVED_00,	// 11 bytes which can be used by application
    ZTEX_MODULE_RESERVED_01,
    ZTEX_MODULE_RESERVED_02,
    ZTEX_MODULE_RESERVED_03,
    ZTEX_MODULE_RESERVED_04,
    ZTEX_MODULE_RESERVED_05,
    ZTEX_MODULE_RESERVED_06,
    ZTEX_MODULE_RESERVED_07,
    ZTEX_MODULE_RESERVED_08,
    ZTEX_MODULE_RESERVED_09,
    ZTEX_MODULE_RESERVED_10,
    
    207,			// must be 207 for FX3 firmwares
    
    '0','0','0','0', '0','0','0','0', '0','0'	// 1 bytes serial number string
};

#undef W2B
#undef DIR_IN
#undef DIR_OUT
#undef TYPE_ISO
#undef TYPE_BULK
#undef TYPE_INT

uint8_t ztex_ep0buf[4096] __attribute__ ((aligned (32)));

const char ztex_manufacturer_string[] = ZTEX_MANUFACTURER_STRING;

char ztex_product_string[64] = ZTEX_PRODUCT_STRING;

char ztex_sn_string[] = "0000000000";

#endif // _ZTEX_DESCRIPTORS_C_
