/*! *********************************************************************************
* Copyright (c) 2014, Freescale Semiconductor, Inc.
* Copyright 2016-2020 NXP
*
* \file
*
* This is a source file for the main application.
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
/* Drv */
#include "LED.h"

/* Fwk */
#include "fsl_os_abstraction.h"
#include "MemManager.h"
#include "TimersManager.h"
#include "RNG_Interface.h"
#include "Messaging.h"
#include "Flash_Adapter.h"
#include "SecLib.h"
#include "Panic.h"

#if defined(gFsciIncluded_c) && (gFsciIncluded_c == 1)
#include "FsciInterface.h"
#if gFSCI_IncludeLpmCommands_c
#include "FsciCommands.h"
#endif
#endif

/* KSDK */
#include "board.h"
#if defined(MULTICORE_CONNECTIVITY_CORE) && (MULTICORE_CONNECTIVITY_CORE)
  #if FSL_FEATURE_SOC_CAU3_COUNT
  #include "fsl_cau3.h"
  #endif
#endif

#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
#include "PWR_Interface.h"
#endif
#if defined(cPWR_FullPowerDownMode) && (cPWR_FullPowerDownMode) && (configUSE_TICKLESS_IDLE != 0)
#include "TMR_Adapter.h"
#endif

#if gUsePdm_d
#include "PDM.h"
#endif
#ifdef FSL_RTOS_FREE_RTOS
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "portmacro.h"
#endif

#include "ApplMain.h"

#if (defined(CPU_QN908X) || defined(CPU_JN518X))
#include "controller_interface.h"
#endif

#include "fsl_rtc.h"

#ifdef CPU_QN908X
#include "fsl_wdt.h"
#include "clock_config.h"
#if (defined(BOARD_XTAL1_CLK_HZ) && (BOARD_XTAL1_CLK_HZ != CLK_XTAL_32KHZ))
#include "rco32k_calibration.h"
#endif /* BOARD_XTAL1_CLK_HZ != CLK_XTAL_32KHZ */
#endif /* CPU_QN908X */

#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1) && ((gFsciBleBBox_d == 1) || (gFsciBleTest_d == 1))
#include "fsci_ble_gap.h"
#include "fsci_ble_gatt.h"
#include "fsci_ble_l2cap_cb.h"
#endif

#if (!cPWR_UsePowerDownMode)
#include "fsl_xcvr.h"
#endif

#if gOTA_InitAtBoot_d
#include "OtaSupport.h"
#endif

#if defined (SOTA_ENABLED)
#include "blob_manager_app.h"
#endif

#ifdef DUAL_MODE_APP
#include "app_dual_mode_switch.h"
#endif

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/

/* Application Events */
#define gAppEvtMsgFromHostStack_c       (1U << 0U)
#define gAppEvtAppCallback_c            (1U << 1U)

#ifdef FSL_RTOS_FREE_RTOS
    #if (configUSE_IDLE_HOOK)
        #define mAppIdleHook_c 1
    #endif

    #if (configUSE_TICKLESS_IDLE != 0)
        #define mAppEnterLpFromIdleTask_c (0)
        #define mAppTaskWaitTime_c        (1000U) /* milliseconds */
        #define mAppOverheadTicks_c       (1U)    /* Application processing overhead in OS ticks */
    #endif
#endif

#if defined mAppUseTickLessMode_c && (mAppUseTickLessMode_c > 0)
#define mAppIdleHook_c 1
#endif

#ifndef mAppIdleHook_c
#define mAppIdleHook_c 0
#endif

#ifndef mAppEnterLpFromIdleTask_c
#define mAppEnterLpFromIdleTask_c 1
#endif

#ifndef mAppTaskWaitTime_c
#define mAppTaskWaitTime_c (osaWaitForever_c)
#endif

#if defined (MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE)
#define MULTICORE_STATIC
#else
#if defined DUAL_MODE_APP
#define MULTICORE_STATIC
#else
#define MULTICORE_STATIC static
#endif
#endif


/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/
/* Host to Application Messages Types */
typedef enum {
    gAppGapGenericMsg_c = 0U,
    gAppGapConnectionMsg_c,
    gAppGapAdvertisementMsg_c,
    gAppGapScanMsg_c,
    gAppGattServerMsg_c,
    gAppGattClientProcedureMsg_c,
    gAppGattClientNotificationMsg_c,
    gAppGattClientIndicationMsg_c,
    gAppL2caLeDataMsg_c,
    gAppL2caLeControlMsg_c,
    gAppSecLibMultiplyMsg_c,
}appHostMsgType_t;

/* Host to Application Connection Message */
typedef struct connectionMsg_tag{
    deviceId_t              deviceId;
    gapConnectionEvent_t    connEvent;
}connectionMsg_t;

/* Host to Application GATT Server Message */
typedef struct gattServerMsg_tag{
    deviceId_t          deviceId;
    gattServerEvent_t   serverEvent;
}gattServerMsg_t;

/* Host to Application GATT Client Procedure Message */
typedef struct gattClientProcMsg_tag{
    deviceId_t              deviceId;
    gattProcedureType_t     procedureType;
    gattProcedureResult_t   procedureResult;
    bleResult_t             error;
}gattClientProcMsg_t;

/* Host to Application GATT Client Notification/Indication Message */
typedef struct gattClientNotifIndMsg_tag{
    uint8_t*    aValue;
    uint16_t    characteristicValueHandle;
    uint16_t    valueLength;
    deviceId_t  deviceId;
}gattClientNotifIndMsg_t;

/* L2ca to Application Data Message */
typedef struct l2caLeCbDataMsg_tag{
    deviceId_t  deviceId;
    uint16_t    channelId;
    uint16_t    packetLength;
    uint8_t     aPacket[1];
}l2caLeCbDataMsg_t;

/* SecLib to Application Data Message */
typedef struct secLibMsgData_tag{
    computeDhKeyParam_t *pData;
} secLibMsgData_t;

typedef struct appMsgFromHost_tag{
    uint32_t    msgType;
    union {
        gapGenericEvent_t       genericMsg;
        gapAdvertisingEvent_t   advMsg;
        connectionMsg_t         connMsg;
        gapScanningEvent_t      scanMsg;
        gattServerMsg_t         gattServerMsg;
        gattClientProcMsg_t     gattClientProcMsg;
        gattClientNotifIndMsg_t gattClientNotifIndMsg;
        l2caLeCbDataMsg_t       l2caLeCbDataMsg;
        l2capControlMessage_t   l2caLeCbControlMsg;
        secLibMsgData_t         secLibMsgData;
    } msgData;
}appMsgFromHost_t;

typedef struct appMsgCallback_tag{
    appCallbackHandler_t   handler;
    appCallbackParam_t     param;
}appMsgCallback_t;
/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
************************************************************************************/
/* Actions required to be performed in IdleTask*/
#if defined DUAL_MODE_APP
#define IdleTask_DualMode_c   1
#else
#define IdleTask_DualMode_c   0
#endif
#if (defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode) && mAppEnterLpFromIdleTask_c)
#define IdleTask_LowPower_c 2
#else
#define IdleTask_LowPower_c 0
#endif

/* Board specific treatment is mandated if Xtal 32M temperature compensation is required.
 * Otherwise a default WEAK implementation is defined in this file.
 */
#define  IdleTask_BoardSpecific_c 8

#define IdleTaskAct_c (IdleTask_DualMode_c | IdleTask_LowPower_c | IdleTask_BoardSpecific_c)

#if (IdleTaskAct_c != 0)
  #if (mAppIdleHook_c)
    #define AppIdle_TaskInit()
    #define App_Idle_Task()
  #else /* mAppIdleHook_c */
    #if (IdleTask_DualMode_c)
    osaStatus_t AppIdle_TaskInit(void);
    #else
    static osaStatus_t AppIdle_TaskInit(void);
    #endif
    static void App_Idle_Task(osaTaskParam_t argument);
  #endif /* mAppIdleHook_c */
#endif

#if gKeyBoardSupported_d && (gKBD_KeysCount_c > 0) && !defined (DUAL_MODE_APP)
static void App_KeyboardCallBack(uint8_t events);
#endif

static void App_HandleHostMessageInput(appMsgFromHost_t* pMsg);

#ifdef CPU_QN908X
#if (defined(BOARD_XTAL1_CLK_HZ) && (BOARD_XTAL1_CLK_HZ != CLK_XTAL_32KHZ))
#if (defined(CFG_CALIBRATION_ON_IDLE_TASK) && (CFG_CALIBRATION_ON_IDLE_TASK > 0))
static void App_Rco32KCalibrate(void);
#endif /* CFG_CALIBRATION_ON_IDLE_TASK */
#endif /* BOARD_XTAL1_CLK_HZ != CLK_XTAL_32KHZ */
#endif /* CPU_QN908X */


MULTICORE_STATIC void App_ConnectionCallback (deviceId_t peerDeviceId, gapConnectionEvent_t* pConnectionEvent);
MULTICORE_STATIC void App_AdvertisingCallback (gapAdvertisingEvent_t* pAdvertisingEvent);
MULTICORE_STATIC void App_ScanningCallback (gapScanningEvent_t* pScanningEvent);
MULTICORE_STATIC void App_GattServerCallback (deviceId_t peerDeviceId, gattServerEvent_t* pServerEvent);
MULTICORE_STATIC void App_GattClientProcedureCallback
(
    deviceId_t              deviceId,
    gattProcedureType_t     procedureType,
    gattProcedureResult_t   procedureResult,
    bleResult_t             error
);
MULTICORE_STATIC void App_GattClientNotificationCallback
(
    deviceId_t      deviceId,
    uint16_t        characteristicValueHandle,
    uint8_t*        aValue,
    uint16_t        valueLength
);
MULTICORE_STATIC void App_GattClientIndicationCallback
(
    deviceId_t      deviceId,
    uint16_t        characteristicValueHandle,
    uint8_t*        aValue,
    uint16_t        valueLength
);

MULTICORE_STATIC void App_L2caLeDataCallback
(
    deviceId_t deviceId,
    uint16_t channelId,
    uint8_t* pPacket,
    uint16_t packetLength
);

MULTICORE_STATIC void App_L2caLeControlCallback
(
    l2capControlMessage_t* pMessage
);

MULTICORE_STATIC void App_SecLibMultCallback
(
    computeDhKeyParam_t *pData
);

#if !defined (SOTA_ENABLED) && !defined (DUAL_MODE_APP)
#if !defined(gUseHciTransportDownward_d) || (!gUseHciTransportDownward_d)
static void BLE_SignalFromISRCallback(void);

#if (!defined(KW37A4_SERIES) && !defined(KW37Z4_SERIES) && !defined(KW38A4_SERIES) && !defined(KW38Z4_SERIES) && !defined(KW39A4_SERIES))
extern void (*pfBLE_SignalFromISR)(void);
#endif
#endif /* gUseHciTransportDownward_d */
#endif /* SOTA_ENABLED */

#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode) && (configUSE_TICKLESS_IDLE != 0)
extern void vTaskStepTick( const TickType_t xTicksToJump );
#endif

/*! *********************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
********************************************************************************** */
void App_Thread (uint32_t param);

#if defined(gMWS_Enabled_d) && (gMWS_Enabled_d)
extern void App_Init(void);
#endif

