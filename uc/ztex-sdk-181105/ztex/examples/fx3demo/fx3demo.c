/*%
   fx3demo -- Demonstrates common features of the FX3
   Copyright (C) 2009-2017 ZTEX GmbH.
   http://www.ztex.de

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
%*/
#include "cyu3system.h"
#include "cyu3os.h"
#include "cyu3dma.h"
#include "math.h"

// loads default configuration macros
#include "ztex-conf.c"

CyU3PDmaChannel dma_out_handle, dma_in_handle, dma_st_out_handle;

/* 
  Define endpoints 1, 2 and 4 which belong to interface 0 (in/out are seen from the host)
  EP 1 is used for the speed test, EP 2 and 4 for the uppercase conversion.
  Bulk size is 1 for EP 2 and 4 and 16 for EP 1.
  DMA buffer size if 2x1K for EP 2 and 4 and 2x32K for EP 1
*/  
#undef EP_SETUP
#define EP_SETUP \
    INTERFACE(0, \
	EP_BULK(4, OUT, 1, /* direction as seen from the host */ \
	    DMA(dma_in_handle, CY_U3P_DMA_TYPE_MANUAL_IN, 1, 2, CY_U3P_CPU_SOCKET_CONS, /* direction as seen from device */ \
		CB(0,0) \
	    ) \
	) \
	EP_BULK(2, IN, 1, \
	    DMA(dma_out_handle, CY_U3P_DMA_TYPE_MANUAL_OUT, 1, 2, CY_U3P_CPU_SOCKET_PROD, ) \
	) \
	EP_BULK(1, IN, 16, \
	    DMA(dma_st_out_handle, CY_U3P_DMA_TYPE_MANUAL_OUT, 32, 2, CY_U3P_CPU_SOCKET_PROD, ) \
	) \
    )

#undef ZTEX_PRODUCT_STRING 
#define ZTEX_PRODUCT_STRING "Demo for FX3 Boards"

#include "ztex.c" 	

void usb_start() {
    // start USB transfers as soon cable is connected
    ZTEX_REC(CyU3PDmaChannelSetXfer (&dma_in_handle, 0));
    ZTEX_REC(CyU3PDmaChannelSetXfer (&dma_out_handle, 0));
    ZTEX_REC(CyU3PDmaChannelSetXfer (&dma_st_out_handle, 0));
}

void usb_stop() {
    // nothing required here
}

void run () {
    CyU3PDmaBuffer_t inbuf, outbuf;
    CyU3PReturnStatus_t status;
    uint32_t i;

    ztex_log ( "Starting FX3 Demo" );

    while (1) {
        if (ztex_usb_is_connected) {
    	    /* Check for free buffer on the speed test EP. The call will fail if there was
    	     * an error or if the USB connection was reset / disconnected. In case of error,
    	     * invoke the error handler and in case of reset / disconnection, continue to 
    	     * beginning of the loop. If no free buffer is available timeout error occurs
             * and firmware continues with the upper case conversion test */
           status = CyU3PDmaChannelGetBuffer (&dma_st_out_handle, &outbuf, 50);
           if ( !ztex_usb_is_connected ) continue;
           if ( status != CY_U3P_ERROR_TIMEOUT ) {
    	        ZTEX_REC_CONT(status);
        	// Commit the buffer to the speed test EP 
        	status = CyU3PDmaChannelCommitBuffer (&dma_st_out_handle, outbuf.size, 0);
		//ZTEX_LOG("EC=%d,  ST: %d bytes", status, outbuf.size);
    		if (!ztex_usb_is_connected) continue;
        	ZTEX_REC(status);
    	    }
            else {
        	/* Uppercase conversion test: Wait for receiving a buffer from the producer socket
                 * (OUT endpoint). */
        	status = CyU3PDmaChannelGetBuffer (&dma_in_handle, &inbuf, CYU3P_NO_WAIT );
		// ZTEX_LOG("EC=%d,  Read %d bytes", status, inbuf.count);
        	if (!ztex_usb_is_connected || status==CY_U3P_ERROR_TIMEOUT) continue;
        	ZTEX_REC_CONT(status);

        	/* Wait for a free buffer to transmit the received data. */
        	status = CyU3PDmaChannelGetBuffer (&dma_out_handle, &outbuf, CYU3P_WAIT_FOREVER);
        	if (!ztex_usb_is_connected || status==CY_U3P_ERROR_TIMEOUT) continue;
        	ZTEX_REC(status);

        	/* Convert the data from the producer channel to the consumer channel. 
                 * The inbuf.count holds the amount of valid data received. */
        	CyU3PMemCopy (outbuf.buffer, inbuf.buffer, inbuf.count);
        	for (i=0; i<inbuf.count; i++ )
        	    outbuf.buffer[i] = inbuf.buffer[i]>='a' && inbuf.buffer[i]<='z' ? inbuf.buffer[i] - 32 : inbuf.buffer[i];
        	 i = round(sqrt(inbuf.count)*10.0);
		 ZTEX_LOG("Info: Converted %d.%d^2 bytes", i/10, i%10);

        	/* Now discard the data from the producer channel so that the buffer is made available
                 * to receive more data. */
        	status = CyU3PDmaChannelDiscardBuffer (&dma_in_handle);
        	if (!ztex_usb_is_connected) continue;
        	ZTEX_REC(status);

        	/* Commit the received data to the consumer pipe so that the data can be
        	 * transmitted back to the USB host. Since the same data is sent back, the
        	 * count shall be same as received and the status field of the call shall
                 * be 0 for default use case. */
        	status = CyU3PDmaChannelCommitBuffer (&dma_out_handle, inbuf.count, 0);
		// ZTEX_LOG("EC=%d,  Sent %d bytes", status, outbuf.status);
        	if (!ztex_usb_is_connected) continue;
        	ZTEX_REC(status);
    	    }
        }
        else {
            /* No active data transfer. Sleep for a small amount of time. */
    	    CyU3PThreadSleep (100);
        }
    }
    
}

/*
 * Main function
 */
int main (void)
{
    // global configuration
//    ztex_app_thread_stack = 0x2000;	// stack size of application thread, default: 0x1000
//    ztex_app_thread_prio = 7;		// priority of application thread, should be 7..15, default: 8
//    ztex_allow_lpm = CyFalse;		// do not allow transition into low power mode, default: allowed

    ztex_app_thread_run = run;
    ztex_usb_start = usb_start;
    ztex_usb_stop = usb_stop;
    
    ztex_main();  	// starts the OS and never returns
    return 0;		// makes the compiler happy
}
