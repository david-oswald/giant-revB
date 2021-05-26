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
    Implements the low speed interface of default firmware.
*/

/*        
    The following macros (containing GPIO numbers) must be defined:
        GPIO_RESET 
	GPIO_GPIO0
        GPIO_GPIO1
    	GPIO_GPIO2
    	GPIO_GPIO3
	GPIO_CLK
	GPIO_DATA
	GPIO_STOP

    This macros (containing Endpoint numbers) may be defined:
	OUT_ENDPOINT
	IN_ENDPOINT
*/

#ifndef _ZTEX_LSI_
#define _ZTEX_LSI_
#endif // _ZTEX_LSI_

#ifdef _ZTEX_INCLUDE_2_
#ifndef _ZTEX_LSI_2_
#define _ZTEX_LSI_2_

#ifndef OUT_ENDPOINT
#define OUT_ENDPOINT 255
#endif

#ifndef IN_NDPOINT
#define IN_NDPOINT 255
#endif

#define LSI_VERSION 1
#define LSI_SUB_VERSION 4

CyBool_t next_clk;
#define LSI_CLOCK { ztex_gpio_set(GPIO_CLK, next_clk); next_clk=!next_clk; }

// VC 0x60
// value != 0: reset signal is left active 
uint8_t vc_default_reset(uint16_t value, uint16_t index, uint16_t length ) {
    if ( length>0 ) {
	CyU3PUsbGetEP0Data (length, ztex_ep0buf, NULL);  // there should be no data
    } else {
	CyU3PUsbAckSetup();
    }
    ztex_gpio_set(GPIO_RESET, CyTrue); 
	
    if ( value ) return 0;
    CyU3PThreadSleep(1);
    ztex_gpio_set(GPIO_RESET, CyFalse); 
    return 0;
}

// VR 0x61
// index: mask
// value: value
uint8_t vr_default_gpio_ctl(uint16_t value, uint16_t index, uint16_t length ) {
    if (index & 1) ztex_gpio_set(GPIO_GPIO0, (value & 1) == 0);
    if (index & 2) ztex_gpio_set(GPIO_GPIO1, (value & 2) == 0);
    if (index & 4) ztex_gpio_set(GPIO_GPIO2, (value & 4) == 0);
    if (index & 8) ztex_gpio_set(GPIO_GPIO3, (value & 8) == 0);
    ztex_ep0buf[0] = ~(0xf0 | (ztex_gpio_get(GPIO_GPIO3)<<3) | (ztex_gpio_get(GPIO_GPIO2)<<2) | (ztex_gpio_get(GPIO_GPIO1)<<1) | ztex_gpio_get(GPIO_GPIO0) );
    ZTEX_REC_RET ( CyU3PUsbSendEP0Data( length, ztex_ep0buf ) );
    return 0;
}

// VC 0x62
// data format is 4 byte data (little endian) + 1 byte address
uint8_t vc_default_lsi_write(uint16_t value, uint16_t index, uint16_t length ) {
    ZTEX_REC_RET ( CyU3PUsbGetEP0Data (length, ztex_ep0buf, NULL) );
    LSI_CLOCK;
    for (int i=0; i+4<length; i+=5) {
	ztex_gpio_set(GPIO_STOP, CyFalse); 
	
	for (int j=0; j<5; j++ ) {
	    uint8_t b = ztex_ep0buf[i+j];
	    for ( int k=0; k<8; k++ ) {
		ztex_gpio_set(GPIO_DATA, b & 1);
		LSI_CLOCK;
		b>>=1;
	    }
	}
	ztex_gpio_set(GPIO_DATA, CyFalse); 	
	ztex_gpio_set(GPIO_STOP, CyTrue); 	
	LSI_CLOCK;
    } 
    ztex_gpio_set(GPIO_STOP, CyFalse); 
    return 0;
}

