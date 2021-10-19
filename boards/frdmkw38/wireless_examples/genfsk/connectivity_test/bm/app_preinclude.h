/*! *********************************************************************************
 * \defgroup app
 * @{
 ********************************************************************************** */
/*
* Copyright 2019 NXP
* All rights reserved.
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef _APP_PREINCLUDE_H_
#define _APP_PREINCLUDE_H_

/*! *********************************************************************************
 * 	Drivers Configuration
 ********************************************************************************** */
 /* Defines the number of available keys for the keyboard module for this application */
#define gKBD_KeysCount_c        2
 
 /* Defines the number of available tsi electrodes for this application */
#define gKBD_TsiElectdCount_c   0

/* Specifies the number of physical LEDs on the target board */
#define gLEDsOnTargetBoardCnt_c 4

/* Specifies if the LED operation is inverted. LED On = GPIO Set */
#define gLED_InvertedMode_d     1

/* Enable Generic FSK protocol in XCVR */
#define XCVR_GENFSK_ENABLED     1

/* Enable FRDM Shield board (antenna switch) support */
#define gBoard_Antenna_Diversity_Support_d    0

/*! *********************************************************************************
 * 	Framework Configuration
 ********************************************************************************** */
/* Defines Num of Serial Manager interfaces */
#define gSerialManagerMaxInterfaces_c   1

/* Defines Size for Timer Task*/
#define gTmrTaskStackSize_c  384

/* Defines pools by block size and number of blocks. Must be aligned to 4 bytes.*/
#define PoolsDetails_c \
         _block_size_  32  _number_of_blocks_    6 _eol_  \
         _block_size_  64  _number_of_blocks_    3 _eol_  \
         _block_size_ 128  _number_of_blocks_    4 _eol_  \
         _block_size_ 512  _number_of_blocks_    4 _eol_

/* Defines number of timers needed by the application */
#define gTmrApplicationTimers_c         1

/* Defines number of timers needed by the protocol stack */
#define gTmrStackTimers_c               3
             
/* Enable/Disable Low Power Timer */
#define gTMR_EnableLowPowerTimers       1

/* Enable/Disable PowerDown functionality in PwrLib */
#define cPWR_UsePowerDownMode           0

/* Enable/Disable GENFSK Link Layer DSM */
#define cPWR_GENFSK_LL_Enable           1
             
/* Default Deep Sleep Mode*/
#define cPWR_DeepSleepMode              7

/* Disable all pins when entering Low Power */           
#define APP_DISABLE_PINS_IN_LOW_POWER   1
           
/* Default deep sleep duration in ms */ 
#define cPWR_DeepSleepDurationMs        30000
             
#if defined(gBoard_Antenna_Diversity_Support_d) && (gBoard_Antenna_Diversity_Support_d > 0)
    /* Enables / Disables the DCDC platform component */
    #define gDCDC_Enabled_d             1

    /* Default DCDC Mode used by the application gDCDC_Buck_c or gDCDC_Bypass_c */
    #define APP_DCDC_MODE               gDCDC_Buck_c

    /* Configure GPIOs with 3.3V output, required for RF swicthers */
    #define gDCDC_3p3V_Output_d         1

    /* Specifies the number of physical LEDs on the target board */
    #define gFrdmShield_Gpio_Cnt_c      3
#else
    /* Enables / Disables the DCDC platform component */
    #define gDCDC_Enabled_d             0   

    /* Default DCDC Mode used by the application */           
    #define APP_DCDC_MODE               gDCDC_Mode_Buck_c
#endif

/* Default DCDC Battery Level Monitor interval */
#define APP_DCDC_VBAT_MONITOR_INTERVAL  600000  
           
/*! *********************************************************************************
 * 	RTOS Configuration
 ********************************************************************************** */
/* Defines number of OS events used */
#define osNumberOfEvents        4

/* Defines number of OS semaphores used */
#define osNumberOfSemaphores    1
             
/*! *********************************************************************************
 * 	Auto Configuration
 ********************************************************************************** */
/* Disable LEDs when enabling low power */           
#if cPWR_UsePowerDownMode || gMWS_UseCoexistence_d
#define gLEDSupported_d 0           
#endif
           
#if gMWS_UseCoexistence_d   
#undef gKBD_KeysCount_c
#define gKBD_KeysCount_c        1
#endif

/* Include common configuration file and board configuration file */
#include "app_preinclude_common.h"

#endif /* _APP_PREINCLUDE_H_ */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
