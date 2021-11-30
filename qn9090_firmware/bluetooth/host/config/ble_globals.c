/*! *********************************************************************************
* \addtogroup BLE
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2020 NXP
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

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
#include "ble_general.h"
#include "att_interface.h"
#include "ble_config.h"
#include "gap_types.h"
#include "Messaging.h"
#include "ModuleInfo.h"
#include "gatt_types.h"

/************************************************************************************
*************************************************************************************
* Private constants & macros
*************************************************************************************
************************************************************************************/
/* WARNING: Do not change these defines */
#define gAttConnStorageSize_c            (6U)
#define gActiveDevicesStorageSize_c      (88U)
#define gL2caLeCbChannelEntrySize_c      (40U)
#define gL2caLePsmEntrySize_c            (4U)

/* This enables/disables the HCI Reset command sent by the Host at init sequence
   Default value is disabled, only for gUseHciTransportDownward_d is required */
#ifndef gHostInitResetController_c
    #if gUseHciTransportDownward_d || defined CPU_JN518X
        #define gHostInitResetController_c TRUE
    #else
        #define gHostInitResetController_c FALSE
    #endif
#endif

/************************************************************************************
*************************************************************************************
* Public memory declarations - external references from Host library
*************************************************************************************
************************************************************************************/
/*! App to Host message queue for the Host Task */
extern  msgQueue_t   gApp2Host_TaskQueue;
/*! HCI to Host message queue for the Host Task */
extern msgQueue_t   gHci2Host_TaskQueue;
/*! Event for the Host Task Queue */
extern osaEventId_t gHost_TaskEvent;

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
#if (gMaxBondedDevices_c > 0)
static bleBondIdentityHeaderBlob_t gaBondIdentityHeaderBlobs[gMaxBondedDevices_c];
#endif
#if (gMaxResolvingListSize_c > 0)
static gapIdentityInformation_t mControllerPrivacyIdentities[gMaxResolvingListSize_c];
static gapCarSupport_t mCAR_Support[gMaxResolvingListSize_c];
#endif
static uint16_t gGattWriteNotificationHandles[gMaxWriteNotificationHandles_c];
static uint16_t gGattReadNotificationHandles[gMaxReadNotificationHandles_c];
static gapServiceSecurityRequirements_t gaServiceSecurityRequirements[gGapMaxServiceSpecificSecurityRequirements_c];

/* Queued Writes server globals, supporting gAppMaxConnections_c clients */
#if (gAppMaxConnections_c > 0)
static uint8_t gaGattDbPrepareWriteQueueIndexes[gAppMaxConnections_c];
static attPrepareQueueItem *gPrepareWriteQueues[gAppMaxConnections_c][gPrepareWriteQueueSize_c];
#endif

/* The following definitions are required by the VERSION_TAGS. DO NOT MODIFY or REMOVE */
extern const moduleInfo_t BLE_HOST_VERSION;
#if defined ( __IAR_SYSTEMS_ICC__ )
#pragma required=BLE_HOST_VERSION /* force the linker to keep the symbol in the current compilation unit */
uint8_t ble_dummy; /* symbol suppressed by the linker as it is unused in the compilation unit, but necessary because
                             to avoid warnings related to #pragma required */
#elif defined(__GNUC__)
static const moduleInfo_t *const dummy __attribute__((__used__)) = &BLE_HOST_VERSION;
#endif /* __IAR_SYSTEMS_ICC__ */

/* BLE Host connection storage */
#if (gAppMaxConnections_c > 0)
static uint32_t gAttConnStorage[(gAttConnStorageSize_c * gAppMaxConnections_c + 3) / 4];
static uint32_t gActiveDevicesStorage[(gActiveDevicesStorageSize_c * gAppMaxConnections_c + 3) / 4];

#if (gMaxBondedDevices_c > 0)
/* Service changed indication buffer */
gattHandleRange_t gServiceChangedIndicationStorage[gMaxBondedDevices_c];
#endif

/* LE credit-based channels storage */
static uint32_t gL2caPsmStorage[(gL2caLePsmEntrySize_c * gL2caMaxLePsmSupported_c + 3) / 4];
static uint32_t gL2caCbChannelStorage[(gL2caLeCbChannelEntrySize_c * gL2caMaxLeCbChannels_c + 3) / 4];
#endif

