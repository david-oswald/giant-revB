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
    Main include file. See the examples for usage.
*/
    
#include "cyu3system.h"
#include "cyu3os.h"
#include "cyu3dma.h"
#include "cyu3error.h"
#include "cyu3usb.h"
#include "cyu3uart.h"
#include "cyu3pib.h"

#define	ZTEX_VENDOR_REQ_MAX 	50			// maximum amount of vendor requests
#define	ZTEX_VENDOR_CMD_MAX 	50			// maximum amount of vendor commands

typedef uint8_t (*ztex_vendor_func) (uint16_t value, uint16_t index, uint16_t length );

// global configuration
uint32_t ztex_app_thread_stack = 0x1000;	// stack size of application thread
uint32_t ztex_app_thread_prio = 8;		// priority of application thread, should be 7..15
void (*ztex_app_thread_run)() = 0;
void (*ztex_usb_start)() = 0;			// called when USB connection is started
void (*ztex_usb_stop)() = 0;			// called when USB connection is stopped

CyBool_t ztex_allow_lpm = CyFalse;		// whether to allow low power mode transitions

// strings for interfaces 0..7, can be overwritten by user
char* ztex_interface_string[] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

/* 
   PIB clock settings for user application.
*/   
CyU3PPibClock_t ztex_pib_clock = {
    .clkDiv = 6,		// 69.33 MHz @ 26 MHz external clock
    .clkSrc = CY_U3P_SYS_CLK,
    .isHalfDiv = CyFalse,
    .isDllEnable = CyTrue
};

/* 
   PIB clock settings which are currently used. 
   They may be under control of a board specific module, e.g. to load
   different clock settings for FPGA configuration.
*/   
CyU3PPibClock_t *ztex_pib_clock2 = &ztex_pib_clock;

// called to clean up a transfer when a clear stall request is received, see 
void ep_cleanup_default_handler(uint16_t);
void (*ztex_ep_cleanup_handler)(uint16_t) = ep_cleanup_default_handler;

// system stuff
uint8_t ztex_usb_is_connected = 0;

// super speed error counters
#define ZTEX_USB3_SND_ERROR_COUNT (*((uint16_t*) 0xe0033014))
#define ZTEX_USB3_RCV_ERROR_COUNT (*((uint16_t*) 0xe0033016))

#ifndef EP_SETUP_FPGACONF
#define EP_SETUP_FPGACONF
#endif

#define EP_SETUP_ALL EP_SETUP EP_SETUP_FPGACONF

#include "ztex-descriptors.c"
#include "ztex-debug.c"
#include "ztex-usb.c"
#include "ztex-ep0.c"
#include "ztex-gpio.c"


// SPI Flash support
#ifdef ENABLE_SPI_FLASH
#define ENABLE_SPI
#include "ztex-flash.c"
#endif

// SD Flash support
#ifdef ENABLE_SD_FLASH
#include "ztex-sd.c"
#endif

#ifdef ENABLE_I2C
#include "ztex-i2c.c"
#endif


#define _ZTEX_INCLUDE_2_
#ifdef _ZTEX_CONF_UFM_2_02_C1_
#include "ztex-ufm-2_02.c"
#endif
#ifdef _ZTEX_CONF_UFM_2_14_C1_
#include "ztex-ufm-2_14.c"
#endif
#ifdef _ZTEX_CONF_UFM_2_18_C1_
#include "ztex-ufm-2_18.c"
#endif
#ifdef _ZTEX_LSI_
#include "ztex-lsi.c"
#endif



void ztex_usb_start_main() {
    ztex_usb_start_usb();

    if ( ztex_usb_start != 0) 
	ztex_usb_start();

    CyU3PUSBSpeed_t usbSpeed = CyU3PUsbGetSpeed();
    ZTEX_LOG ("Info: USB setup finished: %s", usbSpeed == CY_U3P_SUPER_SPEED ? "super speed" : usbSpeed == CY_U3P_HIGH_SPEED ? "high speed" : usbSpeed == CY_U3P_FULL_SPEED ? "full speed" : "not connected" );
    
    ztex_usb_is_connected = 1;
}    

void ztex_usb_stop_main() {
    ztex_usb_is_connected = 0;

    if ( ztex_usb_stop != 0) 
	ztex_usb_stop();

#ifdef ENABLE_SPI_FLASH
    ztex_usb_stop_flash();
#endif
#ifdef ENABLE_SD_FLASH
    ztex_usb_stop_sd();
#endif
#ifdef _ZTEX_BOARD_
    ztex_board_stop();
#endif    

    ztex_usb_stop_usb();
    
    ztex_log ("Info: USB disconnected."); 
}

