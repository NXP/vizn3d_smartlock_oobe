/*! *********************************************************************************
 * \addtogroup BLE
 * @{
 ********************************************************************************* */
/*! *********************************************************************************
* Copyright 2016-2020 NXP
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
#include "EmbeddedTypes.h"
#include "MemManager.h"
#include "Panic.h"

#include "ble_general.h"
#include "gap_types.h"
#include "gap_interface.h"
#include "gatt_client_interface.h"
#include "gatt_server_interface.h"

#include "ble_service_discovery.h"
#include "ble_config.h"

#include "ApplMain.h"

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/
typedef struct servDiscInfo_tag
{
    /* Buffer used for Service Discovery */
    gattService_t   *mpServiceDiscoveryBuffer;

    /* Buffer used for Characteristic Discovery */
    gattCharacteristic_t *mpCharDiscoveryBuffer;

    /* Buffer used for Characteristic Descriptor Discovery */
    gattAttribute_t *mpCharDescriptorBuffer;

    /* Counters */
    uint8_t mCurrentServiceInDiscoveryIndex;
    uint8_t mCurrentCharInDiscoveryIndex;
    uint8_t mCurrentDescInDiscoveryIndex;
    uint8_t mcPrimaryServices;
    bool_t  mServDiscInProgress;

}servDiscInfo_t;

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
************************************************************************************/
#if (gMaxServicesCount_d > 0)
static void BleServDisc_Reset(deviceId_t peerDeviceId);
static void BleServDisc_Finished(deviceId_t peerDeviceId, bool_t result);
static void BleServDisc_NewService(deviceId_t peerDeviceId, gattService_t *pService);
#endif
/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
#if (gMaxServicesCount_d > 0)
static servDiscCallback_t pfServDiscCallback = NULL;
static servDiscInfo_t maServDiscInfo[gAppMaxConnections_c];
#endif
/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

void BleServDisc_RegisterCallback(servDiscCallback_t pServDiscCallback)
{
    #if (gMaxServicesCount_d > 0)
    pfServDiscCallback = pServDiscCallback;
    #else
    (void)pServDiscCallback;
    #endif
}

bleResult_t BleServDisc_Start(deviceId_t peerDeviceId)
{
    bleResult_t result = gBleSuccess_c;

#if (gMaxServicesCount_d > 0)
    if (!maServDiscInfo[peerDeviceId].mServDiscInProgress)
    {
        /* Allocate memory for Service Discovery */
        maServDiscInfo[peerDeviceId].mpServiceDiscoveryBuffer = MEM_BufferAlloc(sizeof(gattService_t) * (uint32_t)gMaxServicesCount_d);
        maServDiscInfo[peerDeviceId].mpCharDiscoveryBuffer = MEM_BufferAlloc(sizeof(gattCharacteristic_t) * (uint32_t)gMaxServiceCharCount_d);
        maServDiscInfo[peerDeviceId].mpCharDescriptorBuffer = MEM_BufferAlloc(sizeof(gattAttribute_t) * (uint32_t)gMaxCharDescriptorsCount_d);

        if (maServDiscInfo[peerDeviceId].mpServiceDiscoveryBuffer != NULL &&
            maServDiscInfo[peerDeviceId].mpCharDiscoveryBuffer != NULL &&
            maServDiscInfo[peerDeviceId].mpCharDescriptorBuffer != NULL)
        {
            maServDiscInfo[peerDeviceId].mServDiscInProgress = TRUE;

            /* Start Service Discovery*/
            result = GattClient_DiscoverAllPrimaryServices(
                                        peerDeviceId,
                                        maServDiscInfo[peerDeviceId].mpServiceDiscoveryBuffer,
                                        gMaxServicesCount_d,
                                        &maServDiscInfo[peerDeviceId].mcPrimaryServices);
        }
        else
        {
            BleServDisc_Reset(peerDeviceId);
            result = gBleOutOfMemory_c;
        }
    }
    else
    {
        result = gBleInvalidState_c;
    }
#endif
    return result;
}