#if !defined(MULTICORE_CONNECTIVITY_CORE) || (!MULTICORE_CONNECTIVITY_CORE)
void App_GenericCallback (gapGenericEvent_t* pGenericEvent);
#endif

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
#if (IdleTaskAct_c != 0)
#if (!mAppIdleHook_c)
static OSA_TASK_DEFINE( App_Idle_Task, gAppIdleTaskPriority_c, 1, gAppIdleTaskStackSize_c, FALSE );
static osaTaskId_t mAppIdleTaskId = NULL;
#endif
#endif  /* cPWR_UsePowerDownMode */



typedef struct {
    uint16_t pdmId;
    uint16_t nbRamWrite;
    bleBondIdentityHeaderBlob_t  aBondingHeader;
    bleBondDataDynamicBlob_t     aBondingDataDynamic;
    bleBondDataStaticBlob_t      aBondingDataStatic;
    bleBondDataDeviceInfoBlob_t  aBondingDataDeviceInfo;
    bleBondDataDescriptorBlob_t  aBondingDataDescriptor[gcGapMaximumSavedCccds_c];
} bleBondDeviceEntry;

#if (gMaxBondedDevices_c > 0)
static bleBondDeviceEntry bondEntries[gMaxBondedDevices_c];
static osaMutexId_t bondingMutex;
#endif

#ifndef DUAL_MODE_APP
static osaEventId_t  mAppEvent;
/* Application input queues */
static anchor_t mHostAppInputQueue;
static anchor_t mAppCbInputQueue;
#else
extern osaEventId_t  mAppEvent;
anchor_t mHostAppInputQueue;
anchor_t mAppCbInputQueue;
#endif

static gapGenericCallback_t pfGenericCallback = NULL;
static gapAdvertisingCallback_t pfAdvCallback = NULL;
static gapScanningCallback_t pfScanCallback = NULL;
static gapConnectionCallback_t  pfConnCallback = NULL;
static gattServerCallback_t pfGattServerCallback = NULL;
static gattClientProcedureCallback_t pfGattClientProcCallback = NULL;
static gattClientNotificationCallback_t pfGattClientNotifCallback = NULL;
static gattClientNotificationCallback_t pfGattClientIndCallback = NULL;
static l2caLeCbDataCallback_t           pfL2caLeCbDataCallback = NULL;
static l2caLeCbControlCallback_t        pfL2caLeCbControlCallback = NULL;

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/

extern const uint8_t gUseRtos_c;

#ifdef CPU_QN908X
#if (defined(BOARD_XTAL1_CLK_HZ) && (BOARD_XTAL1_CLK_HZ != CLK_XTAL_32KHZ))
extern volatile bool_t gRco32kCalibrationRequest;
extern void RCO32K_Calibrate(void);
#endif /* BOARD_XTAL1_CLK_HZ  */
#endif /* CPU_QN908X */
/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

#if !defined(gHybridApp_d) || (!gHybridApp_d)
#ifndef DUAL_MODE_APP
/*! *********************************************************************************
* \brief  This is the first task created by the OS. This task will initialize
*         the system
*
* \param[in]  param
*
********************************************************************************** */
void main_task(uint32_t param)
{
    static bool_t platformInitialized = FALSE;

    if (FALSE == platformInitialized)
    {
        platformInitialized = TRUE;

#if !defined(CPU_QN908X) && !defined (CPU_JN518X) && defined(gDCDC_Enabled_d) && (gDCDC_Enabled_d)
        /* Init DCDC module */
        BOARD_DCDCInit();
#endif
#if defined (CPU_JN518X)
        BOARD_SetFaultBehaviour();
#endif

        (void)MEM_Init();

        /* Framework init */
#if defined(gRngSeedStorageAddr_d) || defined(gXcvrDacTrimValueSorageAddr_d)\
     || gAppUseNvm_d ||  (defined gRadioUsePdm_d && gRadioUsePdm_d)
        NV_Init();
#endif

        TMR_Init();
        /* Check if RTC was started already: if not do it now */
        if (RTC->CTRL & RTC_CTRL_SWRESET_MASK)
        {
            CLOCK_EnableClock(kCLOCK_Rtc);
            CLOCK_SetClkDiv(kCLOCK_DivRtcClk, 32, false);    /* 1ms / 1kHz tick */
            CLOCK_SetClkDiv(kCLOCK_DivRtc1HzClk, 1U, false);
            RTC_Init(RTC);
            PWR_Start32kCounter();
        }

        /* Cryptographic and RNG hardware initialization */
  #if defined(MULTICORE_CONNECTIVITY_CORE) && (MULTICORE_CONNECTIVITY_CORE)
    #if defined (FSL_FEATURE_SOC_MMCAU_COUNT) && (FSL_FEATURE_SOC_MMCAU_COUNT > 0)
        /* Make sure the clock is provided by M0+ in order to avoid issues after exiting low power mode from M0+ */
        CLOCK_EnableClock(kCLOCK_Cau3);
      #if defined(FSL_CAU3_USE_HW_SEMA) && (FSL_CAU3_USE_HW_SEMA > 0)
        CLOCK_EnableClock(FSL_CAU3_SEMA42_CLOCK_NAME);
      #endif /* FSL_CAU3_USE_HW_SEMA */
    #endif /* FSL_FEATURE_SOC_MMCAU_COUNT */
  #else
        SecLib_Init();
  #endif /* MULTICORE_CONNECTIVITY_CORE */
        /* Set external multiplication callback if we don't have support for hardware elliptic curve
         * multiplication */
#if defined(FSL_FEATURE_SOC_CAU3_COUNT) && (FSL_FEATURE_SOC_CAU3_COUNT > 0)
#else
        SecLib_SetExternalMultiplicationCb(App_SecLibMultCallback);
#endif
#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE)
        Board_StartSecondaryCoreApp();
        CLOCK_DisableClock(kCLOCK_Cau3);
#else
        /* RNG software initialization and PRNG initial seeding (from hardware) */
        (void)RNG_Init();
        RNG_SetPseudoRandomNoSeed(NULL);
#endif /* MULTICORE_APPLICATION_CORE */
        LED_Init();
  #if gKeyBoardSupported_d && (gKBD_KeysCount_c > 0)
        KBD_Init(App_KeyboardCallBack);
  #endif

#ifdef CPU_QN908X
        /* Initialize QN9080 BLE Controller. Requires that MEM_Manager and SecLib to be already initialized */
        BLE_Init(gAppMaxConnections_c);
#endif /* CPU_QN908X */

  #if (defined gRadioUsePdm_d && gRadioUsePdm_d) || (defined gBleControllerUsePdm_d && gBleControllerUsePdm_d)
        PDM_Init();
  #endif

#if !defined (SOTA_ENABLED)
#if !gUseHciTransportDownward_d
        pfBLE_SignalFromISR = BLE_SignalFromISRCallback;
#endif /* !gUseHciTransportDownward_d */
#endif /* SOTA_ENABLED */

#if (IdleTaskAct_c != 0) /* Idle task required */
#if (!mAppIdleHook_c)
        (void)AppIdle_TaskInit();
#endif
#endif
  #if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
        PWR_Init();
        PWR_DisallowDeviceToSleep();
  #else
        Led1Flashing();
        Led2Flashing();
        Led3Flashing();
        Led4Flashing();
  #endif

  #if defined (SOTA_ENABLED)
        BLOBM_Init();
  #endif
        /* Initialize peripheral drivers specific to the application */
        BleApp_Init();

        /* Create application event */
        mAppEvent = OSA_EventCreate(TRUE);
        if( NULL == mAppEvent )
        {
            panic(0,0,0,0);
            return;
        }

        /* Prepare application input queue.*/
        MSG_InitQueue(&mHostAppInputQueue);

        /* Prepare callback input queue.*/
        MSG_InitQueue(&mAppCbInputQueue);

        App_NvmInit();
  #if !defined(MULTICORE_CONNECTIVITY_CORE) || (!MULTICORE_CONNECTIVITY_CORE)
        /* BLE Host Stack Init */
        if (Ble_Initialize(App_GenericCallback) != gBleSuccess_c)
        {
            panic(0,0,0,0);
            return;
        }
  #endif /* MULTICORE_CONNECTIVITY_CORE */

  #if defined(gMWS_Enabled_d) && (gMWS_Enabled_d)
        App_Init();
  #endif
    }

    /* Call application task */
    App_Thread( param );
}
#endif /* gHybridApp_d */
#endif

