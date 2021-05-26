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
    Defines the GPIF-II waveform for FPGA configuration.
*/    

#ifndef _ZTEX_FPGACONF1_C_
#define _ZTEX_FPGACONF1_C_

#include "cyu3types.h"
#include "cyu3gpif.h"

#define ZTEX_FPGACONF1_SOCKET_ID CY_U3P_PIB_SOCKET_10

//Transition function values used in the state machine.
uint16_t ztex_fpgaconf1_gpif_transition[]  = {
    0x0000, 0xAAAA, 0xFFFF
};

/* Table containing the transition information for various states. 
   This table has to be stored in the WAVEFORM Registers.
   This array consists of non-replicated waveform descriptors and acts as a 
   waveform table. */
CyU3PGpifWaveData ztex_fpgaconf1_gpif_wavedata[]  = {
    {{0x1E739A01,0x040000C0,0x80000000},{0x00000000,0x00000000,0x00000000}},
    {{0x2E739C02,0x04400040,0x80000000},{0x00000000,0x00000000,0x00000000}},
    {{0x1E739A03,0x040000C0,0x80000000},{0x00000000,0x00000000,0x00000000}}
};

// Table that maps state indexes to the descriptor table indexes.
uint8_t ztex_fpgaconf1_gpif_wavedata_position[]  = {
    0,1,2,1
};

