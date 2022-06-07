/*
 * Copyright 2019-2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

/*
 * @brief AT commands hal layer implementation.
 */

#include <FreeRTOS.h>
#include <queue.h>

#include "board_define.h"
#include "board.h"
#include "fsl_lpuart_freertos.h"
#include "fsl_lpuart.h"

#include "fwk_log.h"
#include "fwk_message.h"
#include "fwk_sln_task.h"
#include "fwk_input_manager.h"
#include "fwk_output_manager.h"
#include "fwk_lpm_manager.h"
#include "hal_event_descriptor_face_rec.h"
#include "hal_input_dev.h"
#include "hal_smart_lock_config.h"
#include "sln_at_commands.h"

#define AT_COMMANDS_NAME                "at_commands"
#define AT_COMMANDS_LPUART              LPUART12
#define AT_COMMANDS_LPUART_BAUDRATE     115200
#define AT_COMMANDS_LPUART_CLOCK        kCLOCK_Root_Lpuart12
#define AT_COMMANDS_LPUART_IRQ          LPUART12_IRQn
#define AT_COMMANDS_LPUART_IRQ_PRIORITY 5
#define AT_COMMANDS_LPUART_BUFFER_SIZE  512

static lpuart_rtos_handle_t s_LpuartRTOSHandle;
static lpuart_handle_t s_LpuartHandle;
static uint8_t s_LpuartRingBuffer[AT_COMMANDS_LPUART_BUFFER_SIZE];

static hal_input_status_t HAL_InputDev_ATCommands_Init(input_dev_t *dev, input_dev_callback_t callback);
static hal_input_status_t HAL_InputDev_ATCommands_Deinit(const input_dev_t *dev);
static hal_input_status_t HAL_InputDev_ATCommands_Start(const input_dev_t *dev);
static hal_input_status_t HAL_InputDev_ATCommands_Stop(const input_dev_t *dev);
static hal_input_status_t HAL_InputDev_ATCommands_InputNotify(const input_dev_t *dev, void *param);
static hal_output_status_t HAL_OutputDev_ATCommands_Start(const output_dev_t *dev);
static hal_output_status_t HAL_OutputDev_ATCommands_InferComplete(const output_dev_t *dev,
                                                                      output_algo_source_t source,
                                                                      void *inferResult);
static hal_output_status_t HAL_OutputDev_ATCommands_InputNotify(const output_dev_t *dev, void *param);

const static input_dev_operator_t s_InputDev_ATCommandsOps = {
    .init        = HAL_InputDev_ATCommands_Init,
    .deinit      = HAL_InputDev_ATCommands_Deinit,
    .start       = HAL_InputDev_ATCommands_Start,
    .stop        = HAL_InputDev_ATCommands_Stop,
    .inputNotify = HAL_InputDev_ATCommands_InputNotify,
};

static input_dev_t s_InputDev_ATCommands = {
    .id = 1, .name = AT_COMMANDS_NAME, .ops = &s_InputDev_ATCommandsOps, .cap = {.callback = NULL}};

static hal_lpm_request_t s_LpmReq = {.dev = &s_InputDev_ATCommands, .name = "s_InputDev_ATCommands"};

const static output_dev_event_handler_t s_OutputDev_ATCommandsHandler = {
    .inferenceComplete = HAL_OutputDev_ATCommands_InferComplete,
    .inputNotify       = HAL_OutputDev_ATCommands_InputNotify,
};

const static output_dev_operator_t s_OutputDev_ATCommandsOps = {
    .init   = NULL,
    .deinit = NULL,
    .start  = HAL_OutputDev_ATCommands_Start,
    .stop   = NULL,
};

static output_dev_t s_OutputDev_ATCommands = {
    .name         = AT_COMMANDS_NAME,
    .attr.type    = kOutputDevType_Other,
    .attr.reserve = NULL,
    .ops          = &s_OutputDev_ATCommandsOps,
};

