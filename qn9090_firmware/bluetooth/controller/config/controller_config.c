/*!
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2017 NXP
*
* \file
*
* This file contains configuration data for the application and stack
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* o Redistributions of source code must retain the above copyright notice, this list
*   of conditions and the following disclaimer.
*
* o Redistributions in binary form must reproduce the above copyright notice, this
*   list of conditions and the following disclaimer in the documentation and/or
*   other materials provided with the distribution.
*
* o Neither the name of Freescale Semiconductor, Inc. nor the names of its
*   contributors may be used to endorse or promote products derived from this
*   software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/

#include "fsl_os_abstraction.h"
#include "fsl_debug_console.h"
#include "fsl_reset.h"
#include "fsl_xcvr.h"
#if defined (gMWS_UseCoexistence_d) && (gMWS_UseCoexistence_d)
#include "MWS.h"
#endif
#include "board.h"
#include "Panic.h"
#include "RNG_Interface.h"
#include "PWR_Interface.h"
#include "ble_controller_task_config.h"
#include "qnble_config.h"
#include "ble_general.h"
#include "controller_interface.h"
#if gBleControllerLocalNameSupport
#include "PDM.h"
#include "FunctionLib.h"
#endif

#if defined (DUAL_MODE_APP) && (defined (gEnableBleInactivityTimeNotify) && gEnableBleInactivityTimeNotify == 1)
#define MAC_DYNAMIC_SUPPORT
#endif

#ifdef MAC_DYNAMIC_SUPPORT
/* MAC include */
#include "MacDynamic.h"
#endif

#if gStressTesting_d
#include "fsl_dma.h"
#endif

/*! *********************************************************************************
*************************************************************************************
* Private constant & macros
*************************************************************************************
********************************************************************************** */

#define MEM_ALIGN(len)          (((len) + (sizeof(uint32_t) - 1)) / sizeof(uint32_t))

/*! @brief Exchange memory definitions from _reg_ble_em_*.h */
#define REG_BLE_EM_CS_SIZE              (90)
#define REG_BLE_EM_WPB_SIZE             (6)
#define REG_BLE_EM_WPV_SIZE             (6)
#define REG_BLE_EM_RAL_SIZE             (52)
#define REG_BLE_EM_TX_DESC_SIZE         (8)
#define REG_BLE_EM_RX_DESC_SIZE         (12)
#define REG_BLE_EM_TX_BUFFER_CNTL_SIZE  (38)

/*! @brief Exchange memory common size before Control Structures */
#define EM_FT_OFFSET                    (416)   // Offset of RF frequency table (0x1A0)
#define EM_FT_SIZE                      (40)    // Frequency table size
#define EM_CIPHER_SIZE                  (2*16)  // Ciphering size
#define EM_COMMON_SIZE                  (EM_FT_OFFSET + EM_FT_SIZE + EM_CIPHER_SIZE)
#define EM_BLE_CS_OFFSET                (EM_COMMON_SIZE)

/// Number of Control Structures
#define CFG_BLE_CS_COUNT                (CFG_CON_DEV_MAX + 1)
/// Number of TX data buffer
#define CFG_BLE_TX_BUFF_DATA_COUNT      (CFG_CON_DEV_MAX)
/// Number of TX advertising buffer
#define CFG_BLE_TX_BUFF_ADV_COUNT       (3) // Worst case (1 for CONNECT_REQ, 1 for ADV_DATA and 1 for SCAN_RESP_DATA)
/// Number of TX control buffer
#define CFG_BLE_TX_BUFF_CNTL_COUNT      (CFG_CON_DEV_MAX) // Worst case (1 dedicated packet by link)
/// Number of elements in the TX Descriptor pool
// Dedicated for DATA
// Worst case (1 packets (251 bytes) fragmented into 27 byte =  10)
#define CFG_BLE_TX_DESC_DATA_COUNT      ((CFG_BLE_TX_BUFF_DATA_COUNT) * 10)
// Dedicated for CONTROL
#define CFG_BLE_TX_DESC_CNTL_COUNT      (CFG_BLE_TX_BUFF_CNTL_COUNT)
// Dedicated for ADVERTISING
#define CFG_BLE_TX_DESC_ADV_COUNT       (CFG_BLE_TX_BUFF_ADV_COUNT)
/// Number of TX Buffers
#define CFG_BLE_TX_BUFFER_COUNT         (CFG_BLE_TX_BUFF_DATA_COUNT)
/// Total number of elements in the TX Descriptor pool
#define CFG_BLE_TX_DESC_COUNT           (CFG_BLE_TX_DESC_CNTL_COUNT + CFG_BLE_TX_DESC_ADV_COUNT + CFG_BLE_TX_DESC_DATA_COUNT)
/// Number of RX Buffers
#define CFG_BLE_RX_BUFFER_COUNT         (8)
/// Number of RX Descriptors
#define CFG_BLE_RX_DESC_COUNT           (CFG_BLE_RX_BUFFER_COUNT)


