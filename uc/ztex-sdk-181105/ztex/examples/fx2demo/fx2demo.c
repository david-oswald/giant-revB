/*%
   fx2demo -- Demonstrates common features of the FX2
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

// define endpoints 2 and 4, both belong to interface 0 (in/out are from the point of view of the host)
EP_CONFIG(2,0,BULK,IN,512,2);	 
EP_CONFIG(4,0,BULK,OUT,512,2);	 

// thin initializes the debug helper with a 32 messages stack and 4 bytes per message
ENABLE_DEBUG(32,4);

// this product string is also used for identification by the host software
#define[PRODUCT_STRING]["fx2demo for EZ-USB devices"]

// include the main part of the firmware kit, define the descriptors, ...
#include[ztex.h]

void main(void)	
{
    WORD i,size,j;
    BYTE b;

// init everything
    init_USB();

    REVCTL = 0x0;
    SYNCDELAY; 
    
    IFCONFIG = bmBIT7;	// Internal source, 48MHz
    SYNCDELAY; 

    EP2CS &= ~bmBIT0;	// stall = 0
    SYNCDELAY; 
    EP4CS &= ~bmBIT0;	// stall = 0
    SYNCDELAY; 

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

    EP4BCL = 0x80;	// skip package, (re)arm EP4
    SYNCDELAY;
    EP4BCL = 0x80;	// skip package, (re)arm EP4
    SYNCDELAY;

    while (1) {	
	if ( !(EP4CS & bmBIT2) ) {				// EP4 is not empty
	    size = (EP4BCH << 8) | EP4BCL;
	    if ( size>0 && size<=512 && !(EP2CS & bmBIT3)) {	// EP2 is not full
		j = 0;
		for ( i=0; i<size; i++ ) {
		    b = EP4FIFOBUF[i];		// data from EP4 ... 
		    if ( b>=(BYTE)'a' && b<=(BYTE)'z' )	{	// ... is converted to uppercase ...
			b-=32;
			j++;
		    }
		    EP2FIFOBUF[i] = b;		// ... and written back to EP2 buffer
		} 

		debug_msg_buf[0] = size;	// write statistics to the debug buffer
		debug_msg_buf[1] = size >> 8;
		debug_msg_buf[2] = j;
		debug_msg_buf[3] = j >> 8;
		debug_add_msg();
		
		EP2BCH = size >> 8;
		SYNCDELAY; 
		EP2BCL = size & 255;		// arm EP2
	    }
	    SYNCDELAY; 
	    EP4BCL = 0x80;			// skip package, (re)arm EP4
	}
    }
}