// GPIF II configuration register values.
uint32_t ztex_fpgaconf1_gpif_reg_value[]  = {
    0x800083B0,  /*  CY_U3P_PIB_GPIF_CONFIG */
    0x00000003,  /*  CY_U3P_PIB_GPIF_BUS_CONFIG */
    0x00000000,  /*  CY_U3P_PIB_GPIF_BUS_CONFIG2 */
    0x00000046,  /*  CY_U3P_PIB_GPIF_AD_CONFIG */
    0x00000000,  /*  CY_U3P_PIB_GPIF_STATUS */
    0x00000000,  /*  CY_U3P_PIB_GPIF_INTR */
    0x00000000,  /*  CY_U3P_PIB_GPIF_INTR_MASK */
    0x00000082,  /*  CY_U3P_PIB_GPIF_SERIAL_IN_CONFIG */
    0x00000782,  /*  CY_U3P_PIB_GPIF_SERIAL_OUT_CONFIG */
    0x00100000,  /*  CY_U3P_PIB_GPIF_CTRL_BUS_DIRECTION */
    0x0000FFFF,  /*  CY_U3P_PIB_GPIF_CTRL_BUS_DEFAULT */
    0x00000000,  /*  CY_U3P_PIB_GPIF_CTRL_BUS_POLARITY */
    0x00000000,  /*  CY_U3P_PIB_GPIF_CTRL_BUS_TOGGLE */
    0x00000000,  /*  CY_U3P_PIB_GPIF_CTRL_BUS_SELECT */
    0x00000000,  /*  CY_U3P_PIB_GPIF_CTRL_BUS_SELECT */
    0x00000000,  /*  CY_U3P_PIB_GPIF_CTRL_BUS_SELECT */
    0x00000000,  /*  CY_U3P_PIB_GPIF_CTRL_BUS_SELECT */
    0x00000000,  /*  CY_U3P_PIB_GPIF_CTRL_BUS_SELECT */
    0x00000000,  /*  CY_U3P_PIB_GPIF_CTRL_BUS_SELECT */
    0x00000000,  /*  CY_U3P_PIB_GPIF_CTRL_BUS_SELECT */
    0x00000000,  /*  CY_U3P_PIB_GPIF_CTRL_BUS_SELECT */
    0x00000000,  /*  CY_U3P_PIB_GPIF_CTRL_BUS_SELECT */
    0x00000000,  /*  CY_U3P_PIB_GPIF_CTRL_BUS_SELECT */
    0x00000008,  /*  CY_U3P_PIB_GPIF_CTRL_BUS_SELECT */
    0x00000000,  /*  CY_U3P_PIB_GPIF_CTRL_BUS_SELECT */
    0x00000000,  /*  CY_U3P_PIB_GPIF_CTRL_BUS_SELECT */
    0x00000000,  /*  CY_U3P_PIB_GPIF_CTRL_BUS_SELECT */
    0x00000000,  /*  CY_U3P_PIB_GPIF_CTRL_BUS_SELECT */
    0x00000000,  /*  CY_U3P_PIB_GPIF_CTRL_BUS_SELECT */
    0x00000006,  /*  CY_U3P_PIB_GPIF_CTRL_COUNT_CONFIG */
    0x00000000,  /*  CY_U3P_PIB_GPIF_CTRL_COUNT_RESET */
    0x0000FFFF,  /*  CY_U3P_PIB_GPIF_CTRL_COUNT_LIMIT */
    0x0000010A,  /*  CY_U3P_PIB_GPIF_ADDR_COUNT_CONFIG */
    0x00000000,  /*  CY_U3P_PIB_GPIF_ADDR_COUNT_RESET */
    0x0000FFFF,  /*  CY_U3P_PIB_GPIF_ADDR_COUNT_LIMIT */
    0x00000000,  /*  CY_U3P_PIB_GPIF_STATE_COUNT_CONFIG */
    0x0000FFFF,  /*  CY_U3P_PIB_GPIF_STATE_COUNT_LIMIT */
    0x0000010A,  /*  CY_U3P_PIB_GPIF_DATA_COUNT_CONFIG */
    0x00000000,  /*  CY_U3P_PIB_GPIF_DATA_COUNT_RESET */
    0x0000FFFF,  /*  CY_U3P_PIB_GPIF_DATA_COUNT_LIMIT */
    0x00000000,  /*  CY_U3P_PIB_GPIF_CTRL_COMP_VALUE */
    0x00000000,  /*  CY_U3P_PIB_GPIF_CTRL_COMP_MASK */
    0x00000000,  /*  CY_U3P_PIB_GPIF_DATA_COMP_VALUE */
    0x00000000,  /*  CY_U3P_PIB_GPIF_DATA_COMP_MASK */
    0x00000000,  /*  CY_U3P_PIB_GPIF_ADDR_COMP_VALUE */
    0x00000000,  /*  CY_U3P_PIB_GPIF_ADDR_COMP_MASK */
    0x00000000,  /*  CY_U3P_PIB_GPIF_DATA_CTRL */
    0x00000000,  /*  CY_U3P_PIB_GPIF_INGRESS_DATA */
    0x00000000,  /*  CY_U3P_PIB_GPIF_INGRESS_DATA */
    0x00000000,  /*  CY_U3P_PIB_GPIF_INGRESS_DATA */
    0x00000000,  /*  CY_U3P_PIB_GPIF_INGRESS_DATA */
    0x00000000,  /*  CY_U3P_PIB_GPIF_EGRESS_DATA */
    0x00000000,  /*  CY_U3P_PIB_GPIF_EGRESS_DATA */
    0x00000000,  /*  CY_U3P_PIB_GPIF_EGRESS_DATA */
    0x00000000,  /*  CY_U3P_PIB_GPIF_EGRESS_DATA */
    0x00000000,  /*  CY_U3P_PIB_GPIF_INGRESS_ADDRESS */
    0x00000000,  /*  CY_U3P_PIB_GPIF_INGRESS_ADDRESS */
    0x00000000,  /*  CY_U3P_PIB_GPIF_INGRESS_ADDRESS */
    0x00000000,  /*  CY_U3P_PIB_GPIF_INGRESS_ADDRESS */
    0x00000000,  /*  CY_U3P_PIB_GPIF_EGRESS_ADDRESS */
    0x00000000,  /*  CY_U3P_PIB_GPIF_EGRESS_ADDRESS */
    0x00000000,  /*  CY_U3P_PIB_GPIF_EGRESS_ADDRESS */
    0x00000000,  /*  CY_U3P_PIB_GPIF_EGRESS_ADDRESS */
    0x80010400,  /*  CY_U3P_PIB_GPIF_THREAD_CONFIG */
    0x80010401,  /*  CY_U3P_PIB_GPIF_THREAD_CONFIG */
    0x80010402,  /*  CY_U3P_PIB_GPIF_THREAD_CONFIG */
    0x80010403,  /*  CY_U3P_PIB_GPIF_THREAD_CONFIG */
    0x00000000,  /*  CY_U3P_PIB_GPIF_LAMBDA_STAT */
    0x00000000,  /*  CY_U3P_PIB_GPIF_ALPHA_STAT */
    0x00000000,  /*  CY_U3P_PIB_GPIF_BETA_STAT */
    0x00000000,  /*  CY_U3P_PIB_GPIF_WAVEFORM_CTRL_STAT */
    0x00000000,  /*  CY_U3P_PIB_GPIF_WAVEFORM_SWITCH */
    0x00000000,  /*  CY_U3P_PIB_GPIF_WAVEFORM_SWITCH_TIMEOUT */
    0x00000000,  /*  CY_U3P_PIB_GPIF_CRC_CONFIG */
    0x00000000,  /*  CY_U3P_PIB_GPIF_CRC_DATA */
    0xFFFFFFC1  /*  CY_U3P_PIB_GPIF_BETA_DEASSERT */
};


