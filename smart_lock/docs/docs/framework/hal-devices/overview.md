---
sidebar_position: 1
---

# Overview

One of the most important steps in the the creation of any embedded software project is peripheral integration.
Unfortunately, this step can often be one of the most time intensive steps of the process.
Additionally,
peripheral drivers are often heavily tied to the specific platform which those drivers were originally written for,
which makes upgrading/moving to another platform difficult and costly.

The **Hardware Abstraction Layer (HAL)** component of the framework architecture was designed in direct response to these issues.

HAL devices are designed to be written "on top of" lower level driver code,
helping to increase code understandability by abstracting many of the underlying details,
HAL devices are also designed to be reused across different projects and even different NXP platforms,
increasing code reuse which can help cut down on development time.

## Device Registration

In order for a manager to communicate with a HAL device,
that device must first be registered to its respective manager.
Registration of each HAL device takes place at the beginning of application startup when `main()` calls the `APP_RegisterHalDevices()` function as shown below:

```c title="source/main.cpp" {10}
int main(void)
{
    /* Init board hardware. */
    APP_BoardInit();
    LOGD("[MAIN]:Started");
    /* init the framework*/
    APP_InitFramework();

    /* register the hal devices*/
    APP_RegisterHalDevices();

    /* start the framework*/
    APP_StartFramework();

    // start
    vTaskStartScheduler();

    while (1)
    {
        LOGD("#");
    }

    return 0;
}
```

To register a device to its manager,
each HAL device implements a registration function which is called prior to starting the managers themselves.
For example, the "register" function for the push button input device looks as follows:

```c title="HAL/common/hal_input_push_buttons.c"
int HAL_InputDev_PushButtons_Register()
{
    int error = 0;
    LOGD("input_dev_push_buttons_register");
    error = FWK_InputManager_DeviceRegister(&s_InputDev_PushButtons);
    return error;
}
```

Because HAL devices do not have header `.h` files associated with them,
the registration function for each device is exposed via the `board_define.h` file found inside the `boards` folder.
Each HAL device to be registered on startup must be added to the `APP_RegisterHalDevices` function in the `board_hal_registration.c` file.
The `board_hal_registration.c` file is also found in the `boards` folder.

## Device Types

There are several different device types to encapsulate the various peripherals which a user may wish to incorporate into their project.
These device types include:

- Input
- Output
- Camera
- Display
- VAlgo (Vision/Voice)

As well as a few others which are not listed here.

Each device type has specific methods and fields based on the unique characteristics of that device type.
For example, the camera HAL device definition looks as follows:

```c title="framework/hal_api/hal_camera_dev.h"
/**
 * @brief Callback function to notify camera manager that one frame is dequeued
 * @param dev Device structure of the camera device calling this function
 * @param event id of the event that took place
 * @param param Parameters
 * @param fromISR True if this operation takes place in an irq, 0 otherwise
 * @return 0 if the operation was successfully
 */
typedef int (*camera_dev_callback_t)(const camera_dev_t *dev, camera_event_t event, void *param, uint8_t fromISR);

/*! @brief Operation that needs to be implemented by a camera device */
typedef struct _camera_dev_operator
{
    /* initialize the dev */
    hal_camera_status_t (*init)(camera_dev_t *dev, int width, int height, camera_dev_callback_t callback, void *param);
    /* deinitialize the dev */
    hal_camera_status_t (*deinit)(camera_dev_t *dev);
    /* start the dev */
    hal_camera_status_t (*start)(const camera_dev_t *dev);
    /* enqueue a buffer to the dev */
    hal_camera_status_t (*enqueue)(const camera_dev_t *dev, void *data);
    /* dequeue a buffer from the dev */
    hal_camera_status_t (*dequeue)(const camera_dev_t *dev, void **data, pixel_format_t *format);
    /* postProcess a buffer from the dev */
    /*
     * Only do the minimum determination(data point and the format) of the frame in the dequeue.
     *
     * And split the CPU based post process(IR/Depth/... processing) to postProcess as they will eat CPU
     * which is critical for the whole system as camera manager is running with the highest priority.
     *
     * Camera manager will do the postProcess if there is a consumer of this frame.
     *
     * Note:
     * Camera manager will call multiple times of the posProcess of the same frame determinted by dequeue.
     * The HAL driver needs to guarantee the postProcess only do once for the first call.
     *
     */
    hal_camera_status_t (*postProcess)(const camera_dev_t *dev, void **data, pixel_format_t *format);
    /* input notify */
    hal_camera_status_t (*inputNotify)(const camera_dev_t *dev, void *data);
} camera_dev_operator_t;

/*! @brief Structure that characterize the camera device. */
typedef struct
{
    /* buffer resolution */
    int height;
    int width;
    int pitch;
    /* active rect */
    int left;
    int top;
    int right;
    int bottom;
    /* rotate degree */
    cw_rotate_degree_t rotate;
    /* flip */
    flip_mode_t flip;
    /* swap byte per two bytes */
    int swapByte;
} camera_dev_static_config_t;
```

