---
sidebar_position: 2
---

# Input Devices

The `Input` HAL device provides an abstraction to implement a variety of devices which may capture data in many different ways,
and whose data can represent many different things.
The Input HAL device definition is designed to encapsulate everything from physical devices like push buttons,
to "virtual" devices like a command line interface using UART.

Input devices are used to acquire external input data and forward that data to other HAL devices via the Input Manager so that those devices can respond to that data accordingly.
The Input Manager communicates to other devices within the framework using `inputNotify` event messages.
For more information about events and event handling, see [Events](../events/overview.md).

As with other device types,
`Input` devices are controlled via their manager.
The Input Manager is responsible for managing all registered input HAL devices,
and invoking input device operators (`init`, `start`, `dequeue`, etc.) as necessary.
Additionally,
the Input Manager allows for multiple input devices to be registered and operate at once.

## Device Definition

The HAL device definition for `Input` devices can be found under `framework/hal_api/hal_input_dev.h` and is reproduced below:

```c title="input_dev_t"
/*! @brief Attributes of an input device */
typedef struct _input_dev
{
    /* unique id which is assigned by input manager during the registration */
    int id;
    /* name of the device */
    char name[DEVICE_NAME_MAX_LENGTH];
    /* operations */
    const input_dev_operator_t *ops;
    /* private capability */
    input_dev_private_capability_t cap;
} input_dev_t;
```

