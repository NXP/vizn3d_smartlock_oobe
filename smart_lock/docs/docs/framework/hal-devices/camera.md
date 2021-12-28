---
sidebar_position: 4
---

# Camera Devices

The `Camera` HAL device provides an abstraction to represent many different camera devices
which may have different resolutions, color formats, and even connection interfaces.

For example, the same GC0308 RGB camera can connect with CSI or via a FlexIO interface.

:::info
A camera HAL device represents a camera sensor + interface,
meaning a separate device driver is required for the same camera sensor using different interfaces.
:::

As with other device types,
camera devices are controlled via their manager.
The Camera Manager is responsible for managing all registered camera HAL devices,
and invoking camera device operators (`init`, `start`, `dequeue`, etc.) as necessary.
Additionally,
the Camera Manager allows for multiple camera devices to be registered and operate at once.

## Device Definition

The HAL device definition for `Camera` devices can be found under `framework/hal_api/hal_camera_dev.h` and is reproduced below:

```c title="camera_dev_t"
typedef struct _camera_dev camera_dev_t;
/*! @brief Attributes of a camera device. */
struct _camera_dev
{
    /* unique id which is assigned by Camera Manager during the registration */
    int id;
    /* state in which the device is found */
    hal_device_state_t state;
    /* name of the device */
    char name[DEVICE_NAME_MAX_LENGTH];

    /* operations */
    const camera_dev_operator_t *ops;
    /* static configs */
    camera_dev_static_config_t config;
    /* private capability */
    camera_dev_private_capability_t cap;
};
```