/* 
   Default handler to clean up a transfer. Since we do no known how endpoints
   are associated we reset the whole connection. This may no be what is
   expected by the host.
*/    
void ep_cleanup_default_handler(uint16_t ep) {
    ztex_usb_stop_main();
    CyU3PThreadSleep (1);  // Give a chance for the main thread loop to run
    ztex_usb_start_main();
};

// USB event handler
void ztex_usb_event_handler ( CyU3PUsbEventType_t evtype, uint16_t evdata) {
//    ZTEX_LOG("Event: %d",evtype);
    switch (evtype)
    {
        case CY_U3P_USB_EVENT_SETCONF:
            // stop the connection before restarting
            if (ztex_usb_is_connected) {
                ztex_usb_stop_main();
            }
            // start the connection
            ztex_usb_start_main();
            break;

        case CY_U3P_USB_EVENT_RESET:
        case CY_U3P_USB_EVENT_DISCONNECT:
            // stops the connection
            if (ztex_usb_is_connected) {
                ztex_usb_stop_main();
            }
            break;

        default:
            break;
    }
}



// LPM (link power management request). Return value CyTrue allows transitions to low power modes.
CyBool_t ztex_lpm_handler ( CyU3PUsbLinkPowerMode link_mode)
{
    return ztex_allow_lpm;
}


CyU3PThread ztex_app_thread;	 			// ztex application thread structure 

// entry function for the application thread
void ztex_app_thread_entry (uint32_t input)
{
    ztex_debug_init();
    
    ZTEX_REC(CyU3PUsbStart());						// start USB functionality

    CyU3PUsbRegisterSetupCallback(ztex_ep0_handler, CyTrue);		// register EP0 handler

    CyU3PUsbRegisterLPMRequestCallback(ztex_lpm_handler);    		// register link power management handler

    CyU3PUsbRegisterEventCallback(ztex_usb_event_handler);		// register USB event handler
    
    // Set the USB registers
    ZTEX_REC( CyU3PUsbSetDesc(CY_U3P_USB_SET_SS_DEVICE_DESCR, 0, (uint8_t *)ztex_usb3_device_descriptor) );		// Super speed device descriptor
    ZTEX_REC( CyU3PUsbSetDesc(CY_U3P_USB_SET_HS_DEVICE_DESCR, 0, (uint8_t *)ztex_usb2_device_descriptor) );		// High speed device descriptor
    ZTEX_REC( CyU3PUsbSetDesc(CY_U3P_USB_SET_SS_BOS_DESCR, 0, (uint8_t *)ztex_bos_descriptor) ); 			// BOS descriptor
    ZTEX_REC( CyU3PUsbSetDesc(CY_U3P_USB_SET_DEVQUAL_DESCR, 0, (uint8_t *)ztex_device_qualifier_descriptor) ); 		// Device qualifier descriptor
    ZTEX_REC( CyU3PUsbSetDesc(CY_U3P_USB_SET_SS_CONFIG_DESCR, 0, (uint8_t *)ztex_usb3_config_descriptor) );     	// Super speed configuration descriptor
    ZTEX_REC( CyU3PUsbSetDesc(CY_U3P_USB_SET_HS_CONFIG_DESCR, 0, (uint8_t *)ztex_usb2_config_descriptor) );     	// High speed configuration descriptor
    ZTEX_REC( CyU3PUsbSetDesc(CY_U3P_USB_SET_FS_CONFIG_DESCR, 0, (uint8_t *)ztex_usb1_config_descriptor) );  		// Full speed configuration descriptor
    CyU3PUsbSetDesc(CY_U3P_USB_SET_STRING_DESCR, 0, (uint8_t *)ztex_lang_string_descriptor);				// String descriptor 0 must not be handled by ep0_handler for an undocumented reason

    ztex_gpio_init();
    
#ifdef _ZTEX_BOARD_
    ztex_board_init();
#endif    

#ifdef ENABLE_SPI_FLASH
    ztex_flash_init();
#endif    
#ifdef ENABLE_SD_FLASH
    ztex_sd_init();
#endif    
#ifdef ENABLE_I2C
    ztex_i2c_init();
#endif    
#ifdef _ZTEX_LSI_
    ztex_lsi_init();
#endif    
#ifdef _ZTEX_FLASH_CONFIG_FUNC_
    _ZTEX_FLASH_CONFIG_FUNC_
#endif

    // Connect the USB Pins with super speed operation enabled.
    ZTEX_REC( CyU3PConnectState(CyTrue, CyTrue) );

    if ( ztex_app_thread_run != 0) 
	ztex_app_thread_run();
    
    for (;;)
    {
        CyU3PThreadSleep (1000);
    }
}