In many ways, HAL devices can be thought of as similar to interfaces in C++ and other object-oriented languages.

## Anatomy of a HAL device

HAL devices are made up of several components which can vary by device type.
However,
each HAL device regardless of type has at least 3 components:

- `id`
- `name`
- [`operators`](#operators)

The `id` field is a unique device identifier which is assigned by the device's manager when the device is first registered.

The `name` field is used to help identify the device during various function calls and when debugging.

The `operators` field is a struct which contains function pointers to each of the functions that the HAL device is required to implement.
The operators which a device is required to implement will vary based on the device type.

A HAL device's definition is stored in a struct which gets passed to that device's respective manager when the device is registered.
This gives the manager information about the device
and allows the manager to call the device's operators when necessary.

### Operators

Operators are functions that "operate" on the device itself,
and are used by the device's manager to control the device and/or augment its behavior.
Operators are used for initializing, starting, and stopping devices,
as well as serving many other functions depending on the device.

As mentioned previously,
the operators a HAL device must implement varies based on device type.
For example,
input devices must implement an `init`, `deinit`, `start`, `stop`, and `inputNotify` function.

```c title="sln_framework/hal_api/hal_input_dev.h"
typedef struct
{
    /* initialize the dev */
    hal_input_status_t (*init)(input_dev_t *dev, input_dev_callback_t callback);
    /* deinitialize the dev */
    hal_input_status_t (*deinit)(const input_dev_t *dev);
    /* start the dev */
    hal_input_status_t (*start)(const input_dev_t *dev);
    /* stop the dev */
    hal_input_status_t (*stop)(const input_dev_t *dev);
    /* notify the input_dev */
    hal_input_status_t (*inputNotify)(const input_dev_t *dev, void *param);
} input_dev_operator_t;
```

Generally, each device regardless of type will have at least a `start`, `stop`, `init`, and `deinit` function.
Additionally,
most devices will also implement an `inputNotify` function which is used for [event handling](../events/event-handlers.md).

:::caution
Failing to implement a function will not prevent the HAL device from being registered,
but is likely to prevent certain functionality from working.
For example,
failing to provide an implementation for a HAL device's `start` function will prevent its respective manager from starting that device.
:::

## Configs

:::warning
This section describes a feature which is currently being developed.
:::

Configs represent the individual, configurable attributes specific to a HAL device.
The configs available for a device varies from device to device,
but can be altered during runtime via user input or by other devices and can be saved to flash to retain the same value through power cycles.

For example, the HAL device for the IR/White LEDs may only have a "brightness" config, while a speaker device may have configs for "volume", "left/right balance", etc.

:::note
Each device can have a maximum of `MAXIMUM_CONFIGS_PER_DEVICE` configs (see `framework/inc/fwk_common.h`).
:::

Each device config regardless of device type has the same fields:

- [`name`](#name)
- [`expectedValue`](#expectedValue)
- [`description`](#description)
- [`value`](#value)
- [`get`](#get)
- [`set`](#set)

### name

A string containing the name of the config.
The string length should be less than DEVICE_CONFIG_NAME_MAX_LENGTH.

```c
char name[DEVICE_CONFIG_NAME_MAX_LENGTH];
```

### expectedValue

A string which provides a description of the valid values associated with the config.
The length of the string should be less than `DEVICE_CONFIG_EXPECTED_VAL_MAX_LENGTH`.

```c
char expectedValue[DEVICE_CONFIG_EXPECTED_VAL_MAX_LENGTH];
```

### description

A string which provides a description of the config.
The length of the string should be less than DEVICE_CONFIG_DESCRIPTION_MAX_LENGTH.

```c
char description[DEVICE_CONFIG_DESCRIPTION_MAX_LENGTH];
```

### value

An int which stores the internal value of the config. 
`value` should be set using the `set` function and retrieved using the `get` function.

```c
uint32_t value;
```

### get

A function which returns the `value` of the config.


```c
status_t (*get)(char *valueToString);
```

### set

A function which sets the `value` of the config.

```c
status_t (*set)(char *configName, uint32_t value);
```