/*! *********************************************************************************
* \brief  This function represents the Application task.
*         This task reuses the stack allocated for the MainThread.
*         This function is called to process all events for the task. Events
*         include timers, messages and any other user defined events.
* \param[in]  argument
*
* \remarks  For bare-metal, process only one type of message at a time,
*           to allow other higher priority task to run.
*
********************************************************************************** */
void App_Thread (uint32_t param)
{
#if !defined(gHybridApp_d) || (!gHybridApp_d)
    osaEventFlags_t event = 0U;

    for (;;)
    {
        (void)OSA_EventWait(mAppEvent, osaEventFlagsAll_c, FALSE, mAppTaskWaitTime_c , &event);
#else
    {
#endif /* gHybridApp_d */
        /* Check for existing messages in queue */
        if (MSG_Pending(&mHostAppInputQueue))
        {
            /* Pointer for storing the messages from host. */
            appMsgFromHost_t *pMsgIn = MSG_DeQueue(&mHostAppInputQueue);

            if (pMsgIn != NULL)
            {
                /* Process it */
                App_HandleHostMessageInput(pMsgIn);

                /* Messages must always be freed. */
                (void)MSG_Free(pMsgIn);
            }
        }

        /* Check for existing messages in queue */
        if (MSG_Pending(&mAppCbInputQueue))
        {
            /* Pointer for storing the callback messages. */
            appMsgCallback_t *pMsgIn = MSG_DeQueue(&mAppCbInputQueue);

            if (pMsgIn != NULL)
            {
                /* Execute callback handler */
                if (pMsgIn->handler != NULL)
                {
                    pMsgIn->handler(pMsgIn->param);
                }

                /* Messages must always be freed. */
                (void)MSG_Free(pMsgIn);
            }
        }
#if !defined(gHybridApp_d) || (!gHybridApp_d)
        /* Signal the App_Thread again if there are more messages pending */
        event = MSG_Pending(&mHostAppInputQueue) ? gAppEvtMsgFromHostStack_c : 0U;
        event |= MSG_Pending(&mAppCbInputQueue) ? gAppEvtAppCallback_c : 0U;

        if (event != 0U)
        {
            (void)OSA_EventSet(mAppEvent, gAppEvtAppCallback_c);
        }

        /* For BareMetal break the while(1) after 1 run */
        if( gUseRtos_c == 0U )
        {
            break;
        }
#endif /* gHybridApp_d */
    }
}

/*
* board_specific_action_on_idle is declared weak.
* Its actual implementation is expected in borad.c
*/
WEAK void board_specific_action_on_idle(void)
{
}

#ifdef CPU_QN908X
#if (defined(BOARD_XTAL1_CLK_HZ) && (BOARD_XTAL1_CLK_HZ != CLK_XTAL_32KHZ))
#if (defined(CFG_CALIBRATION_ON_IDLE_TASK) && (CFG_CALIBRATION_ON_IDLE_TASK > 0))
static void App_Rco32KCalibrate(void)
{
    if(gRco32kCalibrationRequest)
    {
        /* perform RCO32K calibration */
        RCO32K_Calibrate();
        /* clear the calibration request flag */
        gRco32kCalibrationRequest = FALSE;
    }
}
#endif /* CFG_CALIBRATION_ON_IDLE_TASK */
#endif /* BOARD_XTAL1_CLK_HZ != CLK_XTAL_32KHZ */
#endif /* CPU_QN908X */

#ifndef DUAL_MODE_APP
#if (IdleTaskAct_c & IdleTask_LowPower_c )
/*! *********************************************************************************
* \brief The idle task is created when applications enable the usage of the Framework
*        Low-Power module
*
* \param[in] none
*
* \return  none
********************************************************************************** */
static void App_Idle(void)
{
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
    if( PWR_CheckIfDeviceCanGoToSleep() )
    {
        /* Enter Low Power */
        (void)PWR_EnterLowPower();

#if gFSCI_IncludeLpmCommands_c
        /* Send Wake Up indication to FSCI */
        FSCI_SendWakeUpIndication();
#endif

#if gKBD_KeysCount_c > 0
#if !defined(CPU_JN518X)   // ON QN9090, PWR will directly notify the GPIO module for the source of IO wakeup
        /* Woke up on Keyboard Press */
        if(PWRLib_MCU_WakeupReason.Bits.FromKeyBoard != 0U)
        {
  #if defined(cPWR_EnableDeepSleepMode_8) && (cPWR_EnableDeepSleepMode_8)
            /* Skip over the key scan timer to improve low power consumption. */
            BleApp_HandleKeys(gKBD_EventPressPB1_c);
  #else
            KBD_SwitchPressedOnWakeUp();
  #endif
        }
#endif  /*  !defined(CPU_JN518X) */
#endif  /* gKBD_KeysCount_c */
    }
    else
    {
        /* Enter MCU Sleep */
        PWR_EnterSleep();
    }
#else

    /* check if any Radio calibration is needed, it requires asking for a dry run BLE sleep */
    if(XCVR_GetRecalDuration())
    {
        BLE_get_sleep_mode();
    }
#endif /* cPWR_UsePowerDownMode */
}
#endif /*  (IdleTaskAct_c & IdleTask_LowPower_c) */
#endif /* DUAL_MODE_APP */

#if (IdleTaskAct_c != 0)
/*
 * Treatment to be executed in Idle
 * - With an RTOS whether from the vApplicationIdleHook function or from a
 * dedicated task.
 * - in Bare metal mode  from the OSA Idle Task
 */
static void AppIdleCommon(void)
{
#if (gMaxBondedDevices_c > 0)

    #define PDM_UPDATE_RECORD_TIME_US 8000

    uint16_t i;
    PDM_teStatus  pdmSt;
    OSA_MutexLock(bondingMutex, osaWaitForever_c);
    for (i=0; i<gMaxBondedDevices_c; i++)
    {
        if (bondEntries[i].nbRamWrite > 0)
        {
            uint32_t nextBleEventValue = BLE_TimeBeforeNextBleEvent();
            /* Avoid to block the LL during PDM save record
             * PDM_eSaveRecordData could take up to 8ms to save a 1 page record so we need to
             * make sure that no LL event is scheduled during a call to SaveRecordData
             */
            if (nextBleEventValue < PDM_UPDATE_RECORD_TIME_US)
                break;
            APP_DBG_LOG("nextBleEventValue = %d",nextBleEventValue);
            APP_DBG_LOG("entry (%d) will be saved in PDM nbRamWrite = %d",i,bondEntries[i].nbRamWrite);
            bondEntries[i].nbRamWrite = 0;
            pdmSt = PDM_eSaveRecordData(bondEntries[i].pdmId,
                                                  &bondEntries[i],
                                                  sizeof(bleBondDeviceEntry));
            NOT_USED(pdmSt);
            assert(pdmSt == PDM_E_STATUS_OK);
            break;
        }
    }
    OSA_MutexUnlock(bondingMutex);
#endif
#if   (IdleTaskAct_c & IdleTask_BoardSpecific_c )
    board_specific_action_on_idle();
#endif
#if (IdleTaskAct_c & IdleTask_DualMode_c )
        dm_switch_IdleTask();
#elif (IdleTaskAct_c & IdleTask_LowPower_c )
    App_Idle();
#endif
}

#if (mAppIdleHook_c)
/*! *********************************************************************************
* \brief Is linked by the application in order to hook the idle task
*
* \param[in] none
*
* \return  none
********************************************************************************** */

void vApplicationIdleHook(void)
{
#ifdef CPU_QN908X
#if (defined(BOARD_XTAL1_CLK_HZ) && (BOARD_XTAL1_CLK_HZ != CLK_XTAL_32KHZ))
#if (defined(CFG_CALIBRATION_ON_IDLE_TASK) && (CFG_CALIBRATION_ON_IDLE_TASK > 0))
    App_Rco32KCalibrate();
#endif /* CFG_CALIBRATION_ON_IDLE_TASK */
#endif /* BOARD_XTAL1_CLK_HZ != CLK_XTAL_32KHZ */
#endif /* CPU_QN908X */

    AppIdleCommon();
}

#else /* mAppIdleHook_c */
static void App_Idle_Task(osaTaskParam_t argument)
{
    for (;;)
    {
#ifdef CPU_QN908X
#if (defined(BOARD_XTAL1_CLK_HZ) && (BOARD_XTAL1_CLK_HZ != CLK_XTAL_32KHZ))
#if (defined(CFG_CALIBRATION_ON_IDLE_TASK) && (CFG_CALIBRATION_ON_IDLE_TASK > 0))
        App_Rco32KCalibrate();
#endif /* CFG_CALIBRATION_ON_IDLE_TASK */
#endif /* BOARD_XTAL1_CLK_HZ != CLK_XTAL_32KHZ */
#endif /* CPU_QN908X */
        AppIdleCommon();

        /* For BareMetal break the while(1) after 1 run */
        if (gUseRtos_c == 0U)
        {
            break;
        }
    }
}

/*! *********************************************************************************
* \brief Initializes task
*
* \param[in]  none
*
* \return  osaStatus_Error or osaStatus_Success.
*
********************************************************************************** */
#ifndef DUAL_MODE_APP
static osaStatus_t AppIdle_TaskInit(void)
#else
osaStatus_t AppIdle_TaskInit(void)
#endif
{
    osaStatus_t st;
    do {
        if(NULL != mAppIdleTaskId)
        {
            st = osaStatus_Error;
            break;
        }

    /* Task creation */
        mAppIdleTaskId = OSA_TaskCreate(OSA_TASK(App_Idle_Task), NULL);

        if( NULL == mAppIdleTaskId )
        {
            panic(0,0,0,0);
            st = osaStatus_Error;
            break;
        }
        st = osaStatus_Success;
    } while (0);
    return st;
}
#endif /* mAppIdleHook_c */
#endif /*IdleTaskAct_c != 0 */


/*! *********************************************************************************
* \brief  Application wrapper function for Gap_Connect.
*
* \param[in] pParameters  Connection Request parameters
* \param[in] connCallback Callback used to receive connection events.
*
* \return  gBleSuccess_c or error.
*
********************************************************************************** */
bleResult_t App_Connect(
    gapConnectionRequestParameters_t*   pParameters,
    gapConnectionCallback_t             connCallback
)
{
    pfConnCallback = connCallback;

    return Gap_Connect(pParameters, App_ConnectionCallback);
}

/*! *********************************************************************************
* \brief  Application wrapper function for Gap_StartAdvertising.
*
* \param[in] advertisingCallback   Callback used by the application to receive advertising events.
*                                  Can be NULL.
* \param[in] connectionCallback    Callback used by the application to receive connection events.
*                                  Can be NULL.
*
* \return  gBleSuccess_c or error.
*
********************************************************************************** */
bleResult_t App_StartAdvertising(
    gapAdvertisingCallback_t    advertisingCallback,
    gapConnectionCallback_t     connectionCallback
)
{
    pfAdvCallback = advertisingCallback;
    pfConnCallback = connectionCallback;

    return Gap_StartAdvertising(App_AdvertisingCallback, App_ConnectionCallback);
}

/*! *********************************************************************************
* \brief  Application wrapper function for Gap_StartExtAdvertising.
*
* \param[in] advertisingCallback   Callback used by the application to receive advertising events.
*                                  Can be NULL.
* \param[in] connectionCallback    Callback used by the application to receive connection events.
*                                  Can be NULL.
* \param[in] handle                The ID of the advertising set
* \param[in] duration              The duration of the advertising
* \param[in] maxExtAdvEvents       The maximum number of advertising events
*
* \return  gBleSuccess_c or error.
*
********************************************************************************** */
bleResult_t App_StartExtAdvertising(
    gapAdvertisingCallback_t    advertisingCallback,
    gapConnectionCallback_t     connectionCallback,
    uint8_t                     handle,
    uint16_t                    duration,
    uint8_t                     maxExtAdvEvents
)
{
    pfAdvCallback = advertisingCallback;
    pfConnCallback = connectionCallback;

    return Gap_StartExtAdvertising(App_AdvertisingCallback, App_ConnectionCallback, handle, duration, maxExtAdvEvents);
}

/*! *********************************************************************************
* \brief  Application wrapper function for Gap_StartScanning.
*
* \param[in] pScanningParameters    The scanning parameters; may be NULL.
* \param[in] scanningCallback       The scanning callback.
* \param[in] enableFilterDuplicates Enable duplicate advertising reports filtering
* \param[in] duration               Scan duration
* \param[in] period                 Scan period
*
* \return    gBleSuccess_c or error.
*
********************************************************************************** */
bleResult_t App_StartScanning(
    gapScanningParameters_t*    pScanningParameters,
    gapScanningCallback_t       scanningCallback,
    gapFilterDuplicates_t       enableFilterDuplicates,
    uint16_t                    duration,
    uint16_t                    period
)
{
    pfScanCallback = scanningCallback;

    return Gap_StartScanning(pScanningParameters, App_ScanningCallback,  enableFilterDuplicates, duration, period);
}

/*! *********************************************************************************
* \brief  Application wrapper function for GattServer_RegisterCallback.
*
* \param[in] serverCallback Application-defined callback to be triggered by this module.
*
* \return    gBleSuccess_c or error.
*
********************************************************************************** */
bleResult_t App_RegisterGattServerCallback(gattServerCallback_t  serverCallback)
{
    pfGattServerCallback = serverCallback;

    return GattServer_RegisterCallback(App_GattServerCallback);
}

/*! *********************************************************************************
* \brief  Application wrapper function for App_RegisterGattClientProcedureCallback.
*
* \param[in] callback Application-defined callback to be triggered by this module.
*
* \return    gBleSuccess_c or error.
*
********************************************************************************** */
bleResult_t App_RegisterGattClientProcedureCallback(gattClientProcedureCallback_t  callback)
{
    pfGattClientProcCallback = callback;

    return GattClient_RegisterProcedureCallback(App_GattClientProcedureCallback);
}

/*! *********************************************************************************
* \brief  Application wrapper function for GattClient_RegisterNotificationCallback.
*
* \param[in] callback   Application defined callback to be triggered by this module.
*
* \return  gBleSuccess_c or error.
*
********************************************************************************** */
bleResult_t App_RegisterGattClientNotificationCallback(gattClientNotificationCallback_t  callback)
{
    pfGattClientNotifCallback = callback;

    return GattClient_RegisterNotificationCallback(App_GattClientNotificationCallback);
}

/*! *********************************************************************************
* \brief  Application wrapper function for GattClient_RegisterIndicationCallback.
*
* \param[in] callback   Application defined callback to be triggered by this module.
*
* \return  gBleSuccess_c or error.
*
********************************************************************************** */
bleResult_t App_RegisterGattClientIndicationCallback(gattClientIndicationCallback_t  callback)
{
    pfGattClientIndCallback = callback;

    return GattClient_RegisterIndicationCallback(App_GattClientIndicationCallback);
}

/*! *********************************************************************************
* \brief  Application wrapper function for L2ca_RegisterLeCbCallbacks.
*
* \param[in] pCallback     L2cap Data Callback
* \param[in] pCtrlCallback L2cap Control Callback
*
* \return  gBleSuccess_c or error.
*
********************************************************************************** */
bleResult_t App_RegisterLeCbCallbacks
(
    l2caLeCbDataCallback_t      pCallback,
    l2caLeCbControlCallback_t   pCtrlCallback
)
{
    pfL2caLeCbDataCallback = pCallback;
    pfL2caLeCbControlCallback = pCtrlCallback;

    return L2ca_RegisterLeCbCallbacks(App_L2caLeDataCallback, App_L2caLeControlCallback);
}

/*! *********************************************************************************
* \brief  Posts an application event containing a callback handler and parameter.
*
* \param[in] handler Handler function, to be executed when the event is processed.
* \param[in] param   Parameter for the handler function.
*
* \return  gBleSuccess_c or error.
*
********************************************************************************** */
bleResult_t App_PostCallbackMessage
(
    appCallbackHandler_t   handler,
    appCallbackParam_t     param
)
{
    appMsgCallback_t *pMsgIn = NULL;

    /* Allocate a buffer with enough space to store the packet */
    pMsgIn = MSG_Alloc(sizeof (appMsgCallback_t));

    if (pMsgIn == NULL)
    {
        return gBleOutOfMemory_c;
    }

    pMsgIn->handler = handler;
    pMsgIn->param = param;

    /* Put message in the Cb App queue */
    (void)MSG_Queue(&mAppCbInputQueue, pMsgIn);

    /* Signal application */
    (void)OSA_EventSet(mAppEvent, gAppEvtAppCallback_c);

    return gBleSuccess_c;
}

/*! *********************************************************************************
* \brief  Initialize bonded devices list and restore bonded data from PDM
*
* \param[in] none
*
* \return  none
*
********************************************************************************** */
void App_NvmInit(void)
{
#if (gMaxBondedDevices_c > 0)
    /* Init the bonded device list */
    uint16_t i = 0;
    PDM_teStatus  pdmSt;
    uint16_t pu16DataBytesRead = 0;
    FLib_MemSet(bondEntries, 0, sizeof(bondEntries));
    bondingMutex = OSA_MutexCreate();
    assert(bondingMutex != NULL);
    for (i=0; i<gMaxBondedDevices_c; i++)
    {
        bondEntries[i].pdmId = pdmId_BondEntry0+i;
        bondEntries[i].nbRamWrite = 0;
        /* Try to load the data from the PDM */
        if (PDM_bDoesDataExist(bondEntries[i].pdmId, &pu16DataBytesRead))
        {
            APP_DBG_LOG("Record = 0x%x loaded", bondEntries[i].pdmId);
            pdmSt = PDM_eReadDataFromRecord(bondEntries[i].pdmId, &bondEntries[i],
                                                         sizeof(bleBondDeviceEntry),
                                                         &pu16DataBytesRead);
            NOT_USED(pdmSt);
            assert(pdmSt == PDM_E_STATUS_OK);
            assert(sizeof(bleBondDeviceEntry) == pu16DataBytesRead);
        }
    }
#endif
}
/*! *********************************************************************************
* \brief  Performs NVM erase operation
*
* \param[in] mEntryIdx Bonded device index
*
* \return  none
*
********************************************************************************** */

void App_NvmErase(uint8_t mEntryIdx)
{
#if (gMaxBondedDevices_c > 0)
    APP_DBG_LOG("EntryIdx=%d", mEntryIdx);
    OSA_MutexLock(bondingMutex, osaWaitForever_c);
    if(mEntryIdx < (uint8_t)gMaxBondedDevices_c)
    {
        /* Check if a write is required */
        if (!FLib_MemCmpToVal(&bondEntries[mEntryIdx].aBondingHeader, 0, sizeof(bondEntries[mEntryIdx].aBondingHeader)) ||
            !FLib_MemCmpToVal(&bondEntries[mEntryIdx].aBondingDataDynamic, 0, sizeof(bondEntries[mEntryIdx].aBondingDataDynamic)) ||
            !FLib_MemCmpToVal(&bondEntries[mEntryIdx].aBondingDataStatic, 0, sizeof(bondEntries[mEntryIdx].aBondingDataStatic)) ||
            !FLib_MemCmpToVal(&bondEntries[mEntryIdx].aBondingDataDeviceInfo, 0, sizeof(bondEntries[mEntryIdx].aBondingDataDeviceInfo)) ||
            !FLib_MemCmpToVal(&bondEntries[mEntryIdx].aBondingDataDescriptor[0], 0, sizeof(bondEntries[mEntryIdx].aBondingDataDescriptor)))
        {
            FLib_MemSet(&bondEntries[mEntryIdx].aBondingHeader, 0, sizeof(bondEntries[mEntryIdx].aBondingHeader));
            FLib_MemSet(&bondEntries[mEntryIdx].aBondingDataDynamic, 0, sizeof(bondEntries[mEntryIdx].aBondingDataDynamic));
            FLib_MemSet(&bondEntries[mEntryIdx].aBondingDataStatic, 0, sizeof(bondEntries[mEntryIdx].aBondingDataStatic));
            FLib_MemSet(&bondEntries[mEntryIdx].aBondingDataDeviceInfo, 0, sizeof(bondEntries[mEntryIdx].aBondingDataDeviceInfo));
            FLib_MemSet(&bondEntries[mEntryIdx].aBondingDataDescriptor[0], 0, sizeof(bondEntries[mEntryIdx].aBondingDataDescriptor));
            bondEntries[mEntryIdx].nbRamWrite++;
        }
    }
    OSA_MutexUnlock(bondingMutex);
#else
    NOT_USED(mEntryIdx);
#endif
    APP_DBG_LOG("==> end");
}

/*! *********************************************************************************
* \brief  Performs NVM write operation
*
* \param[in] mEntryIdx            Bonded device index
* \param[in] pBondHeader          Bond Identity Header
* \param[in] pBondDataDynamic     Bond Data Dynamic
* \param[in] pBondDataStatic      Bond Data Static
* \param[in] pBondDataDeviceInfo  Bond Data Device Info
* \param[in] pBondDataDescriptor  Bond Data Descriptor
* \param[in] mDescriptorIndex     Descriptor Index
*
* \return    none
*
********************************************************************************** */
void App_NvmWrite
(
    uint8_t  mEntryIdx,
    void*    pBondHeader,
    void*    pBondDataDynamic,
    void*    pBondDataStatic,
    void*    pBondDataDeviceInfo,
    void*    pBondDataDescriptor,
    uint8_t  mDescriptorIndex
)
{
#if (gMaxBondedDevices_c > 0)
    APP_DBG_LOG("EntryIdx=%d DescIdx=%d", mEntryIdx, mDescriptorIndex);
    OSA_MutexLock(bondingMutex, osaWaitForever_c);
    if(mEntryIdx < (uint8_t)gMaxBondedDevices_c)
    {
        if (pBondHeader != NULL)
        {
            FLib_MemCpy((void*)&bondEntries[mEntryIdx].aBondingHeader, pBondHeader, sizeof(bondEntries[mEntryIdx].aBondingHeader));
            bondEntries[mEntryIdx].nbRamWrite++;
            APP_DBG_LOG("pBondHeader");
        }

        if (pBondDataDynamic != NULL)
        {
            FLib_MemCpy((void*)&bondEntries[mEntryIdx].aBondingDataDynamic, pBondDataDynamic, sizeof(bondEntries[mEntryIdx].aBondingDataDynamic));
            bondEntries[mEntryIdx].nbRamWrite++;
            APP_DBG_LOG("pBondDataDynamic");
        }

        if (pBondDataStatic != NULL)
        {
            FLib_MemCpy((void*)&bondEntries[mEntryIdx].aBondingDataStatic, pBondDataStatic, sizeof(bondEntries[mEntryIdx].aBondingDataStatic));
            bondEntries[mEntryIdx].nbRamWrite++;
            APP_DBG_LOG("pBondDataStatic");
        }

        if (pBondDataDeviceInfo != NULL)
        {
            FLib_MemCpy((void*)&bondEntries[mEntryIdx].aBondingDataDeviceInfo, pBondDataDeviceInfo, sizeof(bondEntries[mEntryIdx].aBondingDataDeviceInfo));
            bondEntries[mEntryIdx].nbRamWrite++;
            APP_DBG_LOG("pBondDataDeviceInfo");
        }

        if (pBondDataDescriptor != NULL && mDescriptorIndex<gcGapMaximumSavedCccds_c)
        {
            FLib_MemCpy((void*)&bondEntries[mEntryIdx].aBondingDataDescriptor[mDescriptorIndex], pBondDataDescriptor, gBleBondDataDescriptorSize_c);
            bondEntries[mEntryIdx].nbRamWrite++;
            APP_DBG_LOG("pBondDataDescriptor");
        }
    }
    OSA_MutexUnlock(bondingMutex);
#else
    NOT_USED(mEntryIdx);
    NOT_USED(pBondHeader);
    NOT_USED(pBondDataDynamic);
    NOT_USED(pBondDataStatic);
    NOT_USED(pBondDataDeviceInfo);
    NOT_USED(pBondDataDescriptor);
    NOT_USED(mDescriptorIndex);
#endif
    APP_DBG_LOG("==> end");
}

/*! *********************************************************************************
* \brief  Performs NVM read operation
*
* \param[in] mEntryIdx            Bonded device index
* \param[in] pBondHeader          Bond Identity Header
* \param[in] pBondDataDynamic     Bond Data Dynamic
* \param[in] pBondDataStatic      Bond Data Static
* \param[in] pBondDataDeviceInfo  Bond Data Device Info
* \param[in] pBondDataDescriptor  Bond Data Descriptor
* \param[in] mDescriptorIndex     Descriptor Index
*
* \return  none
*
********************************************************************************** */
void App_NvmRead
(
    uint8_t  mEntryIdx,
    void*    pBondHeader,
    void*    pBondDataDynamic,
    void*    pBondDataStatic,
    void*    pBondDataDeviceInfo,
    void*    pBondDataDescriptor,
    uint8_t  mDescriptorIndex
)
{
#if (gMaxBondedDevices_c > 0)
    APP_DBG_LOG("EntryIdx=%d DescIdx=%d", mEntryIdx, mDescriptorIndex);
    OSA_MutexLock(bondingMutex, osaWaitForever_c);
    if(mEntryIdx < (uint8_t)gMaxBondedDevices_c)
    {
        if (pBondHeader != NULL)
        {
            FLib_MemCpy(pBondHeader, (void*)&bondEntries[mEntryIdx].aBondingHeader, sizeof(bondEntries[mEntryIdx].aBondingHeader));
        }

        if (pBondDataDynamic != NULL)
        {
            FLib_MemCpy(pBondDataDynamic, (void*)&bondEntries[mEntryIdx].aBondingDataDynamic, sizeof(bondEntries[mEntryIdx].aBondingDataDynamic));
        }

        if (pBondDataStatic != NULL)
        {
            FLib_MemCpy(pBondDataStatic, (void*)&bondEntries[mEntryIdx].aBondingDataStatic, sizeof(bondEntries[mEntryIdx].aBondingDataStatic));

        }

        if (pBondDataDeviceInfo != NULL)
        {
            FLib_MemCpy(pBondDataDeviceInfo, (void*)&bondEntries[mEntryIdx].aBondingDataDeviceInfo, sizeof(bondEntries[mEntryIdx].aBondingDataDeviceInfo));

        }

        if (pBondDataDescriptor != NULL && mDescriptorIndex<gcGapMaximumSavedCccds_c)
        {
            FLib_MemCpy(pBondDataDescriptor, (void*)&bondEntries[mEntryIdx].aBondingDataDescriptor[mDescriptorIndex], gBleBondDataDescriptorSize_c);
        }
    }
    OSA_MutexUnlock(bondingMutex);
#else
    NOT_USED(mEntryIdx);
    NOT_USED(pBondHeader);
    NOT_USED(pBondDataDynamic);
    NOT_USED(pBondDataStatic);
    NOT_USED(pBondDataDeviceInfo);
    NOT_USED(pBondDataDescriptor);
    NOT_USED(mDescriptorIndex);
#endif
    APP_DBG_LOG("==> end");
}

#if !defined(MULTICORE_CONNECTIVITY_CORE) || (!MULTICORE_CONNECTIVITY_CORE)

/*! *********************************************************************************
* \brief Sends the GAP Generic Event triggered by the Host Stack to the application
*
* \param[in] pGenericEvent Pointer to the generic event
*
* \return  none
*
********************************************************************************** */
void App_GenericCallback (gapGenericEvent_t* pGenericEvent)
{
    appMsgFromHost_t *pMsgIn = NULL;

    pMsgIn = MSG_Alloc(sizeof(uint32_t) + sizeof(gapGenericEvent_t));

    if (pMsgIn == NULL)
    {
        return;
    }

    pMsgIn->msgType = (uint32_t)gAppGapGenericMsg_c;
    FLib_MemCpy(&pMsgIn->msgData.genericMsg, pGenericEvent, sizeof(gapGenericEvent_t));

    /* Put message in the Host Stack to App queue */
    (void)MSG_Queue(&mHostAppInputQueue, pMsgIn);

    /* Signal application */
    (void)OSA_EventSet(mAppEvent, gAppEvtMsgFromHostStack_c);
}
#endif /* MULTICORE_CONNECTIVITY_CORE */

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

/*****************************************************************************
* Handles all key events for this device.
* Interface assumptions: None
* Return value: None
*****************************************************************************/
#if gKeyBoardSupported_d && (gKBD_KeysCount_c > 0) && !defined (DUAL_MODE_APP)
static void App_KeyboardCallBack
  (
  uint8_t events  /*IN: Events from keyboard module  */
  )
{
    BleApp_HandleKeys(events);
}
#endif

/*****************************************************************************
* Handles all messages received from the host task.
* Interface assumptions: None
* Return value: None
*****************************************************************************/
static void App_HandleHostMessageInput(appMsgFromHost_t* pMsg)
{
    switch ( pMsg->msgType )
    {
        case (uint32_t)gAppGapGenericMsg_c:
        {
            if (pfGenericCallback != NULL)
            {
                pfGenericCallback(&pMsg->msgData.genericMsg);
            }
            else
            {
                BleApp_GenericCallback(&pMsg->msgData.genericMsg);
            }
            break;
        }
        case (uint32_t)gAppGapAdvertisementMsg_c:
        {
            if (pfAdvCallback != NULL)
            {
                pfAdvCallback(&pMsg->msgData.advMsg);
            }
            break;
        }
        case (uint32_t)gAppGapScanMsg_c:
        {
            if (pfScanCallback != NULL)
            {
                pfScanCallback(&pMsg->msgData.scanMsg);
            }
            break;
        }
        case (uint32_t)gAppGapConnectionMsg_c:
        {
            if (pfConnCallback != NULL)
            {
                pfConnCallback(pMsg->msgData.connMsg.deviceId, &pMsg->msgData.connMsg.connEvent);
            }
            break;
        }
        case (uint32_t)gAppGattServerMsg_c:
        {
            if (pfGattServerCallback != NULL)
            {
                pfGattServerCallback(pMsg->msgData.gattServerMsg.deviceId, &pMsg->msgData.gattServerMsg.serverEvent);
            }
            break;
        }
        case (uint32_t)gAppGattClientProcedureMsg_c:
        {
            if (pfGattClientProcCallback != NULL)
            {
                pfGattClientProcCallback(
                    pMsg->msgData.gattClientProcMsg.deviceId,
                    pMsg->msgData.gattClientProcMsg.procedureType,
                    pMsg->msgData.gattClientProcMsg.procedureResult,
                    pMsg->msgData.gattClientProcMsg.error);
            }
            break;
        }
        case (uint32_t)gAppGattClientNotificationMsg_c:
        {
            if (pfGattClientNotifCallback != NULL)
            {
                pfGattClientNotifCallback(
                    pMsg->msgData.gattClientNotifIndMsg.deviceId,
                    pMsg->msgData.gattClientNotifIndMsg.characteristicValueHandle,
                    pMsg->msgData.gattClientNotifIndMsg.aValue,
                    pMsg->msgData.gattClientNotifIndMsg.valueLength);
            }
            break;
        }
        case (uint32_t)gAppGattClientIndicationMsg_c:
        {
            if (pfGattClientIndCallback != NULL)
            {
                pfGattClientIndCallback(
                    pMsg->msgData.gattClientNotifIndMsg.deviceId,
                    pMsg->msgData.gattClientNotifIndMsg.characteristicValueHandle,
                    pMsg->msgData.gattClientNotifIndMsg.aValue,
                    pMsg->msgData.gattClientNotifIndMsg.valueLength);
            }
            break;
        }
        case (uint32_t)gAppL2caLeDataMsg_c:
        {
            if (pfL2caLeCbDataCallback != NULL)
            {
                pfL2caLeCbDataCallback(
                    pMsg->msgData.l2caLeCbDataMsg.deviceId,
                    pMsg->msgData.l2caLeCbDataMsg.channelId,
                    pMsg->msgData.l2caLeCbDataMsg.aPacket,
                    pMsg->msgData.l2caLeCbDataMsg.packetLength);
            }
            break;
        }
        case (uint32_t)gAppL2caLeControlMsg_c:
        {
            if (pfL2caLeCbControlCallback != NULL)
            {
                pfL2caLeCbControlCallback(&pMsg->msgData.l2caLeCbControlMsg);
            }
            break;
        }
#if !(defined(FSL_FEATURE_SOC_CAU3_COUNT) && (FSL_FEATURE_SOC_CAU3_COUNT > 0))
        case (uint32_t)gAppSecLibMultiplyMsg_c:
        {
            computeDhKeyParam_t *pData = pMsg->msgData.secLibMsgData.pData;
            bool_t ready = SecLib_HandleMultiplyStep(pData);

            if(ready == FALSE)
            {
                SecLib_ExecMultiplicationCb(pData);
            }
            else
            {
                bleResult_t status;

                status = Gap_ResumeLeScStateMachine(pData);
                if (status != gBleSuccess_c)
                {
                    /* Not enough memory to resume LE SC operations */
                    panic(0, (uint32_t)Gap_ResumeLeScStateMachine, 0, 0);
                }
            }

            break;
        }
#endif
        default:
        {
            ; /* No action required */
            break;
        }
    }
}

/*! *********************************************************************************
* \brief Sends the GAP Connection Event triggered by the Host Stack to the application
*
* \param[in] peerDeviceId The id of the peer device
* \param[in] pConnectionEvent Pointer to the connection event
*
* \return  none
*
********************************************************************************** */
MULTICORE_STATIC void App_ConnectionCallback (deviceId_t peerDeviceId, gapConnectionEvent_t* pConnectionEvent)
{
#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1) && ((gFsciBleBBox_d == 1) || (gFsciBleTest_d == 1))
    fsciBleGapConnectionEvtMonitor(peerDeviceId, pConnectionEvent);
#else
    appMsgFromHost_t *pMsgIn = NULL;

    uint32_t msgLen = sizeof(uint32_t) + sizeof(connectionMsg_t);

    if(pConnectionEvent->eventType == gConnEvtKeysReceived_c)
    {
        gapSmpKeys_t    *pKeys = pConnectionEvent->eventData.keysReceivedEvent.pKeys;

        /* add room for pMsgIn->msgType */
        msgLen = sizeof(uint32_t);
        /* add room for pMsgIn->msgData.connMsg.deviceId */
        msgLen += sizeof(uint32_t);
        /* add room for pMsgIn->msgData.connMsg.connEvent.eventType */
        msgLen += sizeof(uint32_t);
        /* add room for pMsgIn->msgData.connMsg.connEvent.eventData.keysReceivedEvent.pKeys pointer */
        msgLen += sizeof(void*);
        /* add room for pMsgIn->msgData.connMsg.connEvent.eventData.keysReceivedEvent.pKeys data */
        msgLen += sizeof(gapSmpKeys_t);
        if (pKeys->aLtk != NULL)
        {
            /* add room for LTK and Rand data */
            msgLen += (uint32_t)pKeys->cLtkSize + (uint32_t)pKeys->cRandSize;
        }
        /* add room for IRK data */
        msgLen += (pKeys->aIrk != NULL) ? (gcSmpIrkSize_c + gcBleDeviceAddressSize_c) : 0U;
        /*  add room for CSRK data */
        msgLen += (pKeys->aCsrk != NULL) ? gcSmpCsrkSize_c : 0U;
        /* add room for device address data */
        msgLen += (pKeys->aAddress != NULL) ? (gcBleDeviceAddressSize_c) : 0U;
    }

    pMsgIn = MSG_Alloc(msgLen);

    if (pMsgIn == NULL)
    {
        return;
    }

    pMsgIn->msgType = (uint32_t)gAppGapConnectionMsg_c;
    pMsgIn->msgData.connMsg.deviceId = peerDeviceId;

    if(pConnectionEvent->eventType == gConnEvtKeysReceived_c)
    {
        union
        {
            uint8_t      *pu8;
            gapSmpKeys_t *pObject;
        } temp; /* MISRA rule 11.3 */

        gapSmpKeys_t    *pKeys = pConnectionEvent->eventData.keysReceivedEvent.pKeys;
        uint8_t         *pCursor = (uint8_t*)&pMsgIn->msgData.connMsg.connEvent.eventData.keysReceivedEvent.pKeys;

        pMsgIn->msgData.connMsg.connEvent.eventType = gConnEvtKeysReceived_c;
        pCursor += sizeof(void*); /* skip pKeys pointer */

        temp.pu8 = pCursor;
        pMsgIn->msgData.connMsg.connEvent.eventData.keysReceivedEvent.pKeys = temp.pObject;

        /* Copy SMP Keys structure */
        FLib_MemCpy(pCursor, pConnectionEvent->eventData.keysReceivedEvent.pKeys, sizeof(gapSmpKeys_t));
        pCursor += sizeof(gapSmpKeys_t);

        if (pKeys->aLtk != NULL)
        {
            /* Copy LTK */
            pMsgIn->msgData.connMsg.connEvent.eventData.keysReceivedEvent.pKeys->cLtkSize = pKeys->cLtkSize;
            pMsgIn->msgData.connMsg.connEvent.eventData.keysReceivedEvent.pKeys->aLtk = pCursor;
            FLib_MemCpy(pCursor, pKeys->aLtk, pKeys->cLtkSize);
            pCursor += pKeys->cLtkSize;

            /* Copy RAND */
            pMsgIn->msgData.connMsg.connEvent.eventData.keysReceivedEvent.pKeys->cRandSize = pKeys->cRandSize;
            pMsgIn->msgData.connMsg.connEvent.eventData.keysReceivedEvent.pKeys->aRand = pCursor;
            FLib_MemCpy(pCursor, pKeys->aRand, pKeys->cRandSize);
            pCursor += pKeys->cRandSize;
        }

        if (pKeys->aIrk != NULL)
        {
            /* Copy IRK */
            pMsgIn->msgData.connMsg.connEvent.eventData.keysReceivedEvent.pKeys->aIrk = pCursor;
            FLib_MemCpy(pCursor, pKeys->aIrk, gcSmpIrkSize_c);
            pCursor += gcSmpIrkSize_c;

            /* Copy Address*/
            pMsgIn->msgData.connMsg.connEvent.eventData.keysReceivedEvent.pKeys->addressType = pKeys->addressType;
            pMsgIn->msgData.connMsg.connEvent.eventData.keysReceivedEvent.pKeys->aAddress = pCursor;
            FLib_MemCpy(pCursor, pKeys->aAddress, gcBleDeviceAddressSize_c);
            pCursor += gcBleDeviceAddressSize_c;
        }

        if (pKeys->aCsrk != NULL)
        {
            /* Copy CSRK */
            pMsgIn->msgData.connMsg.connEvent.eventData.keysReceivedEvent.pKeys->aCsrk = pCursor;
            FLib_MemCpy(pCursor, pKeys->aCsrk, gcSmpCsrkSize_c);
        }
    }
    else
    {
        FLib_MemCpy(&pMsgIn->msgData.connMsg.connEvent, pConnectionEvent, sizeof(gapConnectionEvent_t));
    }

    /* Put message in the Host Stack to App queue */
    (void)MSG_Queue(&mHostAppInputQueue, pMsgIn);

    /* Signal application */
    (void)OSA_EventSet(mAppEvent, gAppEvtMsgFromHostStack_c);
#endif /* (MULTICORE_APPLICATION_CORE == 1) && (gFsciBleBBox_d == 1) */
}

/*! *********************************************************************************
* \brief Sends the GAP Advertising Event triggered by the Host Stack to the application
*
* \param[in] pAdvertisingEvent Pointer to the advertising event
*
* \return  none
*
********************************************************************************** */
MULTICORE_STATIC void App_AdvertisingCallback (gapAdvertisingEvent_t* pAdvertisingEvent)
{
#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1) && ((gFsciBleBBox_d == 1) || (gFsciBleTest_d == 1))
    fsciBleGapAdvertisingEvtMonitor(pAdvertisingEvent);
#else
    appMsgFromHost_t *pMsgIn = NULL;

    pMsgIn = MSG_Alloc(sizeof(uint32_t) + sizeof(gapAdvertisingEvent_t));

    if (pMsgIn == NULL)
    {
        return;
    }

    pMsgIn->msgType = (uint32_t)gAppGapAdvertisementMsg_c;
    pMsgIn->msgData.advMsg.eventType = pAdvertisingEvent->eventType;
    pMsgIn->msgData.advMsg.eventData = pAdvertisingEvent->eventData;

    /* Put message in the Host Stack to App queue */
    (void)MSG_Queue(&mHostAppInputQueue, pMsgIn);

    /* Signal application */
    (void)OSA_EventSet(mAppEvent, gAppEvtMsgFromHostStack_c);
#endif /* (MULTICORE_APPLICATION_CORE == 1) && (gFsciBleBBox_d == 1) */
}

/*! *********************************************************************************
* \brief Sends the GAP Scanning Event triggered by the Host Stack to the application
*
* \param[in] pScanningEvent Pointer to the scanning event
*
* \return  none
*
********************************************************************************** */
MULTICORE_STATIC void App_ScanningCallback (gapScanningEvent_t* pScanningEvent)
{
#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1) && ((gFsciBleBBox_d == 1) || (gFsciBleTest_d == 1))
    fsciBleGapScanningEvtMonitor(pScanningEvent);
#else
    appMsgFromHost_t *pMsgIn = NULL;

    uint32_t msgLen = sizeof(uint32_t) + sizeof(gapScanningEvent_t);

    if (pScanningEvent->eventType == gDeviceScanned_c)
    {
        msgLen += pScanningEvent->eventData.scannedDevice.dataLength;
    }
    else if (pScanningEvent->eventType == gExtDeviceScanned_c)
    {
        msgLen += pScanningEvent->eventData.extScannedDevice.dataLength;
    }
    else if (pScanningEvent->eventType == gPeriodicDeviceScanned_c)
    {
        msgLen += pScanningEvent->eventData.periodicScannedDevice.dataLength;
    }
    else
    {
        /* msgLen does not modify for all other event types */
    }

    pMsgIn = MSG_Alloc(msgLen);

    if (pMsgIn == NULL)
    {
        return;
    }

    pMsgIn->msgType = (uint32_t)gAppGapScanMsg_c;
    pMsgIn->msgData.scanMsg.eventType = pScanningEvent->eventType;

    if (pScanningEvent->eventType == gScanCommandFailed_c)
    {
        pMsgIn->msgData.scanMsg.eventData.failReason = pScanningEvent->eventData.failReason;
    }
    else if (pScanningEvent->eventType == gDeviceScanned_c)
    {
        FLib_MemCpy(&pMsgIn->msgData.scanMsg.eventData.scannedDevice,
                    &pScanningEvent->eventData.scannedDevice,
                    sizeof(pScanningEvent->eventData.scannedDevice));

        /* Copy data after the gapScanningEvent_t structure and update the data pointer*/
        pMsgIn->msgData.scanMsg.eventData.scannedDevice.data = (uint8_t*)&pMsgIn->msgData + sizeof(gapScanningEvent_t);
        FLib_MemCpy(pMsgIn->msgData.scanMsg.eventData.scannedDevice.data,
                    pScanningEvent->eventData.scannedDevice.data,
                    pScanningEvent->eventData.scannedDevice.dataLength);
    }
    else if (pScanningEvent->eventType == gExtDeviceScanned_c)
    {
        FLib_MemCpy(&pMsgIn->msgData.scanMsg.eventData.extScannedDevice,
                    &pScanningEvent->eventData.extScannedDevice,
                    sizeof(pScanningEvent->eventData.extScannedDevice));

        /* Copy data after the gapScanningEvent_t structure and update the data pointer*/
        pMsgIn->msgData.scanMsg.eventData.extScannedDevice.pData = (uint8_t*)&pMsgIn->msgData + sizeof(gapScanningEvent_t);
        FLib_MemCpy(pMsgIn->msgData.scanMsg.eventData.extScannedDevice.pData,
                    pScanningEvent->eventData.extScannedDevice.pData,
                    pScanningEvent->eventData.extScannedDevice.dataLength);
    }
    else if (pScanningEvent->eventType == gPeriodicDeviceScanned_c)
    {
        FLib_MemCpy(&pMsgIn->msgData.scanMsg.eventData.periodicScannedDevice,
                    &pScanningEvent->eventData.periodicScannedDevice,
                    sizeof(pScanningEvent->eventData.periodicScannedDevice));

        pMsgIn->msgData.scanMsg.eventData.periodicScannedDevice.pData = (uint8_t*)&pMsgIn->msgData + sizeof(gapScanningEvent_t);
        FLib_MemCpy(pMsgIn->msgData.scanMsg.eventData.periodicScannedDevice.pData,
                    pScanningEvent->eventData.periodicScannedDevice.pData,
                    pScanningEvent->eventData.periodicScannedDevice.dataLength);
    }
    else if (pScanningEvent->eventType == gPeriodicAdvSyncEstablished_c)
    {
        FLib_MemCpy(&pMsgIn->msgData.scanMsg.eventData.syncEstb,
                    &pScanningEvent->eventData.syncEstb,
                    sizeof(pScanningEvent->eventData.syncEstb));
    }
    else if (pScanningEvent->eventType == gPeriodicAdvSyncLost_c)
    {
        FLib_MemCpy(&pMsgIn->msgData.scanMsg.eventData.syncLost,
                    &pScanningEvent->eventData.syncLost,
                    sizeof(pScanningEvent->eventData.syncLost));
    }
    else
    {
        /* no action for all other event types */
    }

    /* Put message in the Host Stack to App queue */
    (void)MSG_Queue(&mHostAppInputQueue, pMsgIn);

    /* Signal application */
    (void)OSA_EventSet(mAppEvent, gAppEvtMsgFromHostStack_c);
#endif /* (MULTICORE_APPLICATION_CORE == 1) && (gFsciBleBBox_d == 1) */
}

/*! *********************************************************************************
* \brief Handles the server events received from one of the peer devices
*
* \param[in] peerDeviceId The id of the peer device
* \param[in] pServerEvent The Gatt server event received
*
* \return  none
*
********************************************************************************** */
MULTICORE_STATIC void App_GattServerCallback
(
    deviceId_t          peerDeviceId,
    gattServerEvent_t*  pServerEvent
)
{
#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1) && ((gFsciBleBBox_d == 1) )
    fsciBleGattServerEvtMonitor(peerDeviceId, pServerEvent);
#else
    appMsgFromHost_t *pMsgIn = NULL;
    uint32_t msgLen = sizeof(uint32_t) + sizeof(gattServerMsg_t);

    if (pServerEvent->eventType == gEvtAttributeWritten_c ||
        pServerEvent->eventType == gEvtAttributeWrittenWithoutResponse_c)
    {
        msgLen += pServerEvent->eventData.attributeWrittenEvent.cValueLength;
    }

    pMsgIn = MSG_Alloc(msgLen);

    if (pMsgIn == NULL)
    {
        return;
    }

    pMsgIn->msgType = (uint32_t)gAppGattServerMsg_c;
    pMsgIn->msgData.gattServerMsg.deviceId = peerDeviceId;
    FLib_MemCpy(&pMsgIn->msgData.gattServerMsg.serverEvent, pServerEvent, sizeof(gattServerEvent_t));

    if ((pMsgIn->msgData.gattServerMsg.serverEvent.eventType == gEvtAttributeWritten_c) ||
        (pMsgIn->msgData.gattServerMsg.serverEvent.eventType == gEvtAttributeWrittenWithoutResponse_c))
    {
        /* Copy value after the gattServerEvent_t structure and update the aValue pointer*/
        pMsgIn->msgData.gattServerMsg.serverEvent.eventData.attributeWrittenEvent.aValue =
          (uint8_t *)&pMsgIn->msgData.gattServerMsg.serverEvent.eventData.attributeWrittenEvent.aValue + sizeof(uint8_t*);
        FLib_MemCpy(pMsgIn->msgData.gattServerMsg.serverEvent.eventData.attributeWrittenEvent.aValue,
                    pServerEvent->eventData.attributeWrittenEvent.aValue,
                    pServerEvent->eventData.attributeWrittenEvent.cValueLength);

    }

    /* Put message in the Host Stack to App queue */
    (void)MSG_Queue(&mHostAppInputQueue, pMsgIn);

    /* Signal application */
    (void)OSA_EventSet(mAppEvent, gAppEvtMsgFromHostStack_c);
#endif /* (MULTICORE_APPLICATION_CORE == 1) && (gFsciBleBBox_d == 1) */
}

/*! *********************************************************************************
* \brief Handles a client procedure result reveived from a peer device
*
* \param[in] deviceId The id of the peer device
* \param[in] procedureType The received procedure type
* \param[in] procedureResult The received procedure result
* \param[in] error The received error
*
* \return  none
*
********************************************************************************** */
MULTICORE_STATIC void App_GattClientProcedureCallback
(
    deviceId_t              deviceId,
    gattProcedureType_t     procedureType,
    gattProcedureResult_t   procedureResult,
    bleResult_t             error)
{
#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1) && ((gFsciBleBBox_d == 1) )
    fsciBleGattClientProcedureEvtMonitor(deviceId, procedureType, procedureResult, error);
#else
    appMsgFromHost_t *pMsgIn = NULL;

    pMsgIn = MSG_Alloc(sizeof(uint32_t) + sizeof(gattClientProcMsg_t));

    if (pMsgIn == NULL)
    {
        return;
    }

    pMsgIn->msgType = (uint32_t)gAppGattClientProcedureMsg_c;
    pMsgIn->msgData.gattClientProcMsg.deviceId = deviceId;
    pMsgIn->msgData.gattClientProcMsg.procedureType = procedureType;
    pMsgIn->msgData.gattClientProcMsg.error = error;
    pMsgIn->msgData.gattClientProcMsg.procedureResult = procedureResult;

    /* Put message in the Host Stack to App queue */
    (void)MSG_Queue(&mHostAppInputQueue, pMsgIn);

    /* Signal application */
    (void)OSA_EventSet(mAppEvent, gAppEvtMsgFromHostStack_c);
#endif /* (MULTICORE_APPLICATION_CORE == 1) && (gFsciBleBBox_d == 1) */
}

/*! *********************************************************************************
* \brief Handles a notification received from a client
*
* \param[in] deviceId The id of the peer device
* \param[in] characteristicValueHandle The characteristic value handle
* \param[in] aValue The received value
* \param[in] valueLength The length of the value
*
* \return  none
*
********************************************************************************** */
MULTICORE_STATIC void App_GattClientNotificationCallback
(
    deviceId_t      deviceId,
    uint16_t        characteristicValueHandle,
    uint8_t*        aValue,
    uint16_t        valueLength
)
{
#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1) && ((gFsciBleBBox_d == 1) )
    fsciBleGattClientNotificationEvtMonitor(deviceId, characteristicValueHandle, aValue, valueLength);
#else
    appMsgFromHost_t *pMsgIn = NULL;

    /* Allocate a buffer with enough space to store also the notified value*/
    pMsgIn = MSG_Alloc(sizeof(uint32_t) + sizeof(gattClientNotifIndMsg_t) + (uint32_t)valueLength);

    if (pMsgIn == NULL)
    {
        return;
    }

    pMsgIn->msgType = (uint32_t)gAppGattClientNotificationMsg_c;
    pMsgIn->msgData.gattClientNotifIndMsg.deviceId = deviceId;
    pMsgIn->msgData.gattClientNotifIndMsg.characteristicValueHandle = characteristicValueHandle;
    pMsgIn->msgData.gattClientNotifIndMsg.valueLength = valueLength;

    /* Copy value after the gattClientNotifIndMsg_t structure and update the aValue pointer*/
    pMsgIn->msgData.gattClientNotifIndMsg.aValue = (uint8_t*)&pMsgIn->msgData + sizeof(gattClientNotifIndMsg_t);
    FLib_MemCpy(pMsgIn->msgData.gattClientNotifIndMsg.aValue, aValue, valueLength);

    /* Put message in the Host Stack to App queue */
    (void)MSG_Queue(&mHostAppInputQueue, pMsgIn);

    /* Signal application */
    (void)OSA_EventSet(mAppEvent, gAppEvtMsgFromHostStack_c);
#endif /* (MULTICORE_APPLICATION_CORE == 1) && (gFsciBleBBox_d == 1) */
}

/*! *********************************************************************************
* \brief Handles a indication received from a client
*
* \param[in] deviceId The id of the peer device
* \param[in] characteristicValueHandle The characteristic value handle
* \param[in] aValue The received value
* \param[in] valueLength The length of the value
*
* \return  none
*
********************************************************************************** */
MULTICORE_STATIC void App_GattClientIndicationCallback
(
    deviceId_t      deviceId,
    uint16_t        characteristicValueHandle,
    uint8_t*        aValue,
    uint16_t        valueLength
)
{
#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1) && ((gFsciBleBBox_d == 1) )
    fsciBleGattClientIndicationEvtMonitor(deviceId, characteristicValueHandle, aValue, valueLength);
#else
    appMsgFromHost_t *pMsgIn = NULL;

    /* Allocate a buffer with enough space to store also the notified value*/
    pMsgIn = MSG_Alloc(sizeof(uint32_t) + sizeof(gattClientNotifIndMsg_t)
                        + (uint32_t)valueLength);

    if (pMsgIn == NULL)
    {
        return;
    }

    pMsgIn->msgType = (uint32_t)gAppGattClientIndicationMsg_c;
    pMsgIn->msgData.gattClientNotifIndMsg.deviceId = deviceId;
    pMsgIn->msgData.gattClientNotifIndMsg.characteristicValueHandle = characteristicValueHandle;
    pMsgIn->msgData.gattClientNotifIndMsg.valueLength = valueLength;

    /* Copy value after the gattClientIndIndMsg_t structure and update the aValue pointer*/
    pMsgIn->msgData.gattClientNotifIndMsg.aValue = (uint8_t*)&pMsgIn->msgData + sizeof(gattClientNotifIndMsg_t);
    FLib_MemCpy(pMsgIn->msgData.gattClientNotifIndMsg.aValue, aValue, valueLength);

    /* Put message in the Host Stack to App queue */
    (void)MSG_Queue(&mHostAppInputQueue, pMsgIn);

    /* Signal application */
    (void)OSA_EventSet(mAppEvent, gAppEvtMsgFromHostStack_c);
#endif /* (MULTICORE_APPLICATION_CORE == 1) && (gFsciBleBBox_d == 1) */
}

/*! *********************************************************************************
* \brief L2cap data callback
*
* \param[in] deviceId     The id of the peer device
* \param[in] channelId    The id of the channel
* \param[in] pPacket      Pointer to the received packet
* \param[in] packetLength The length of the packet
*
* \return  none
*
********************************************************************************** */
MULTICORE_STATIC void App_L2caLeDataCallback
(
    deviceId_t deviceId,
    uint16_t channelId,
    uint8_t* pPacket,
    uint16_t packetLength
)
{
#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1) && ((gFsciBleBBox_d == 1) )
    fsciBleL2capCbLeCbDataEvtMonitor(deviceId, channelId, pPacket, packetLength);
#else
    appMsgFromHost_t *pMsgIn = NULL;

    /* Allocate a buffer with enough space to store the packet */
    pMsgIn = MSG_Alloc(sizeof(uint32_t) + (sizeof(l2caLeCbDataMsg_t) - 1U)
                        + (uint32_t)packetLength);

    if (pMsgIn == NULL)
    {
        return;
    }

    pMsgIn->msgType = (uint32_t)gAppL2caLeDataMsg_c;
    pMsgIn->msgData.l2caLeCbDataMsg.deviceId = deviceId;
    pMsgIn->msgData.l2caLeCbDataMsg.channelId = channelId;
    pMsgIn->msgData.l2caLeCbDataMsg.packetLength = packetLength;

    FLib_MemCpy(pMsgIn->msgData.l2caLeCbDataMsg.aPacket, pPacket, packetLength);

    /* Put message in the Host Stack to App queue */
    (void)MSG_Queue(&mHostAppInputQueue, pMsgIn);

    /* Signal application */
    (void)OSA_EventSet(mAppEvent, gAppEvtMsgFromHostStack_c);
#endif /* (MULTICORE_APPLICATION_CORE == 1) && (gFsciBleBBox_d == 1) */
}

/*! *********************************************************************************
* \brief L2Cap Control Callback
*
* \param[in] pMessage L2cap Control Message
*
* \return  none
*
********************************************************************************** */
MULTICORE_STATIC void App_L2caLeControlCallback
(
    l2capControlMessage_t* pMessage
)
{
#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1) && ((gFsciBleBBox_d == 1) )
    fsciBleL2capCbLeCbControlEvtMonitor(pMessage);
#else
    appMsgFromHost_t *pMsgIn = NULL;
    uint8_t messageLength = 0U;

    switch (pMessage->messageType) {
        case gL2ca_LePsmConnectRequest_c:
        {
            messageLength = sizeof(l2caLeCbConnectionRequest_t);
            break;
        }
        case gL2ca_LePsmConnectionComplete_c:
        {
            messageLength = sizeof(l2caLeCbConnectionComplete_t);
            break;
        }
        case gL2ca_LePsmDisconnectNotification_c:
        {
            messageLength = sizeof(l2caLeCbDisconnection_t);
            break;
        }
        case gL2ca_NoPeerCredits_c:
        {
            messageLength = sizeof(l2caLeCbNoPeerCredits_t);
            break;
        }
        case gL2ca_LocalCreditsNotification_c:
        {
            messageLength = sizeof(l2caLeCbLocalCreditsNotification_t);
            break;
        }
        case gL2ca_Error_c:
        {
            messageLength = sizeof(l2caLeCbError_t);
            break;
        }
        default:
        {
            messageLength = 0U;
            break;
        }
    }

    if (messageLength == 0U)
    {
        return;
    }

    /* Allocate a buffer with enough space to store the biggest packet */
    pMsgIn = MSG_Alloc(sizeof(uint32_t) + sizeof(l2capControlMessage_t));

    if (pMsgIn == NULL)
    {
          return;
    }

    pMsgIn->msgType = (uint32_t)gAppL2caLeControlMsg_c;
    pMsgIn->msgData.l2caLeCbControlMsg.messageType = pMessage->messageType;

    FLib_MemCpy(&pMsgIn->msgData.l2caLeCbControlMsg.messageData, &pMessage->messageData, messageLength);

    /* Put message in the Host Stack to App queue */
    (void)MSG_Queue(&mHostAppInputQueue, pMsgIn);

    /* Signal application */
    (void)OSA_EventSet(mAppEvent, gAppEvtMsgFromHostStack_c);
#endif /* (MULTICORE_APPLICATION_CORE == 1) && (gFsciBleBBox_d == 1) */
}

/*! *********************************************************************************
* \brief SecLib Callback
*
* \param[in] pData SecLib message data
*
* \return  none
*
********************************************************************************** */
MULTICORE_STATIC void App_SecLibMultCallback
(
    computeDhKeyParam_t *pData
)
{
    appMsgFromHost_t *pMsgIn = NULL;

    /* Allocate a buffer with enough space to store also the notified value*/
    pMsgIn = MSG_Alloc(sizeof(uint32_t) + sizeof(secLibMsgData_t));

    if (pMsgIn == NULL)
    {
        return;
    }

    pMsgIn->msgType = (uint32_t)gAppSecLibMultiplyMsg_c;
    pMsgIn->msgData.secLibMsgData.pData = pData;

    /* Put message in the Host Stack to App queue */
    (void)MSG_Queue(&mHostAppInputQueue, pMsgIn);

    /* Signal application */
    (void)OSA_EventSet(mAppEvent, gAppEvtMsgFromHostStack_c);
}
#if !defined (SOTA_ENABLED) && !defined (DUAL_MODE_APP)
#if !defined(gUseHciTransportDownward_d) || (!gUseHciTransportDownward_d)

/*! *********************************************************************************
* \brief Notifies the application to deny sleep
*
* \param[in] none
*
* \return  none
*
* Called by BLE when a connect is received
*
********************************************************************************** */
static void BLE_SignalFromISRCallback(void)
{
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
#if (!defined(CPU_MKW37A512VFT4) && !defined(CPU_MKW37Z512VFT4) && !defined(CPU_MKW38A512VFT4) && !defined(CPU_MKW38Z512VFT4) && !defined(CPU_MKW39A512VFT4) && !defined(CPU_MKW39Z512VFT4))
    PWR_DisallowDeviceToSleep();
#endif /* CPU_MKW37xxx, CPU_MKW38xxx and CPU_MKW39xxx*/
#endif /* cPWR_UsePowerDownMode */
}
#endif /* !gUseHciTransportDownward_d */


/*! *********************************************************************************
* \brief  This function will try to put the MCU into a deep sleep mode for at most
*         the maximum OS idle time specified. Else the MCU will enter a sleep mode
*         until the first IRQ.
*
* \param[in]  xExpectedIdleTime  The idle time in OS ticks
*
* \remarks  This feature is available only for FreeRTOS.
*           This function will replace the default implementation from
*           fsl_tickless_systick.c which is defined as weak.
*
********************************************************************************** */
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode) && (configUSE_TICKLESS_IDLE != 0)
#ifndef CPU_JN518X
void vPortSuppressTicksAndSleep( TickType_t xExpectedIdleTime )
{
    PWRLib_WakeupReason_t wakeupReason;

    if( PWR_CheckIfDeviceCanGoToSleep() && (xExpectedIdleTime > mAppOverheadTicks_c))
    {
        /* Set deep-sleep duration. Compensate for the application processing overhead time */
        PWR_SetDeepSleepTimeInMs((xExpectedIdleTime - mAppOverheadTicks_c) * portTICK_PERIOD_MS);
        PWR_ResetTotalSleepDuration();

        /* Enter Low Power */
        wakeupReason = PWR_EnterLowPower();
        (void)wakeupReason;
/* On KW37, KW38, KW39 series, key handle is processed directly in the lowpower FWK module */
#if (!defined(KW37A4_SERIES) && !defined(KW37Z4_SERIES) && !defined(KW38A4_SERIES) && !defined(KW38Z4_SERIES) && !defined(KW39A4_SERIES))
#if gKBD_KeysCount_c > 0
        /* Woke up on Keyboard Press */
        if(wakeupReason.Bits.FromKeyBoard != 0U)
        {
#if defined(cPWR_EnableDeepSleepMode_8) && (cPWR_EnableDeepSleepMode_8)
            /* Skip over the key scan timer to improve low power consumption. */
            BleApp_HandleKeys(gKBD_EventPressPB1_c);
#else
            KBD_SwitchPressedOnWakeUp();
#endif
        }
#endif
#endif
        /* Get actual deep sleep time, and converted to OS ticks */
        xExpectedIdleTime = PWR_GetTotalSleepDurationMS() / portTICK_PERIOD_MS;

        portENTER_CRITICAL();
        /* Update the OS time ticks. */
        vTaskStepTick( xExpectedIdleTime );
        portEXIT_CRITICAL();
    }
    else
    {
        /* Enter MCU Sleep */
        PWR_EnterSleep();
    }
}
#else   /* CPU_JN518X */
typedef struct {
    uint32_t initial_count;
    bool next_rtcIrq_not_forTMR;
} lp_tickless_rtc_t;