// VR 0x63
// data format is 4 byte data (little endian)
// FX3 and FPGA clock are asynchronous and there is no acknowledgment. For this reason 
// minimum recommended for the ZTEX LSI core is 20 MHz. For much slower clock this interface
// may be to fast.
uint8_t vr_default_lsi_read(uint16_t value, uint16_t index, uint16_t length ) {
    LSI_CLOCK;
    for (int i=0; i+3<length; i+=4) {
        ztex_gpio_set(GPIO_STOP, CyFalse); 

        uint8_t b = index++;
        for (int k=0; k<8; k++) { 
    	    ztex_gpio_set(GPIO_DATA, b & 1);
    	    LSI_CLOCK;
    	    b>>=1;
    	}
	    
        ztex_gpio_set(GPIO_DATA, CyTrue); 	
        ztex_gpio_set(GPIO_STOP, CyTrue); 	
        LSI_CLOCK;
        for (int j=0; j<64; j++ ) {}	   // give FPGA some extra time to load data
	
        for (int j=0; j<4; j++ ) {
    	    b=0;
    	    for (int k=0; k<8; k++) { 
    		b = ( b >> 1 ) | ( ztex_gpio_get(GPIO_DATA) << 7);
		LSI_CLOCK;
    	    }
	    ztex_ep0buf[i+j] = b;
	}
    }
    ztex_gpio_set(GPIO_STOP, CyFalse); 
    ZTEX_REC_RET ( CyU3PUsbSendEP0Data( length, ztex_ep0buf ) );
    return 0;
}

// VR 0x64
uint8_t vr_default_info(uint16_t value, uint16_t index, uint16_t length ) {
    ztex_ep0buf[0] = LSI_VERSION;
    ztex_ep0buf[1] = OUT_ENDPOINT;	// OUT Endpoint
    ztex_ep0buf[2] = IN_ENDPOINT;	// IN Endpoint
    ztex_ep0buf[3] = LSI_SUB_VERSION;
    ztex_ep0buf[4] = 0;			// reserved for future use
    ztex_ep0buf[5] = 0;			// reserved for future use
    ztex_ep0buf[6] = 0;			// reserved for future use
    ztex_ep0buf[7] = 0;			// reserved for future use
    if (length>8) length = 8;
    ZTEX_REC_RET( CyU3PUsbSendEP0Data( length, ztex_ep0buf ) );
    return 0;
}


void ztex_lsi_init () {
    ztex_register_vendor_cmd(0x60, vc_default_reset);
    ztex_register_vendor_req(0x61, vr_default_gpio_ctl);
    ztex_register_vendor_cmd(0x62, vc_default_lsi_write);
    ztex_register_vendor_req(0x63, vr_default_lsi_read);
    ztex_register_vendor_req(0x64, vr_default_info);
}


// reset signal is left active 
void ztex_lsi_start() {
    ztex_gpio_set_output(GPIO_RESET, CyTrue); 

    ztex_gpio_set_open_drain(GPIO_GPIO0, CyTrue);
    ztex_gpio_set_open_drain(GPIO_GPIO1, CyTrue);
    ztex_gpio_set_open_drain(GPIO_GPIO2, CyTrue);
    ztex_gpio_set_open_drain(GPIO_GPIO3, CyTrue);

    ztex_gpio_set_output(GPIO_CLK, CyFalse); next_clk=CyTrue;
    ztex_gpio_set_open_drain(GPIO_DATA, CyTrue); 
    ztex_gpio_set_output(GPIO_STOP, CyFalse); 
}


void ztex_lsi_stop() {
    ztex_gpio_set_input(GPIO_RESET); 

    ztex_gpio_set_input(GPIO_GPIO0);
    ztex_gpio_set_input(GPIO_GPIO1);
    ztex_gpio_set_input(GPIO_GPIO2);
    ztex_gpio_set_input(GPIO_GPIO3);

    ztex_gpio_set_input(GPIO_CLK);
    ztex_gpio_set_input(GPIO_DATA); 
    ztex_gpio_set_input(GPIO_STOP); 
}

#endif // _ZTEX_LSI_2_
#endif // _ZTEX_INCLUDE_2_