The device [operators](#operators) associated with input HAL devices are as shown below:

```c
/*! @brief Operation that needs to be implemented by an input device */
typedef struct
{
    /* initialize the dev */
    hal_input_status_t (*init)(input_dev_t *dev, input_dev_callback_t callback);
    /* deinitialize the dev */
    hal_input_status_t (*deinit)(const input_dev_t *dev);
    /* start the dev */
    hal_input_status_t (*start)(const input_dev_t *dev);
    /* start the dev */
    hal_input_status_t (*stop)(const input_dev_t *dev);
    /* notify the input_dev */
    hal_input_status_t (*inputNotify)(const input_dev_t *dev, void *param);
} input_dev_operator_t;
```

The device [capabilities](#capabilities) associated with input HAL devices are as shown below:

```c
typedef struct
{
    /* callback */
    input_dev_callback_t callback;
} input_dev_private_capability_t;
```

## Operators

Operators are functions which "operate" on a HAL device itself.
Operators are akin to "public methods" in object oriented-languages,
and are used by the Input Manager to setup, start, etc. each of its registered input devices.

For more information about operators, see [Operators](overview.md#Operators)

### Init

```c
/* initialize the dev */
hal_input_status_t (*init)(input_dev_t *dev, input_dev_callback_t callback);
```

Initialize the input device.

`Init` should initialize any hardware resources the input device requires (I/O ports, IRQs, etc.), turn on the hardware, and perform any other setup the device requires.

The [callback function](#callback) to the device's manager is typically installed as part of the `Init` function as well.

This operator will be called by the Input Manager when the Input Manager task first starts.

### Deinit

```c
/* deinitialize the dev */
hal_input_status_t (*deinit)(const input_dev_t *dev);
```

"Deinitialize" the input device.

`DeInit` should release any hardware resources the input device uses (I/O ports, IRQs, etc.), turn off the hardware, and perform any other shutdown the device requires.

This operator will be called by the Input Manager when the Input Manager task ends[^1].

:::note
[^1]The `DeInit` function generally will not be called under normal operation.
:::

### Start

```c
/* start the dev */
hal_input_status_t (*start)(const input_dev_t *dev);
```

Start the input device.

The `Start` operator will be called in the initialization stage of the Input Manager's task after the call to the `Init` operator.
The startup of the display sensor and interface should be implemented in this operator.
This includes, for example, starting the interface and enabling the IRQ of the DMA used by the interface.

### Stop

```c
/* start the dev */
hal_input_status_t (*stop)(const input_dev_t *dev);
```

Stop the input device.

The `Stop` operator functions as the inverse of the `Start` function and will generally not be called under normal operation.

### InputNotify

```c
/* notify the input_dev */
hal_input_status_t (*inputNotify)(const input_dev_t *dev, void *param);
```

Handle input events.

The `InputNotify` operator is called by the Input Manager whenever a `kFWKMessageID_InputNotify` message received by and forwarded from the Input Manager's message queue.

For more information regarding events and event handling, see [Events](../events/overview.md).

## Capabilities

```c title="input_dev_private_capability_t"
typedef struct
{
    /* callback */
    input_dev_callback_t callback;
} input_dev_private_capability_t;
```

The `capabilities` struct is primarily used for storing a callback to communicate information from the device back to the Input Manager.
This callback function is typically installed via a device's `init` operator.

### callback

```c
/**
 * @brief callback function to notify input manager with an async event
 * @param dev Device structure
 * @param eventId Id of the event that took place
 * @param receiverList List with managers that should be notify
 * @param event Pointer to a event structure.
 * @param size If size is 0 event should be in a persistent memory zone else the framework will allocate memory for the
 * object Note the message delivery might go slow if the size is too much.
 * @param fromISR True if this operation takes place in an irq, 0 otherwise
 * @return 0 if the operation was successfully
 */
typedef int (*input_dev_callback_t)(const input_dev_t *dev,
                                    input_event_id_t eventId,
                                    unsigned int receiverList,
                                    input_event_t *event,
                                    unsigned int size,
                                    uint8_t fromISR);
```

Callback to the Input Manager.

The `capabilities` struct is primarily used for storing a callback to communicate information from the device back to the Input Manager.

The Vision Algorithm manager will provide the callback to the device when the `init` operator is called.
As a result, the HAL device should make sure to store the callback in the `init` operator's implementation.

```c title="Example Input Dev Init" {9-11}
static hal_input_status_t HAL_InputDev_PushButtons_Init(input_dev_t *dev, input_dev_callback_t callback)
{
    hal_input_status_t error = 0;

    /* PERFORM INIT FUNCTIONALITY HERE */

    /* Installing callback function from manager... */
    memset(&dev->cap, 0, sizeof(dev->cap));
    dev->cap.callback = callback;

    return ret;
}
```

The HAL device invokes this callback to notify the vision algorithm manager of specific events.

The definition for `valgo_dev_callback_t` is as shown below:

```c title="input_dev_callback_t"
typedef int (*input_dev_callback_t)(const input_dev_t *dev,
                                    input_event_id_t eventId,
                                    unsigned int receiverList,
                                    input_event_t *event,
                                    unsigned int size,
                                    uint8_t fromISR);
```

The fields passed as part of the callback are described in more detail below.

### eventId

```c
typedef enum _input_event_id
{
    kInputEventID_Recv,
    kInputEventID_AudioRecv,
    kInputEventID_FrameworkRecv,
} input_event_id_t;
```

Describes the type of source event being sent/received.

### receiverList

```c
typedef enum _fwk_task_id
{
    kFWKTaskID_Camera = 0, /* This should always stay first */
    kFWKTaskID_Display,
    kFWKTaskID_VisionAlgo,
    kFWKTaskID_VoiceAlgo,
    kFWKTaskID_Output,
    kFWKTaskID_Input,
    kFWKTaskID_Audio,
    kFWKTaskID_APPStart, /* APP task ID should always start from here */
    kFWKTaskID_COUNT = (kFWKTaskID_APPStart + APP_TASK_COUNT)
} fwk_task_id_t;
```

List of device managers meant to receive the input event message.

### event

```c
typedef struct _input_event
{
    union
    {
        /* Valid when message is kInputEventID_RECV */
        void *inputData;

        /* Valid when eventId is kInputEventID_AudioRECV */
        void *audioData;

        /* Valid when framework information is needed GET_FRAMEWORK_INFO*/
        framework_request_t *frameworkRequest;
    };
} input_event_t;
```

## Example

The SLN-VIZN3D-IOT Smart Lock project has several input devices implemented for use as-is or for use as reference for implementing new input devices.
Source files for these input HAL devices can be found under `HAL/common/` and `HAL/face_rec`.

Below is an example of a push button input HAL device driver:

```c title="Example Input Device"
static input_event_t inputEvent;

const static input_dev_operator_t s_InputDev_ExampleDevOps = {
    .init        = HAL_InputDev_ExampleDev_Init,
    .deinit      = HAL_InputDev_ExampleDev_Deinit,
    .start       = HAL_InputDev_ExampleDev_Start,
    .stop        = HAL_InputDev_ExampleDev_Stop,
    .inputNotify = HAL_InputDev_ExampleDev_InputNotify,
};

static input_dev_t s_InputDev_ExampleDev = { 
    .name = "buttons", 
    .ops = &s_InputDev_ExampleDevOps, 
    .cap = {
        .callback = NULL
    },
};

/* here assume buttons push event will call this handler */
void HAL_InputDev_ExampleDev_EvtHandler(void)
{
    /* Add manager task list need notify, the id is from fwk_task_id_t. 
     * Note: here can set not only one task manager.
     */
    receiverList = 1 << kFWKTaskID_Display;

    /* load input data */
    inputEvent.inputData = NULL;
    
    /* callback inputmanager notify the corresponding manager from receiverList */
    inputDev.cap.callback(&inputDev, kInputEventID_Recv, receiverList, &inputEvent, 0, fromISR);
}

hal_input_status_t HAL_InputDev_ExampleDev_Init(input_dev_t *dev, input_dev_callback_t callback)
{
    hal_input_status_t ret = kStatus_HAL_InputSuccess;

    /* install manager callback for device */
    dev->cap.callback = callback;

    /* put hardware init here */

    return ret;
}

hal_input_status_t HAL_InputDev_ExampleDev_Deinit(const input_dev_t *dev)
{
    hal_input_status_t ret = kStatus_HAL_InputSuccess;

    /* put device deinit here */

    return ret;
}

hal_input_status_t HAL_InputDev_ExampleDev_Start(const input_dev_t *dev)
{
    hal_input_status_t ret = kStatus_HAL_InputSuccess;

    /* put device start here */

    return ret;
}

hal_input_status_t HAL_InputDev_ExampleDev_Stop(const input_dev_t *dev)
{
    hal_input_status_t ret = kStatus_HAL_InputSuccess;

    /* put device stop here */

    return ret;
}

hal_input_status_t HAL_InputDev_ExampleDev_InputNotify(const input_dev_t *dev, void *param)
{
    hal_input_status_t ret = kStatus_HAL_InputSuccess;
    
    /* add device notify handler here */

    return ret;
}

int HAL_InputDev_ExampleDev_Register(void)
{
    int ret = 0;
    ret = FWK_InputManager_DeviceRegister(&s_InputDev_ExampleDev);
    return ret;
}
```
