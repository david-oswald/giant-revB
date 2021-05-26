/*%
   mmio -- Memory mapped I/O example for ZTEX USB-FPGA Module 2.16
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

#include[ztex-conf.h]	// Loads the configuration macros, see ztex-conf.h for the available macros
#include[ztex-utils.h]	// include basic functions

// configure endpoints 2 and 4, both belong to interface 0 (in/out are from the point of view of the host)
EP_CONFIG(2,0,BULK,IN,512,2);	 
EP_CONFIG(4,0,BULK,OUT,512,2);	 

// select ZTEX USB FPGA Module 2.16 as target (required for FPGA configuration)
IDENTITY_UFM_2_16(10.16.0.0,0);	 

// enables high speed FPGA configuration, (re)use EP 4
ENABLE_HS_FPGA_CONF(4);

// this product string is also used for identification by the host software
#define[PRODUCT_STRING]["mmio example for UFM 2.16"]

__xdata BYTE run;

#define[PRE_FPGA_RESET][PRE_FPGA_RESET
    run = 0;
]

#define[POST_FPGA_CONFIG][POST_FPGA_CONFIG
    REVCTL = 0x0;	// reset 
    SYNCDELAY; 

    IFCONFIG = bmBIT7;	        // internel 30MHz clock, drive IFCLK ouput, slave FIFO interface
    SYNCDELAY; 

    EP2CS &= ~bmBIT0;	// stall = 0
    SYNCDELAY; 
    EP4CS &= ~bmBIT0;	// stall = 0
    SYNCDELAY;		// first two packages are waste

    EP2FIFOCFG = 0;
    SYNCDELAY;
    EP4FIFOCFG = 0;
    SYNCDELAY;

    FIFORESET = 0x80;	// NAK-ALL
    SYNCDELAY;
    FIFORESET = 0x84;   // reset EP4
    SYNCDELAY;
    FIFORESET = 0x02;	// reset EP2, clear EP memory (no NAK bit)
    SYNCDELAY;
    FIFORESET = 0x00;	// release NAK-ALL
    SYNCDELAY;
    
    EP4BCL = 0x80;	// skip packet, (re)arm EP4
    SYNCDELAY;
    EP4BCL = 0x80;	// skip packet, (re)arm EP4

    run = 1;
]

// include the main part of the firmware kit, define the descriptors, ...
#include[ztex.h]


__xdata __at 0x5001 volatile BYTE OUT_REG;	// FPGA register where the data is written to
__xdata __at 0x5002 volatile BYTE IN_REG;	// FPGA register where the result is read from


void main(void)	
{
    WORD i,size;
    
// init everything
    init_USB();
    
    while (1) {	
	if ( run & !(EP4CS & bmBIT2) ) {	// EP4 is not empty
	    size = (EP4BCH << 8) | EP4BCL;
	    if ( size>0 && size<=512 && !(EP2CS & bmBIT3)) {	// EP2 is not full
		for ( i=0; i<size; i++ ) {
		    OUT_REG = EP4FIFOBUF[i];	// data from EP4 is converted to uppercase by the FPGA ...
		    EP2FIFOBUF[i] = IN_REG;	// ... and written back to EP2 buffer
		} 
		EP2BCH = size >> 8;
		SYNCDELAY; 
		EP2BCL = size & 255;		// arm EP2
		SYNCDELAY; 
		INPKTEND = 0x2;
	    }
	    SYNCDELAY; 
	    EP4BCL = 0x80;			// (re)arm EP4
	}
    }
}
