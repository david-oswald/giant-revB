/*%
   Default firmware and loader for ZTEX USB-FPGA Modules 2.04
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

// select ZTEX USB FPGA Module 2.04 as target  (required for FPGA configuration)
IDENTITY_UFM_2_04(10.19.0.0,0);	 

// this product string is also used for identification by the host software
#define[PRODUCT_STRING]["USB-FPGA Module 2.04 default FW"]

#define[D_RESET][IOA7]

#define[GPIO_CLK][IOC1]
#define[GPIO_DIR][IOC2]				// 1: µC reads
#define[GPIO_DAT][IOC3]
#define[GPIO_RE][{OEC = OEC & ~bmBIT3;}]	// seen from µC
#define[GPIO_WE][{OEC = OEC | bmBIT3;}]		// seen from µC

#define[LSI_MISO][IOC4]
#define[LSI_MOSI][IOC5]
#define[LSI_CLK][IOC6]
#define[LSI_STOP][IOC7]

#define[OEA_MASK][bmBIT7] 
#define[OEA_UMASK][0]

#define[OEC_MASK][(bmBIT1 | bmBIT2 | bmBIT5 | bmBIT6 | bmBIT7)] 
#define[OEC_UMASK][(bmBIT4)] 

#define[OEE_MASK][0] 
#define[OEE_UMASK][0] 

#include[ztex-default.h]			// template for default firmware
