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
    Example implementation of SD card as secondary flash device.
*/    

#include "cyu3system.h"
#include "cyu3os.h"
#include "cyu3usb.h"
#include "cyu3dma.h"
#include "cyu3error.h"
#include "cyu3sib.h"

#define ZTEX_SD_EC_TIMEOUT 2
#define ZTEX_SD_EC_BUSY 3
#define ZTEX_SD_EC_PENDING 4
#define ZTEX_SD_EC_NOTSUPPORTED 7
#define ZTEX_SD_EC_RUNTIME 8

#define ENABLE_SPORT0

#define ZTEX_SD_DONE_EVENT      (1 << 0)        /* SIB transfer done event. */

struct __attribute__((__packed__)) ztex_sd_t {
    uint8_t enabled;			// 0	1: enabled, 0:disabled
    uint16_t sector_size; 		// 1    in bytes
    uint32_t sectors;			// 3	number of sectors
    uint8_t ec; 	       		// 7	error code
    
    uint16_t page_sectors;		// maximum amount of sectors per transfer
};

struct ztex_sd_t ztex_sd;

CyU3PDmaChannel ztex_sd_wr_handle, ztex_sd_rd_handle;
CyU3PEvent ztex_sd_event;

/* *********************************************************************
   ***** ztex_sd_callback **********************************************
   ********************************************************************* */
void ztex_sd_callback (uint8_t portId, CyU3PSibEventType evt, CyU3PReturnStatus_t status) {
    if ( evt == CY_U3P_SIB_EVENT_XFER_CPLT ) CyU3PEventSet (&ztex_sd_event, ZTEX_SD_DONE_EVENT, CYU3P_EVENT_OR);
}

/* *********************************************************************
   ***** ztex_sd_get_info **********************************************
   ********************************************************************* */
void ztex_sd_get_info () {
    CyU3PReturnStatus_t status = CY_U3P_SUCCESS;
    CyU3PSibDevInfo_t devInfo;

    ztex_sd.enabled = 0;
    ztex_sd.ec = ZTEX_SD_EC_NOTSUPPORTED;
    ztex_sd.sector_size = 512;
    ztex_sd.sectors = 0;
    ztex_sd.page_sectors = 8;
    
    // read SD card properties
    ZTEX_REC( status = CyU3PSibQueryDevice (0, &devInfo) );
    if (status != CY_U3P_SUCCESS) return;
    
    if ( (devInfo.numUnits == 0) || ( (devInfo.cardType != CY_U3P_SIB_DEV_SD) && (devInfo.cardType != CY_U3P_SIB_DEV_MMC)) ) return;  // not supported
    
    if ( devInfo.blkLen > 2048 ) {
	ZTEX_LOG("Unsupported sector size: %d", devInfo.blkLen);
	return;
    }
    
    ztex_sd.enabled = 1;
    ztex_sd.sector_size = devInfo.blkLen;
    ztex_sd.sectors = devInfo.numBlks;
    ztex_sd.page_sectors = 4096 / devInfo.blkLen;
    ztex_sd.ec = 0;
}    

/* *********************************************************************
   ***** ztex_sd_read **************************************************
   ********************************************************************* */
uint8_t ztex_sd_read (uint8_t* buf, uint32_t sector, uint32_t nblk) {
    CyU3PReturnStatus_t status = CY_U3P_SUCCESS;

    CyU3PDmaBuffer_t buf_p;
    uint32_t b2, evStat;

    buf_p.buffer = buf;
    buf_p.status = 0;
    
    while ( nblk>0 ) {
	b2 = nblk;
	if ( b2 > ztex_sd.page_sectors ) b2 = ztex_sd.page_sectors;

        buf_p.size  = b2 * ztex_sd.sector_size;
	buf_p.count = 0;

	ZTEX_REC(status = CyU3PDmaChannelSetupRecvBuffer (&ztex_sd_rd_handle, &buf_p));
	if (status != CY_U3P_SUCCESS) goto errreturn1;

	ZTEX_REC(status = CyU3PSibReadWriteRequest (CyTrue, 0, 0, b2, sector, (uint8_t)CY_U3P_SIB_SOCKET_1) );
	if (status != CY_U3P_SUCCESS) goto errreturn1;

	CyU3PEventGet(&ztex_sd_event, ZTEX_SD_DONE_EVENT, CYU3P_EVENT_OR_CLEAR, &evStat, 1000);
	if (status != CY_U3P_SUCCESS) goto errreturn2;
	
//	ZTEX_REC(status = CyU3PDmaChannelWaitForCompletion (&ztex_sd_rd_handle, 1000));
	ZTEX_REC(status = CyU3PDmaChannelWaitForRecvBuffer (&ztex_sd_rd_handle, &buf_p, 1000) );
	if (status != CY_U3P_SUCCESS) goto errreturn2;

        nblk-=b2;
        buf_p.buffer+=b2 * ztex_sd.sector_size;
        sector+=b2;
    }
    return 0;

errreturn2:
    CyU3PSibAbortRequest (0);
errreturn1:
    CyU3PDmaChannelReset (&ztex_sd_rd_handle);
    ztex_sd.ec = ZTEX_SD_EC_RUNTIME;
    return ztex_sd.ec;
} 

