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
    SPI flash support.
*/
    
#include "cyu3system.h"
#include "cyu3os.h"
#include "cyu3usb.h"
#include "cyu3spi.h"
#include "cyu3dma.h"
#include "cyu3error.h"

#define ZTEX_SPI_DMA
#define ZTEX_SPI_PAGE_SIZE 256

#ifdef ZTEX_SPI_DMA
CyU3PDmaChannel ztex_spi_wr_handle, ztex_spi_rd_handle;
#endif

#define FLASH_EC_TIMEOUT 2
#define FLASH_EC_BUSY 3
#define FLASH_EC_PENDING 4
#define FLASH_EC_NOTSUPPORTED 7
#define FLASH_EC_RUNTIME 8

struct __attribute__((__packed__)) ztex_flash_t {
    uint8_t enabled;			// 0	1: enabled, 0:disabled
    uint16_t sector_size; 		// 1    sector size <sector size> = MSB==0 : flash_sector_size and 0x7fff ? 1<<(flash_sector_size and 0x7fff)
    uint32_t sectors;			// 3	number of sectors
    uint8_t ec; 	       		// 7	error code
    
    uint8_t vendor;			// 0
    uint8_t device;			// 1
    uint8_t memtype;			// 2
    uint8_t erase_cmd;			// 3
    uint8_t last_cmd;			// 4
    uint8_t buf[4];			// 5
    uint8_t cs;
    
    uint32_t ep0_read_addr;
    uint8_t need_pp;
    uint32_t write_addr;
};

struct ztex_flash_t ztex_flash;

void ztex_flash_select () {
    CyU3PSpiSetSsnLine(CyFalse);
    ztex_flash.cs = 1;
}

void ztex_flash_deselect () {
    CyU3PSpiSetSsnLine(CyTrue);
    ztex_flash.cs = 0;
}

uint8_t ztex_flash_wait() {
    uint16_t i;
    CyU3PReturnStatus_t status = CY_U3P_SUCCESS;

    ztex_flash.ec = ztex_flash.cs ? FLASH_EC_PENDING : ztex_flash.enabled ? 0 : FLASH_EC_NOTSUPPORTED;
    if ( ztex_flash.ec ) return ztex_flash.ec;

    ztex_flash.buf[0]=ztex_flash.last_cmd=5;
    ztex_flash_select();
    ZTEX_REC(status = CyU3PSpiTransmitWords (ztex_flash.buf, 1));
    if (status != CY_U3P_SUCCESS) goto errreturn;
    
    for ( i=0; i<11000; i++) {  // wait up to 11s
	ZTEX_REC(status = CyU3PSpiReceiveWords (ztex_flash.buf, 4));
	if (status != CY_U3P_SUCCESS) goto errreturn;
	if ( !(ztex_flash.buf[0] & 1) ) break;
	CyU3PThreadSleep(1);	// 1ms
    }
    ztex_flash_deselect();
    ztex_flash.ec = ztex_flash.buf[0] & 1 ? FLASH_EC_TIMEOUT : 0;
    return ztex_flash.ec;

errreturn:
    ztex_flash.ec = FLASH_EC_RUNTIME;
    ztex_flash_deselect();
    return ztex_flash.ec;
}

uint8_t ztex_flash_read (uint8_t* buf, uint32_t addr, uint32_t size) {
    CyU3PReturnStatus_t status = CY_U3P_SUCCESS;

    if ( ztex_flash_wait() ) return ztex_flash.ec;
    
    ztex_flash.buf[0]=ztex_flash.last_cmd=0xb;
    ztex_flash.buf[1]=addr>>16;
    ztex_flash.buf[2]=addr>>8;
    ztex_flash.buf[3]=addr;
    ztex_flash_select();
    ZTEX_REC(status = CyU3PSpiTransmitWords (ztex_flash.buf, 5));
    if (status != CY_U3P_SUCCESS) goto errreturn;
    
#ifdef ZTEX_SPI_DMA
    CyU3PDmaBuffer_t buf_p;
    uint32_t s2;
    
    buf_p.buffer = buf;
    buf_p.status = 0;
    
    while ( size>0 ) {
	s2 = size;
	if ( s2 > ZTEX_SPI_PAGE_SIZE ) s2 = ZTEX_SPI_PAGE_SIZE;

        buf_p.size  = s2;
	buf_p.count = s2;

	CyU3PSpiSetBlockXfer (0, s2);
	ZTEX_REC(status = CyU3PDmaChannelSetupRecvBuffer (&ztex_spi_rd_handle, &buf_p));
	if (status != CY_U3P_SUCCESS) goto errreturn;
	
	ZTEX_REC(status = CyU3PDmaChannelWaitForCompletion (&ztex_spi_rd_handle, 500));
	if (status != CY_U3P_SUCCESS) {
    	    CyU3PDmaChannelReset (&ztex_spi_rd_handle);
    	    CyU3PSpiDisableBlockXfer (CyFalse, CyTrue);
	    goto errreturn;
	}
        CyU3PSpiDisableBlockXfer (CyFalse, CyTrue);
        
        size-=s2;
        buf_p.buffer+=s2;
//        CyU3PThreadSleep(1);
    }
#else
    ZTEX_REC(status = CyU3PSpiReceiveWords(buf, size) );
    if (status != CY_U3P_SUCCESS) goto errreturn;
#endif
    ztex_flash_deselect();
    return 0;

errreturn:
    ztex_flash.ec = FLASH_EC_RUNTIME;
    ztex_flash_deselect();
    return ztex_flash.ec;
}


