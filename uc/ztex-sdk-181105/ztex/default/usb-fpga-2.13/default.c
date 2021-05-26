/*%
   Default firmware and loader for ZTEX USB-FPGA Modules 2.13
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
#include[ztex-utils.h]	// include basic functions and variables

// select ZTEX USB FPGA Module 2.13 as target  (required for FPGA configuration)
IDENTITY_UFM_2_13(10.17.0.0,0);	 

// this product string is also used for identification by the host software
#define[PRODUCT_STRING]["USB-FPGA Module 2.13 default FW"]

#define[D_RESET][IOA7]

#define[GPIO_CLK][IOA0]
#define[GPIO_DIR][IOA1]				// 1: µC reads
#define[GPIO_DAT][IOA3]
#define[GPIO_RE][{OEA = OEA & ~bmBIT3;}]	// seen from µC
#define[GPIO_WE][{OEA = OEA | bmBIT3;}]		// seen from µC

#define[LSI_MISO][IOC0]
#define[LSI_MOSI][IOC1]
#define[LSI_CLK][IOC2]
#define[LSI_STOP][IOC3]

#define[OEA_MASK][(bmBIT0 | bmBIT1 | bmBIT7)] 
#define[OEA_UMASK][0]

#define[OEC_MASK][(bmBIT1 | bmBIT2 | bmBIT3)] 
#define[OEC_UMASK][(bmBIT0)] 

#define[OEE_MASK][0] 
#define[OEE_UMASK][0] 

#include[ztex-default.h]			// template for default firmware
