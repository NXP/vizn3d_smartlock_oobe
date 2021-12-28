/*
 * Copyright 2019-2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

/*
 * @brief Push button HAL input device implementation. Receives input from the board's push buttons, generating an
 * "InputNotify" event for other HAL devices.
 */

#include <FreeRTOS.h>
#include <stdlib.h>
#include <task.h>
#include <timers.h>

#include "board.h"
#include "fsl_gpio.h"

#include "fwk_input_manager.h"
#include "fwk_log.h"
#include "fwk_lpm_manager.h"

#include "hal_input_dev.h"

#define INPUT_DEV_PB_WAKE_GPIO        BOARD_USER_BUTTON_GPIO
#define INPUT_DEV_PB_WAKE_GPIO_PIN    BOARD_USER_BUTTON_GPIO_PIN
#define INPUT_DEV_SW1_GPIO            BOARD_BUTTON_SW1_GPIO
#define INPUT_DEV_SW1_GPIO_PIN        BOARD_BUTTON_SW1_PIN
#define INPUT_DEV_SW2_GPIO            BOARD_BUTTON_SW2_GPIO
#define INPUT_DEV_SW2_GPIO_PIN        BOARD_BUTTON_SW2_PIN
#define INPUT_DEV_SW3_GPIO            BOARD_BUTTON_SW3_GPIO
#define INPUT_DEV_SW3_GPIO_PIN        BOARD_BUTTON_SW3_PIN
#define INPUT_DEV_PUSH_BUTTONS_IRQ    GPIO13_Combined_0_31_IRQn
#define INPUT_DEV_PUSH_BUTTON_SW1_IRQ BOARD_BUTTON_SW1_IRQ
#define INPUT_DEV_PUSH_BUTTON_SW2_IRQ BOARD_BUTTON_SW2_IRQ
#define INPUT_DEV_PUSH_BUTTON_SW3_IRQ BOARD_BUTTON_SW3_IRQ

#define DEBOUNCE_TIME_MS          10
#define LONG_PRESS_TIMEOUT_MS     1500
#define BLOCKING_TIME_MS          2000
/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef enum _switch_press_type
{
    kSwitchPressType_Long  = 0,
    kSwitchPressType_Short = 1,
} switch_press_type_t;

typedef enum _switch_state
{
    kSwitchState_Pressed  = 0,
    kSwitchState_Released = 1,
} switch_state_t;

typedef enum _switch_id
{
    kSwitchID_1 = 0,
    kSwitchID_2,
    kSwitchID_3,
    kSwitchID_Count,
} switch_id_t;

typedef struct _button_data
{
    TimerHandle_t releaseTimer;
    uint32_t lastDebounce;
    switch_id_t buttonId;
    switch_state_t state;
    IRQn_Type irq;
    GPIO_Type *base;
    uint32_t pin;
} button_data_t;

hal_input_status_t HAL_InputDev_PushButtons_Init(input_dev_t *dev, input_dev_callback_t callback);
hal_input_status_t HAL_InputDev_PushButtons_Deinit(const input_dev_t *dev);
hal_input_status_t HAL_InputDev_PushButtons_Start(const input_dev_t *dev);
hal_input_status_t HAL_InputDev_PushButtons_Stop(const input_dev_t *dev);
hal_input_status_t HAL_InputDev_PushButtons_InputNotify(const input_dev_t *dev, void *param);

/*******************************************************************************
 * Variables
 ******************************************************************************/

const static input_dev_operator_t s_InputDev_PushButtonsOps = {
    .init        = HAL_InputDev_PushButtons_Init,
    .deinit      = HAL_InputDev_PushButtons_Deinit,
    .start       = HAL_InputDev_PushButtons_Start,
    .stop        = HAL_InputDev_PushButtons_Stop,
    .inputNotify = HAL_InputDev_PushButtons_InputNotify,
};

static input_dev_t s_InputDev_PushButtons = {
    .id = 1, .name = "buttons", .ops = &s_InputDev_PushButtonsOps, .cap = {.callback = NULL}};

