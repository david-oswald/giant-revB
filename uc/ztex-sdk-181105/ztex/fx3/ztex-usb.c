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
    USB setup functions.
*/   
    
void ztex_usb_start_usb() {
    CyU3PEpConfig_t epCfg;
    CyU3PDmaChannelConfig_t dmaCfg;
    CyU3PUSBSpeed_t usbSpeed = CyU3PUsbGetSpeed();
    usbSpeed = usbSpeed; 				// makes the compiler happy if no endpoints are defined
    
    ZTEX_REC( CyU3PPibInit(CyTrue, ztex_pib_clock2) ); // init PIB
    
    CyU3PMemSet ((uint8_t *)&epCfg, 0, sizeof (epCfg));
    epCfg.enable = CyTrue;
    epCfg.streams = 0;

    CyU3PMemSet ((uint8_t *)&dmaCfg, 0, sizeof (dmaCfg));
    dmaCfg.dmaMode = CY_U3P_DMA_MODE_BYTE;

#define DIR_IN 128
#define DIR_OUT 0
#define TYPE_ISO 1
#define TYPE_BULK 2
#define TYPE_INT 3
#define IS_SUPER_SPEED ( usbSpeed == CY_U3P_SUPER_SPEED )
#define INTERFACE(num,body) body    
#define EP(num,dir,type,epsize,burst,interval,body) \
    switch (usbSpeed) { \
        case CY_U3P_FULL_SPEED: \
            epCfg.pcktSize = 64; \
            break; \
        case CY_U3P_HIGH_SPEED: \
            epCfg.pcktSize = ((TYPE_##type==2) && (epsize>512)) ? 512 : epsize; \
            break; \
        default: \
            epCfg.pcktSize = epsize; \
            break; \
    } \
    epCfg.epType = TYPE_##type; \
    epCfg.burstLen = usbSpeed == CY_U3P_SUPER_SPEED ? burst : 1; \
    dmaCfg.size = ((epCfg.pcktSize*epCfg.burstLen + 0x0F) & ~0x0F); \
    epCfg.isoPkts = 1; \
    ZTEX_REC(CyU3PSetEpConfig(DIR_##dir | num, &epCfg)); \
    ZTEX_REC(CyU3PUsbFlushEp(num)); \
    ZTEX_REC(CyU3PUsbStall (num, CyFalse, CyTrue)); \
    ZTEX_REC(CyU3PUsbResetEp(num)); \
    dmaCfg.prodSckId = DIR_##dir ? 0xffff : CY_U3P_UIB_SOCKET_PROD_##num; \
    dmaCfg.consSckId = DIR_##dir ? CY_U3P_UIB_SOCKET_CONS_##num : 0xffff; \
    dmaCfg.notification = 0; \
    dmaCfg.cb = 0; \
    body
#define DMA(handle,dmatype,dmasize,dmacount,socket,body) \
    if (dmaCfg.prodSckId==0xffff) dmaCfg.prodSckId=socket; \
    if (dmaCfg.consSckId==0xffff) dmaCfg.consSckId=socket; \
    dmaCfg.size = dmasize*1024; \
    dmaCfg.count = dmacount; \
    body \
    ZTEX_REC(CyU3PDmaChannelCreate (&handle, dmatype, &dmaCfg));
#define CB(cbfunc,not) \
    dmaCfg.notification = not; \
    dmaCfg.cb = cbfunc;
    EP_SETUP_ALL
#undef EP
#undef DMA
#undef CB
#undef IS_SUPER_SPEED
}


void ztex_usb_stop_usb() {
    CyU3PEpConfig_t epCfg;
    CyU3PMemSet ((uint8_t *)&epCfg, 0, sizeof (epCfg));

// first destry DMA channels
#define EP(num,dir,type,epsize,burst,interval,body) \
    body \
    ZTEX_REC(CyU3PUsbFlushEp(num)); \
    ZTEX_REC(CyU3PSetEpConfig(num, &epCfg)); 
#define DMA(handle,dmatype,dmasize,dmacount,socket,body) \
    ZTEX_REC(CyU3PDmaChannelDestroy(&handle)); 
    EP_SETUP_ALL;
#undef EP
#undef DMA
    ZTEX_REC( CyU3PPibDeInit() );
}
