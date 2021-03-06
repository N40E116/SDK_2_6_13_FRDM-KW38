/*! *********************************************************************************
* \addtogroup Health Thermometer Service
* @{
 ********************************************************************************** */
/*! *********************************************************************************
* Copyright (c) 2014, Freescale Semiconductor, Inc.
* Copyright 2016-2019 NXP
* All rights reserved.
*
* \file
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
#include "FunctionLib.h"

#include "ble_general.h"
#include "gatt_db_app_interface.h"
#include "gatt_server_interface.h"
#include "gap_interface.h"

#include "current_time_interface.h"
#include "health_thermometer_interface.h"
/************************************************************************************
*************************************************************************************
* Private constants & macros
*************************************************************************************
************************************************************************************/

/***********************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
********************************************************************************** */

/***********************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
********************************************************************************** */
/*! Health Thermometer Service - Subscribed Client */
static deviceId_t mHts_ClientDeviceId = gInvalidDeviceId_c;

/***********************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
********************************************************************************** */
static bleResult_t Hts_UpdateTempMeasurementCharacteristic
(
 uint16_t handle,
 htsMeasurement_t *pMeasurement
);

static void Hts_SendTempMeasurementIndication
(
  uint16_t handle
);

static void Hts_SendIntermediateTempNotification
(
  uint16_t handle
);

static bleResult_t Hts_SetTemperatureType
(
 uint16_t serviceHandle,
 htsTempType_t tempType
);

/***********************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
********************************************************************************** */

bleResult_t Hts_Start (htsConfig_t *pServiceConfig)
{
    bleResult_t result;

    mHts_ClientDeviceId = gInvalidDeviceId_c;

    /* Set temperature type characteristic */
    result = Hts_SetTemperatureType(pServiceConfig->serviceHandle, pServiceConfig->tempType);

    if (result == gBleSuccess_c)
    {
        /* Set measurement interval */
        result = Hts_SetMeasurementInterval(pServiceConfig, pServiceConfig->measurementInterval);
    }
    return result;
}

bleResult_t Hts_Stop (htsConfig_t *pServiceConfig)
{
    return Hts_Unsubscribe();
}

bleResult_t Hts_Subscribe(deviceId_t clientdeviceId)
{
    mHts_ClientDeviceId = clientdeviceId;
    return gBleSuccess_c;
}

bleResult_t Hts_Unsubscribe(void)
{
    mHts_ClientDeviceId = gInvalidDeviceId_c;
    return gBleSuccess_c;
}

bleResult_t Hts_RecordTemperatureMeasurement (uint16_t serviceHandle, htsMeasurement_t *pMeasurement)
{
    uint16_t  handle;
    bleResult_t result;
    bleUuid_t uuid = {.uuid16 = gBleSig_TemperatureMeasurement_d};

    /* Get handle of characteristic */
    result = GattDb_FindCharValueHandleInService(serviceHandle,
        gBleUuidType16_c, &uuid, &handle);

    if (result == gBleSuccess_c)
    {
        /* Update characteristic value and send indication */
        if (gBleSuccess_c == Hts_UpdateTempMeasurementCharacteristic(handle, pMeasurement))
        {
            Hts_SendTempMeasurementIndication(handle);
        }
    }

    return result;
}

bleResult_t Hts_RecordIntermediateTemperature(uint16_t serviceHandle, htsMeasurement_t *pMeasurement)
{
    uint16_t  handle;
    bleResult_t result;
    bleUuid_t uuid = {.uuid16 = gBleSig_IntermediateTemperature_d};

    /* Get handle of characteristic */
    result = GattDb_FindCharValueHandleInService(serviceHandle,
        gBleUuidType16_c, &uuid, &handle);

    if (result == gBleSuccess_c)
    {
        /* Update characteristic value and send indication */
        if (gBleSuccess_c == Hts_UpdateTempMeasurementCharacteristic(handle, pMeasurement))
        {
            Hts_SendIntermediateTempNotification(handle);
        }
    }

    return result;
}

