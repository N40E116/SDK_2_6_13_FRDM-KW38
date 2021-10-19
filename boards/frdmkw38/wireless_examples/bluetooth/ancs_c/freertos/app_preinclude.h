/*! *********************************************************************************
 * \defgroup app
 * @{
 ********************************************************************************** */
/*!
 * Copyright 2019-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_PREINCLUDE_H_
#define _APP_PREINCLUDE_H_

/*!
 *  Application specific configuration file only
 *  Board Specific Configuration shall be added to board.h file directly such as :
 *  - Number of button on the board,
 *  - Number of LEDs,
 *  - etc...
 */
/*! *********************************************************************************
 *     Board Configuration
 ********************************************************************************** */
 /* Defines the number of required keys for the keyboard module */
#define gKBD_KeysCount_c        2

/* Specifies the number of required LEDs for the LED module */
#define gLEDsOnTargetBoardCnt_c 4

/* Specifies if the LED operation is inverted. LED On = GPIO Set */
#define gLED_InvertedMode_d     1


/*! *********************************************************************************
 *     App Configuration
 ********************************************************************************** */
/*! Enable/disable use of bonding capability */
#define gAppUseBonding_d        1

/*! Enable/disable use of pairing procedure */
#define gAppUsePairing_d        1

/*! Enable/disable use of privacy */
#define gAppUsePrivacy_d        0

#define gPasskeyValue_c         999999


/*! Maximum Number of Services that can be stored during Service Discovery */
#define gMaxServicesCount_d             10U

/*! Maximum Number of Characteristics per Service that can be stored during Service Discovery */
#define gMaxServiceCharCount_d          10U

/*! Maximum Number of Descriptors per Service that can be stored during Service Discovery */
#define gMaxCharDescriptorsCount_d      5U

/*! Repeated Attempts - Mitigation for pairing attacks */
#define gRepeatedAttempts_d             0
/*! *********************************************************************************
 *     Framework Configuration
 ********************************************************************************** */
/* enable NVM to be used as non volatile storage management by the host stack */
#define gAppUseNvm_d            1

/* Defines Rx Buffer Size for Serial Manager */
#define gSerialMgrRxBufSize_c   32

/* Defines Tx Queue Size for Serial Manager */
#define gSerialMgrTxQueueSize_c 5

/* Defines Size for Serial Manager Task*/
#define gSerialTaskStackSize_c  700

/* Defines pools by block size and number of blocks. Must be aligned to 4 bytes.*/
#define AppPoolsDetails_c \
         _block_size_  88  _number_of_blocks_   10 _eol_  \
         _block_size_ 248  _number_of_blocks_    3 _eol_  \
         _block_size_ 312  _number_of_blocks_    1 _eol_  \
         _block_size_ 400  _number_of_blocks_    3 _eol_

/* Defines number of timers needed by the application */
#if gRepeatedAttempts_d
#define gTmrApplicationTimers_c         6
#else
#define gTmrApplicationTimers_c         5
#endif

/* Set this define TRUE if the PIT frequency is an integer number of MHZ */
#define gTMR_PIT_FreqMultipleOfMHZ_d    0

/* Enables / Disables the precision timers platform component */
#define gTimestamp_Enabled_d            0

/* Enable/Disable Low Power Timer */
#define gTMR_EnableLowPowerTimers       0


/*! *********************************************************************************
 *     RTOS Configuration
 ********************************************************************************** */
/* Defines the RTOS used */
#define FSL_RTOS_FREE_RTOS      1

/* Defines number of OS events used */
#define osNumberOfEvents        5

/* Defines main task stack size */
#define gMainThreadStackSize_c  1024

/* Defines total heap size used by the OS */
#define gTotalHeapSize_c        9500


/*! *********************************************************************************
 *     BLE Stack Configuration
 ********************************************************************************** */


/*! *********************************************************************************
 *     BLE LL Configuration
 ***********************************************************************************/
/*  ble_ll_config.h file lists the parameters with their default values. User can override
 *    the parameter here by defining the parameter to a user defined value. */

/*
 * Specific configuration of LL pools by block size and number of blocks for this application.
 * Optimized using the MEM_OPTIMIZE_BUFFER_POOL feature in MemManager,
 * we find that the most optimized combination for LL buffers.
 *
 * If LlPoolsDetails_c is not defined, default LL buffer configuration in app_preinclude_common.h
 * will be applied.
 */

/* Include common configuration file and board configuration file */
#include "app_preinclude_common.h"
#endif /* _APP_PREINCLUDE_H_ */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