CyU3PPibClock_t ztex_fpgaconf1_pib_clock = {
    .clkDiv = 8,		// approx. 52 MHz
    .clkSrc = CY_U3P_SYS_CLK,
    .isHalfDiv = CyFalse,
    .isDllEnable = CyTrue
};

const CyU3PGpifConfig_t ztex_fpgaconf1_gpif_data  = {
    (uint16_t)(sizeof(ztex_fpgaconf1_gpif_wavedata_position)/sizeof(uint8_t)),
    ztex_fpgaconf1_gpif_wavedata,
    ztex_fpgaconf1_gpif_wavedata_position,
    (uint16_t)(sizeof(ztex_fpgaconf1_gpif_transition)/sizeof(uint16_t)),
    ztex_fpgaconf1_gpif_transition,
    (uint16_t)(sizeof(ztex_fpgaconf1_gpif_reg_value)/sizeof(uint32_t)),
    ztex_fpgaconf1_gpif_reg_value
};

CyU3PDmaChannel ztex_fpgaconf1_handle;
CyU3PDmaChannel* ztex_fpgaconf1_handle_p = NULL;

/* *********************************************************************
   ***** ztex_fpgaconf1_start *****************************************
   ********************************************************************* */
uint8_t ztex_fpgaconf1_start(CyU3PDmaSocketId_t socket) {
    
    if ( ztex_fpgaconf1_handle_p == NULL && socket==0) {
	// create dma channel    
        CyU3PDmaChannelConfig_t dmaConfig;
    
	CyU3PMemSet ((uint8_t *)&dmaConfig, 0, sizeof(dmaConfig));
	dmaConfig.size           = 4096;
	dmaConfig.count          = 0;
	dmaConfig.prodAvailCount = 0;
	dmaConfig.dmaMode        = CY_U3P_DMA_MODE_BYTE;
	dmaConfig.prodHeader     = 0;
	dmaConfig.prodFooter     = 0;
	dmaConfig.consHeader     = 0;
	dmaConfig.consSckId      = ZTEX_FPGACONF1_SOCKET_ID;
	dmaConfig.prodSckId      = CY_U3P_CPU_SOCKET_PROD;
	dmaConfig.notification   = 0;
	dmaConfig.cb             = NULL;

	ZTEX_REC( CyU3PDmaChannelCreate (&ztex_fpgaconf1_handle, CY_U3P_DMA_TYPE_MANUAL_OUT, &dmaConfig) );
	ztex_fpgaconf1_handle_p = &ztex_fpgaconf1_handle;
    }

    ZTEX_REC_RET( CyU3PGpifLoad ( &ztex_fpgaconf1_gpif_data ) );
    ZTEX_REC_RET( CyU3PGpifSocketConfigure(1, socket > 0 ? socket : ZTEX_FPGACONF1_SOCKET_ID, 2, CyFalse, 1) );

    ZTEX_REC_RET( CyU3PGpifSMStart (0, 0) );

    return 0;
}    


/* *********************************************************************
   ***** ztex_fpgaconf1_stop *******************************************
   ********************************************************************* */
uint8_t ztex_fpgaconf1_stop() {
    CyU3PGpifDisable(CyTrue);
    if ( ztex_fpgaconf1_handle_p != NULL ) {
        ZTEX_REC( CyU3PDmaChannelDestroy (ztex_fpgaconf1_handle_p) );
        ztex_fpgaconf1_handle_p = NULL;
    }
    return 0;
}    

/* *********************************************************************
   ***** ztex_fpgaconf1_send *******************************************
   ********************************************************************* */
uint8_t ztex_fpgaconf1_send (uint8_t* buf, uint32_t size) {
    CyU3PDmaBuffer_t buf_p;

    buf_p.size  = size;
    buf_p.count = size;
    buf_p.buffer = buf;
    buf_p.status = 0;

    ZTEX_REC_RET( CyU3PDmaChannelSetupSendBuffer (&ztex_fpgaconf1_handle, &buf_p) );

    ZTEX_REC_RET( CyU3PDmaChannelWaitForCompletion (&ztex_fpgaconf1_handle, 500) );

    return 0;
}  

#endif // _ZTEX_FPGACONF1_C_

