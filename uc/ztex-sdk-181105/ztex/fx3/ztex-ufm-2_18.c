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
   Board specific functions for ZTEX-USB FPGA Module 2.18
*/

#ifndef _ZTEX_CONF_UFM_2_18_C1_
#define _ZTEX_CONF_UFM_2_18_C1_

#include "cyu3pib.h"

// product ID's for ZTEX USB-FPGA Module 2.18 are 10.42.*.*
#undef ZTEX_PRODUCT_ID_0
#define ZTEX_PRODUCT_ID_0	(10)
#undef ZTEX_PRODUCT_ID_1
#define ZTEX_PRODUCT_ID_1	(42)

/* 
   This macro defines the Product string. Limited to 31 characters. 
*/
#undef ZTEX_PRODUCT_STRING
#define ZTEX_PRODUCT_STRING "ZTEX USB-FPGA Module 2.18"

#define ENABLE_SPORT0

// GPIO's
#define ZTEX_GPIO_MODE0 50
#define ZTEX_GPIO_MODE1 45

#define ZTEX_GPIO_LED   52

#define ZTEX_GPIO_FPGA_RESET 51
#define ZTEX_GPIO_FPGA_INIT_B 37
#define ZTEX_GPIO_FPGA_RDWR_B 38
#define ZTEX_GPIO_FPGA_CSI_B 39
#define ZTEX_GPIO_FPGA_DONE 40

#define ZTEX_GPIO_OTG_EN 57 

#undef ZTEX_GPIO_SIMPLE_BITMAP0
#undef ZTEX_GPIO_SIMPLE_BITMAP1
#define ZTEX_GPIO_SIMPLE_BITMAP0 0
#define ZTEX_GPIO_SIMPLE_BITMAP1 ( 1 << (ZTEX_GPIO_MODE0-32) | 1 << (ZTEX_GPIO_MODE1-32) | 1 << (ZTEX_GPIO_LED-32) \
	| 1 << (ZTEX_GPIO_FPGA_RESET-32) | 1 << (ZTEX_GPIO_FPGA_DONE-32) | 1 << (ZTEX_GPIO_FPGA_INIT_B-32) | 1 << (ZTEX_GPIO_FPGA_RDWR_B-32)  | 1 << (ZTEX_GPIO_FPGA_CSI_B-32)  \
	| 1 << (ZTEX_GPIO_OTG_EN-32) \
    )

#define ZTEX_FPGA_CONFIGURED ( ztex_gpio_get(ZTEX_GPIO_FPGA_DONE) )

#define _ZTEX_BOARD_
#define _ZTEX_FPGA_

void ztex_disable_flash();

#endif // _ZTEX_CONF_UFM_2_18_C1_

#ifdef _ZTEX_INCLUDE_2_
#ifndef _ZTEX_CONF_UFM_2_18_C2_
#define _ZTEX_CONF_UFM_2_18_C2_

#include "ztex-fpgaconf1.c"

/* USB system is restarted after FPGA reset and after successful FPGA configuration.
   (de)initialization code should be written to ztex_usb_stop() and ztex_usb_start().
   See ztex-default.c (Template for default firmware) for recommended usage.
*/

uint8_t ztex_fpga_cs = 0;				// check sum
uint32_t ztex_fpga_bytes = 0;				// transferred bytes
uint8_t ztex_fpga_init_b = 0;				// init b 

uint8_t ztex_fpga_config_started = 0;

void ztex_usb_start_main();
void ztex_usb_stop_main();

/* *********************************************************************
   ***** ztex_cpld_set *************************************************
   ********************************************************************* */
void ztex_cpld_set( CyBool_t enable_flash, CyBool_t reset_fpga) {
    ztex_gpio_set(ZTEX_GPIO_MODE0, !enable_flash);
    ztex_gpio_set(ZTEX_GPIO_FPGA_RESET, !reset_fpga);
    ztex_gpio_set(ZTEX_GPIO_MODE1, CyFalse); 
    if ( reset_fpga ) return;
    ztex_gpio_set(ZTEX_GPIO_MODE1, CyTrue);
}