bleResult_t Hts_SetMeasurementInterval(htsConfig_t *pServiceConfig, uint16_t measurementInterval)
{
    uint16_t  handle;
    bleResult_t result;
    bleUuid_t uuid = {.uuid16 = gBleSig_MeasurementInterval_d};

    /* Get handle of characteristic */
    result = GattDb_FindCharValueHandleInService(pServiceConfig->serviceHandle,
        gBleUuidType16_c, &uuid, &handle);

    if (result == gBleSuccess_c)
    {
        /* Update characteristic value */
        result = GattDb_WriteAttribute(handle, sizeof(uint16_t), (uint8_t*)&measurementInterval);

        if (result == gBleSuccess_c)
        {
            /* Indicate the current value of the characteristic the next time there is a connection */
            if (mHts_ClientDeviceId != gInvalidDeviceId_c)
            {
                pServiceConfig->pUserData->measIntervalIndPending = TRUE;
            }
            else
            {
                Hts_SendMeasurementIntervalIndication(handle);
            }
        }
    }

    return result;
}

bleResult_t Hts_StoreTemperatureMeasurement(htsUserData_t *pUserData, htsMeasurement_t *pMeasurement)
{
#if gHts_EnableStoredMeasurements_d
    /* Store measurement */
    FLib_MemCpy(&pUserData->pStoredMeasurements[pUserData->measurementCursor],
                pMeasurement,
                sizeof(htsMeasurement_t));

    /* Update cursor and count */
    pUserData->measurementCursor = (pUserData->measurementCursor + 1U) % gHts_MaxNumOfStoredMeasurements_c;
    pUserData->cMeasurements = MIN(pUserData->cMeasurements + 1U, gHts_MaxNumOfStoredMeasurements_c);
#endif
    return gBleSuccess_c;
}

void Hts_SendStoredTemperatureMeasurement(htsConfig_t *pServiceConfig)
{
#if gHts_EnableStoredMeasurements_d
    uint8_t oldestMeasurementIdx;

    if (pServiceConfig->pUserData->cMeasurements != 0U)
    {
        oldestMeasurementIdx = (pServiceConfig->pUserData->measurementCursor + (gHts_MaxNumOfStoredMeasurements_c + pServiceConfig->pUserData->cMeasurements)) % gHts_MaxNumOfStoredMeasurements_c;

        (void)Hts_RecordTemperatureMeasurement(pServiceConfig->serviceHandle,
                                           &pServiceConfig->pUserData->pStoredMeasurements[oldestMeasurementIdx]);

        /* Update measurement count */
        pServiceConfig->pUserData->cMeasurements -= 1U;
    }
#endif
}

attErrorCode_t Hts_MeasurementIntervalWriting(htsConfig_t *pServiceConfig, uint16_t handle, uint16_t newMeasurementInterval)
{
    bleUuid_t uuid = Uuid16(gBleSig_ValidRangeDescriptor_d);
    uint16_t  descHandle, minValue, maxValue, descValueLength;
    uint8_t aRangeValues[4];
    attErrorCode_t result = gAttErrCodeNoError_c;

    /* Find range descriptor*/
    (void)GattDb_FindDescriptorHandleForCharValueHandle(handle,
                                                  gBleUuidType16_c,
                                                  &uuid,
                                                  &descHandle);
    /* Read range descriptor */
    (void)GattDb_ReadAttribute(descHandle, (uint16_t)sizeof(aRangeValues), &aRangeValues[0], &descValueLength);

    minValue = Utils_ExtractTwoByteValue(&aRangeValues[0]);
    maxValue = Utils_ExtractTwoByteValue(&aRangeValues[2]);

    if ((newMeasurementInterval != 0U) &&
        ((newMeasurementInterval < minValue) || (newMeasurementInterval > maxValue)))
    {
        result = (attErrorCode_t)gAttAppErrCodeOutOfRange_c;
    }
    else
    {
        /* Write new value in database*/
        if (gGattDbSuccess_c != Hts_SetMeasurementInterval(pServiceConfig, newMeasurementInterval))
        {
            result = gAttErrCodeUnlikelyError_c;
        }
    }

    return result;
}