// sends pp command
uint8_t ztex_flash_pp () {
    CyU3PReturnStatus_t status = CY_U3P_SUCCESS;

    ztex_flash_deselect();			// finish previous write cmd
    ztex_flash.need_pp = 0;
    if ( ztex_flash_wait() ) return ztex_flash.ec;
    
//    ZTEX_LOG("flash_pp: %d",ztex_flash.write_addr);

    ztex_flash.buf[0]=ztex_flash.last_cmd=0x06;
    ztex_flash_select();
    ZTEX_REC(status = CyU3PSpiTransmitWords (ztex_flash.buf, 1));
    if (status != CY_U3P_SUCCESS) goto errreturn;
    ztex_flash_deselect();

    ztex_flash.buf[0]=ztex_flash.last_cmd=0x02;
    ztex_flash.buf[1]=ztex_flash.write_addr >> 16;
    ztex_flash.buf[2]=ztex_flash.write_addr >> 8;
    ztex_flash.buf[3]=ztex_flash.write_addr;
    ztex_flash_select();
    ZTEX_REC(status = CyU3PSpiTransmitWords (ztex_flash.buf, 4));
    if (status != CY_U3P_SUCCESS) goto errreturn;

    return 0;

errreturn:
    ztex_flash.ec = FLASH_EC_RUNTIME;
    ztex_flash_deselect();
    return ztex_flash.ec;
}

// has to be called at begin of a 64K write sequence
uint8_t ztex_flash_write_start (uint16_t sector) {
    CyU3PReturnStatus_t status = CY_U3P_SUCCESS;

    if ( ztex_flash_wait() ) return ztex_flash.ec;

    // write enable command
    ztex_flash.buf[0]=ztex_flash.last_cmd=0x06;
    ztex_flash_select();
    ZTEX_REC(status = CyU3PSpiTransmitWords (ztex_flash.buf, 1));
    if (status != CY_U3P_SUCCESS) goto errreturn;
    ztex_flash_deselect();

    // erase  command
//    ztex_log("flash_erase");
    ztex_flash.buf[0]=ztex_flash.last_cmd=ztex_flash.erase_cmd;
    ztex_flash.buf[1]=sector;
    ztex_flash.buf[2]=0;
    ztex_flash.buf[3]=0;
    ztex_flash_select();
    ZTEX_REC(status = CyU3PSpiTransmitWords (ztex_flash.buf, 4));
    if (status != CY_U3P_SUCCESS) goto errreturn;
    ztex_flash_deselect();
    
    ztex_flash.need_pp = 1;
    ztex_flash.write_addr = sector << 16;
    return 0;
    
errreturn:
    ztex_flash.ec = FLASH_EC_RUNTIME;
    ztex_flash_deselect();
    return ztex_flash.ec;
}

