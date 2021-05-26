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
    Implementation of the Endpoint 0 functionality.
*/    

/* 
    return values of ztex_vendor_func:
     0     : success
     1..254: RTOS or API error (see cyu3error.h)
     255   : other error
*/    
uint8_t vendor_req_last = 0;				// last vendor request + 1
uint8_t vendor_cmd_last = 0;				// last vendor command + 1
uint8_t vendor_req_idx[ZTEX_VENDOR_REQ_MAX];		// indexes of vendor requests
uint8_t vendor_cmd_idx[ZTEX_VENDOR_CMD_MAX];    	// indexes of vendor commands
ztex_vendor_func vendor_req[ZTEX_VENDOR_REQ_MAX];	// indexes of vendor requests
ztex_vendor_func vendor_cmd[ZTEX_VENDOR_CMD_MAX];   	// indexes of vendor commands

void (*ztex_disable_flash_boot)() = 0;			// this is board specific and called to disable boot from flash 

uint8_t ztex_register_vendor_req(uint8_t idx, ztex_vendor_func f) {
    uint8_t i=0;
    while ( (i<vendor_req_last) && (vendor_req_idx[i]!=idx) ) i++;
    ZTEX_ASSERT_RET((i<ZTEX_VENDOR_REQ_MAX));
    vendor_req_idx[i]=idx;
    vendor_req[i]=f;
    if (i==vendor_req_last) vendor_req_last++;
    return 0;
}

uint8_t ztex_register_vendor_cmd(uint8_t idx, ztex_vendor_func f) {
    uint8_t i=0;
    while ( (i<vendor_cmd_last) && (vendor_cmd_idx[i]!=idx) ) i++;
    ZTEX_ASSERT_RET(i<ZTEX_VENDOR_CMD_MAX);
    vendor_cmd_idx[i]=idx;
    vendor_cmd[i]=f;
    if (i==vendor_cmd_last) vendor_cmd_last++;
    return 0;
}


uint8_t ztex_send_string_descriptor (char* str) {
    uint8_t l = 1;
    if ( str == NULL ) {
	ztex_ep0buf[0]=4;
	ztex_ep0buf[2]=0;
	ztex_ep0buf[3]=0;
    } else {
        l = strlen(str);
	ztex_ep0buf[0]=l*2+2;
	for ( uint8_t i = 0; i<l; i++ ) {
	    ztex_ep0buf[i*2+2] = str[i];
	    ztex_ep0buf[i*2+3] = 0;
	}
    }
    ztex_ep0buf[1]=CY_U3P_USB_STRING_DESCR;
    return CyU3PUsbSendEP0Data(l*2+2, ztex_ep0buf);
}

#define SEND_DESCR(d) ZTEX_REC(status=CyU3PUsbSendEP0Data (((uint8_t *)d)[0], (uint8_t *)d));
CyBool_t ztex_ep0_handler ( uint32_t setupdat0, uint32_t setupdat1)
{
    // Decode the fields from the setup request. 
    uint8_t bRequestType = (setupdat0 & CY_U3P_USB_REQUEST_TYPE_MASK);
    uint8_t bType = (bRequestType & CY_U3P_USB_TYPE_MASK);
    uint8_t bTarget  = (bRequestType & CY_U3P_USB_TARGET_MASK);
    uint8_t bRequest = ((setupdat0 & CY_U3P_USB_REQUEST_MASK) >> CY_U3P_USB_REQUEST_POS);
    uint16_t wValue   = ((setupdat0 & CY_U3P_USB_VALUE_MASK) >> CY_U3P_USB_VALUE_POS);
    uint16_t wIndex   = ((setupdat1 & CY_U3P_USB_INDEX_MASK) >> CY_U3P_USB_INDEX_POS);
    uint16_t wLength   = ((setupdat1 & CY_U3P_USB_LENGTH_MASK) >> CY_U3P_USB_LENGTH_POS);
    uint8_t isHandled = 0;
    uint8_t status = 0;

    // handle strings
    if ( bRequest==6 && (wValue>>8)==3 ) {  
	switch (wValue & 255) {
	    case 1: 
		ZTEX_REC(status = ztex_send_string_descriptor((char *)ztex_manufacturer_string));
                break;
            case 2:
		ZTEX_REC(status = ztex_send_string_descriptor((char *)ztex_product_string));
                break;
            case 3:
		ZTEX_REC(status = ztex_send_string_descriptor((char *)ztex_sn_string));
                break;
            default:
        	// 4..12 descriptions for interfaces 0..7
		ZTEX_REC(status = ztex_send_string_descriptor( (((wValue & 255) >= 4) && ((wValue & 255) < 12)) ? ztex_interface_string[(wValue & 255)-4] : NULL) );
                break;
	}
        isHandled = 1;
    }
    else if (bType == CY_U3P_USB_STANDARD_RQT) {
    
        if ( (bTarget == CY_U3P_USB_TARGET_INTF) && ((bRequest == CY_U3P_USB_SC_SET_FEATURE) || (bRequest == CY_U3P_USB_SC_CLEAR_FEATURE)) && (wValue==0) ) {
            if ( ztex_usb_is_connected )
                CyU3PUsbAckSetup();
            else
                CyU3PUsbStall(0, CyTrue, CyFalse);
            isHandled = 1;
        }

        /* Clear stall requests for endpoint is always passed to the setup callback.
         * It's handled by ztex_ep_cleanup_handler */
        if ( (bTarget == CY_U3P_USB_TARGET_ENDPT) && (bRequest == CY_U3P_USB_SC_CLEAR_FEATURE)  && (wValue == CY_U3P_USBX_FS_EP_HALT))
        {
            if ( ztex_usb_is_connected )
            {
        	ztex_ep_cleanup_handler(wIndex);
                CyU3PUsbStall (wIndex, CyFalse, CyTrue);
                CyU3PUsbAckSetup ();
                isHandled = CyTrue;
            }
        }

    }
    // Handle vendor requests.
    else if (bRequestType == 0xc0 ) {
        isHandled = 0;

        switch (bRequest)
        {
            case 0x22:		// send ZTEX descriptor 
        	SEND_DESCR(ztex_descriptor);
    		isHandled = 1;
                break;

            default:
        	for ( int i=0; i<vendor_req_last; i++ ) {
        	    if ( bRequest==vendor_req_idx[i] ) {
        		status = (vendor_req[i])(wValue, wIndex, wLength);
        		isHandled = 1;
        	    }
        	}
                // unknown request 
                break;
        }
    }
    // Handle vendor command
    else if (bRequestType == 0x40 ) {
        isHandled = 0;
        switch (bRequest)
        {
            case 0xA1:		// system reset
        	if ( (wValue == 0) && (ztex_disable_flash_boot != 0) ) ztex_disable_flash_boot();
        	CyU3PDeviceReset(CyFalse);
    		isHandled = 1;
                break;
            default:
        	for ( int i=0; i<vendor_cmd_last; i++ ) {
        	    if ( bRequest==vendor_cmd_idx[i] ) {
        		status = (vendor_cmd[i])(wValue, wIndex, wLength);
        		isHandled = 1;
        	    }
        	}
                // unknown request 
                break;
        }
    }
    return isHandled && (status == 0);
}
