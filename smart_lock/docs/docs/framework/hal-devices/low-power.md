---
sidebar_position: 7
---

# Low Power Devices

The Low Power/`LPM` HAL device represents an abstraction used to implement a device which controls the power management of the device by configuring the chip-level power mode (normal operation, SNVS, etc.).

Unlike other devices which may represent a real, physical device, the low power HAL device is purely a "virtual" abstraction mechanism representing the chip's power regulation controls.
As a result,
the low power HAL device is platform-dependent
because it relies on the different power modes and configuration options made available by the platform being used.
Additionally,
only one low power HAL device can (and is necessary to) be registered at a time
because a chip's power regulatory functionality will not typically require multiple disparate components.
This means that the API calls to the [Low Power Manager](../device-managers/lpm_manager.md#APIs) are
essentially wrappers over the single LPM device's [operators](#operators).

As for functionality,
the low power HAL device provides:

- Multi-level low-power switching
- Manual power state configuration
- Automatic power state configuration via periodic idle checks and other flags

The low power mode device also provides an exit mechanism which is called before entering low power mode,
to ensure components are properly shut down before sleeping.
This is achieved by using a series of timers,
one as a periodic idle check to wait for a specified timeout period before shutting down,
and the other as an "exit timer" which reserves a sufficient amount of time for other HAL devices to properly shutdown.

## Device Definition

The HAL device definition for LPM devices can be found under `framework/hal_api/hal_lpm_dev.h` and is reproduced below:

```c title="framework/hal_api/hal_lpm_dev.h"
/*! @brief Attributes of a lpm device */
struct _lpm_dev
{
    /* unique id which is assigned by lpm manager during the registration */
    int id;
    /* operations */
    const lpm_dev_operator_t *ops;
    /* timer */
    TimerHandle_t timer;
    /* pre-enter sleep timer */
    TimerHandle_t preEnterSleepTimer;
    /* lock */
    SemaphoreHandle_t lock;
    /* callback */
    lpm_manager_timer_callback_t callback;
    /* preEnterSleepCallback */
    lpm_manager_timer_callback_t preEnterSleepCallback;
};
```

The device [operators](#operators) associated with LPM HAL devices are as shown below:

```c
/*! @brief Callback function to timeout check requester list busy status. */
typedef int (*lpm_manager_timer_callback_t)(lpm_dev_t *dev);

/*! @brief Operation that needs to be implemented by a lpm device */
typedef struct _lpm_dev_operator
{
    hal_lpm_status_t (*init)(lpm_dev_t *dev,
                             lpm_manager_timer_callback_t callback,
                             lpm_manager_timer_callback_t preEnterSleepTimer);
    hal_lpm_status_t (*deinit)(const lpm_dev_t *dev);
    hal_lpm_status_t (*openTimer)(const lpm_dev_t *dev);
    hal_lpm_status_t (*stopTimer)(const lpm_dev_t *dev);
    hal_lpm_status_t (*openPreEnterTimer)(const lpm_dev_t *dev);
    hal_lpm_status_t (*stopPreEnterTimer)(const lpm_dev_t *dev);
    hal_lpm_status_t (*enterSleep)(const lpm_dev_t *dev, hal_lpm_mode_t mode);
    hal_lpm_status_t (*lock)(const lpm_dev_t *dev);
    hal_lpm_status_t (*unlock)(const lpm_dev_t *dev);
} lpm_dev_operator_t;

typedef struct _hal_lpm_request
{
    void *dev;                              /* request dev handle */
    char name[LPM_REQUEST_NAME_MAX_LENGTH]; /* request name */
} hal_lpm_request_t;
```

## Operators

Operators are functions which "operate" on a HAL device itself.
Operators are akin to "public methods" in object oriented-languages,
and are used by the Low Power Manager to setup, start, etc. its registered low power device.

For more information about operators, see [Operators](overview.md#Operators)

### Init

```c
hal_lpm_status_t (*init)(lpm_dev_t *dev, lpm_manager_timer_callback_t callback, 
                                         lpm_manager_timer_callback_t preEnterSleepTimer);
```

Initialize the lpm device.

`Init` should initialize any hardware resources the lpm device requires (I/O ports, IRQs, etc.), turn on the hardware, and perform any other setup the device requires.

The [callback function](#callback) to the device's manager is typically installed as part of the `Init` function as well.

This operator will be called by the Input Manager when the Input Manager task first starts.

### Deinit

```c
hal_lpm_status_t (*deinit)(const lpm_dev_t *dev);
```

"Deinitialize" the lpm device.

`DeInit` should release any hardware resources the lpm device uses (I/O ports, IRQs, etc.), turn off the hardware, and perform any other shutdown the device requires.

This operator will be called by the Input Manager when the Input Manager task ends[^1].

:::note
[^1]The `DeInit` function generally will not be called under normal operation.
:::

### OpenTimer

```c
hal_lpm_status_t (*openTimer)(const lpm_dev_t *dev);
```

Kicks off the periodic idle check timer.

### StopTimer

```c
hal_lpm_status_t (*stopTimer)(const lpm_dev_t *dev);
```

Stops the periodic idle check timer.

After all busy requests (BLE connection established, face registration in progress) have ceased,
this function will be called and begin the shutdown process for other HAL devices.

### OpenPreEnterTimer

```c
hal_lpm_status_t (*openPreEnterTimer)(const lpm_dev_t *dev);
```

Kicks off the `preEnterSleepTimer`.

The `preEnterSleepTimer` is used to provide other HAL devices sufficient time to properly shutdown before the board enters sleep mode.
This function will be called after the periodic idle check timer has stopped (due to a timeout).

### StopPreEnterTimer

```c
hal_lpm_status_t (*stopPreEnterTimer)(const lpm_dev_t *dev);
```

Stops the `preEnterSleepTimer`.

This function is called to stop the timer associated with the pre-sleep shutdown process.
After this timer ends,
the [`EnterSleep`](#entersleep) function will be called and the device will power down.

### EnterSleep

```c
hal_lpm_status_t (*enterSleep)(const lpm_dev_t *dev, hal_lpm_mode_t mode);
```

Enter sleep mode using the low power mode specified in the function call[^2].

:::note
[^2] The power modes available vary based on the platform in use.
:::

### Lock

```c
hal_lpm_status_t (*lock)(const lpm_dev_t *dev);
```

Acquire the lock for the low power device.

The low power manager uses a lock-based system to prevent accidentally entering sleep mode before all devices are ready to enter sleep.
The `Lock` function is called by the Low Power manager in response to a HAL device signaling that it is performing a critical function which requires that the board does not enter sleep until complete.

<!-- TODO: Verify this is still 100% accurate -->

### Unlock

```c
hal_lpm_status_t (*unlock)(const lpm_dev_t *dev);
```

Release the lock for the low power device.

The low power manager uses a lock-based system to prevent accidentally entering sleep mode before all devices are ready to enter sleep.
The `Unlock` function is called by the Low Power manager in response to a HAL device signaling that it is finished performing a critical function which required that the board did not enter sleep until it was completed.

## Components

### timer

```c
/* timer */
TimerHandle_t timer;
```

This timer is use to periodically check busy requests from other HAL devices.

### preEnterSleepTimer

```c
/* pre-enter sleep timer */
TimerHandle_t preEnterSleepTimer;
```

This timer is used to provide a sufficient amount of time for HAL devices to shutdown prior to entering sleep mode.

### lock

```c
/* lock */
SemaphoreHandle_t lock;
```

This lock is used to maintain thread safety when multiple task need to call the Low Power Manager,
and is managed by the Low Power Manager.

### callback

```c
/* callback */
lpm_manager_timer_callback_t callback;
```

Callback to the Low Power Manager.
The HAL device invokes this callback to notify the vision algorithm manager of specific events.

The Low Power Manager will provide this callback to the device when the `init` operator is called.
As a result, the HAL device should make sure to store the callback in the `init` operator's implementation.

```c title="HAL/common/hal_sln_lpm.c" {10}
hal_lpm_status_t HAL_LpmDev_Init(lpm_dev_t *dev,
                                lpm_manager_timer_callback_t callback,
                                lpm_manager_timer_callback_t preEnterSleepCallback)
{
    int ret = kStatus_HAL_LpmSuccess;

    dev->callback              = callback;
    dev->preEnterSleepCallback = preEnterSleepCallback;
```

### PreEnterSleepCallback

```c
/* preEnterSleepCallback */
lpm_manager_timer_callback_t preEnterSleepCallback;
```

Callback function which is called after the "preEnterSleep" timer terminates.

:::note
This callback comes from the LPM Manager
:::

## Example

Because only one low power device can be registered at a time per the design of the framework,
the SLN-VIZN3D-IOT Smart Lock project has only one low power device implemented.

The source file for this low power device can be found at `HAL/common/hal_sln_lpm.c`.

In this example,
we will demonstrate the use of a low power device (using FreeRTOS for timers, etc.)
in conjunction with a device/manager of a different type.

The [LPM Manager Device](#lpm-manager-device) implements all the power switching functionality we need,
while the [secondary device/manager](#other-manager-device) will attempt to make busy requests (lock the LPM device)
and enable/disable low power mode.

### LPM Manager Device

```c title="HAL/common/hal_sln_lpm.c"
/* Here call periodic callback to check idle status. */
static void HAL_LpmDev_TimerCallback(TimerHandle_t handle)
{
    if (handle == NULL)
    {
        return;
    }

    lpm_dev_t *pDev = (lpm_dev_t *)pvTimerGetTimerID(handle);
    if (pDev->callback != NULL)
    {
        pDev->callback(pDev);
    }
}

/* Here call preEnterSleepCallback. Duing this time, all device have already exit. So this callback will call enterSleep operator to enter low power mode. */
static void HAL_LpmDev_PreEnterSleepTimerCallback(TimerHandle_t handle)
{
    if (handle == NULL)
    {
        return;
    }

    lpm_dev_t *pDev = (lpm_dev_t *)pvTimerGetTimerID(handle);
    if (pDev->preEnterSleepCallback != NULL)
    {
        pDev->preEnterSleepCallback(pDev);
    }
}

hal_lpm_status_t HAL_LpmDev_Init(lpm_dev_t *dev,
                                lpm_manager_timer_callback_t callback,
                                lpm_manager_timer_callback_t preEnterSleepCallback)
{
    int ret = kStatus_HAL_LpmSuccess;

    dev->callback              = callback;
    dev->preEnterSleepCallback = preEnterSleepCallback;

    /* put low power hardware init here */

    /* put periodic timer create and init here */
    dev->timer = xTimerCreate("LpmTimer", pdMS_TO_TICKS(1000), pdTRUE, (void *)dev, HAL_LpmDev_TimerCallback);
    if (dev->timer == NULL)
    {
        return kStatus_HAL_LpmTimerNull;
    }

    /* put exit timer create and init here */
    dev->preEnterSleepTimer = xTimerCreate("LpmPreEnterSleepTimer", pdMS_TO_TICKS(1500), pdTRUE, (void *)dev,
                                           HAL_LpmDev_PreEnterSleepTimerCallback);
    if (dev->preEnterSleepTimer == NULL)
    {
        return kStatus_HAL_LpmTimerNull;
    }

    /* put lock create and init here */
    dev->lock = xSemaphoreCreateMutex();
    if (dev->lock == NULL)
    {
        return kStatus_HAL_LpmLockNull;
    }

    /* put init low power mode and status here, detial can find in lpm_manager. */
    FWK_LpmManager_SetSleepMode(kLPMMode_SNVS);
    FWK_LpmManager_EnableSleepMode(kLPMManagerStatus_SleepDisable);

    return ret;
}

hal_lpm_status_t HAL_LpmDev_Deinit(const lpm_dev_t *dev)
{
    int ret = kStatus_HAL_LpmSuccess;

    return ret;
}

hal_lpm_status_t HAL_LpmDev_OpenTimer(const lpm_dev_t *dev)
{
    int ret = kStatus_HAL_LpmSuccess;

    if (dev->timer == NULL)
    {
        return kStatus_HAL_LpmTimerNull;
    }

    if (xTimerStart(dev->timer, 0) != pdPASS)
    {
        ret = kStatus_HAL_LpmTimerFail;
    }

    return ret;
}

hal_lpm_status_t HAL_LpmDev_StopTimer(const lpm_dev_t *dev)
{
    int ret = kStatus_HAL_LpmSuccess;

    if (dev->timer == NULL)
    {
        return kStatus_HAL_LpmTimerNull;
    }

    if (xTimerStop(dev->timer, 0) != pdPASS)
    {
        ret = kStatus_HAL_LpmTimerFail;
    }

    return ret;
}

hal_lpm_status_t HAL_LpmDev_OpenPreEnterSleepTimer(const lpm_dev_t *dev)
{
    int ret = kStatus_HAL_LpmSuccess;

    if (dev->preEnterSleepTimer == NULL)
    {
        return kStatus_HAL_LpmTimerNull;
    }

    if (xTimerStart(dev->preEnterSleepTimer, 0) != pdPASS)
    {
        ret = kStatus_HAL_LpmTimerFail;
    }

    return ret;
}

hal_lpm_status_t HAL_LpmDev_StopPreEnterSleepTimer(const lpm_dev_t *dev)
{
    int ret = kStatus_HAL_LpmSuccess;

    if (dev->preEnterSleepTimer == NULL)
    {
        return kStatus_HAL_LpmTimerNull;
    }

    if (xTimerStop(dev->preEnterSleepTimer, 0) != pdPASS)
    {
        ret = kStatus_HAL_LpmTimerFail;
    }

    return ret;
}

hal_lpm_status_t HAL_LpmDev_EnterSleep(const lpm_dev_t *dev, hal_lpm_mode_t mode)
{
    int ret = kStatus_HAL_LpmSuccess;
    switch (mode)
    {
        case kLPMMode_SNVS:
        {
            /* put enter SNVS low power mode here*/
        }
        break;

        default:
            break;
    }

    return ret;
}

hal_lpm_status_t HAL_LpmDev_Lock(const lpm_dev_t *dev)
{
    uint8_t fromISR = __get_IPSR();

    if (dev->lock == NULL)
    {
        return kStatus_HAL_LpmLockNull;
    }

    if (fromISR)
    {
        BaseType_t HigherPriorityTaskWoken = pdFALSE;
        if (xSemaphoreTakeFromISR(dev->lock, &HigherPriorityTaskWoken) != pdPASS)
        {
            return kStatus_HAL_LpmLockError;
        }
    }
    else
    {
        if (xSemaphoreTake(dev->lock, portMAX_DELAY) != pdPASS)
        {
            return kStatus_HAL_LpmLockError;
        }
    }

    return kStatus_HAL_LpmSuccess;
}

hal_lpm_status_t HAL_LpmDev_Unlock(const lpm_dev_t *dev)
{
    uint8_t fromISR = __get_IPSR();

    if (dev->lock == NULL)
    {
        return kStatus_HAL_LpmLockNull;
    }

    if (fromISR)
    {
        BaseType_t HigherPriorityTaskWoken = pdFALSE;
        if (xSemaphoreGiveFromISR(dev->lock, &HigherPriorityTaskWoken) != pdPASS)
        {
            return kStatus_HAL_LpmLockError;
        }
    }
    else
    {
        if (xSemaphoreGive(dev->lock) != pdPASS)
        {
            return kStatus_HAL_LpmLockError;
        }
    }

    return kStatus_HAL_LpmSuccess;
}

static lpm_dev_operator_t s_LpmDevOperators = {
    .init              = HAL_LpmDev_Init,
    .deinit            = HAL_LpmDev_Deinit,
    .openTimer         = HAL_LpmDev_OpenTimer,
    .stopTimer         = HAL_LpmDev_StopTimer,
    .openPreEnterTimer = HAL_LpmDev_OpenPreEnterSleepTimer,
    .stopPreEnterTimer = HAL_LpmDev_StopPreEnterSleepTimer,
    .enterSleep        = HAL_LpmDev_EnterSleep,
    .lock              = HAL_LpmDev_Lock,
    .unlock            = HAL_LpmDev_Unlock,
};

static lpm_dev_t s_LpmDev = {
    .id  = 0,
    .ops = &s_LpmDevOperators,
};

int HAL_LpmDev_Register()
{
    int ret = 0;

    FWK_LpmManager_DeviceRegister(&s_LpmDev);

    return ret;
}
```

### Requesting Device

As part of this example,
we assume an LPM device is running at the same time as a "requesting device" (camera, vision algo,
etc.) of a different type which is performing some critical functionality.

Supposing this example "requesting device" (aptly named "ExampleDev") performs some critical
functionality inside
`HAL_InputDev_ExampleDev_Critical` will set the request busy by calling `FWK_LpmManager_RuntimeGet`,
thus acquiring the lock which prevents changes to the current power mode state.

After the device has completed its critical functionality, it will use use `FWK_LpmManager_RuntimePut` to release the lock which prevents changes to the current power mode state.

```c
static hal_lpm_request_t s_LpmReq = {
    .dev  = &s_InputDev,
    .name = "lpm device",
};

int HAL_InputDev_ExampleDev_Critical(void)
{
    FWK_LpmManager_RuntimeGet(&s_LpmReq);

    /* perform critical function here */

    FWK_LpmManager_RuntimePut(&s_LpmReq);
}

int HAL_InputDev_ExampleDev_Register(void)
{
    hal_input_status_t status = kStatus_HAL_InputSuccess;

    status = FWK_LpmManager_RegisterRequestHandler(&s_LpmReq);

    return status;
}
```