/* *********************************************************************
   ***** ztex_disable_flash ********************************************
   ********************************************************************* */
// disables flash after soft reset
void ztex_disable_flash() {
    ztex_cpld_set(CyFalse, CyFalse);
}

/* *********************************************************************
   ***** ztex_enable_flash *********************************************
   ********************************************************************* */
// enables flash after soft reset
void ztex_enable_flash() {
    ztex_cpld_set(CyTrue, CyFalse);
}

/* *********************************************************************
   ***** ztex_spi_FX3_flash ********************************************
   ********************************************************************* */
// SPI Master: FX3, slave: Flash
void ztex_spi_FX3_flash() {
    ztex_cpld_set(CyTrue, CyFalse);
}

/* *********************************************************************
   ***** ztex_spi_FX3_FPGA *********************************************
   ********************************************************************* */
// SPI Master: FX3, slave: FPGA
void ztex_spi_FX3_FPGA() {
    ztex_gpio_set(ZTEX_GPIO_MODE0, CyFalse);
    ztex_gpio_set(ZTEX_GPIO_MODE0, CyTrue);
}

/* *********************************************************************
   ***** ztex_spi_FPGA_Flash *******************************************
   ********************************************************************* */
// SPI Master: FPGA, slave: Flash
void ztex_spi_FPGA_Flash() {
    ztex_gpio_set(ZTEX_GPIO_MODE0, CyTrue);
    ztex_gpio_set(ZTEX_GPIO_MODE0, CyFalse);
}

/* *********************************************************************
   ***** ztex_fpga_configured ******************************************
   ********************************************************************* */
CyBool_t ztex_fpga_configured() {
    return ztex_gpio_get(ZTEX_GPIO_FPGA_DONE);
}

/* *********************************************************************
   ***** ztex_fpga_reset ***********************************************
   ********************************************************************* */
void ztex_fpga_reset() {
    if ( ZTEX_FPGA_CONFIGURED || ztex_fpga_config_started ) { // restart USB and reset pib clock
	ZTEX_LOG("Info: Preparing USB for FPGA configuration");
	ztex_usb_stop_main();
	ztex_pib_clock2 = &ztex_fpgaconf1_pib_clock;
	ztex_cpld_set(CyTrue, CyTrue);
	ztex_usb_start_main();
	ztex_fpga_config_started = 0;
    }
    
    ztex_cpld_set(CyTrue, CyTrue);

    ztex_gpio_set_output(ZTEX_GPIO_FPGA_RDWR_B, CyFalse);
    ztex_gpio_set_output(ZTEX_GPIO_FPGA_CSI_B, CyFalse);

    CyU3PThreadSleep (20);
    
    ztex_cpld_set(CyTrue, CyFalse);
}

/* *********************************************************************
   ***** ztex_fpga_config_start ****************************************
   ********************************************************************* */
// socket should be 0 for configuration from CPU
void ztex_fpga_config_start(CyU3PDmaSocketId_t socket) {
    uint8_t mode = socket > 0 ? 1 : 2;
    if ( ztex_fpga_config_started == mode ) return;  // already started in correct mode

    ztex_fpga_reset();

    ztex_fpga_config_started = mode;
    
    ztex_fpgaconf1_start(socket);	// start gpif
    
    if ( socket > 0) {			// start auto transfers 
	CyU3PDmaChannel* dma_p = CyU3PDmaChannelGetHandle(socket);
	if ( dma_p != NULL ) ZTEX_REC(CyU3PDmaChannelSetXfer (dma_p, 0));
    }

    uint8_t i = 0;
    while ( (!ztex_gpio_get(ZTEX_GPIO_FPGA_INIT_B)) && i<255 ) {
	CyU3PThreadSleep (1);
	i++;
    }

    ztex_fpga_init_b = ztex_gpio_get(ZTEX_GPIO_FPGA_INIT_B) ? 200 : 100;
    ztex_fpga_cs = 0;
    ztex_fpga_bytes = 0;
}

/* *********************************************************************
   ***** ztex_fpga_config_done *****************************************
   ********************************************************************* */