static hal_input_status_t HAL_InputDev_ATCommands_Init(input_dev_t *dev, input_dev_callback_t callback)
{
    hal_input_status_t status = kStatus_HAL_InputSuccess;

    lpuart_rtos_config_t config = {
        .base        = AT_COMMANDS_LPUART,
        .baudrate    = AT_COMMANDS_LPUART_BAUDRATE,
        .parity      = kLPUART_ParityDisabled,
        .stopbits    = kLPUART_OneStopBit,
        .buffer      = s_LpuartRingBuffer,
        .buffer_size = sizeof(s_LpuartRingBuffer),
#if defined(FSL_FEATURE_LPUART_HAS_MODEM_SUPPORT) && FSL_FEATURE_LPUART_HAS_MODEM_SUPPORT
        .enableRxRTS = 0,
        .enableTxCTS = 0,
        .txCtsSource = 0,
        .txCtsConfig = 0,
#endif
    };

    config.srcclk = CLOCK_GetRootClockFreq(AT_COMMANDS_LPUART_CLOCK);

    NVIC_SetPriority(AT_COMMANDS_LPUART_IRQ, AT_COMMANDS_LPUART_IRQ_PRIORITY);
    if (kStatus_Success != LPUART_RTOS_Init(&s_LpuartRTOSHandle, &s_LpuartHandle, &config))
    {
        vTaskSuspend(NULL);
    }

    dev->cap.callback = callback;

    return status;
}

static hal_input_status_t HAL_InputDev_ATCommands_Deinit(const input_dev_t *dev)
{
    hal_input_status_t status = kStatus_HAL_InputSuccess;

    LPUART_RTOS_Deinit(&s_LpuartRTOSHandle);

    return status;
}

static hal_input_status_t HAL_InputDev_ATCommands_Start(const input_dev_t *dev)
{
    hal_input_status_t status = kStatus_HAL_InputSuccess;

    Start_ATCommands_Processing(&s_LpuartRTOSHandle, &s_InputDev_ATCommands);

    return status;
}

static hal_input_status_t HAL_InputDev_ATCommands_Stop(const input_dev_t *dev)
{
    hal_input_status_t status = kStatus_HAL_InputSuccess;
    return status;
}

static hal_input_status_t HAL_InputDev_ATCommands_InputNotify(const input_dev_t *dev, void *param)
{
    hal_input_status_t status = kStatus_HAL_InputSuccess;
    return status;
}

static hal_output_status_t HAL_OutputDev_ATCommands_Start(const output_dev_t *dev)
{
    hal_output_status_t status = kStatus_HAL_OutputSuccess;
    if (FWK_OutputManager_RegisterEventHandler(dev, &s_OutputDev_ATCommandsHandler) != 0)
        status = kStatus_HAL_OutputError;
    return status;
}

static hal_output_status_t HAL_OutputDev_ATCommands_InferComplete(const output_dev_t *dev,
                                                                      output_algo_source_t source,
                                                                      void *inferResult)
{
    hal_output_status_t status = kStatus_HAL_OutputSuccess;

    APP_OutputDev_ATCommands_InferCompleteDecode(source, (*(vision_algo_result_t *)inferResult).oasisLite);

    return status;
}

static hal_output_status_t HAL_OutputDev_ATCommands_InputNotify(const output_dev_t *dev, void *param)
{
    hal_output_status_t status = kStatus_HAL_OutputSuccess;
    return status;
}

int HAL_Dev_ATCommands_Register()
{
    hal_input_status_t status = kStatus_HAL_InputSuccess;
    LOGD("HAL_Dev_ATCommands_Register");

    status = FWK_InputManager_DeviceRegister(&s_InputDev_ATCommands);
    if (status)
    {
        return status;
    }

    status = FWK_OutputManager_DeviceRegister(&s_OutputDev_ATCommands);
    if (status)
    {
        return status;
    }

    status = FWK_LpmManager_RegisterRequestHandler(&s_LpmReq);
    return status;
}
