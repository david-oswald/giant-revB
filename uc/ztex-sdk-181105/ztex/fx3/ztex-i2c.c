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
    MAC-EEPROM support.
*/
    
#include <cyu3i2c.h>

#ifndef _ZTEX_I2C_C_
#define _ZTEX_I2C_C_

#define ZTEX_MAC_EEPROM_ADDR 0xA6

uint8_t ztex_mac_eeprom_ec = 0;

uint8_t ztex_config_data_valid = 0;

const char ztex_hexdigits[] = "0123456789ABCDEF";    

uint8_t ztex_mac_eeprom_write(uint8_t addr, uint8_t *buf, uint8_t size) {
    CyU3PI2cPreamble_t preamble;
    uint8_t size2;

    ztex_mac_eeprom_ec = 1;
    
    while ( size > 0 ) {
        preamble.length    = 2;
        preamble.buffer[0] = ZTEX_MAC_EEPROM_ADDR;
        preamble.buffer[1] = addr;
        preamble.ctrlMask  = 0x0000;
        size2 = 8-(addr & 7);
        if (size<size2) size2 = size;

	ZTEX_REC_RET( CyU3PI2cTransmitBytes (&preamble, buf, size2, 0) );
	
	addr+=size2;
	buf+=size2;
	size-=size2;

        preamble.length = 1;
        ZTEX_REC_RET( CyU3PI2cWaitForAck(&preamble, 600) );

        CyU3PThreadSleep (1);
    }
    
    ztex_mac_eeprom_ec = 0;
    return 0;
}

uint8_t ztex_mac_eeprom_read(uint8_t addr, uint8_t *buf, uint8_t size) {
    CyU3PI2cPreamble_t preamble;

    ztex_mac_eeprom_ec = 1;

    preamble.length    = 3;
    preamble.buffer[0] = ZTEX_MAC_EEPROM_ADDR;
    preamble.buffer[1] = addr;
    preamble.buffer[2] = ZTEX_MAC_EEPROM_ADDR | 1;
    preamble.ctrlMask  = 0x0002;
    
    ZTEX_REC_RET ( CyU3PI2cReceiveBytes (&preamble, buf, size, 0) );


        ztex_mac_eeprom_ec = 0;

    return 0;
}


// VR 0x3D
uint8_t vr_mac_eeprom_info(uint16_t value, uint16_t index, uint16_t length ) {
    ztex_ep0buf[0]=ztex_mac_eeprom_ec;
    ZTEX_REC_RET( CyU3PUsbSendEP0Data( 1, ztex_ep0buf ) );
    return 0;
}

// VR 0x3B
uint8_t vr_mac_eeprom_read(uint16_t value, uint16_t index, uint16_t length ) {
    if ( ztex_mac_eeprom_read( (uint8_t)(value & 255), ztex_ep0buf, length) ) return 255;
    ZTEX_REC_RET ( CyU3PUsbSendEP0Data( length, ztex_ep0buf ) );
    return 0;
}

// VC 0x3C
uint8_t vc_mac_eeprom_write(uint16_t value, uint16_t index, uint16_t length ) {
    ZTEX_REC_RET ( CyU3PUsbGetEP0Data (length, ztex_ep0buf, NULL) );
    if ( ztex_mac_eeprom_write( (uint8_t)(value & 255), ztex_ep0buf, length)  ) return 255;
    return 0;
}


void ztex_i2c_init() {
    CyU3PReturnStatus_t status = CY_U3P_SUCCESS;
    
    CyU3PI2cConfig_t i2cConfig;
    uint8_t buf[5];

    ZTEX_REC( status=CyU3PI2cInit() );
    ztex_mac_eeprom_ec = status != CY_U3P_SUCCESS;

    CyU3PMemSet ((uint8_t *)&i2cConfig, 0, sizeof(i2cConfig));
    i2cConfig.bitRate    = 100000;
    i2cConfig.busTimeout = 0xFFFFFFFF;
    i2cConfig.dmaTimeout = 0xFFFF;
    i2cConfig.isDma      = CyFalse;
    ZTEX_REC( status = CyU3PI2cSetConfig (&i2cConfig, NULL) );
    ztex_mac_eeprom_ec = status != CY_U3P_SUCCESS;

    ztex_register_vendor_req(0x3D, vr_mac_eeprom_info);
    ztex_register_vendor_req(0x3B, vr_mac_eeprom_read);
    ztex_register_vendor_cmd(0x3C, vc_mac_eeprom_write);
    
    // check for configuration data
    if ( ztex_mac_eeprom_read(0, buf, 3)==0 && buf[0]==67 && buf[1]==68 && buf[2]==48 ) {	// check signature
	ztex_config_data_valid = 1;
	ztex_mac_eeprom_read ( 16, (uint8_t *)ztex_sn_string, 10 );	// copy serial number

	if ( ztex_mac_eeprom_read ( 32, buf, 5 )==0 ) {			// USB ID's plus 1st char of product string
	    if ( (buf[0]!=0) && (buf[0]!=255) && (buf[1]!=0) && (buf[1]!=255) ) { 
		for (int i=0; i<4; i++ ) {
		    ztex_usb3_device_descriptor[8+i] = ztex_usb2_device_descriptor[8+i] = buf[i];
		}
	    }
	    if ( buf[4]!=0 ) {
		ztex_mac_eeprom_read ( 36, (uint8_t*)ztex_product_string, 32 );	// copy product string
		ztex_product_string[33]='\0';
	    }
	}
    }
    else {
	ztex_config_data_valid = 0;
    }
    
    // check for configuration data
    for (int i=0; i<10; i++) {	// abort if SN != "0000000000"
	if ( ztex_sn_string[i] != '0' )
	    return;
    }

    if ( ztex_mac_eeprom_read ( 0xfb, buf, 5 )) return;			// read the last 5 MAC digits
    for (int i=0; i<5; i++) {						// convert to MAC to SN string
	ztex_sn_string[i*2]   = ztex_hexdigits[buf[i]>>4];
	ztex_sn_string[i*2+1] = ztex_hexdigits[buf[i] & 15];
    } 
}

#endif // _ZTEX_I2C_C_