/*! @brief Offset of the public white list area */
#define APP_EM_BLE_WPB_OFFSET            (EM_BLE_CS_OFFSET + CFG_BLE_CS_COUNT * REG_BLE_EM_CS_SIZE)
/*! @brief Offset of the private white list area */
#define APP_EM_BLE_WPV_OFFSET            (APP_EM_BLE_WPB_OFFSET + CFG_BLE_WHITELIST_MAX * REG_BLE_EM_WPB_SIZE)
/*! @brief Offset of the private white list area */
#define APP_EM_BLE_RAL_OFFSET            (APP_EM_BLE_WPV_OFFSET + CFG_BLE_WHITELIST_MAX * REG_BLE_EM_WPV_SIZE)
/*! @brief Offset of the TX descriptor area */
#define APP_EM_BLE_TX_DESC_OFFSET        (APP_EM_BLE_RAL_OFFSET + CFG_BLE_RESOL_ADDR_LIST_MAX * REG_BLE_EM_RAL_SIZE)
/*! @brief Offset of the RX descriptor area */
#define APP_EM_BLE_RX_DESC_OFFSET        (APP_EM_BLE_TX_DESC_OFFSET + CFG_BLE_TX_DESC_COUNT * REG_BLE_EM_TX_DESC_SIZE)
/*! @brief Offset of the TX buffer area */
#define APP_EM_BLE_TX_BUFFER_CNTL_OFFSET (APP_EM_BLE_RX_DESC_OFFSET + CFG_BLE_RX_DESC_COUNT * REG_BLE_EM_RX_DESC_SIZE)
/*! @brief Offset of the TX buffer area */
#define APP_EM_BLE_TX_BUFFER_DATA_OFFSET (APP_EM_BLE_TX_BUFFER_CNTL_OFFSET + (CFG_BLE_TX_BUFF_CNTL_COUNT + CFG_BLE_TX_BUFF_ADV_COUNT) * REG_BLE_EM_TX_BUFFER_CNTL_SIZE)
/*! @brief Offset of the RX buffer area */
#define APP_EM_BLE_RX_BUFFER_OFFSET      (APP_EM_BLE_TX_BUFFER_DATA_OFFSET + CFG_BLE_TX_BUFF_DATA_COUNT * CFG_REG_BLE_EM_TX_BUFFER_DATA_SIZE)
/*! @brief End of BLE EM */
#define APP_EM_BLE_END                   (APP_EM_BLE_RX_BUFFER_OFFSET + CFG_BLE_RX_BUFFER_COUNT * CFG_REG_BLE_EM_RX_BUFFER_SIZE)


#define CFG_API_FLAG_HCI                (0x01)
#define CFG_API_FLAG_FAST_CORRECT       (0x08)
#define CFG_API_FLAG_DRIFT_FREQ         (0x10)

/* Sizing of heap for Environment variables */
#define BLE_HEAP_ENV_CONSTANT           (324)
#define BLE_HEAP_ENV_NB_CNX_FACTOR      (620)
#define BLE_HEAP_ENV_NB_RX_BUF_FACTOR   (16)