static bleHostGlobalConfig_t hostGlobalConfig = {
    .gapGlobalConfig = {
        .gapMaximumBondedDevicesField = gMaxBondedDevices_c,
        .gapGapMaximumSavedCccdsField = gcGapMaximumSavedCccds_c,
        .gapControllerResolvingListSizeField = gMaxResolvingListSize_c,
		.gapGapMaxServiceSpecificSecurityRequirementsField = gGapMaxServiceSpecificSecurityRequirements_c,
		.pServiceSecurityRequirementsField = (uint8_t *) &gaServiceSecurityRequirements[0],
#if (gMaxBondedDevices_c > 0)
        .pBondIdentityHeaderBlobsField = &gaBondIdentityHeaderBlobs[0],
#else
        .pBondIdentityHeaderBlobsField = NULL,
#endif
#if (gMaxResolvingListSize_c > 0)
        .pControllerPrivacyIdentitiesField = (uint8_t *) &mControllerPrivacyIdentities[0],
        .pCAR_SupportField = (uint8_t *) &mCAR_Support[0],
#else
        .pControllerPrivacyIdentitiesField = (uint8_t *) NULL,
        .pCAR_SupportField = (uint8_t *)NULL,
#endif
        .gapDefaultTxOctetsField = gBleDefaultTxOctets_c,
        .gapDefaultTxTimeField = gBleDefaultTxTime_c,
        .gapHostPrivacyTimeoutField = gBleHostPrivacyTimeout_c,
        .gapControllerPrivacyTimeoutField = gBleControllerPrivacyTimeout_c,
        .gapLeSecureConnectionsOnlyModeField = gBleLeSecureConnectionsOnlyMode_c,
        .gapLeScOobHasMitmProtectionField =gBleLeScOobHasMitmProtection_c,
        .maxAdvReportQueueSize = gMaxAdvReportQueueSize_c,
        .gapSimultaneousEAChainedReportsField = gGapSimultaneousEAChainedReports_c,
        .gapFreeEAReportMsField = gBleHostFreeEAReportTimeoutMs_c,
    },
    .gattGlobalConfig = {
        .gcGattServerMtuField = gAttMaxMtu_c,
        .gattMaxHandleCountForWriteNotificationsField = gMaxWriteNotificationHandles_c,
        .pGattWriteNotificationHandlesField = &gGattWriteNotificationHandles[0],
        .gattMaxHandleCountForReadNotificationsField = gMaxReadNotificationHandles_c,
        .pGattReadNotificationHandlesField = &gGattReadNotificationHandles[0],
        /* Queued Writes server globals, supporting gAppMaxConnections_c clients */
        .gattDbMaxPrepareWriteOperationsInQueueField = gPrepareWriteQueueSize_c,
        .gattDbMaxPrepareWriteClientsField = gAppMaxConnections_c,
#if (gAppMaxConnections_c > 0)
        .pGattDbPrepareWriteQueueIndexesField = &gaGattDbPrepareWriteQueueIndexes[0],
        .ppPrepareWriteQueuesField = (uint8_t **) &gPrepareWriteQueues[0],
#else
        .pGattDbPrepareWriteQueueIndexesField = NULL,
        .ppPrepareWriteQueuesField = (uint8_t **) NULL,
#endif
#if (gMaxBondedDevices_c > 0)
        .pGattServiceChangedIndicationStorageField = (uint32_t* )&gServiceChangedIndicationStorage[0],
#else
        .pGattServiceChangedIndicationStorageField = (uint32_t* ) NULL,
#endif
    },
    /* BLE Host connection storage */
    .connStorageGlobalConfig = {
        .bleMaxActiveConnectionsField = gAppMaxConnections_c,
#if (gAppMaxConnections_c > 0)
        .pAttConnStorageField = &gAttConnStorage[0],
        .pActiveDevicesStorageField = &gActiveDevicesStorage[0],
#else
        .pAttConnStorageField = NULL,
        .pActiveDevicesStorageField = NULL,
#endif
    },
    /* LE credit-based channels storage */
    .l2caGlobalConfig = {
        .l2caMaxLePsmSupportedField = gL2caMaxLePsmSupported_c,
        .l2caMaxLeCbChannelsField = gL2caMaxLeCbChannels_c,
#if (gAppMaxConnections_c > 0)
        .pL2caPsmStorageField = &gL2caPsmStorage[0],
        .pL2caCbChannelStorageField = &gL2caCbChannelStorage[0],
#else
        .pL2caPsmStorageField = NULL,
        .pL2caCbChannelStorageField = NULL,
#endif
        .maxL2caQueueSizeField = gMaxL2caQueueSize_c,
    },
    .hostGlobalControllerConfig = {
        .hostInitResetControllerField = gHostInitResetController_c,
    },
    .hostGlobalHostTaskConfig = {
        .pApp2Host_TaskQueue = (uint8_t *) &gApp2Host_TaskQueue,
        .pHci2Host_TaskQueue = (uint8_t *) &gHci2Host_TaskQueue,
        .pHost_TaskEvent = &gHost_TaskEvent,
    },
    .fwkConfig = {
        .useRtosField = &gUseRtos_c,
    }
};

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/
bool_t Ble_CheckMemoryStorage(void)
{

    bool_t status = FALSE;

    do {
#if (gAppMaxConnections_c > 0)
        if (Ble_HostConfigMemoryCheck(eAttConnStorageSize, 
                                      gAppMaxConnections_c, 
                                      sizeof(gAttConnStorage)) != gBleSuccess_c) break;
        if (Ble_HostConfigMemoryCheck(eActiveDevicesStorageSize,
                                      gAppMaxConnections_c, 
                                      sizeof(gActiveDevicesStorage)) != gBleSuccess_c) break;
        if (Ble_HostConfigMemoryCheck(eL2caLeCbChannelEntrySize,
                                      gAppMaxConnections_c,
                                      gL2caLeCbChannelEntrySize_c) != gBleSuccess_c) break;
        if (Ble_HostConfigMemoryCheck(eL2caLePsmEntrySize, 
                                      gAppMaxConnections_c,
                                      gL2caLePsmEntrySize_c) != gBleSuccess_c) break;
#endif

        status = TRUE;
    } while (0);
    return status;
}

bool_t Ble_ConfigureHostStackConfig(void)
{
    bool_t status = TRUE;

    if (Ble_HostConfigMemoryCheck(eBleHostGlobalConfigSize, gAppMaxConnections_c, sizeof(bleHostGlobalConfig_t)) != gBleSuccess_c)
    {
        status = FALSE;
    }
    else
    {
        Ble_HostConfigInit(&hostGlobalConfig);
    }

    return status;
}

/*! *********************************************************************************
* @}
********************************************************************************** */