bleResult_t BleServDisc_FindService(deviceId_t peerDeviceId, bleUuidType_t uuidType, bleUuid_t *pUuid)
{
    bleResult_t result;
#if (gMaxServicesCount_d > 0)
    result = gBleSuccess_c;
    if (!maServDiscInfo[peerDeviceId].mServDiscInProgress)
    {
        /* Allocate memory for Service Discovery */
        maServDiscInfo[peerDeviceId].mpServiceDiscoveryBuffer = MEM_BufferAlloc(sizeof(gattService_t));
        maServDiscInfo[peerDeviceId].mpCharDiscoveryBuffer = MEM_BufferAlloc(sizeof(gattCharacteristic_t) * (uint32_t)gMaxServiceCharCount_d);
        maServDiscInfo[peerDeviceId].mpCharDescriptorBuffer = MEM_BufferAlloc(sizeof(gattAttribute_t) * (uint32_t)gMaxCharDescriptorsCount_d);

        if (maServDiscInfo[peerDeviceId].mpServiceDiscoveryBuffer != NULL &&
            maServDiscInfo[peerDeviceId].mpCharDiscoveryBuffer != NULL &&
            maServDiscInfo[peerDeviceId].mpCharDescriptorBuffer != NULL)
        {
            maServDiscInfo[peerDeviceId].mServDiscInProgress = TRUE;

            /* Start Service Discovery*/
            result = GattClient_DiscoverPrimaryServicesByUuid(
                                                    peerDeviceId,
                                                    uuidType,
                                                    pUuid,
                                                    maServDiscInfo[peerDeviceId].mpServiceDiscoveryBuffer,
                                                    1,
                                                    &maServDiscInfo[peerDeviceId].mcPrimaryServices);
        }
        else
        {
            BleServDisc_Reset(peerDeviceId);
            result = gBleOutOfMemory_c;
        }
    }
    else
    {
        result = gBleInvalidState_c;
    }
#else
    result = gBleUnavailable_c;
#endif
    return result;
}

void BleServDisc_Stop(deviceId_t peerDeviceId)
{
#if (gMaxServicesCount_d > 0)
    if (maServDiscInfo[peerDeviceId].mServDiscInProgress)
    {
        maServDiscInfo[peerDeviceId].mServDiscInProgress = FALSE;
        BleServDisc_Reset(peerDeviceId);
    }
#endif
}