#define RWIP_HEAP_ENV_SIZE              (BLE_HEAP_ENV_CONSTANT                                  \
                                       + BLE_HEAP_ENV_NB_CNX_FACTOR    * CFG_CON_DEV_MAX        \
                                       + BLE_HEAP_ENV_NB_RX_BUF_FACTOR * CFG_BLE_RX_BUFFER_COUNT)

/* Sizing of heap for kernel messages */
/** Size of the heap
 * - For KE messages: (N+1) x 256
 * - For LLC environment: N x 80 Bytes
 * - For LLD events/intervals: (2N+1) x (80 + 16)
 */
#define RWIP_HEAP_MSG_SIZE              (((CFG_CON_DEV_MAX + 1) * 256)                           \
                                       + ((CFG_CON_DEV_MAX) * 80)                                \
                                       + ((CFG_CON_DEV_MAX * 2 + 1) * (80 + 16)))

/* Sizing of heap for Non retention memory */
#if defined(CFG_HEAP_NON_RET_SIZE)
#define RWIP_HEAP_NON_RET_SIZE          CFG_HEAP_NON_RET_SIZE
#else
#define RWIP_HEAP_NON_RET_SIZE          (( 512 * CFG_CON_DEV_MAX ) + 4096)
#endif

// Heap header size is 12 bytes
#define RWIP_HEAP_HEADER                (12 / sizeof(uint32_t))
#define RWIP_MEM_ALIGN(len)              (((len) + (sizeof(uint32_t) - 1)) / sizeof(uint32_t))
#define RWIP_CALC_HEAP_LEN(len)          (RWIP_MEM_ALIGN(len) + RWIP_HEAP_HEADER)

#if gStressTesting_d
#define DMA_COPY_SIZE    4000
#endif

/* Current valid configuration uses the values below */
#define APP_WAKEUP_DELAY                 (27) /* Optimized value , used to be 20 */
#define APP_WAKEUP_ADVANCE               (2)  /* Optimized value , used to be 3 */


#define WEAK __attribute__((weak))


#ifdef  gBleControllerLocalNameSupport
#define pdmId_LocalDeviceData 0x4010U
#define MAX_LOCAL_NAME_SIZE 248
#define CLASS_OF_DEV_SZ 3
typedef struct
{
    uint8_t local_name[MAX_LOCAL_NAME_SIZE];
    /* Other fields may need to be added */
    uint8_t class_of_device[CLASS_OF_DEV_SZ];
    uint8_t padding[512 - MAX_LOCAL_NAME_SIZE - CLASS_OF_DEV_SZ];

} LocalDeviceData_t;


extern WEAK uint32_t PdmReadLocalName(uint8_t name[MAX_LOCAL_NAME_SIZE]);
extern WEAK uint32_t PdmWriteLocalName(const uint8_t *name);
extern WEAK uint32_t PdmReadClassOfDevice(uint8_t class[CLASS_OF_DEV_SZ]);
extern WEAK uint32_t PdmWriteClassOfDevice(const uint8_t *class);
#endif

#if (gEnableBleInactivityTimeNotify == 1)
extern WEAK void BleAppInactivityCallback(uint32_t);
extern WEAK int BleAppNewActivityCallback(uint32_t);
extern WEAK void BleAppWakeupEndCallback(void);
#endif

#ifdef MAC_DYNAMIC_SUPPORT
extern WEAK bool_t BleAppIsDynSlaveProtocolActive(void);
#endif

#ifdef OPERATION_TEST_SUPPORT
extern int OperationTest_Prepare(uint8_t operationType);
extern int OperationTest_Process();
extern int OperationTest_Stop();
#endif

/*! *********************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
********************************************************************************** */

/*! *********************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
********************************************************************************** */

#if gStressTesting_d
static void dma_test_start(void);
#endif

#if defined (gMWS_UseCoexistence_d) && (gMWS_UseCoexistence_d)
uint32_t BleCoexRegister(void *callback)
{
    return (mwsStatus_t) MWS_CoexistenceRegister(gMWS_BLE_c, (pfMwsCallback) callback);
}