The device [operators](#operators) associated with camera HAL devices are as shown below:

```c title="Operators"
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
     * which is critical for the whole system as Camera Manager is running with the highest priority.
     *
     * Camera Manager will do the postProcess if there is a consumer of this frame.
     *
     * Note:
     * Camera Manager will call multiple times of the posProcess of the same frame determinted by dequeue.
     * The HAL driver needs to guarantee the postProcess only do once for the first call.
     *
     */
    hal_camera_status_t (*postProcess)(const camera_dev_t *dev, void **data, pixel_format_t *format);
    /* input notify */
    hal_camera_status_t (*inputNotify)(const camera_dev_t *dev, void *data);
} camera_dev_operator_t;
```

The [static configs](#static-config) associated with camera HAL devices are as shown below:

```c title="config"
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

The device [capabilities](#capabilities) associated with camera HAL devices are as shown below:

```c title="config"
/*! @brief Structure that capability of the camera device. */
typedef struct
{
    /* callback */
    camera_dev_callback_t callback;
    /* param for the callback */
    void *param;
} camera_dev_private_capability_t;
```

## Operators

Operators are functions which "operate" on a HAL device itself.
Operators are akin to "public methods" in object oriented-languages,
and are used by the Camera Manager to setup, start, etc. each of its registered camera devices.

For more information about operators, see [Operators](overview.md#Operators)

### Init

```c
hal_camera_status_t (*init)(camera_dev_t *dev, 
                            int width,
                            int height,
                            camera_dev_callback_t callback,
                            void *param);
```

Initialize the camera device.

`Init` should initialize any hardware resources the camera device requires (I/O ports, IRQs, etc.), turn on the hardware, and perform any other setup the device requires.

This operator will be called by the Camera Manager when the Camera Manager task first starts.

### Deinit

```c
hal_camera_status_t (*deinit)(camera_dev_t *dev);
```

"Deinitialize" the camera device.

`DeInit` should release any hardware resources the camera device uses (I/O ports, IRQs, etc.), turn off the hardware, and perform any other shutdown the device requires.

This operator will be called by the Camera Manager when the Camera Manager task ends[^1].

:::note
[^1]The `DeInit` function generally will not be called under normal operation.
:::

### Start

```c
hal_camera_status_t (*start)(const camera_dev_t *dev);
```

Start the camera device.

The `Start` operator will be called in the initialization stage of the Camera Manager's task after the call to the `Init` operator.
The startup of the camera sensor and interface should be implemented in this operator.
This includes, for example, starting the interface and enabling the IRQ of the DMA used by the interface.

### Enqueue

```c
hal_camera_status_t (*enqueue)(const camera_dev_t *dev,
                                    void *data);
```

Enqueue a single frame.

The `Enqueue` operator is called by the Camera Manager to submit an empty buffer into the camera device's buffer queue.
Once the submitted buffer is filled by the camera device, the camera device should call the Camera Manager's callback function and pass a `kCameraEvent_SendFrame` event.

### Dequeue

```c
hal_camera_status_t (*enqueue)(const camera_dev_t *dev,
                                    void *data);
```

Dequeue a single frame.

The `Dequeue` operator will be called by the Camera Manager to get a camera frame from the device.
The frame address and the format will be determined by this operator.

### PostProcess

```c
hal_camera_status_t (*postProcess)(const camera_dev_t *dev,
                                    void **data,
                                    pixel_format_t *format);
```

Handles the post-processing of the camera frame.

The `PostProcess` operator is called by the Camera Manager to perform any required post-processing of the camera frame.
For example, if a frame needs to be converted from one format to another in some way before it is useable by the display and/or a vision algo device, this would take place in the `PostProcess` operator.

### InputNotify

```c
hal_camera_status_t (*inputNotify)(const camera_dev_t *dev, void *data);
```

Handle input events.

The `InputNotify` operator is called by the Camera Manager whenever a `kFWKMessageID_InputNotify` message is received by and forwarded from the Camera Manager's message queue.

For more information regarding events and event handling, see [Events](../events/overview.md).

## Static Configs

Static configs,
unlike regular, dynamic [configs](overview.md#configs),
are set at compile time and cannot be changed on-the-fly.

### height

```c
int height;    
```

The height of the camera buffer.

### width

```c
int width;    
```

The width of the camera buffer.

### pitch

```c
int pitch;
```

The total number of bytes in a single row of a camera frame.

### left

```c
int left;    
```

The left edge of the active area in a camera buffer.

### top

```c
int top;    
```

The top edge of the active area in a camera buffer.

### right

```c
int right;    
```

The right edge of the active area in a camera buffer.

### bottom

```c
int bottom;
```

The bottom edge of the active area in a camera buffer.

### rotate

```c
typedef enum _cw_rotate_degree
{
    kCWRotateDegree_0 = 0,
    kCWRotateDegree_90,
    kCWRotateDegree_180,
    kCWRotateDegree_270
} cw_rotate_degree_t;
```

```c
cw_rotate_degree_t rotate;
```

The rotate degree of the camera sensor.

### flip

```c
typedef enum _flip_mode
{
    kFlipMode_None = 0,
    kFlipMode_Horizontal,
    kFlipMode_Vertical,
    kFlipMode_Both
} flip_mode_t;
```

```c
flip_mode_t flip;
```

Determines whether to flip the frame while processing the frame for the algorithm and display.

### swapByte

```c
int swapByte;
```

Determines whether to enable swapping bytes while processing a frame for algorithm and display devices.
<!-- TODO: What does this mean? -->

## Capabilities

```c title="Camera 'capabilities' Struct"
typedef struct
{
    /* callback */
    camera_dev_callback_t callback;
    /* param for the callback */
    void *param;
} camera_dev_private_capability_t;
```

The `capabilities` struct is primarily used for storing a callback to communicate information from the device back to the Camera Manager.
This callback function is typically installed via a device's `init` operator.

### callback

```c title="camera_dev_callback_t"
/**
* @brief Callback function to notify Camera Manager that one frame is dequeued
* @param dev Device structure of the camera device calling this function
* @param event id of the event that took place
* @param param Parameters
* @param fromISR True if this operation takes place in an irq, 0 otherwise
* @return 0 if the operation was successfully
*/
typedef int (*camera_dev_callback_t)(const camera_dev_t *dev,
                                camera_event_t event,
                                void *param,
                                uint8_t fromISR); 
```

```c
camera_dev_callback_t callback;   
```

Callback to the Camera Manager.

The HAL device invokes this callback to notify the Camera Manager of specific events like "frame dequeued."

The Camera Manager will provide this callback to the device when the `init` operator is called.
As a result, the HAL device should make sure to store the callback in the `init` operator's implementation.

```c title="Example Camera Dev Init" {10}
static hal_camera_status_t HAL_CameraDev_ExampleDev_Init(
    camera_dev_t *dev, int width, int height, camera_dev_callback_t callback, void *param)
{
    hal_camera_status_t ret = kStatus_HAL_CameraSuccess;

    /* PERFORM INIT FUNCTIONALITY HERE */

    ...

    /* Installing callback function from manager... */
    dev->cap.callback  = callback;

    return ret;
}
```

### param

```c
void *param;
```

The parameter of the callback for `kCameraEvent_SendFrame` event.
The Camera Manager will provide the parameter while calling the `Init` operator,
so this param should be stored in the HAL device's struct as part of the implementation of the `Init` operator.

:::note
This param should be provided when calling the [`Callback`](#callback) function.
:::

## Example

The SLN-VIZN3D-IOT Smart Lock project has several camera devices implemented for use as-is or for use as reference for implementing new camera devices.
Source files for these camera HAL devices can be found under `HAL/common/`.

Below is an example of the GC0308 RGB FlexIO camera HAL device driver `HAL/common/hal_camera_flexio_gc0308.c`.

```c title="HAL/common/hal_camera_flexio_gc0308.c"
hal_camera_status_t HAL_CameraDev_FlexioGc0308_Init(
    camera_dev_t *dev, int width, int height, camera_dev_callback_t callback, void *param);
static hal_camera_status_t HAL_CameraDev_FlexioGc0308_Deinit(camera_dev_t *dev);
static hal_camera_status_t HAL_CameraDev_FlexioGc0308_Start(const camera_dev_t *dev);
static hal_camera_status_t HAL_CameraDev_FlexioGc0308_Enqueue(const camera_dev_t *dev, void *data);
static hal_camera_status_t HAL_CameraDev_FlexioGc0308_Dequeue(const camera_dev_t *dev,
                                                              void **data,
                                                              pixel_format_t *format);
static int HAL_CameraDev_FlexioGc0308_Notify(const camera_dev_t *dev, void *data);

/* The operators of the FlexioGc0308 Camera HAL Device */
const static camera_dev_operator_t s_CameraDev_FlexioGc0308Ops = {
    .init        = HAL_CameraDev_FlexioGc0308_Init,
    .deinit      = HAL_CameraDev_FlexioGc0308_Deinit,
    .start       = HAL_CameraDev_FlexioGc0308_Start,
    .enqueue     = HAL_CameraDev_FlexioGc0308_Enqueue,
    .dequeue     = HAL_CameraDev_FlexioGc0308_Dequeue,
    .inputNotify = HAL_CameraDev_FlexioGc0308_Notify,
};

/* FlexioGc0308 Camera HAL Device */
static camera_dev_t s_CameraDev_FlexioGc0308 = {
    .id   = 0,
    .name = CAMERA_NAME,
    .ops  = &s_CameraDev_FlexioGc0308Ops,
    .cap =
        {
            .callback = NULL,
            .param    = NULL,
        },
};

hal_camera_status_t HAL_CameraDev_FlexioGc0308_Init(
    camera_dev_t *dev, int width, int height, camera_dev_callback_t callback, void *param)
{
    hal_camera_status_t ret = kStatus_HAL_CameraSuccess;
    LOGD("camera_dev_flexio_gc0308_init");    

    /* store the callback and param for late using*/
    dev->cap.callback  = callback;
    dev->cap.param     = param;

    /* init the low level camera sensor and interface */

    return ret;
}

static hal_camera_status_t HAL_CameraDev_FlexioGc0308_Deinit(camera_dev_t *dev)
{
    hal_camera_status_t ret = kStatus_HAL_CameraSuccess;
    /* Currently do nothing for the Deinit as we didn't support the runtime de-registraion of the device */
    return ret;
}

static hal_camera_status_t HAL_CameraDev_FlexioGc0308_Start(const camera_dev_t *dev)
{
    hal_camera_status_t ret = kStatus_HAL_CameraSuccess;

    /* start the low level camera sensor and interface */

    return ret;
}

static hal_camera_status_t HAL_CameraDev_FlexioGc0308_Enqueue(const camera_dev_t *dev, void *data)
{
    hal_camera_status_t ret = kStatus_HAL_CameraSuccess;

    /* submit one free buffer into the camera's buffer queue */

    return ret;
}

static hal_camera_status_t HAL_CameraDev_FlexioGc0308_Dequeue(const camera_dev_t *dev,
                                                              void **data,
                                                              pixel_format_t *format)
{
    hal_camera_status_t ret = kStatus_HAL_CameraSuccess;
    
    /* get the buffer from camera's buffer queue and determine the format of the frame */

    return ret;
}

static int HAL_CameraDev_FlexioGc0308_Notify(const camera_dev_t *dev, void *data)
{
    int error              = 0;
    event_base_t eventBase = *(event_base_t *)data;

    /* handle the events which are interested in */
    switch (eventBase.eventId)
    {        
        default:
            break;
    }

    return error;
}
                 
```
