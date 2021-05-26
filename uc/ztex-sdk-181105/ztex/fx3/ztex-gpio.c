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
    GPIO functions.
*/    

#include <cyu3gpio.h>

#ifndef _ZTEX_GPIO_C_
#define _ZTEX_GPIO_C_

uint8_t ztex_gpio_set_output(uint8_t num, CyBool_t val) {
    CyU3PGpioSimpleConfig_t cfg;

    cfg.outValue    = val;
    cfg.driveLowEn  = CyTrue;
    cfg.driveHighEn = CyTrue;
    cfg.inputEn     = CyFalse;
    cfg.intrMode    = CY_U3P_GPIO_NO_INTR;
    ZTEX_REC_RET( CyU3PGpioSetSimpleConfig( num, &cfg) );
    return 0;
}

void ztex_gpio_set (uint8_t num, CyBool_t val) {
    CyU3PReturnStatus_t status;
    status = CyU3PGpioSimpleSetValue(num, val);
    if ( status )  ZTEX_LOG("Error setting GPIO %d: %d", num, status);
}

CyBool_t ztex_gpio_get (uint8_t num) {
    CyBool_t val;
    CyU3PReturnStatus_t status;
    status = CyU3PGpioSimpleGetValue(num, &val);
    if ( status )  ZTEX_LOG("Error reading GPIO %d: %d", num, status);
    return val;
}

uint8_t ztex_gpio_set_open_drain(uint8_t num, CyBool_t val) {
    CyU3PGpioSimpleConfig_t cfg;

    cfg.outValue    = val;
    cfg.driveLowEn  = CyTrue;
    cfg.driveHighEn = CyFalse;
    cfg.inputEn     = CyTrue;
    cfg.intrMode    = CY_U3P_GPIO_NO_INTR;
    ZTEX_REC_RET( CyU3PGpioSetSimpleConfig( num, &cfg) );
    return 0;
}

uint8_t ztex_gpio_set_open_source(uint8_t num, CyBool_t val) {
    CyU3PGpioSimpleConfig_t cfg;

    cfg.outValue    = val;
    cfg.driveLowEn  = CyFalse;
    cfg.driveHighEn = CyTrue;
    cfg.inputEn     = CyTrue;
    cfg.intrMode    = CY_U3P_GPIO_NO_INTR;
    ZTEX_REC_RET( CyU3PGpioSetSimpleConfig( num, &cfg) );
    return 0;
}

uint8_t ztex_gpio_set_input(uint8_t num) {
    CyU3PGpioSimpleConfig_t cfg;

    cfg.outValue    = CyFalse;
    cfg.driveLowEn  = CyFalse;
    cfg.driveHighEn = CyFalse;
    cfg.inputEn     = CyTrue;
    cfg.intrMode    = CY_U3P_GPIO_NO_INTR;
    ZTEX_REC_RET( CyU3PGpioSetSimpleConfig( num, &cfg) );
    return 0;
}

void ztex_gpio_init() {
    CyU3PGpioClock_t cfg;

    cfg.fastClkDiv = 2;
    cfg.slowClkDiv = 0;
    cfg.simpleDiv  = CY_U3P_GPIO_SIMPLE_DIV_BY_2;
    cfg.clkSrc     = CY_U3P_SYS_CLK;
    cfg.halfDiv    = 0;

    ZTEX_REC( CyU3PGpioInit (&cfg, GPIO_INT_HANDLER) );
}

#endif // _ZTEX_GPIO_C_