uint32_t BleCoexRequestAccess(uint32_t rfState)
{
    return (mwsStatus_t) MWS_CoexistenceRequestAccess((mwsRfState_t) rfState);
}

uint32_t BleCoexChangeAccess(uint32_t rfState)
{
    return (mwsStatus_t) MWS_CoexistenceChangeAccess((mwsRfState_t) rfState);
}

void BleCoexReleaseAccess(void)
{
    return MWS_CoexistenceReleaseAccess();
}

void BleCoexEnable(void)
{
    return MWS_CoexistenceEnable();
}

void BleCoexDisable(void)
{
    return MWS_CoexistenceDisable();
}
#endif

/*! *********************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
********************************************************************************** */
/* Controller retention memory */
struct unloaded_area_tag sUnloadedArea __attribute__ ((section (".bss.dontinitinboot")));

/* Controller exchange memory */
uint32_t rwip_exchange_memory[MEM_ALIGN(APP_EM_BLE_END)]  __attribute__ ((section (".ll_exchange_mem")));

/* Controller heap memory */
uint32_t rwip_heap_env[RWIP_CALC_HEAP_LEN(RWIP_HEAP_ENV_SIZE)];         /* Memory allocated for environment variables */
uint32_t rwip_heap_msg[RWIP_CALC_HEAP_LEN(RWIP_HEAP_MSG_SIZE)];         /* Memory allocated for kernel messages */

#if defined(__IAR_SYSTEMS_ICC__)
//#pragma location = ".s_start_non_ret"
uint32_t rwip_heap_non_ret[RWIP_CALC_HEAP_LEN(RWIP_HEAP_NON_RET_SIZE)]; /* Non Retention memory block */
#else
uint32_t  rwip_heap_non_ret[RWIP_CALC_HEAP_LEN(RWIP_HEAP_NON_RET_SIZE)]; //__attribute__ ((section (".s_start_non_ret")));/* Non Retention memory block */
#endif
/* Defines the place holder for the states of all the task instances. */
#if (CFG_CON_DEV_MAX > 0)
uint8_t llc_state[CFG_CON_DEV_MAX];
#endif

const struct app_cfg app_configuration = {
     // Application callbacks
    .unloaded_area                  = (struct unloaded_area_tag *)&sUnloadedArea,
    .plf_reset_cb                   = RESET_SystemReset,
    .plf_panic_cb                   = panic,
    .get_ble_mac_addr               = BOARD_GetMCUUid,
    .get_temperature                = BOARD_GetTemperature,
    .hci_user_ext_func              = NULL,
    .hci_common_callback            = NULL,
    .app_pkt_statistic              = NULL,

    // Diagnostics and Debug
#if (gDbgIoCfg_c == 1)
    .IoDbgMode                      = 1,
    .IoSet                          = BOARD_DbgIoSet,
#elif (gDbgIoCfg_c == 2)
    .IoDbgMode                      = 2,
    .IoSet                          = BOARD_DbgIoSet,
#else
    .IoDbgMode                      = 0,
    .IoSet = NULL,
#endif
#if gDbgUseLLDiagPort
        .DiagEna                    = BOARD_DbgDiagEnable,
#else
        .DiagEna                    = NULL,
#endif
#if (gEnableBleInactivityTimeNotify == 1)
    .bleInactivityCallback          = BleAppInactivityCallback,
    .bleNewActivityCallback         = BleAppNewActivityCallback,
    .bleWakeupEndCallback           = BleAppWakeupEndCallback,
#else
    .bleInactivityCallback          = NULL,
    .bleNewActivityCallback         = NULL,
    .bleWakeupEndCallback           = NULL,
#endif
#ifdef MAC_DYNAMIC_SUPPORT
    .bleIsDynSlaveProtocolActive    = BleAppIsDynSlaveProtocolActive,
#else
    .bleIsDynSlaveProtocolActive    = NULL,
#endif
#ifdef OPERATION_TEST_SUPPORT
    .prepareOperation               = OperationTest_Prepare,
    .processOperation               = OperationTest_Process,
    .stopOperation                  = OperationTest_Stop
#else
    .prepareOperation               = NULL,
    .processOperation               = NULL,
    .stopOperation                  = NULL
#endif
};