/* This function is expected and called by OS. */
void CyFxApplicationDefine()
{
    /* create and start the application thread */
    if ( CyU3PThreadCreate(&ztex_app_thread,           	// ztex application thread structure 
                           "21:ztex_app_thread",	// thread ID and name
                          ztex_app_thread_entry,	// entry function
                          0,				// no input parameter
                          CyU3PMemAlloc(ztex_app_thread_stack),	// allocate stack memory
                          ztex_app_thread_stack,	// stack size
                          ztex_app_thread_prio,		// thread priority
                          ztex_app_thread_prio,		// preempt threshold		
                          CYU3P_NO_TIME_SLICE,		// no time slice as recommended
                          CYU3P_AUTO_START		// start the thread immediately
                          ) != 0 
        ) ztex_ec = 105;
}

/*
 * Main function. Configures the IO-Matrix and starts the OS
 */
void ztex_main (void)
{
    ztex_ec = 101;
    
    /* Initialize the device */
    CyU3PSysClockConfig_t clock_cfg;
    clock_cfg.setSysClk400 = CyFalse;
    clock_cfg.cpuClkDiv = 2;
    clock_cfg.dmaClkDiv = 2;
    clock_cfg.mmioClkDiv = 2;
    clock_cfg.useStandbyClk = CyFalse;
    clock_cfg.clkSrc = CY_U3P_SYS_CLK;
    if ( CyU3PDeviceInit(&clock_cfg) != CY_U3P_SUCCESS ) ztex_ec = 102;

    /* Initialize the caches. Enable both Instruction and Data Caches. */
    if ( CyU3PDeviceCacheControl(CyTrue, CyTrue, CyTrue) != CY_U3P_SUCCESS ) ztex_ec = 103;

    CyU3PIoMatrixConfig_t io_cfg;
    CyU3PMemSet((uint8_t *)&io_cfg, 0, sizeof(io_cfg));
    io_cfg.isDQ32Bit = CyFalse;
#ifdef ENABLE_SPORT0
    io_cfg.s0Mode = CY_U3P_SPORT_4BIT;
#else    
    io_cfg.s0Mode = CY_U3P_SPORT_INACTIVE;
#endif
    io_cfg.s1Mode = CY_U3P_SPORT_INACTIVE;
#ifdef ENABLE_UART    
    io_cfg.useUart = CyTrue;
#else    
    io_cfg.useUart = CyFalse;
#endif    
#ifdef ENABLE_I2C
    io_cfg.useI2C = CyTrue;
#else
    io_cfg.useI2C = CyFalse;
#endif
    io_cfg.useI2S = CyFalse;
#ifdef ENABLE_SPI
    io_cfg.useSpi = CyTrue;
#else
    io_cfg.useSpi = CyFalse;
#endif
    io_cfg.lppMode = CY_U3P_IO_MATRIX_LPP_DEFAULT;
    /* No GPIOs are enabled. */
    io_cfg.gpioSimpleEn[0] = ( ZTEX_GPIO_SIMPLE_BITMAP0 | GPIO_SIMPLE_BITMAP0 ) & ( ~(ZTEX_GPIO_COMPLEX_BITMAP0 | GPIO_COMPLEX_BITMAP0 ) );
    io_cfg.gpioSimpleEn[1] = ( ZTEX_GPIO_SIMPLE_BITMAP1 | GPIO_SIMPLE_BITMAP1 ) & ( ~(ZTEX_GPIO_COMPLEX_BITMAP1 | GPIO_COMPLEX_BITMAP1 ) );
    io_cfg.gpioComplexEn[0] = ZTEX_GPIO_COMPLEX_BITMAP0 | GPIO_COMPLEX_BITMAP0;
    io_cfg.gpioComplexEn[1] = ZTEX_GPIO_COMPLEX_BITMAP1 | GPIO_COMPLEX_BITMAP1;
    if (CyU3PDeviceConfigureIOMatrix (&io_cfg) != CY_U3P_SUCCESS) ztex_ec = 104;

    /* This is a non returnable call for initializing the RTOS kernel */
    ztex_ec = 0;
    CyU3PKernelEntry ();
}