// is called between ztex_write_start and ztex_write_finish
uint8_t ztex_flash_write (uint8_t* buf, uint32_t size) {
    uint32_t s2;
    CyU3PReturnStatus_t status = CY_U3P_SUCCESS;

    if ( ztex_flash.need_pp && ztex_flash_pp() ) return ztex_flash.ec;

    while (size>0) {
	s2 = 256-(ztex_flash.write_addr & 255);
	if (s2>size) s2=size;
#ifdef ZTEX_SPI_DMA
	CyU3PDmaBuffer_t buf_p;

        buf_p.size  = s2;
	buf_p.count = s2;
	buf_p.buffer = buf;
	buf_p.status = 0;

	CyU3PSpiSetBlockXfer (s2, 0);
	ZTEX_REC( status = CyU3PDmaChannelSetupSendBuffer (&ztex_spi_wr_handle, &buf_p) );

	ZTEX_REC(status = CyU3PDmaChannelWaitForCompletion (&ztex_spi_wr_handle, 500));
	if (status != CY_U3P_SUCCESS) {
    	    CyU3PSpiDisableBlockXfer (CyTrue, CyFalse);
	    goto errreturn;
	}
        CyU3PSpiDisableBlockXfer (CyTrue, CyFalse);
#else	
	ZTEX_REC(status = CyU3PSpiTransmitWords (buf, s2));
	if (status != CY_U3P_SUCCESS) goto errreturn;
#endif	
	size-=s2;
	buf+=s2;
	ztex_flash.write_addr+=s2;
	if ( size==0 ) {
	    ztex_flash_deselect();	// finish pp
	    ztex_flash.need_pp = 1;	// do not wait
	}
	else {
	    if ( ztex_flash_pp() ) return ztex_flash.ec; // finish pp + wait
	}
    }

    return 0;

errreturn:
    ztex_flash.ec = FLASH_EC_RUNTIME;
    ztex_flash_deselect();
    return ztex_flash.ec;
}

// is called after ztex_flash_write
void ztex_flash_write_finish () {
    ztex_flash_deselect();
}


// VR 0x40
uint8_t vr_flash_info(uint16_t value, uint16_t index, uint16_t length ) {
    if ( ztex_flash.ec==0 && ztex_flash.cs ) ztex_flash.ec = FLASH_EC_PENDING;
    CyU3PMemCopy( ztex_ep0buf, (uint8_t*)&ztex_flash, 8);
    ZTEX_REC_RET( CyU3PUsbSendEP0Data( 8, ztex_ep0buf ) );
    return 0;
}

// VR 0x41
uint8_t vr_flash_read(uint16_t value, uint16_t index, uint16_t length ) {
    index = index >> 8;
    if ( index==0 ) {
//        ZTEX_LOG("FR: %d, %d", index, value);
	ztex_flash.ep0_read_addr = value << 16;
    }
    if ( ztex_flash_read(ztex_ep0buf,ztex_flash.ep0_read_addr,length) ) return 255;
    ztex_flash.ep0_read_addr += length;
    ZTEX_REC_RET ( CyU3PUsbSendEP0Data( length, ztex_ep0buf ) );
    return 0;
}

// VC 0x42
uint8_t vc_flash_write(uint16_t value, uint16_t index, uint16_t length ) {
    index = index >> 8;
//    ZTEX_LOG("FW: %d, %d", index, value);
    if ( index==0 && ztex_flash_write_start(value) ) return 255;

    ZTEX_REC_RET ( CyU3PUsbGetEP0Data (length, ztex_ep0buf, NULL) );
    if ( ztex_flash_write(ztex_ep0buf,length) ) return 255;
    
    if ( index==3 ) ztex_flash_write_finish();
    return 0;
}

// VR 0x43
uint8_t vr_flash_info2(uint16_t value, uint16_t index, uint16_t length ) {
    CyU3PMemCopy( ztex_ep0buf, (uint8_t*)&ztex_flash.ec, 10);
    return CyU3PUsbSendEP0Data( 10, ztex_ep0buf );
}