const struct fw_cfg firmware_configuration = {
    // Ble config
    .ble_con_max                    = CFG_CON_DEV_MAX,
    .ble_whitelist_max              = CFG_BLE_WHITELIST_MAX,
    .ble_resol_addr_list_max        = CFG_BLE_RESOL_ADDR_LIST_MAX,
    .ble_duplicate_filter_max       = CFG_BLE_DUPLICATE_FILTER_MAX,
    .prog_latency_def               = CFG_BLE_PROG_LATENCY_DFT,
    .lld_util_min_instant_con_evt   = CFG_LLD_UTIL_MIN_INSTANT_CON_EVT,
    .min_instant_con_evt            = CFG_MIN_INSTANT_CON_EVT,
#if defined (gPWR_AdvertisingInterSlotInt)
    .adv_pdu_int                    = gPWR_AdvertisingInterSlotInt,
#else
    .adv_pdu_int                    = CFG_ADV_PDU_INT,
#endif
    #if (CFG_CON_DEV_MAX > 0)
    .llc_state                      = llc_state,
    #else
    .llc_state                      = NULL,
    #endif

    // Heap Memory
    .rwip_heap_env                  = rwip_heap_env,
    .rwip_heap_msg                  = rwip_heap_msg,
    .rwip_heap_non_ret              = rwip_heap_non_ret,
    .rwip_heap_env_size             = RWIP_HEAP_ENV_SIZE,
    .rwip_heap_msg_size             = RWIP_HEAP_MSG_SIZE,
    .rwip_heap_non_ret_size         = RWIP_HEAP_NON_RET_SIZE,

    // Exchange Memory
    .em_ble_start                   = (uint32_t)&rwip_exchange_memory[0],
    .em_ble_wpb_offset              = APP_EM_BLE_WPB_OFFSET,
    .em_ble_wpv_offset              = APP_EM_BLE_WPV_OFFSET,
    .em_ble_ral_offset              = APP_EM_BLE_RAL_OFFSET,
    .em_ble_tx_desc_offset          = APP_EM_BLE_TX_DESC_OFFSET,
    .em_ble_rx_desc_offset          = APP_EM_BLE_RX_DESC_OFFSET,
    .em_ble_tx_buffer_ctrl_offset   = APP_EM_BLE_TX_BUFFER_CNTL_OFFSET,
    .em_ble_tx_buffer_data_offset   = APP_EM_BLE_TX_BUFFER_DATA_OFFSET,
    .em_ble_rx_buffer_offset        = APP_EM_BLE_RX_BUFFER_OFFSET,
    .em_ble_tx_buf_data_cnt         = CFG_BLE_TX_BUFF_DATA_COUNT,
    .em_ble_rx_buffer_size          = CFG_REG_BLE_EM_RX_BUFFER_SIZE,
    .em_ble_rx_buffer_cnt           = CFG_BLE_RX_BUFFER_COUNT,
    .em_ble_end                     = APP_EM_BLE_END,

    /* Interrupt mask in GLOBAL_INT_DISABLE() */
    .int_mask0 = 0xFFFFFFFF,
    .int_mask1 = 0xFFFFFFFF,

    /* Define Controller behaviour on reception of invalid PDU */
#if defined gLlInvalidPduHandlingType_c
    .invalid_pdu_handling = 1,
#else
    .invalid_pdu_handling = 0,
#endif
};