void BleServDisc_SignalGattClientEvent(
    deviceId_t              peerDeviceId,
    gattProcedureType_t     procedureType,
    gattProcedureResult_t   procedureResult,
    bleResult_t             error
)
{
#if (gMaxServicesCount_d > 0)
    servDiscInfo_t  *pInfo = &maServDiscInfo[peerDeviceId];
    if (pInfo->mServDiscInProgress)
    {
        if (procedureResult == gGattProcError_c)
        {
            BleServDisc_Finished(peerDeviceId, FALSE);
        }
        else
        {
            switch (procedureType)
            {
                case gGattProcDiscoverPrimaryServicesByUuid_c:      /* Fall through */
                case gGattProcDiscoverAllPrimaryServices_c:
                {
                    /* We found at least one service */
                    if (pInfo->mcPrimaryServices != 0U)
                    {
                        /* Start characteristic discovery with first service*/
                        pInfo->mCurrentServiceInDiscoveryIndex = 0;
                        pInfo->mCurrentCharInDiscoveryIndex = 0;
                        pInfo->mCurrentDescInDiscoveryIndex = 0;

                        pInfo->mpServiceDiscoveryBuffer->aCharacteristics = pInfo->mpCharDiscoveryBuffer;

                        /* Start Characteristic Discovery for current service */
                        (void)GattClient_DiscoverAllCharacteristicsOfService(
                                                    peerDeviceId,
                                                    pInfo->mpServiceDiscoveryBuffer,
                                                    gMaxServiceCharCount_d);
                    }
                    else
                    {
                        BleServDisc_Finished(peerDeviceId, TRUE);
                    }
                }
                break;

                case gGattProcDiscoverAllCharacteristicDescriptors_c:   /* Fall through */
                case gGattProcDiscoverAllCharacteristics_c:
                {
                    if (procedureType == gGattProcDiscoverAllCharacteristicDescriptors_c)
                    {
                        gattService_t        *pCurrentService = pInfo->mpServiceDiscoveryBuffer + pInfo->mCurrentServiceInDiscoveryIndex;
                        gattCharacteristic_t *pCurrentChar = pCurrentService->aCharacteristics + pInfo->mCurrentCharInDiscoveryIndex;

                        pInfo->mCurrentDescInDiscoveryIndex += pCurrentChar->cNumDescriptors;

                        /* Move on to the next characteristic */
                        pInfo->mCurrentCharInDiscoveryIndex++;
                    }

                    gattService_t        *pCurrentService = pInfo->mpServiceDiscoveryBuffer + pInfo->mCurrentServiceInDiscoveryIndex;

                    if (pInfo->mCurrentCharInDiscoveryIndex < pCurrentService->cNumCharacteristics)
                    {
                        gattCharacteristic_t *pCurrentChar = pCurrentService->aCharacteristics + pInfo->mCurrentCharInDiscoveryIndex;

                        /* Find next characteristic with descriptors*/
                        while (pInfo->mCurrentCharInDiscoveryIndex < pCurrentService->cNumCharacteristics - 1U)
                        {
                            /* Check if we have handles available between adjacent characteristics */
                            if (pCurrentChar->value.handle + 2U < (pCurrentChar + 1)->value.handle)
                            {
                                if (pInfo->mCurrentDescInDiscoveryIndex < (uint8_t)gMaxCharDescriptorsCount_d)
                                {
                                    pCurrentChar->aDescriptors = pInfo->mpCharDescriptorBuffer + pInfo->mCurrentDescInDiscoveryIndex;
                                    (void)GattClient_DiscoverAllCharacteristicDescriptors(peerDeviceId,
                                                            pCurrentChar,
                                                            (pCurrentChar + 1)->value.handle,
                                                            gMaxCharDescriptorsCount_d - pInfo->mCurrentDescInDiscoveryIndex);
                                    return;
                                }
                            }

                            pInfo->mCurrentCharInDiscoveryIndex++;
                            pCurrentChar = pCurrentService->aCharacteristics + pInfo->mCurrentCharInDiscoveryIndex;
                        }

                        /* Made it to the last characteristic. Check against service end handle*/
                        if (pCurrentChar->value.handle < pCurrentService->endHandle)
                        {
                            if (pInfo->mCurrentDescInDiscoveryIndex < gMaxCharDescriptorsCount_d)
                            {
                                pCurrentChar->aDescriptors = pInfo->mpCharDescriptorBuffer + pInfo->mCurrentDescInDiscoveryIndex;
                                (void)GattClient_DiscoverAllCharacteristicDescriptors(peerDeviceId,
                                                        pCurrentChar,
                                                        pCurrentService->endHandle,
                                                        gMaxCharDescriptorsCount_d - pInfo->mCurrentDescInDiscoveryIndex);
                                return;
                            }

                        }
                    }

                    /* Signal Discovery of Service */
                    BleServDisc_NewService(peerDeviceId, pCurrentService);

                    /* Move on to the next service */
                    pInfo->mCurrentServiceInDiscoveryIndex++;

                    /* Reset characteristic discovery */
                    pInfo->mCurrentCharInDiscoveryIndex = 0;
                    pInfo->mCurrentDescInDiscoveryIndex = 0;
                    FLib_MemSet(pInfo->mpCharDescriptorBuffer, 0, sizeof(gattAttribute_t) * (uint32_t)gMaxCharDescriptorsCount_d);
                    FLib_MemSet(pInfo->mpCharDiscoveryBuffer, 0, sizeof(gattCharacteristic_t) * (uint32_t)gMaxServiceCharCount_d);

                    if (pInfo->mCurrentServiceInDiscoveryIndex < pInfo->mcPrimaryServices)
                    {
                        /* Allocate memory for Char Discovery */
                        (pInfo->mpServiceDiscoveryBuffer + pInfo->mCurrentServiceInDiscoveryIndex)->aCharacteristics = pInfo->mpCharDiscoveryBuffer;

                         /* Start Characteristic Discovery for current service */
                        (void)GattClient_DiscoverAllCharacteristicsOfService(peerDeviceId,
                                pInfo->mpServiceDiscoveryBuffer + pInfo->mCurrentServiceInDiscoveryIndex,
                                                    gMaxServiceCharCount_d);
                    }
                    else
                    {
                        BleServDisc_Finished(peerDeviceId, TRUE);
                    }
                }
                break;

                default:
                {
                    ; /* No action required */
                }
                break;
            }
        }
    }
#endif
}

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/
#if (gMaxServicesCount_d > 0)
static void BleServDisc_Reset(deviceId_t peerDeviceId)
{
    if (maServDiscInfo[peerDeviceId].mpServiceDiscoveryBuffer != NULL)
    {
        (void)MEM_BufferFree(maServDiscInfo[peerDeviceId].mpServiceDiscoveryBuffer);
        maServDiscInfo[peerDeviceId].mpServiceDiscoveryBuffer = NULL;
    }

    if (maServDiscInfo[peerDeviceId].mpCharDiscoveryBuffer != NULL)
    {
        (void)MEM_BufferFree(maServDiscInfo[peerDeviceId].mpCharDiscoveryBuffer);
        maServDiscInfo[peerDeviceId].mpCharDiscoveryBuffer = NULL;
    }

    if (maServDiscInfo[peerDeviceId].mpCharDescriptorBuffer != NULL)
    {
        (void)MEM_BufferFree(maServDiscInfo[peerDeviceId].mpCharDescriptorBuffer);
        maServDiscInfo[peerDeviceId].mpCharDescriptorBuffer = NULL;
    }
}

static void BleServDisc_Finished(deviceId_t peerDeviceId, bool_t result)
{
    servDiscEvent_t event;

    BleServDisc_Stop(peerDeviceId);

    event.eventType = gDiscoveryFinished_c;
    event.eventData.success = result;
    pfServDiscCallback(peerDeviceId, &event);
}

static void BleServDisc_NewService(deviceId_t peerDeviceId, gattService_t *pService)
{
    servDiscEvent_t event;

    event.eventType = gServiceDiscovered_c;
    event.eventData.pService = pService;
    pfServDiscCallback(peerDeviceId, &event);
}
#endif
/*! *********************************************************************************
* @}
********************************************************************************** */