static button_data_t s_buttons[kSwitchID_Count] = {
    {
        .releaseTimer    = NULL,
        .lastDebounce    = 0,
        .buttonId        = kSwitchID_1,
        .state           = kSwitchState_Released,
        .irq             = INPUT_DEV_PUSH_BUTTON_SW1_IRQ,
        .base            = INPUT_DEV_SW1_GPIO,
        .pin             = INPUT_DEV_SW1_GPIO_PIN,
    },
    {
        .releaseTimer    = NULL,
        .lastDebounce    = 0,
        .buttonId        = kSwitchID_2,
        .state           = kSwitchState_Released,
        .irq             = INPUT_DEV_PUSH_BUTTON_SW2_IRQ,
        .base            = INPUT_DEV_SW2_GPIO,
        .pin             = INPUT_DEV_SW2_GPIO_PIN,
    },
    {
        .releaseTimer    = NULL,
        .lastDebounce    = 0,
        .buttonId        = kSwitchID_3,
        .state           = kSwitchState_Released,
        .irq             = INPUT_DEV_PUSH_BUTTON_SW3_IRQ,
        .base            = INPUT_DEV_SW3_GPIO,
        .pin             = INPUT_DEV_SW3_GPIO_PIN,
    },

};

static void *s_pEvent;
static input_event_t s_inputEvent;
static TimerHandle_t blockingTimer;

/*******************************************************************************
 * Code
 ******************************************************************************/
static void _BlockingTimerCallback(TimerHandle_t xTimer);
static void _ReleaseTimerCallback(TimerHandle_t xTimer);

__attribute__((weak)) int APP_InputDev_PushButtons_SetEvent(switch_id_t button,
                                                            switch_press_type_t pressType,
                                                            void **event,
                                                            uint32_t *receiverList)
{
    LOGI(
        "No handlers currently associated with push buttons. Override the \"%s\" function if this is not "
        "intentional.\r\n",
        __FUNCTION__);
    return kStatus_Fail;
}

void _HAL_InputDev_IrqHandler(button_data_t *button, switch_press_type_t pressType)
{
    if (s_InputDev_PushButtons.cap.callback != NULL)
    {
        uint32_t receiverList;
        if (APP_InputDev_PushButtons_SetEvent(button->buttonId, pressType, &s_pEvent, &receiverList) == kStatus_Success)
        {
            s_inputEvent.inputData = s_pEvent;
            uint8_t fromISR        = __get_IPSR();
            s_InputDev_PushButtons.cap.callback(&s_InputDev_PushButtons, kInputEventID_Recv, receiverList,
                                                &s_inputEvent, 0, fromISR);
        }
        else
        {
            LOGE("No valid event associated with SW%d button %s press", button->buttonId,
                 pressType == kSwitchPressType_Short ? "short" : "long");
        }
    }
}

hal_input_status_t HAL_InputDev_PushButtons_Init(input_dev_t *dev, input_dev_callback_t callback)
{
    hal_input_status_t error = 0;
    LOGD("++HAL_InputDev_PushButtons_Init ");

    dev->cap.callback = callback;
    gpio_pin_config_t input_dev_push_buttons_pin_config;

    memset(&input_dev_push_buttons_pin_config, 0, sizeof(input_dev_push_buttons_pin_config));

    input_dev_push_buttons_pin_config.direction     = kGPIO_DigitalInput;
    input_dev_push_buttons_pin_config.interruptMode = kGPIO_IntRisingOrFallingEdge;

    blockingTimer = xTimerCreate(NULL, pdMS_TO_TICKS(BLOCKING_TIME_MS), pdFALSE,
                                (void *)0, _BlockingTimerCallback);

    for (int i = 0; i < kSwitchID_Count; i++)
    {
        GPIO_PinInit(s_buttons[i].base, s_buttons[i].pin, &input_dev_push_buttons_pin_config);
        s_buttons[i].releaseTimer    = xTimerCreate(NULL, pdMS_TO_TICKS(LONG_PRESS_TIMEOUT_MS), pdFALSE,
                                                    (void *)&s_buttons[i].buttonId, _ReleaseTimerCallback);
        if (s_buttons[i].releaseTimer == NULL)
        {
            LOGE("Create release timer for time switch %d push button failed.", i);
        }
        GPIO_PortClearInterruptFlags(s_buttons[i].base, (1 << s_buttons[i].pin));

        /* Enable Interrupts */
        NVIC_SetPriority(s_buttons[i].irq, configMAX_SYSCALL_INTERRUPT_PRIORITY - 1);
        EnableIRQ(s_buttons[i].irq);
    }

    LOGD("--HAL_InputDev_PushButtons_Init");
    return error;
}

hal_input_status_t HAL_InputDev_PushButtons_Deinit(const input_dev_t *dev)
{
    hal_input_status_t error = 0;
    return error;
}

hal_input_status_t HAL_InputDev_PushButtons_Start(const input_dev_t *dev)
{
    hal_input_status_t error = 0;
    LOGD("++HAL_InputDev_PushButtons_Start");

    for (int i = 0; i < kSwitchID_Count; i++)
    {
        GPIO_PortEnableInterrupts(s_buttons[i].base, (1 << s_buttons[i].pin));
    }

    LOGD("--HAL_InputDev_PushButtons_Start");
    return error;
}