/* *********************************************************************
   ***** ztex_sd_write *************************************************
   ********************************************************************* */
uint8_t ztex_sd_write (uint8_t* buf, uint32_t sector, uint32_t nblk) {
    CyU3PReturnStatus_t status = CY_U3P_SUCCESS;

    CyU3PDmaBuffer_t buf_p;
    uint32_t b2, evStat;

    buf_p.buffer = buf;
    buf_p.status = 0;
    
    while ( nblk>0 ) {
	b2 = nblk;
	if ( b2 > ztex_sd.page_sectors ) b2 = ztex_sd.page_sectors;

        buf_p.size  = b2 * ztex_sd.sector_size;
	buf_p.count = b2 * ztex_sd.sector_size;

	ZTEX_REC(status = CyU3PSibReadWriteRequest (CyFalse, 0, 0, b2, sector, (uint8_t)CY_U3P_SIB_SOCKET_0) );
	if (status != CY_U3P_SUCCESS) goto errreturn1;

	ZTEX_REC(status = CyU3PDmaChannelSetupSendBuffer (&ztex_sd_wr_handle, &buf_p));
	if (status != CY_U3P_SUCCESS) goto errreturn2;

	ZTEX_REC( status = CyU3PEventGet(&ztex_sd_event, ZTEX_SD_DONE_EVENT, CYU3P_EVENT_OR_CLEAR, &evStat, 5000) );
	if (status != CY_U3P_SUCCESS) goto errreturn2;

	ZTEX_REC(status = CyU3PDmaChannelWaitForCompletion (&ztex_sd_wr_handle, 500));
	if (status != CY_U3P_SUCCESS) goto errreturn2;
        nblk-=b2;
        buf_p.buffer+=b2 * ztex_sd.sector_size;
        sector+=b2;
    }
    return 0;

errreturn2:
    CyU3PSibAbortRequest (0);
errreturn1:
    ztex_sd.ec = ZTEX_SD_EC_RUNTIME;
    CyU3PDmaChannelReset (&ztex_sd_wr_handle);
    return ztex_sd.ec;
} 

/* *********************************************************************
   ***** vr_flash2_info ************************************************
   ********************************************************************* */
// VR 0x44
uint8_t vr_flash2_info(uint16_t value, uint16_t index, uint16_t length ) {
    ztex_sd_get_info();
    CyU3PMemCopy( ztex_ep0buf, (uint8_t*)&ztex_sd, 8);
    ZTEX_REC_RET( CyU3PUsbSendEP0Data( 8, ztex_ep0buf ) );
    return 0;
} 

/* *********************************************************************
   ***** vr_flash2_read ************************************************
   ********************************************************************* */
// VR 0x45
uint8_t vr_flash2_read(uint16_t value, uint16_t index, uint16_t length ) {
//    ZTEX_LOG("F2R: %d, %d", index, value);
    if ( ztex_sd_read(ztex_ep0buf, (index << 16) | value, length/ztex_sd.sector_size ) ) return 255;
    ZTEX_REC_RET ( CyU3PUsbSendEP0Data( length, ztex_ep0buf ) );
    return 0;
}

/* *********************************************************************
   ***** vr_flash2_write ************************************************
   ********************************************************************* */
// VC 0x46
uint8_t vc_flash2_write(uint16_t value, uint16_t index, uint16_t length ) {
//    ZTEX_LOG("F2W: %d, %d", index, value);
    ZTEX_REC_RET ( CyU3PUsbGetEP0Data (length, ztex_ep0buf, NULL) );
    if ( ztex_sd_write(ztex_ep0buf, (index << 16) | value, length/ztex_sd.sector_size ) ) return 255;
    return 0;
} 

/* *********************************************************************
   ***** ztex_sd_init **************************************************
   ********************************************************************* */
