/*! *********************************************************************************
 * \addtogroup BLE
 * @{
 ********************************************************************************** */
/*! *********************************************************************************
* Copyright 2016-2018 NXP
* All rights reserved.
*
* \file
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

/************************************************************************************
*************************************************************************************
* DO NOT MODIFY THIS FILE!
*************************************************************************************
************************************************************************************/

#ifndef _BLE_LL_CONFIG_H_
#define _BLE_LL_CONFIG_H_

/************************************************************************************
*************************************************************************************
* Public macros - Do not modify directly! Override in app_preinclude.h if needed.
*************************************************************************************
************************************************************************************/
/* The maximum value for some parameter supported by platform */
/* #if (defined(CPU_MK64FN1M0VLL12) || defined(CPU_MKW38A512VFP4) || defined(CPU_MKW38A512VFT4) || defined(CPU_MKW38A512VHT4)) */
    #define MAX_PLATFORM_SUPPORTED_ADV_SET_NB           2U
    #define MIN_PLATFORM_SUPPORTED_ADV_SET_NB           1U
    #define MAX_PLATFORM_SUPPORTED_EXT_ADV_DATA_LENGTH  1650U
    #define MAX_PLATFORM_SUPPORTED_LEG_ADV_DATA_LENGTH  31U
    #define MAX_PLATFORM_SUPPORTED_WL_SIZE              26U /* Whether periodic or other advertising (extended or legacy). */
    #define MIN_PLATFORM_SUPPORTED_CONNECTION_NBR       1U  /* Link Layer code not tested with only 1 connection context allocated.
                                                            Some code in LL is encapsulated between "#if (LL_MAX_NO_OF_CONNECTIONS_SUPPORTED > 1) 
                                                            #else #endif" statements, so be very cautious if you change MIN_PLATFORM_SUPPORTED_CONNECTION_NBR.*/
    #define MAX_PLATFORM_SUPPORTED_SYNC_ENGINES         2U
/* #endif */

/* Parameter related to BLE 5 features */
/* Extended Advertising */
#ifndef gLlMaxUsedAdvSet_c
#define gLlMaxUsedAdvSet_c             MAX_PLATFORM_SUPPORTED_ADV_SET_NB
#endif

#ifndef gLlMaxExtAdvDataLength_c
#define gLlMaxExtAdvDataLength_c       MAX_PLATFORM_SUPPORTED_EXT_ADV_DATA_LENGTH
#endif

/* Periodic Advertising */
#ifndef gLlUsePeriodicAdvertising_d
#define gLlUsePeriodicAdvertising_d    1
#endif

/* Channel Selection Algorithm 2 */
#ifndef gLlUseChanSelAlgo2_d
#define gLlUseChanSelAlgo2_d    1
#endif
                                                              
/* Number of default max TX acl packets in internal queue */
#ifndef gLlBufferNbrTxAclPkts
#ifdef SOFT_SCALE_EN
#define  gLlBufferNbrTxAclPkts      16
#else
#define  gLlBufferNbrTxAclPkts      8
#endif
#endif
                                                              
/* Number of default max RX acl packets in HCI queue*/
#ifndef gLlBufferNbrRxAclPkts
#define  gLlBufferNbrRxAclPkts      4
#endif

/* Number of duplicate filtering entries */
#ifndef gLlScanDuplicateFiltListSize_c
#define gLlScanDuplicateFiltListSize_c (16)
#endif

#ifndef gLlScanPeriodicAdvertiserListSize_c
#define gLlScanPeriodicAdvertiserListSize_c (0U)
#endif

#ifndef gLlScanAdvertiserListSize_c
#define gLlScanAdvertiserListSize_c (MAX_PLATFORM_SUPPORTED_WL_SIZE)
#endif
                                                                                                                            
#if gLlUsePeriodicAdvertising_d
#ifndef gLlMaxUsedSyncEngine_c
#define gLlMaxUsedSyncEngine_c  MAX_PLATFORM_SUPPORTED_SYNC_ENGINES
#endif /* gLlMaxUsedSyncEngine_c */
#else /* gLlUsePeriodicAdvertising_d */
#define gLlMaxUsedSyncEngine_c  0                                                       
#endif /* gLlUsePeriodicAdvertising_d */
                                                              
#ifndef gLlInvalidPduHandlingType_c
/* 0: ignore PDU, 1: disconnect link */
#define gLlInvalidPduHandlingType_c 0U
#endif                                                        

/* First level of check (at compilation time) to make sure parameter does not exceed 
the maximum values supported by the platform.
There is a second level of check (at run time in function defined in ble_ll_globals.c)
to ensure parameters does not exceed the maximum value configured in the link layer library. */

#if (gLlMaxUsedAdvSet_c > MAX_PLATFORM_SUPPORTED_ADV_SET_NB)
#error "The number of advertising set configured by the application exceeds the number of advertising set supported by this platform."
#endif

#if (gLlMaxUsedAdvSet_c < MIN_PLATFORM_SUPPORTED_ADV_SET_NB)
#error "The number of advertising set configured by the application does not meet the minimal number of advertising set."
#endif

#if (gLlMaxExtAdvDataLength_c > MAX_PLATFORM_SUPPORTED_EXT_ADV_DATA_LENGTH)
#error "The extended advertising length configured by the application exceeds the extended advertising length supported by this platform."
#endif

#if (gLlMaxExtAdvDataLength_c < MAX_PLATFORM_SUPPORTED_LEG_ADV_DATA_LENGTH)
#error "The extended advertising length configured by the application does not meet the minimal extended advertising length."
#endif

#if (gAppMaxConnections_c < MIN_PLATFORM_SUPPORTED_CONNECTION_NBR)
#error "The connection number configured by the application is not in the range supported by this platform."
#endif

#if ((gLlScanAdvertiserListSize_c+gLlScanPeriodicAdvertiserListSize_c) > MAX_PLATFORM_SUPPORTED_WL_SIZE)
#error "The sum of periodic and advertiser list size configured by the application is not in the range supported by this platform."
#endif
                                                              
#if (gLlMaxUsedSyncEngine_c > MAX_PLATFORM_SUPPORTED_SYNC_ENGINES)
#error "The number of sync engines configured by the application exceeds the number of sync engines supported by this platform."
#endif

#endif /* _BLE_LL_CONFIG_H_ */

/*! *********************************************************************************
* @}
********************************************************************************** */