struct fwk_cfg framework_configuration = {
    .lp_cfg = {
            .wakeup_delay               = APP_WAKEUP_DELAY,   /*!< Number of 32k clock ticks */
            .wakeup_advance             = APP_WAKEUP_ADVANCE, /*!< Number of 625us slots the wake up must be anticipated before the actual activity */
            .timebase_compensate        = 0,                  /*!< Duration in microseconds used to offset computation */
            #if (defined(BOARD_XTAL1_CLK_HZ) && (BOARD_XTAL1_CLK_HZ == CLK_XTAL_32KHZ))
            .sw_32k_calib_enable = false,
            #else
            .sw_32k_calib_enable        = false,
            #endif
            .bg_sleep_duration_external = CFG_MAX_SLEEP_DURATION_EXTERNAL_WAKEUP,
            .bg_sleep_duration_periodic = CFG_MAX_SLEEP_DURATION_PERIODIC_WAKEUP,
    },
    .xcvr_api = {
            .xcvr_wake_up_init                  = XCVR_WakeUpInit,
            .xcvr_read_rssi                     = XCVR_ReadRSSI,
            .xcvr_get_recal_duration            = XCVR_GetRecalDuration,
            .xcvr_recalibrate                   = XCVR_Recalibrate,
            .xcvr_temperature_update            = XCVR_TemperatureUpdate,
            .xcvr_disable_ble_fast_tx           = XCVR_DisableBLEFastTX,
            .xcvr_enable_ble_fast_tx            = XCVR_EnableBLEFastTX,
            .xcvr_lockup_check_and_abort_radio  = XCVR_LockupCheckAndAbortRadio,
            .xcvr_set_tx_pwr                    = XCVR_SetTxPwr,
#if defined (gMWS_UseCoexistence_d) && (gMWS_UseCoexistence_d)
            .xcvr_register_rf_activity_cb       = XCVR_RegisterRfActivityCallback,
#else
            .xcvr_register_rf_activity_cb       = NULL,
#endif
    },
    .coex_api = {
#if defined (gMWS_UseCoexistence_d) && (gMWS_UseCoexistence_d)
            .coex_register                      = BleCoexRegister,
            .coex_request_access                = BleCoexRequestAccess,
            .coex_change_access                 = BleCoexChangeAccess,
            .coex_release_access                = BleCoexReleaseAccess,
            .coex_enable                        = BleCoexEnable,
            .coex_disable                       = BleCoexDisable,
#else
            .coex_register                      = NULL,
            .coex_request_access                = NULL,
            .coex_release_access                = NULL,
            .coex_enable                        = NULL,
            .coex_disable                       = NULL,
#endif
    },
    #if defined (gBleControllerUsePdm_d) && (gBleControllerUsePdm_d == 1)
    .controllerUsePdm = TRUE,
    #else
    .controllerUsePdm = FALSE,
    #endif

    .pdm_api = {
#if defined gBleControllerLocalNameSupport
        .get_local_name = PdmReadLocalName,
        .set_local_name = PdmWriteLocalName,
		.get_class_of_device = PdmReadClassOfDevice,
		.set_class_of_device = PdmWriteClassOfDevice,
#else
        .get_local_name = NULL,
        .set_local_name = NULL,
		.get_class_of_device = NULL,
		.set_class_of_device = NULL,
#endif
    },

};

struct dyn_cfg dynamic_configuration;

/*! *********************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
********************************************************************************** */

#if gStressTesting_d
static uint32_t dma_buffer[DMA_COPY_SIZE];
static dma_handle_t g_DMA_Handle;
#endif

/*! *********************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
********************************************************************************** */

/*! @brief Initialize BLE configuration. */
void BLE_ControllerConfig(struct ble_config_st *cfg)
{
    cfg->app = &app_configuration;
    cfg->fw  = &firmware_configuration;
    cfg->fwk = &framework_configuration;
    cfg->dyn = &dynamic_configuration;

    cfg->dyn->lp_dyn = PWR_GetHk32kHandle();

    // Set API flags
    cfg->dyn->flags = CFG_API_FLAG_HCI;

#if defined (gFastCorrect_d) && (gFastCorrect_d == 1)
    cfg->dyn->flags |= CFG_API_FLAG_FAST_CORRECT;
#endif

#if (gClkUseFro32K && gPWR_UseAlgoTimeBaseDriftCompensate)
    cfg->dyn->flags |= CFG_API_FLAG_DRIFT_FREQ;
#endif

    RNG_Init();
    RNG_HwGetRandomNo(&(cfg->dyn->fw_seed));

#if gStressTesting_d
    dma_test_start();
#endif
}

