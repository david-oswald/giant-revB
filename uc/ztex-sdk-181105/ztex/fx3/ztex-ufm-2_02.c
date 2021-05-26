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
   Board specific functions for prototype board ZTEX-USB FPGA Module 2.02
*/

#ifndef _ZTEX_CONF_UFM_2_02_C1_
#define _ZTEX_CONF_UFM_2_02_C1_

#include "cyu3pib.h"

/* 
   This macro defines the Product string. Limited to 31 characters. 
*/
#undef ZTEX_PRODUCT_STRING
#define ZTEX_PRODUCT_STRING "ZTEX USB-FPGA Module 2.02b"

// GPIO's
#define ZTEX_GPIO_MODE0 46
#define ZTEX_GPIO_MODE1 48

#define ZTEX_GPIO_LED   47

#define ZTEX_GPIO_FPGA_RESET 41
#define ZTEX_GPIO_FPGA_DONE 45
#define ZTEX_GPIO_FPGA_INIT_B 43
#define ZTEX_GPIO_FPGA_RDWR_B 44

#undef ZTEX_GPIO_SIMPLE_BITMAP0
#undef ZTEX_GPIO_SIMPLE_BITMAP1
#define ZTEX_GPIO_SIMPLE_BITMAP0 0
#define ZTEX_GPIO_SIMPLE_BITMAP1 ( 1 << (ZTEX_GPIO_MODE0-32) | 1 << (ZTEX_GPIO_MODE1-32) | 1 << (ZTEX_GPIO_LED-32) \
	| 1 << (ZTEX_GPIO_FPGA_RESET-32) | 1 << (ZTEX_GPIO_FPGA_DONE-32) | 1 << (ZTEX_GPIO_FPGA_INIT_B-32) | 1 << (ZTEX_GPIO_FPGA_RDWR_B-32) \
    )

#define ZTEX_FPGA_CONFIGURED ( ztex_gpio_get(ZTEX_GPIO_FPGA_DONE) )

#define _ZTEX_BOARD_
#define _ZTEX_FPGA_

void ztex_disable_flash();

#endif // _ZTEX_CONF_UFM_2_02_C1_

#ifdef _ZTEX_INCLUDE_2_
#ifndef _ZTEX_CONF_UFM_2_02_C2_
#define _ZTEX_CONF_UFM_2_02_C2_

#include "ztex-fpgaconf1.c"

void (*ztex_fpga_config_start_app)() = 0;		// called before FPGA configuration
void (*ztex_fpga_config_done_app)() = 0;		// called after FPGA configuration

uint8_t ztex_fpga_cs = 0;				// check sum
uint32_t ztex_fpga_bytes = 0;				// transferred bytes
uint8_t ztex_fpga_init_b = 0;				// init b 

uint8_t ztex_fpga_config_started = 0;

void ztex_disable_flash() {
    ztex_gpio_set(ZTEX_GPIO_MODE1, CyFalse);
    ztex_gpio_set(ZTEX_GPIO_MODE0, CyTrue);
    ztex_gpio_set(ZTEX_GPIO_MODE1, CyTrue);
    ztex_gpio_set(ZTEX_GPIO_MODE0, CyFalse);
}

void ztex_enable_flash() {
    ztex_gpio_set(ZTEX_GPIO_MODE1, CyFalse);
    ztex_gpio_set(ZTEX_GPIO_MODE0, CyFalse);
}

CyBool_t ztex_fpga_configured() {
    return ztex_gpio_get(ZTEX_GPIO_FPGA_DONE);
}

void ztex_fpga_reset() {
    if ( ztex_fpga_config_start_app != 0 ) ztex_fpga_config_start_app();

    ztex_gpio_set(ZTEX_GPIO_FPGA_RESET, CyFalse);
    
    ztex_gpio_set_input(ZTEX_GPIO_FPGA_INIT_B);    	CyU3PGpioSetIoMode(ZTEX_GPIO_FPGA_INIT_B, CY_U3P_GPIO_IO_MODE_WPU);
    ztex_gpio_set_output(ZTEX_GPIO_FPGA_RDWR_B, CyFalse);

    CyU3PThreadSleep (20);
    ztex_gpio_set(ZTEX_GPIO_FPGA_RESET, CyTrue);
}

