/*%
   Default firmware and loader for ZTEX USB-FPGA Modules 2.18
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
#include "cyu3error.h"
#include "cyu3usb.h"
#include "cyu3spi.h"

// loads ztex header files and defeult configuration macros (fixed part of the SDK)
#include "ztex-conf.c"
#include "ztex-ufm-2_18.c"

#define OUT_ENDPOINT	2
#define IN_ENDPOINT	4
#define ZTEX_FPGA_CONF_FAST_EP	6

#define GPIO_RESET 25

#define GPIO_GPIO0 24
#define GPIO_GPIO1 23
#define GPIO_GPIO2 39
#define GPIO_GPIO3 38

#define GPIO_CLK 26
#define GPIO_DATA 28
#define GPIO_STOP 29

#undef ZTEX_PRODUCT_STRING 
#define ZTEX_PRODUCT_STRING "Default Firmware for ZTEX USB-FPGA Modules 2.18"


#include "ztex-default.c"