void ztex_flash_init () {
    CyU3PReturnStatus_t status = CY_U3P_SUCCESS;
    CyU3PSpiConfig_t spiConfig;

    ztex_flash.enabled = 0;
    ztex_flash.ec = FLASH_EC_NOTSUPPORTED;
    ztex_flash.cs = 0;

    ztex_register_vendor_req(0x40, vr_flash_info);
    ztex_register_vendor_req(0x41, vr_flash_read);
    ztex_register_vendor_cmd(0x42, vc_flash_write);
    ztex_register_vendor_req(0x43, vr_flash_info2);

    // Start the SPI module and configure the master 
    ZTEX_REC(status = CyU3PSpiInit());
    if (status != CY_U3P_SUCCESS) goto errreturn;

    /* Start the SPI master block. Run the SPI clock at 8MHz
       and configure the word length to 8 bits. Also configure
       the slave select using FW */
    CyU3PMemSet ((uint8_t *)&spiConfig, 0, sizeof(spiConfig));
    spiConfig.isLsbFirst = CyFalse;
    spiConfig.cpol       = CyTrue;
    spiConfig.ssnPol     = CyFalse;
    spiConfig.cpha       = CyTrue;
    spiConfig.leadTime   = CY_U3P_SPI_SSN_LAG_LEAD_HALF_CLK;
    spiConfig.lagTime    = CY_U3P_SPI_SSN_LAG_LEAD_HALF_CLK;
    spiConfig.ssnCtrl    = CY_U3P_SPI_SSN_CTRL_FW;
    spiConfig.clock      = 33000000;
//    spiConfig.clock      = 10000000;
    spiConfig.wordLen    = 8;
    ZTEX_REC(status =  CyU3PSpiSetConfig (&spiConfig, NULL) );
    if (status != CY_U3P_SUCCESS) goto errreturn;

#ifdef ZTEX_SPI_DMA
    CyU3PDmaChannelConfig_t dmaConfig;

    // DMA channels for SPI write 
    CyU3PMemSet ((uint8_t *)&dmaConfig, 0, sizeof(dmaConfig));
    dmaConfig.size           = ZTEX_SPI_PAGE_SIZE;
    // No buffers need to be allocated as this channel will be used only in override mode. */
    dmaConfig.count          = 0;
    dmaConfig.prodAvailCount = 0;
    dmaConfig.dmaMode        = CY_U3P_DMA_MODE_BYTE;
    dmaConfig.prodHeader     = 0;
    dmaConfig.prodFooter     = 0;
    dmaConfig.consHeader     = 0;
    dmaConfig.notification   = 0;
    dmaConfig.cb             = NULL;

    /* Channel to write to SPI flash. */
    dmaConfig.prodSckId = CY_U3P_CPU_SOCKET_PROD;
    dmaConfig.consSckId = CY_U3P_LPP_SOCKET_SPI_CONS;
    ZTEX_REC(status =  CyU3PDmaChannelCreate (&ztex_spi_wr_handle, CY_U3P_DMA_TYPE_MANUAL_OUT, &dmaConfig) );
    if (status != CY_U3P_SUCCESS) goto errreturn;

    /* Channel to read from SPI flash. */
    dmaConfig.prodSckId = CY_U3P_LPP_SOCKET_SPI_PROD;
    dmaConfig.consSckId = CY_U3P_CPU_SOCKET_CONS;
    ZTEX_REC(status =  CyU3PDmaChannelCreate (&ztex_spi_rd_handle, CY_U3P_DMA_TYPE_MANUAL_IN, &dmaConfig) );
    if (status != CY_U3P_SUCCESS) goto errreturn;
#endif

    // CMD 0x90 may not be supported by all devices
    ztex_flash.buf[0]=0x90;
    ztex_flash.buf[1]=0;
    ztex_flash.buf[2]=0;
    ztex_flash.buf[3]=0;
    ztex_flash_select();
    ZTEX_REC(status = CyU3PSpiTransmitWords (ztex_flash.buf, 4));
    if (status != CY_U3P_SUCCESS) goto errreturn;
    ztex_flash.device = CyU3PSpiReceiveWords (ztex_flash.buf, 2) == CY_U3P_SUCCESS ? ztex_flash.buf[1] : 127;
    ztex_flash_deselect();

    // CMD 0x9F: JEDEC ID
    ztex_flash.buf[0]=ztex_flash.last_cmd=0x9F;
    ztex_flash_select();
    ZTEX_REC(status = CyU3PSpiTransmitWords (ztex_flash.buf, 1));
    if (status != CY_U3P_SUCCESS) goto errreturn;
    ZTEX_REC(status = CyU3PSpiReceiveWords (ztex_flash.buf, 3));
    if (status != CY_U3P_SUCCESS) goto errreturn;
    ztex_flash_deselect();
    
    if ( ztex_flash.buf[2]<16 || ztex_flash.buf[2]>24 ) {
	ztex_log("Error: Invalid Flash size");
	return;
    }
    ztex_flash.vendor = ztex_flash.buf[0];
    ztex_flash.memtype = ztex_flash.buf[1];
    ztex_flash.sector_size = 0x8010;  		// only 64 KByte sectors are supported because erasing 4 KByte is extremely slow
    ztex_flash.sectors = 1 << (ztex_flash.buf[2]-16);
    ztex_flash.erase_cmd = 0xd8;
    ztex_flash.ec = 0;
    ztex_flash.enabled = 1;

    ZTEX_LOG("Info: Found %d MBit SPI Flash",ztex_flash.sectors>>1);
    return;
    
errreturn:
    ztex_flash.ec = FLASH_EC_RUNTIME;
    ztex_flash_deselect();
    ztex_log("Error: No SPI Flash found");
}

void ztex_usb_stop_flash() {
#ifdef ZTEX_SPI_DMA
    CyU3PDmaChannelReset (&ztex_spi_wr_handle);
    CyU3PDmaChannelReset (&ztex_spi_rd_handle);
#endif    
}