hal_input_status_t HAL_InputDev_PushButtons_Stop(const input_dev_t *dev)
{
    hal_input_status_t error = 0;
    LOGD("++HAL_InputDev_PushButtons_Stop");
    for (int i = 0; i < kSwitchID_Count; i++)
    {
        GPIO_PortDisableInterrupts(s_buttons[i].base, (1 << s_buttons[i].pin));
    }

    LOGD("--HAL_InputDev_PushButtons_Stop");
    return error;
}

hal_input_status_t HAL_InputDev_PushButtons_InputNotify(const input_dev_t *dev, void *param)
{
    hal_input_status_t error = kStatus_HAL_InputSuccess;
    return error;
}

int HAL_InputDev_PushButtons_Register()
{
    int error = 0;
    LOGD("input_dev_push_buttons_register");
    error = FWK_InputManager_DeviceRegister(&s_InputDev_PushButtons);
    return error;
}

void INPUT_DEV_PUSH_BUTTONS_IRQHandler(GPIO_Type *base, uint32_t intPin)
{
    // Check for the specific button that triggered the interrupt
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        for (int i = 0; i < kSwitchID_Count; i++)
        {
            button_data_t *button = &s_buttons[i];
            if ((button->base == base) && ((intPin >> button->pin) & 0x01))
            {
                BaseType_t HigherPriorityTaskWoken = pdFALSE;
                uint32_t curr_time                 = xTaskGetTickCountFromISR();
                switch_state_t curr_state          = 0;

                curr_state = GPIO_PinRead(button->base, button->pin);

                if (curr_state != button->state)
                {
                    if (curr_time - button->lastDebounce < DEBOUNCE_TIME_MS)
                    {
                        /* debounce abort */
                        button->lastDebounce = curr_time;
                        break;
                    }

                    /* block all other pins */
                    for (int j = 0; j < kSwitchID_Count; j++)
                    {
                        if (!((s_buttons[j].base == base) && ((intPin >> s_buttons[j].pin) & 0x01)))
                        {
                            /* Disable interrupts on the button */
                            GPIO_PortDisableInterrupts(s_buttons[j].base, (1 << s_buttons[j].pin));
                        }
                    }

                    if (curr_state == kSwitchState_Pressed)
                    {
                        LOGI("SW%d Pressed.", i);
                        button->state = kSwitchState_Pressed;

                        xTimerStartFromISR(button->releaseTimer, 0);
                    }
                    else if (curr_state == kSwitchState_Released)
                    {
                        LOGI("SW%d Released.", i);
                        GPIO_PortDisableInterrupts(button->base, (1 << button->pin));
                        if (curr_time - button->lastDebounce > LONG_PRESS_TIMEOUT_MS)
                        {
                            _HAL_InputDev_IrqHandler(button, kSwitchPressType_Long);
                        }
                        else
                        {
                            _HAL_InputDev_IrqHandler(button, kSwitchPressType_Short);
                        }
                        button->state = kSwitchState_Released;

                        xTimerStopFromISR(button->releaseTimer, 0);
                        xTimerStartFromISR(blockingTimer, &HigherPriorityTaskWoken);
                    }
                    button->lastDebounce = curr_time;
                }

                break;
            }
        }
    }
}

static void _BlockingTimerCallback(TimerHandle_t xTimer)
{
    for (int i = 0; i < kSwitchID_Count; i++)
    {
        GPIO_PortClearInterruptFlags(s_buttons[i].base, (1U << s_buttons[i].pin));
        GPIO_PortEnableInterrupts(s_buttons[i].base, (1U << s_buttons[i].pin));
    }
}

static void _ReleaseTimerCallback(TimerHandle_t xTimer)
{
    for (int i = 0; i < kSwitchID_Count; i++)
    {
        uint8_t id = *(uint8_t *)pvTimerGetTimerID(xTimer);
        if (s_buttons[i].buttonId == id)
        {
            switch_state_t curr_state = GPIO_PinRead(s_buttons[i].base, s_buttons[i].pin);
            if (curr_state == kSwitchState_Pressed)
            {
                LOGI("SW%d Released by long press timeout.", i);
                GPIO_PortDisableInterrupts(s_buttons[i].base, (1 << s_buttons[i].pin));
                _HAL_InputDev_IrqHandler(&s_buttons[i], kSwitchPressType_Long);
                s_buttons[i].state = kSwitchState_Released;

                xTimerStart(blockingTimer, 0);
            }
        }
    }
}