static int32_t tickless_PrePowerDownRtcWakeSet(uint32_t time_ms, lp_tickless_rtc_t * p_lp_ctx)
{
    int32_t tmrMgrExpiryTimeMs = -1; /* no TMR running on account of TimerManager */

    if (time_ms > 0xffff) time_ms = 0xffff;

#if defined  gTimerMgrLowPowerTimers && ( gTimerMgrLowPowerTimers > 0)
    /* TMR_MGR: Get next timer manager expiry time if any */
    /* OSA_InterruptDisable(); useless since called from masked section already */
    /* If gTimerMgrLowPowerTimers is defined it must be gTimerMgrUseLpcRtc_c on current
       QN9090, K32W061, JN5189 chips */
    uint32_t rtc_ctrl = RTC->CTRL & (RTC_CTRL_RTC1KHZ_EN_MASK | RTC_CTRL_RTC_EN_MASK) ;
    if (rtc_ctrl == (RTC_CTRL_RTC1KHZ_EN_MASK | RTC_CTRL_RTC_EN_MASK))
    {
        tmrMgrExpiryTimeMs = (int32_t)RTC_GetWakeupCount(RTC);
    }
    /* OSA_InterruptEnable(); ditto */
#endif
    /* TMR_MGR: Update RTC Threshold only if RTOS needs to wakeup earlier */
    if((int32_t)time_ms < tmrMgrExpiryTimeMs || tmrMgrExpiryTimeMs <= 0)
    {
        PWR_SetDeepSleepTimeInMs(time_ms);
        p_lp_ctx->initial_count = time_ms;
        p_lp_ctx->next_rtcIrq_not_forTMR = true;
    }
    else
    {
        p_lp_ctx->initial_count = tmrMgrExpiryTimeMs;
        p_lp_ctx->next_rtcIrq_not_forTMR = false;
    }
    PWR_DBG_LOG("time_ms=%d tmrMgrExpiryTimeMs=%d", time_ms, tmrMgrExpiryTimeMs);

    return tmrMgrExpiryTimeMs;
}