#if gStressTesting_d
static void DMA_Callback(dma_handle_t *handle, void *param, bool transferDone, uint32_t tcds)
{
    /* Mark end of DMA transfer - double pulse */
    BOARD_DbgIoSet(0, 1);
    BOARD_DbgIoSet(0, 0);
    BOARD_DbgIoSet(0, 1);
    BOARD_DbgIoSet(0, 0);

    /* Mark a pause */
    BOARD_DbgIoSet(0, 0);
    BOARD_DbgIoSet(0, 0);

    /* Start transfer */
    DMA_StartTransfer(&g_DMA_Handle); // Restart DMA transfer once complete

    /* Mark start of a new transfer */
    BOARD_DbgIoSet(0, 1);
    BOARD_DbgIoSet(0, 0);
}

static void dma_test_start(void)
{
    dma_transfer_config_t transferConfig;
    uint32_t *dstAddr = &dma_buffer[0];
    uint32_t *srcAddr = (uint32_t*) EM_BASE_ADDR;

    DMA_Init(DMA0);
    DMA_EnableChannel(DMA0, 0);
    DMA_CreateHandle(&g_DMA_Handle, DMA0, 0);
    DMA_SetCallback(&g_DMA_Handle, DMA_Callback, NULL);
    DMA_PrepareTransfer(&transferConfig, srcAddr, dstAddr, sizeof(uint32_t), DMA_COPY_SIZE, kDMA_MemoryToMemory, NULL);
    DMA_SubmitTransfer(&g_DMA_Handle, &transferConfig);
    DMA_StartTransfer(&g_DMA_Handle);
}
#endif //gStressTesting_d

#ifdef MAC_DYNAMIC_SUPPORT
WEAK void BleAppInactivityCallback(uint32_t inactive_time)
{
    /* Check if a radio re-calibration is required */
    if (XCVR_GetRecalDuration() != 0)
    {
        BLE_get_sleep_mode();
        inactive_time = BLE_TimeBeforeNextBleEvent();
    }
    vMac_DynRadioAvailable(inactive_time);
}

WEAK int BleAppNewActivityCallback(uint32_t activity_time)
{
    return (eMac_DynActivityAdded(activity_time));
}

WEAK void BleAppWakeupEndCallback(void)
{
    /* Empty content */
}

WEAK bool_t BleAppIsDynSlaveProtocolActive(void)
{
    return (eDynGetProtocolState(E_DYN_SLAVE) == E_DYN_STATE_ACTIVE);
}
#endif


#if defined gBleControllerLocalNameSupport
WEAK uint32_t PdmReadLocalName(uint8_t name[MAX_LOCAL_NAME_SIZE])
{
    uint16_t pu16DataBytesRead = 0;
    LocalDeviceData_t local_data;
    PDM_teStatus  pdmSt;
    uint32_t res = ~0UL;
    do {
        if (!PDM_bDoesDataExist(pdmId_LocalDeviceData, &pu16DataBytesRead))
            break;

        pdmSt = PDM_eReadDataFromRecord( pdmId_LocalDeviceData,
                                         &local_data,
                                         sizeof(local_data),
                                         &pu16DataBytesRead);
        if (pdmSt != PDM_E_STATUS_OK)
            break;

        if (pu16DataBytesRead <  sizeof(local_data))
            break;
        for (int i = 0; i < sizeof(local_data.local_name); i++ )
        {
            name[i] = local_data.local_name[i];
            if (name[i] == 0)
            {
                res = i;
                break;
            }
        }
    } while (0);

    return res;

}