void ztex_sd_init () {
    CyU3PReturnStatus_t status = CY_U3P_SUCCESS;
    CyU3PSibIntfParams_t intfParams;

    ztex_sd.enabled = 0;
    ztex_sd.ec = ZTEX_SD_EC_NOTSUPPORTED;

    ztex_register_vendor_req(0x44, vr_flash2_info);
    ztex_register_vendor_req(0x45, vr_flash2_read);
    ztex_register_vendor_cmd(0x46, vc_flash2_write);

    ZTEX_REC( CyU3PDeviceGpioOverride (43, CyTrue) );
    ZTEX_REC( CyU3PDeviceGpioOverride (44, CyTrue) );

    // Configure SD card port
//    intfParams.cardDetType = CY_U3P_SIB_DETECT_DAT_3;   // Card detect based on SD_DAT[3]
    intfParams.cardDetType = CY_U3P_SIB_DETECT_GPIO;    // Card detect based on deticated GPIO pin
    intfParams.writeProtEnable = CyTrue;                // Write protecttion handling enabled
//    intfParams.useDdr = CyFalse;                        // DDR clocking enabled
//    intfParams.maxFreq = CY_U3P_SIB_FREQ_104MHZ;        // No S port clock limitation
    intfParams.useDdr = CyFalse;                        // DDR clocking disabled
    intfParams.maxFreq = CY_U3P_SIB_FREQ_52MHZ;         // S port clock limited to 52 MHz
    intfParams.cardInitDelay   = 5;                     // SD/MMC initialization delay of 5ms 
    intfParams.resetGpio = 0xFF;			// No reset GPIO's
    intfParams.rstActHigh = CyTrue; 
    intfParams.voltageSwGpio = 0xFF;                    // No low voltage switch GPIO's
    intfParams.lvGpioState = CyFalse;
    intfParams.lowVoltage = CyFalse;
    ZTEX_REC( status = CyU3PSibSetIntfParams (0, &intfParams) );
    if (status != CY_U3P_SUCCESS) goto errreturn;
    
    // start the module and initialize SD card
    ZTEX_REC( status = CyU3PSibStart() );
    if (status != CY_U3P_SUCCESS) goto errreturn;

    // init event handler
    ZTEX_REC(status = CyU3PEventCreate (&ztex_sd_event));
    if (status != CY_U3P_SUCCESS) goto errreturn;

    ZTEX_REC(status = CyU3PSibRegisterCbk (ztex_sd_callback));
    if (status != CY_U3P_SUCCESS) goto errreturn;
    
    // create DMA channels
    CyU3PDmaChannelConfig_t dmaConfig;
    CyU3PMemSet ((uint8_t *)&dmaConfig, 0, sizeof(dmaConfig));
    dmaConfig.size           = 4096;
    // No buffers need to be allocated as this channel will be used only in override mode. 
    dmaConfig.count          = 0;
    dmaConfig.prodAvailCount = 0;
    dmaConfig.dmaMode        = CY_U3P_DMA_MODE_BYTE;
    dmaConfig.prodHeader     = 0;
    dmaConfig.prodFooter     = 0;
    dmaConfig.consHeader     = 0;
    dmaConfig.notification   = 0;
    dmaConfig.cb             = NULL;

    // Channel to write to SD flash
    dmaConfig.prodSckId = CY_U3P_CPU_SOCKET_PROD;
    dmaConfig.consSckId = CY_U3P_SIB_SOCKET_0;
    ZTEX_REC(status =  CyU3PDmaChannelCreate (&ztex_sd_wr_handle, CY_U3P_DMA_TYPE_MANUAL_OUT, &dmaConfig) );
    if (status != CY_U3P_SUCCESS) goto errreturn;

    // Channel to read from SD flash
    dmaConfig.prodSckId = CY_U3P_SIB_SOCKET_1;
    dmaConfig.consSckId = CY_U3P_CPU_SOCKET_CONS;
    ZTEX_REC(status =  CyU3PDmaChannelCreate (&ztex_sd_rd_handle, CY_U3P_DMA_TYPE_MANUAL_IN, &dmaConfig) );
    if (status != CY_U3P_SUCCESS) goto errreturn;

    // read SD card properties
    ztex_sd_get_info();
    if ( ztex_sd.enabled == 0 ) goto noflashreturn;
    
    ZTEX_LOG("Found %d MByte SD Flash",(ztex_sd.sectors>>14)*(ztex_sd.sector_size>>6));
    return;
errreturn:
    ztex_sd.ec = ZTEX_SD_EC_RUNTIME;
noflashreturn:
    ztex_log("No SD Flash found");
}

/* *********************************************************************
   ***** ztex_usb_stop_sd **********************************************
   ********************************************************************* */
void ztex_usb_stop_sd() {
    CyU3PDmaChannelReset (&ztex_sd_wr_handle);
    CyU3PDmaChannelReset (&ztex_sd_rd_handle);
}