static void tickless_RTCReprogramDeadline(int32_t programmed_ms, uint32_t sleep_duration)
{
    programmed_ms -= sleep_duration;
    if (programmed_ms > 0)
    {
        PWR_DBG_LOG("Reprogram TMR DelayMs=%d ", programmed_ms);
        RTC_SetWakeupCount(RTC, (uint16_t)programmed_ms);
    }
    else
    {
        /* Timer Manager has a deadline too close to RTOS tick
         * Let RTC interrupt fire since time already elapsed
         */
        PWR_DBG_LOG("late expiry");
    }
}
void vPortSuppressTicksAndSleep( TickType_t xExpectedIdleTime )
{
    /* If the RTOS is expecting a wake-up too close to the current date simply
     * go to Sleep. Same goes if we already know we won't go to Power Down  */
    if( PWR_CheckIfDeviceCanGoToSleep() && (xExpectedIdleTime > mAppOverheadTicks_c))
    {
        PWR_WakeupReason_t wakeupReason;
        int32_t tmrMgrExpiryTimeMs;
        uint32_t sleep_duration_ms;
        xExpectedIdleTime -= mAppOverheadTicks_c;
        lp_tickless_rtc_t lp_rtc_ctx;
        tmrMgrExpiryTimeMs = tickless_PrePowerDownRtcWakeSet(xExpectedIdleTime * portTICK_PERIOD_MS,
                                                             &lp_rtc_ctx);
        wakeupReason = PWR_EnterLowPower();
        PWR_DBG_LOG("wakeReason=%x", (uint16_t)wakeupReason.AllBits);
        sleep_duration_ms = PWR_GetTotalSleepDurationMs(lp_rtc_ctx.initial_count);

        if (wakeupReason.AllBits != 0UL)
        {
            xExpectedIdleTime = sleep_duration_ms / portTICK_PERIOD_MS;

            if (lp_rtc_ctx.next_rtcIrq_not_forTMR)
            {
                if (NVIC_GetPendingIRQ(RTC_IRQn))
                {
                    RTC_ClearStatusFlags(RTC, kRTC_WakeupFlag);
                    NVIC_ClearPendingIRQ(RTC_IRQn);
                }
                /*
                 * If we have not changed the RTC wake for the tickless mode,
                 * let the counter live as is it has kept counting down
                 */
                if (tmrMgrExpiryTimeMs != -1)
                {
                    tickless_RTCReprogramDeadline(tmrMgrExpiryTimeMs, sleep_duration_ms );
                }
            }

            TMR_ReInit();

            /* Restart SysTick so it runs from portNVIC_SYSTICK_LOAD_REG
               again, then set portNVIC_SYSTICK_LOAD_REG back to its standard
               value. The critical section is used to ensure the tick interrupt
               can only execute once in the case that the reload register is near
               zero. */
            /* Systick->VAL and LOAD : already restored by Restore_CM4_registers*/
            portENTER_CRITICAL();
            SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; /* Systick restart now */
            /* Update the OS time ticks. */
            vTaskStepTick( xExpectedIdleTime );
            //SysTick->LOAD = (ulTimerCountsForOneTick - 1UL)  & SysTick_LOAD_RELOAD_Msk;
            portEXIT_CRITICAL();
            PWR_DBG_LOG("SysTick=%x", OSA_TimeGetMsec());

        }
        else
        {
            /* Didn't actually sleep, some activity prevented it  */
     #if defined gTimerMgrLowPowerTimers && (gTimerMgrLowPowerTimers > 0)
            if (tmrMgrExpiryTimeMs != -1)
            {
                tickless_RTCReprogramDeadline (tmrMgrExpiryTimeMs, sleep_duration_ms );
            }
     #endif
        }
    } /* PWR_CheckIfDeviceCanGoToSleep() && (xExpectedIdleTime > mAppOverheadTicks_c) */
    else
    {
        /* We already know we can't power down: Enter MCU Sleep */
        PWR_EnterSleep();
    }
}
#endif /* CPU_JN518X */
#endif /*  (cPWR_UsePowerDownMode) && (configUSE_TICKLESS_IDLE != 0) */


#endif /* SOTA_ENABLED */

void vApplicationStackOverflowHook( void * xTask, char *pcTaskName )
{
    PRINTF("Stack Overflow detected in %s\r\n", pcTaskName);
    while (1);
}