WEAK uint32_t PdmWriteLocalName(const uint8_t *name)
{
    uint16_t pu16DataBytesRead = 0;
    LocalDeviceData_t local_data;
    PDM_teStatus  pdmSt;
    uint32_t res = ~0UL;
    bool write_required = false;
    do {

        /* If LocalDeviceData already exists read it and compare whether local name
           changed : if not return pretending the write worked anyway */
        if (PDM_bDoesDataExist(pdmId_LocalDeviceData, &pu16DataBytesRead))
        {
            pdmSt = PDM_eReadDataFromRecord( pdmId_LocalDeviceData,
                                             &local_data,
                                             sizeof(local_data),
                                             &pu16DataBytesRead);
            if (pdmSt != PDM_E_STATUS_OK)
                break;

            if (pu16DataBytesRead < sizeof(local_data))
                break;

            for (int i = 0; i < sizeof(local_data.local_name); i++ )
            {
                if (name[i] != local_data.local_name[i])
                {
                    /* octet strings differ */
                    write_required = true;
                    break;
                }
                if ((name[i] == 0) && (local_data.local_name[i] == 0))
                    break;
            }
            if (!write_required)
                res = 0;
        }
        else
        {
            write_required = true;
            FLib_MemSet(&local_data, 0, sizeof(local_data));
        }
        if (write_required)
        {
            /* Update local name field */
            for (int i =0; i < sizeof(local_data.local_name); i++ )
            {
                local_data.local_name[i] = name[i];
                if (name[i] == 0) break;
            }
            /* Submit by writing to PDM */
            pdmSt = PDM_eSaveRecordData(pdmId_LocalDeviceData,
                                         &local_data,
                                         sizeof(local_data));
            if (pdmSt != PDM_E_STATUS_OK)
            {
                res = sizeof(local_data.local_name);
            }
        }

    } while (0);


    return res;
}

WEAK uint32_t PdmReadClassOfDevice(uint8_t class[CLASS_OF_DEV_SZ])
{
    uint16_t pu16DataBytesRead = 0;
    LocalDeviceData_t local_data;
    PDM_teStatus  pdmSt;
    uint32_t res = ~0UL;
    do {
        if (!PDM_bDoesDataExist(pdmId_LocalDeviceData, &pu16DataBytesRead))
            break;

        pdmSt = PDM_eReadDataFromRecord( pdmId_LocalDeviceData,
                                         &local_data,
                                         sizeof(local_data),
                                         &pu16DataBytesRead);
        if (pdmSt != PDM_E_STATUS_OK)
            break;

        if (pu16DataBytesRead <  sizeof(local_data))
            break;
        for (int i = 0; i < sizeof(local_data.class_of_device); i++ )
        {
            class[i] = local_data.class_of_device[i];
        }

        res = sizeof(local_data.class_of_device);

    } while (0);

    return res;

}

WEAK uint32_t PdmWriteClassOfDevice(const uint8_t *class)
{
    uint16_t pu16DataBytesRead = 0;
    LocalDeviceData_t local_data;
    PDM_teStatus  pdmSt;
    uint32_t res = ~0UL;
    bool write_required = false;
    do {

        /* If LocalDeviceData already exists read it and compare whether local name
           changed : if not return pretending the write worked anyway */
        if (PDM_bDoesDataExist(pdmId_LocalDeviceData, &pu16DataBytesRead))
        {
            pdmSt = PDM_eReadDataFromRecord( pdmId_LocalDeviceData,
                                             &local_data,
                                             sizeof(local_data),
                                             &pu16DataBytesRead);
            if (pdmSt != PDM_E_STATUS_OK)
                break;

            if (pu16DataBytesRead < sizeof(local_data))
                break;

            for (int i = 0; i < sizeof(local_data.class_of_device); i++ )
            {
                if (class[i] != local_data.class_of_device[i])
                {
                    /* octet strings differ */
                    write_required = true;
                    break;
                }
            }
            if (!write_required)
                res = 0;
        }
        else
        {
            write_required = true;
            FLib_MemSet(&local_data, 0, sizeof(local_data));
        }
        if (write_required)
        {
            /* Update local name field */
            for (int i =0; i < sizeof(local_data.class_of_device); i++ )
            {
                local_data.class_of_device[i] = class[i];
            }
            /* Submit by writing to PDM */
             pdmSt = PDM_eSaveRecordData(pdmId_LocalDeviceData,
                                         &local_data,
                                         sizeof(local_data));
             if (pdmSt == PDM_E_STATUS_OK)
             {
                 res = sizeof(local_data.class_of_device);
             }
        }

    } while (0);


    return res;
}

#endif
