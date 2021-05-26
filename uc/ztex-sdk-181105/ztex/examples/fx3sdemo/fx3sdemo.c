/*%
   fx3sdemo -- Demonstrates common features of the FX3S
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

// loads ztex header files and defeult configuration macros (fixed part of the SDK)
#include "ztex-conf.c"

#undef ZTEX_PRODUCT_STRING 
#define ZTEX_PRODUCT_STRING "Demo for ZTEX FX3S Boards"

//#define ZTEX_GPIO_SIMPLE_BITMAP1 ( 1 << (43-32) | 1 << (44-32) )

#define ENABLE_SD_FLASH


#include "ztex.c" 	

/*
 * Main function
 */
int main (void)
{
    // global configuration
//    ztex_app_thread_stack = 0x2000;	// stack size of application thread, default: 0x1000
//    ztex_app_thread_prio = 7;		// priority of application thread, should be 7..15, default: 8
//    ztex_allow_lpm = CyFalse;		// do not allow transition into low power mode, default: allowed
    
    ztex_main();  	// starts the OS and never returns
    return 0;		// makes the compiler happy
}