void Hts_SendMeasurementIntervalIndication
(
  uint16_t handle
)
{
    uint16_t  hCccd;
    bool_t isIndicationActive;

    /* Get handle of CCCD */
    if (GattDb_FindCccdHandleForCharValueHandle(handle, &hCccd) == gBleSuccess_c)
    {
        if (gBleSuccess_c == Gap_CheckIndicationStatus
            (mHts_ClientDeviceId, hCccd, &isIndicationActive) &&
            TRUE == isIndicationActive)
        {
            (void)GattServer_SendIndication(mHts_ClientDeviceId, handle);
        }
    }
}

/***********************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/
static bleResult_t Hts_UpdateTempMeasurementCharacteristic
(
 uint16_t handle,
 htsMeasurement_t *pMeasurement
)
{
    uint8_t charValue[14];
    uint8_t index = 0;

    /* Add flags */
    charValue[0] = pMeasurement->unit;

    index += 1U;

    /* Add measurements */
    FLib_MemCpy(&charValue[index], &pMeasurement->temperature, sizeof(ieee11073_32BitFloat_t));
    index += sizeof(ieee11073_32BitFloat_t);

    /* Add time stamp */
    if (pMeasurement->timeStampPresent)
    {
        charValue[0] |= gHts_TimeStampPresent_c;
        FLib_MemCpy(&charValue[index], &pMeasurement->timeStamp, sizeof(ctsDateTime_t));
        index += sizeof(ctsDateTime_t);
    }

    /* Add temperature type */
    if (pMeasurement->tempTypePresent)
    {
        charValue[0] |= gHts_TempTypePresent_c;
        charValue[index] = pMeasurement->tempType;
        index += sizeof(htsTempType_t);
    }

    return GattDb_WriteAttribute(handle, index, &charValue[0]);
}


static void Hts_SendTempMeasurementIndication
(
  uint16_t handle
)
{
    uint16_t  hCccd;
    bool_t isIndicationActive;

    /* Get handle of CCCD */
    if (GattDb_FindCccdHandleForCharValueHandle(handle, &hCccd) == gBleSuccess_c)
    {
        if (gBleSuccess_c == Gap_CheckIndicationStatus
            (mHts_ClientDeviceId, hCccd, &isIndicationActive) &&
            TRUE == isIndicationActive)
        {
            (void)GattServer_SendIndication(mHts_ClientDeviceId, handle);
        }
    }
}

static void Hts_SendIntermediateTempNotification
(
  uint16_t handle
)
{
    uint16_t  hCccd;
    bool_t isNotificationActive;

    /* Get handle of CCCD */
    if (GattDb_FindCccdHandleForCharValueHandle(handle, &hCccd) == gBleSuccess_c)
    {
        if (gBleSuccess_c == Gap_CheckNotificationStatus
            (mHts_ClientDeviceId, hCccd, &isNotificationActive) &&
            TRUE == isNotificationActive)
        {
            (void)GattServer_SendNotification(mHts_ClientDeviceId, handle);
        }
    }
}

static bleResult_t Hts_SetTemperatureType
(
 uint16_t serviceHandle,
 htsTempType_t tempType
)
{
    uint16_t  hValue;
    bleUuid_t uuid = {.uuid16 = gBleSig_TemperatureType_d};
    bleResult_t result;

    result = GattDb_FindCharValueHandleInService(serviceHandle,
       gBleUuidType16_c, &uuid, &hValue);

    if(result == gBleSuccess_c)
    {
        result = GattDb_WriteAttribute(hValue, sizeof(htsTempType_t), &tempType);
    }

    return result;
}


/*! *********************************************************************************
* @}
********************************************************************************** */