void ztex_fpga_config_done(CyBool_t fromFlash) { 
    ztex_fpga_init_b += ztex_gpio_get(ZTEX_GPIO_FPGA_INIT_B) ? 22 : 11;
    
    if ( ztex_fpga_config_started == 2)  ztex_fpgaconf1_send(ztex_ep0buf,16); // some extra clock's

    ztex_fpgaconf1_stop();	// stop gpif

    ztex_fpga_config_started = 0;

    if ( fromFlash ) ZTEX_REC( CyU3PPibDeInit() );

    if ( ZTEX_FPGA_CONFIGURED ) {
	ztex_gpio_set_input(ZTEX_GPIO_FPGA_RDWR_B);
	ztex_gpio_set_input(ZTEX_GPIO_FPGA_CSI_B);
	
	ZTEX_LOG("Info: Preparing USB for application");
	if ( ! fromFlash ) ztex_usb_stop_main();	// restart USB and reset PIB clock
	ztex_pib_clock2 = &ztex_pib_clock;
	if ( ! fromFlash ) ztex_usb_start_main();
    }
}

/* *********************************************************************
   ***** vr_fpga_info **************************************************
   ********************************************************************* */
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

/* *********************************************************************
   ***** vc_fpga_reset *************************************************
   ********************************************************************* */
// VC 0x31
uint8_t vc_fpga_reset(uint16_t value, uint16_t index, uint16_t length ) {
    ztex_fpga_reset();
    CyU3PUsbAckSetup();
    return 0;
}

/* *********************************************************************
   ***** vc_fpga_send **************************************************
   ********************************************************************* */
// VC 0x32
uint8_t vc_fpga_send(uint16_t value, uint16_t index, uint16_t length ) {
    ztex_fpga_config_start(0);

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

    if ( length == 0 || ((length & 63) != 0)  ) ztex_fpga_config_done(CyFalse);
    return 0;
}

#ifdef ZTEX_FPGA_CONF_FAST_EP
// ZTEX_FPGA_CONF_FAST_IFACE and ZTEX_FPGA_CONF_FAST_SOCKET must be defined too
/* *********************************************************************
   ***** vc_fpga_fast_info *********************************************
   ********************************************************************* */
// VR 0x33
uint8_t vr_fpga_fast_info(uint16_t value, uint16_t index, uint16_t length ) {
    ztex_ep0buf[0] = ZTEX_FPGA_CONF_FAST_EP;
    ztex_ep0buf[1] = ZTEX_FPGA_CONF_FAST_IFACE;
    ZTEX_REC_RET( CyU3PUsbSendEP0Data( 2, ztex_ep0buf ) );
    return 0;
}  

/* *********************************************************************
   ***** vc_fpga_fast_start ********************************************
   ********************************************************************* */
// VR 0x34
uint8_t vc_fpga_fast_start(uint16_t value, uint16_t index, uint16_t length ) {
    ztex_fpga_config_start(ZTEX_FPGA_CONF_FAST_SOCKET);
    CyU3PUsbAckSetup();
    return 0;
}

/* *********************************************************************
   ***** vc_fpga_fast_finish *******************************************
   ********************************************************************* */
// VR 0x35
uint8_t vc_fpga_fast_finish(uint16_t value, uint16_t index, uint16_t length ) {
    ztex_fpga_config_done(CyFalse);
    CyU3PUsbAckSetup();
    return 0;
}  
#endif


/* *********************************************************************
   ***** ztex_board_init ***********************************************
   ********************************************************************* */
