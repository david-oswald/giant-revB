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
    Debugging and logging functions.
*/

#include "cyu3system.h"
#include "cyu3os.h"
#include "cyu3usb.h"

#define ZTEX_DEBUG_MSG_SIZE_MAX 	32
#define ZTEX_DEBUG_MSG_LIMIT 		ZTEX_DEBUG_MSG_SIZE_MAX  // maximum amount of messages between two get debug messages requests

#define ZTEX_LOG( args... ) { 				\
    uint8_t* buf = CyU3PMemAlloc(512); 			\
    CyU3PDebugStringPrint(buf, 512, args ); 		\
    ztex_debug_add(1, strlen((char*)buf), buf, 2);	\
    CyU3PMemFree(buf);					\
}

#define ZTEX_ASSERT( a ) { 				\
    if (!(a)) ztex_runtime_error( 65530, __FILE__, __LINE__ );	\
}

#define ZTEX_ASSERT_RET( a ) {				\
    if (!(a)) { 					\
	ztex_runtime_error( 65530, __FILE__, __LINE__ );	\
	return 255;					\
    }							\
}

#define ZTEX_REC(a) {					\
    uint16_t ec=(a);					\
    if ( ec != 0 ) { 					\
	ztex_runtime_error( ec, __FILE__, __LINE__ );	\
    }							\
}

#define ZTEX_REC_RET(a) {				\
    uint16_t ec=(a);					\
    if ( ec != 0 ) { 					\
	ztex_runtime_error( ec, __FILE__, __LINE__ );	\
	return 255;					\
    }							\
}

#define ZTEX_REC_CONT(a) {				\
    uint16_t ec=(a);					\
    if ( ec != 0 ) { 					\
	ztex_runtime_error( ec, __FILE__, __LINE__ );	\
	continue;					\
    }							\
}

uint8_t ztex_ec = 0;				// 1 if firmware halted due to fatal error
uint32_t debug_msg_last = 0; 			// index of last debug message + 1
uint16_t debug_msg_cnt = 0;			// number of stored messages
uint16_t debug_msg_limit = ZTEX_DEBUG_MSG_LIMIT;	// current message limit

uint8_t* debug_msg[ZTEX_DEBUG_MSG_SIZE_MAX];		// debug messages
uint16_t debug_msg_size[ZTEX_DEBUG_MSG_SIZE_MAX];  	// length of the message
uint8_t debug_msg_type[ZTEX_DEBUG_MSG_SIZE_MAX];  	// 1: string; 2: line info, other values: data
uint8_t debug_msg_action[ZTEX_DEBUG_MSG_SIZE_MAX]; 	// 0: do nothing, 1 free memory if overwritten, >1 copy + free           

void ztex_debug_add(uint8_t type, uint16_t size, uint8_t* buf, uint8_t action) {
    if (debug_msg_last >= debug_msg_limit ) {
	if ( action >= 1 ) CyU3PMemFree(buf);
	return;
    }
    uint16_t j = debug_msg_last % ZTEX_DEBUG_MSG_SIZE_MAX;
    if ( debug_msg_last >= ZTEX_DEBUG_MSG_SIZE_MAX ) {
	if ( debug_msg_action[j] >= 1 ) CyU3PMemFree(debug_msg[j]);
    }
    else {
     	debug_msg_cnt++;
    }
    debug_msg_last++;
     
    if ( action>1 ) {
        debug_msg[j] = CyU3PMemAlloc(size);
        CyU3PMemCopy(debug_msg[j], buf, size);
    }
    else {
	debug_msg[j]=buf;
    }
    debug_msg_size[j] = size;
    debug_msg_type[j] = type;
    debug_msg_action[j] = action;
}


uint8_t ztex_log ( const char* str ) {
    ztex_debug_add(1, strlen(str), (uint8_t*) str, 0);
    return(255);
}


void ztex_runtime_error ( uint16_t ec, const char* fn, uint16_t line ) {
    uint8_t size=strlen(fn)+4;
    uint8_t* buf=CyU3PMemAlloc(size);
    buf[0]=ec;
    buf[1]=ec>>8;
    buf[2]=line;
    buf[3]=line>>8;
    CyU3PMemCopy(&buf[4], (uint8_t*)fn, size-4);
    ztex_debug_add(2, size, buf, 1);
}

// VR 0x28
uint8_t ztex_vr_debug_send(uint16_t value, uint16_t index, uint16_t length ) {
    uint32_t idx = index | (value << 16);
    if ( length < 10 ) length = 10;
    if ( length > 4096 ) length = 4096;
    
    ztex_ep0buf[0]=ztex_ec;
    ztex_ep0buf[1]=debug_msg_last;
    ztex_ep0buf[2]=debug_msg_last>>8;
    ztex_ep0buf[3]=debug_msg_last>>16;
    ztex_ep0buf[4]=debug_msg_last>>24;
    ztex_ep0buf[5]=debug_msg_cnt;
    ztex_ep0buf[6]=debug_msg_cnt>>8;
    
    if ( (idx<debug_msg_last-debug_msg_cnt) || idx>=debug_msg_last) {
        ztex_ep0buf[7]=0;
        ztex_ep0buf[8]=0;
        ztex_ep0buf[9]=255;
        length=10;
    }
    else {
	uint16_t j = idx % ZTEX_DEBUG_MSG_SIZE_MAX;
	ztex_ep0buf[7]=debug_msg_size[j];
	ztex_ep0buf[8]=debug_msg_size[j]>>8;
	ztex_ep0buf[9]=debug_msg_type[j];
	for ( uint16_t i = 0; i < debug_msg_size[j]; i++ ) 
	    if ( length>i+10) ztex_ep0buf[i+10]=debug_msg[j][i];
	length=10+debug_msg_size[j];
    }

    idx+=ZTEX_DEBUG_MSG_LIMIT;
    if (idx>=debug_msg_limit) debug_msg_limit=idx;
    
    ZTEX_REC_RET( CyU3PUsbSendEP0Data( length, ztex_ep0buf ) );
    return 0;
}

// VR 0x29
uint8_t ztex_vr_usb3_errors(uint16_t value, uint16_t index, uint16_t length ) {
    uint16_t w;
    ztex_ep0buf[0] = w = ZTEX_USB3_SND_ERROR_COUNT;
    ztex_ep0buf[1] = w >> 8;
    ztex_ep0buf[2] = w = ZTEX_USB3_RCV_ERROR_COUNT;
    ztex_ep0buf[3] = w >> 8;
    ztex_ep0buf[3] = 0;			// reserved for future use
    ztex_ep0buf[4] = 0;			// reserved for future use
    ztex_ep0buf[5] = 0;			// reserved for future use
    ztex_ep0buf[6] = 0;			// reserved for future use
    ztex_ep0buf[7] = 0;			// reserved for future use
    if (length>8) length = 8;
    ZTEX_REC_RET( CyU3PUsbSendEP0Data( length, ztex_ep0buf ) );
    return 0;
}

uint8_t ztex_register_vendor_req(uint8_t idx, ztex_vendor_func f);
void ztex_debug_init () {
    ztex_register_vendor_req(0x28, ztex_vr_debug_send);
    ztex_register_vendor_req(0x29, ztex_vr_usb3_errors);
}