void ztex_fpga_config_start() {
    if ( ztex_fpga_config_started ) return;
    ztex_fpga_config_started = 1;

    ztex_gpio_set(ZTEX_GPIO_FPGA_RESET, CyFalse);
    
    // start gpif
    ztex_fpgaconf1_start();

    ztex_fpga_reset();

    uint8_t i = 0;
    while ( (!ztex_gpio_get(ZTEX_GPIO_FPGA_INIT_B)) && i<255 ) {
	CyU3PThreadSleep (1);
	i++;
    }

    ztex_fpga_init_b = ztex_gpio_get(ZTEX_GPIO_FPGA_INIT_B) ? 200 : 100;
    ztex_fpga_cs = 0;
    ztex_fpga_bytes = 0;
}

void ztex_fpga_config_done() {
    ztex_fpga_init_b += ztex_gpio_get(ZTEX_GPIO_FPGA_INIT_B) ? 22 : 11;
    
    ztex_fpgaconf1_send(ztex_ep0buf,16);

    if ( ZTEX_FPGA_CONFIGURED ) {
	CyU3PGpioSetIoMode(ZTEX_GPIO_FPGA_INIT_B, CY_U3P_GPIO_IO_MODE_NONE);
	ztex_gpio_set_input(ZTEX_GPIO_FPGA_RDWR_B);
	if ( ztex_fpga_config_done_app != 0 ) ztex_fpga_config_done_app();
    }	
    
    // stop gpif
    ztex_fpgaconf1_stop();
    ztex_gpio_set(ZTEX_GPIO_LED, !ZTEX_FPGA_CONFIGURED);
    ztex_fpga_config_started = 0;
}


// VR 0x30
uint8_t vr_fpga_info(uint16_t value, uint16_t index, uint16_t length ) {
    ztex_ep0buf[0] = ZTEX_FPGA_CONFIGURED ? 0 : 1;
    ztex_ep0buf[1] = ztex_fpga_cs;
    ztex_ep0buf[2] = ztex_fpga_bytes;
    ztex_ep0buf[3] = ztex_fpga_bytes >> 8;
    ztex_ep0buf[4] = ztex_fpga_bytes >> 16;
    ztex_ep0buf[5] = ztex_fpga_bytes >> 24;
    ztex_ep0buf[6] = ztex_fpga_init_b;
    ztex_ep0buf[7] = 0;         // flash configuration result
    ztex_ep0buf[8] = 0;		// bit order = not swapped
    ZTEX_REC_RET( CyU3PUsbSendEP0Data( 9, ztex_ep0buf ) );
    return 0;
}  

// VC 0x31
uint8_t vc_fpga_reset(uint16_t value, uint16_t index, uint16_t length ) {
    ztex_fpga_reset();
    CyU3PUsbAckSetup();
    return 0;
}

// VC 0x32
uint8_t vc_fpga_send(uint16_t value, uint16_t index, uint16_t length ) {
    ztex_fpga_config_start();

    if ( length > 0 ) {
	ZTEX_REC_RET ( CyU3PUsbGetEP0Data (length, ztex_ep0buf, NULL) );
	for (uint16_t i = 0;  i<length; i++)
	    ztex_fpga_cs += ztex_ep0buf[i];
	ztex_fpga_bytes += length;
	ztex_fpgaconf1_send(ztex_ep0buf,length);
    }
    else {
        CyU3PUsbAckSetup();
    }        

    if ( length == 0 || ((length & 63) != 0)  ) ztex_fpga_config_done();
    return 0;
}

void ztex_board_init() {
    ztex_disable_flash_boot = ztex_disable_flash;
    ztex_fpga_config_started = 0;

    ztex_gpio_set_output(ZTEX_GPIO_LED, CyTrue);
    ztex_gpio_set_output(ZTEX_GPIO_MODE0, CyFalse);
    ztex_gpio_set_output(ZTEX_GPIO_MODE1, CyFalse);
    
    ztex_gpio_set_input(ZTEX_GPIO_FPGA_DONE);
    ztex_gpio_set_output(ZTEX_GPIO_FPGA_RESET, CyTrue);
    
    ztex_register_vendor_req(0x30, vr_fpga_info);
    ztex_register_vendor_cmd(0x31, vc_fpga_reset);
    ztex_register_vendor_cmd(0x32, vc_fpga_send);

    ztex_gpio_set(ZTEX_GPIO_LED, !ZTEX_FPGA_CONFIGURED);
}    


void ztex_board_stop() {
    if ( ztex_fpga_config_started ) {   // USB is stopped during configuration
        ztex_fpgaconf1_stop();
	ztex_fpga_reset();
	ztex_fpga_config_started = 0;
    }
}

#endif // _ZTEX_CONF_UFM_2_02_C2_
#endif // _ZTEX_INCLUDE_2_