void ztex_board_init() {

    ztex_log ( "Info: Initializing USB-FPGA Module 2.18" );
    
    ztex_disable_flash_boot = ztex_disable_flash;
    ztex_fpga_config_started = 0;

    ztex_gpio_set_output(ZTEX_GPIO_LED, CyFalse);
    ztex_gpio_set_output(ZTEX_GPIO_MODE0, CyTrue);
    ztex_gpio_set_output(ZTEX_GPIO_MODE1, CyTrue);
    
    ztex_gpio_set_input(ZTEX_GPIO_FPGA_DONE);
    ztex_gpio_set_output(ZTEX_GPIO_FPGA_RESET, CyTrue);
    ztex_gpio_set_input(ZTEX_GPIO_FPGA_INIT_B);    	CyU3PGpioSetIoMode(ZTEX_GPIO_FPGA_INIT_B, CY_U3P_GPIO_IO_MODE_WPU);

    ztex_gpio_set_output(ZTEX_GPIO_OTG_EN, CyFalse);
    
    ztex_enable_flash();
    
    ztex_register_vendor_req(0x30, vr_fpga_info);
    ztex_register_vendor_cmd(0x31, vc_fpga_reset);
    ztex_register_vendor_cmd(0x32, vc_fpga_send);
#ifdef ZTEX_FPGA_CONF_FAST_EP
    ztex_register_vendor_req(0x33, vr_fpga_fast_info);
    ztex_register_vendor_cmd(0x34, vc_fpga_fast_start);
    ztex_register_vendor_cmd(0x35, vc_fpga_fast_finish);
#endif    
    
    // select pib clock settings
    ztex_pib_clock2 = ZTEX_FPGA_CONFIGURED ? &ztex_pib_clock : &ztex_fpgaconf1_pib_clock;
}    

/* *********************************************************************
   ***** ztex_flash_config *********************************************
   ********************************************************************* */
#ifndef DISABLE_FLASH_CONFIG
#define _ZTEX_FLASH_CONFIG_FUNC_ { ztex_flash_config(); }
void ztex_flash_config() {
    uint8_t buf[6];
    uint16_t bs_start, bs_size;
    if ( ZTEX_FPGA_CONFIGURED || !ztex_config_data_valid || !ztex_flash.enabled ) return;
    if ( ztex_mac_eeprom_read ( 26, buf, 6 ) ) return;
    
    bs_start = ((buf[4] + 15) & 0xf0) | (buf[5] << 8); 		// in 4k sectors
    bs_size = buf[0] | (buf[1] << 8); 				// in 4k sectors

    if (bs_size == 0) return;
	
    ZTEX_REC( CyU3PPibInit(CyTrue, &ztex_fpgaconf1_pib_clock) ); // init PIB
    ztex_fpga_config_start(0);

    for (int i=0; i<bs_size; i++) {
	if ( ztex_flash_read(ztex_ep0buf, (bs_start+i)<<12, 4096) ) {
	    ztex_log ( "Error uploading bitstream from Flash: Flash read error" );
	    ztex_fpga_config_done(CyTrue);
	    return;
	}
	if ( ztex_fpgaconf1_send(ztex_ep0buf, 4096) ) {
	    ztex_log ( "Error uploading bitstream from Flash: Bitstream write error" );
	    ztex_fpga_config_done(CyTrue);
	    return;
	}
    }
    
    ztex_fpga_config_done(CyTrue);
	    
    if ( ZTEX_FPGA_CONFIGURED ) {
        ztex_log ( "Info: Uploaded bitstream from Flash" );
    }
    else {
        ztex_log ( "Error uploading bitstream from Flash: Done pin does not go high" );
    }
}
#endif

/* *********************************************************************
   ***** ztex_board_stop ***********************************************
   ********************************************************************* */
void ztex_board_stop() {
    if ( ztex_fpga_config_started ) {   // USB is stopped during configuration
        ztex_fpgaconf1_stop();
	ztex_fpga_config_started = 0;
    }
}

/* *********************************************************************
   ***** ztex_enable_otg_supply ****************************************
   ********************************************************************* */
void ztex_enable_otg_supply() {
    ztex_gpio_set(ZTEX_GPIO_OTG_EN, CyTrue);

}

/* *********************************************************************
   ***** ztex_disable_otg_supply ***************************************
   ********************************************************************* */
void ztex_disable_otg_supply() {
    ztex_gpio_set(ZTEX_GPIO_OTG_EN, CyFalse);
}

#endif // _ZTEX_CONF_UFM_2_18_C2_
#endif // _ZTEX_INCLUDE_2_
    
