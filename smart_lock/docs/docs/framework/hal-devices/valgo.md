---
sidebar_position: 6
---

# VAlgo Devices

The Vision Algorithm HAL device type represents an abstraction for computer vision algorithms which are used for analysis of digital images, videos, and other visual inputs.

The crux of the design for Vision Algorithm devices is centered around the use of "infer complete" events which communicate information about the results of inferencing which is handled by the device.
For example, in the Smart Lock application, the Vision Algorithm may receive a camera frame containing a recognized face,
perform an inference on that data,
and communicate a "face recognized" message to other devices so that they may act accordingly.
For more information about events and event handling, see [Events](../events/overview.md).

Currently, only one vision algorithm device can be registered to the Vision Manager at a time per the design of the framework.

## Device Definition

The HAL device definition for vision algorithm devices can be found under `framework/hal_api/hal_valgo_dev.h` and is reproduced below:

```c title="vision_algo_dev_t"
/*! @brief definition of a vision algo device */
typedef struct _vision_algo_dev
{
    /* unique id which is assigned by vision algorithm manager during the registration */
    int id;
    /* name to identify */
    char name[DEVICE_NAME_MAX_LENGTH];
    /* private capability */
    valgo_dev_private_capability_t cap;
    /* operations */
    vision_algo_dev_operator_t *ops;
    /* private data */
    vision_algo_private_data_t data;
} vision_algo_dev;
```

The [operators](#operators) associated with the vision algo HAL device are as shown below:

```c
/*! @brief Operation that needs to be implemented by a vision algorithm device */
typedef struct
{
    /* initialize the dev */
    hal_valgo_status_t (*init)(vision_algo_dev_t *dev, valgo_dev_callback_t callback, void *param);
    /* deinitialize the dev */
    hal_valgo_status_t (*deinit)(vision_algo_dev_t *dev);
    /* run the inference */
    hal_valgo_status_t (*run)(const vision_algo_dev_t *dev, void *data);
    /* recv events */
    hal_valgo_status_t (*inputNotify)(const vision_algo_dev_t *receiver, void *data);

} vision_algo_dev_operator_t;
```

The [capabilities](#capabilities) associated with the vision algo HAL device are as shown below:

```c
typedef struct _valgo_dev_private_capability
{
    /* callback */
    valgo_dev_callback_t callback;
    /* param for the callback */
    void *param;
} valgo_dev_private_capability_t;
```

The [private data](#private-data) fields associated with the vision algo HAL device is as shown below:

```c
typedef struct
{
    int autoStart;
    /* frame type definition */
    vision_frame_t frames[kVAlgoFrameID_Count];
} vision_algo_private_data_t;
```

## Operators

Operators are functions which "operate" on a HAL device itself.
Operators are akin to "public methods" in object oriented-languages,
and are used by the Vision Algorithm Manager to setup, start, etc. its registered vision algo device.

For more information about operators, see [Operators](overview.md#Operators)

### Init

```c
hal_valgo_status_t (*init)(vision_algo_dev_t *dev, valgo_dev_callback_t callback, void *param);
```

Init the vision algo HAL device.

`Init` should initialize any hardware resources the device requires (I/O ports, IRQs, etc.), turn on the hardware, and perform any other setup required by the device.

The [callback function](#callback) to the device's manager is typically installed as part of the `Init` function as well.

This operator will be called by the vision algorithm manager when the output manager task first starts.

### Deinit

```c
hal_valgo_status_t (*deinit)(vision_algo_dev_t *dev);
```

The `DeInit` function is used to "deinitialize" the algorithm device.
`DeInit` should release any hardware resources the device uses (I/O ports, IRQs, etc.), turn off the hardware, and perform any other shutdown required by the device.

This operator will be called by the Vision Algorithm Manager when the Vision Algorithm Manager task ends[^1].

:::note
[^1]The `DeInit` function generally will not be called under normal operation.
:::

### Run

```c
hal_valgo_status_t (*run)(const voice_algo_dev_t *dev, void *data);
```

Begin running the vision algorithm.

The `run` operator is used to start running algorithm inference and processing camera frame data.

This operator is called by the Vision Algorithm manager when
a "camera frame ready" message is received from the Camera Manager and forwarded to the algorithm device via the Vision Algorithm Manager.

Once the Vision Algorithm device finishes processing the camera frame data, its manager will forward this message to the Output Manager in the form of an "inference complete" message.

### InputNotify

```c
hal_valgo_status_t (*inputNotify)(const vision_algo_dev_t *receiver, void *data);
```

Handle input events.

The `InputNotify` operator is called by the Vision Algorithm Manager whenever a `kFWKMessageID_InputNotify` message is received and forwarded from the Vision Algorithm Manager's message queue.

For more information regarding events and event handling, see [Events](../events/overview.md).

## Capabilities

The `capabilities` struct is primarily used for storing a callback to communicate information from the device back to the Vision Algorithm Manager.
This callback function is typically installed via a device's `init` operator.

### callback

```c title="valgo_dev_callback_t"
/*!
 * @brief Callback function to notify managers the results of inference
 * valgo_dev* dev Pointer to an algorithm device
 * valgo_event_t event Event which took place
 * void* param Pointer to a struct of data that needs to be forwarded
 * unsigned int size Size of the struct that needs to be forwarded. If size = 0, param should be a pointer to a
 * persistent memory area.
 */

typedef int (*valgo_dev_callback_t)(int devId, valgo_event_t event, void *param, unsigned int size, uint8_t fromISR);
```

```c
valgo_dev_callback_t callback;
```

Callback to the Vision Algorithm Manager.

The Vision Algorithm manager will provide the callback to the device when the `init` operator is called.
As a result, the HAL device should make sure to store the callback in the `init` operator's implementation.

```c title="Example Vision Algorithm Device Init" {9-11}
static hal_valgo_status_t HAL_VisionAlgoDev_ExampleDev_Init(vision_algo_dev_t *dev,
                                                           valgo_dev_callback_t callback,
                                                           void *param)
{
    hal_valgo_status_t ret = kStatus_HAL_ValgoSuccess;

    /* PERFORM INIT FUNCTIONALITY HERE */

    ...

    /* Installing callback function from manager... */
    memset(&dev->cap, 0, sizeof(dev->cap));
    dev->cap.callback = callback;

    return ret;
}
```

The HAL device invokes this callback to notify the Vision Algorithm manager of specific events.

### param

```c
void *param;
```

The param for the callback (optional).

## Private Data

<!-- TODO: Describe the purpose of this field -->
### autoStart

```c
int autoStart;
```

The flag for automatically starting the algorithm.

If `autoStart` is 1, the Vision Algorithm Manager will automatically start requesting camera frames for this algorithm device after its `init` operator is executed.

### frames

```c
vision_frame_t frames[kVAlgoFrameID_Count];
```

The three kinds of frames which are currently supported by the vision framework are `RGB`, `IR` and `Depth` images.

The vision algorithm device needs to specify information for each kind of frame,
so that the framework will properly convert and pass only the frames which correspond to this algorithm device's requirement.

For example, the Smart Lock application uses both 3D Depth and IR camera images to perform liveness
detection and face recognition,
while using RGB frames solely for use as user feedback to help with
aligning a user's face, etc.
Therefore, the algorithm device needs to ensure that it is receiving only the 3D and IR frames and
not any RGB frames.

The definition of `vision_frame_t` is as shown below:

```c
typedef struct _vision_frame
{
    /* is supported by the device for this type of frame */
    /* Vision Algorithm Manager will only request the supported frame for this device */
    int is_supported;

    /* frame resolution */
    int height;
    int width;
    int pitch;

    /* rotate degree */
    cw_rotate_degree_t rotate;
    flip_mode_t flip;
    /* swap byte per two bytes */
    int swapByte;

    /* pixel format */
    pixel_format_t format;

    /* the source pixel format of the requested frame */
    pixel_format_t srcFormat;
    void *data;
} vision_frame_t;
```

## Example

Because only one Vision Algorithm device can be registered at a time per the design of the framework,
the SLN-VIZN3D-IOT Smart Lock project has one Vision Algorithm device implemented.[^2]

:::info
[^2]This example is implemented using NXP's OasisLite face recognition algorithm,
which is the core vision computing algorithm used in the SLN-VIZN3D-IOT Smart Lock project.
:::

This example is reproduced below:

```c
static hal_valgo_status_t HAL_VisionAlgoDev_OasisLite_Init(vision_algo_dev_t *dev,
                                                           valgo_dev_callback_t callback,
                                                           void *param);
static hal_valgo_status_t HAL_VisionAlgoDev_OasisLite_Deinit(vision_algo_dev_t *dev);
static hal_valgo_status_t HAL_VisionAlgoDev_OasisLite_Run(const vision_algo_dev_t *dev, void *data);
static hal_valgo_status_t HAL_VisionAlgoDev_OasisLite_InputNotify(const vision_algo_dev_t *receiver, void *data);

/* vision algorithm device operators */
const static vision_algo_dev_operator_t s_VisionAlgoDev_OasisLiteOps = {
    .init        = HAL_VisionAlgoDev_OasisLite_Init,
    .deinit      = HAL_VisionAlgoDev_OasisLite_Deinit,
    .run         = HAL_VisionAlgoDev_OasisLite_Run,
    .inputNotify = HAL_VisionAlgoDev_OasisLite_InputNotify,
};

/* vision algorithm device */
static vision_algo_dev_t s_VisionAlgoDev_OasisLite3D = {
    .id   = 0,
    .name = "OASIS_3D",
    .ops  = (vision_algo_dev_operator_t *)&s_VisionAlgoDev_OasisLiteOps,
    .cap  = {.param = NULL},
};

/* vision algorithm device Init function*/
static hal_valgo_status_t HAL_VisionAlgoDev_OasisLite_Init(vision_algo_dev_t *dev,
                                                           valgo_dev_callback_t callback,
                                                           void *param)
{
    LOGI("++HAL_VisionAlgoDev_OasisLite_Init");
    hal_valgo_status_t ret = kStatus_HAL_ValgoSuccess;

    // init the device
    memset(&dev->cap, 0, sizeof(dev->cap));
    dev->cap.callback = callback;

    /* set parameters of the requested frames that this vision algorithm dev asks for*/
    /* for example oasisLite algorithm asks for two kind of frames: one is IR, the other is Depth */
    /* firstly set parameters of the requested IR frames */ 
    dev->data.autoStart                             = 1;
    dev->data.frames[kVAlgoFrameID_IR].height       = OASIS_FRAME_HEIGHT;
    dev->data.frames[kVAlgoFrameID_IR].width        = OASIS_FRAME_WIDTH;
    dev->data.frames[kVAlgoFrameID_IR].pitch        = OASIS_FRAME_WIDTH * 3;
    dev->data.frames[kVAlgoFrameID_IR].is_supported = 1;
    dev->data.frames[kVAlgoFrameID_IR].rotate       = kCWRotateDegree_0;
    dev->data.frames[kVAlgoFrameID_IR].flip         = kFlipMode_None;
    dev->data.frames[kVAlgoFrameID_IR].format    = kPixelFormat_BGR;
    dev->data.frames[kVAlgoFrameID_IR].srcFormat = kPixelFormat_Gray16;
    int oasis_lite_rgb_frame_aligned_size        = SDK_SIZEALIGN(OASIS_FRAME_HEIGHT * OASIS_FRAME_WIDTH * 3, 64);
    dev->data.frames[kVAlgoFrameID_IR].data      = pvPortMalloc(oasis_lite_rgb_frame_aligned_size);

    if (dev->data.frames[kVAlgoFrameID_IR].data == NULL)
    {
        OASIS_LOGE("[ERROR]: Unable to allocate memory for kVAlgoFrameID_IR.");
        ret = kStatus_HAL_ValgoMallocError;
        return ret;
    }
    /* secondly set parameters of the requested Depth frames */
    dev->data.frames[kVAlgoFrameID_Depth].height       = OASIS_FRAME_HEIGHT;
    dev->data.frames[kVAlgoFrameID_Depth].width        = OASIS_FRAME_WIDTH;
    dev->data.frames[kVAlgoFrameID_Depth].pitch        = OASIS_FRAME_WIDTH * 2;
    dev->data.frames[kVAlgoFrameID_Depth].is_supported = 1;
    dev->data.frames[kVAlgoFrameID_Depth].rotate       = kCWRotateDegree_0;
    dev->data.frames[kVAlgoFrameID_Depth].flip         = kFlipMode_None;

    dev->data.frames[kVAlgoFrameID_Depth].format    = kPixelFormat_Depth16;
    dev->data.frames[kVAlgoFrameID_Depth].srcFormat = kPixelFormat_Depth16;
    int oasis_lite_depth_frame_aligned_size         = SDK_SIZEALIGN(OASIS_FRAME_HEIGHT * OASIS_FRAME_WIDTH * 2, 64);
    dev->data.frames[kVAlgoFrameID_Depth].data      = pvPortMalloc(oasis_lite_depth_frame_aligned_size);

    if (dev->data.frames[kVAlgoFrameID_Depth].data == NULL)
    {
        OASIS_LOGE("Unable to allocate memory for kVAlgoFrameID_IR");
        ret = kStatus_HAL_ValgoMallocError;
        return ret;
    }
    
    /* do private Algorithm Init here */
    ...

    LOGI("--HAL_VisionAlgoDev_OasisLite_Init");
    return ret;
}

/* vision algorithm device DeInit function*/
static hal_valgo_status_t HAL_VisionAlgoDev_OasisLite_Deinit(vision_algo_dev_t *dev)
{
    hal_valgo_status_t ret = kStatus_HAL_ValgoSuccess;
    LOGI("++HAL_VisionAlgoDev_OasisLite_Deinit");

    /* release resource here */
    ...

    LOGI("--HAL_VisionAlgoDev_OasisLite_Deinit");
    return ret;
}

/* vision algorithm device inference run function*/
static hal_valgo_status_t HAL_VisionAlgoDev_OasisLite_Run(const vision_algo_dev_t *dev, void *data)
{
    hal_valgo_status_t ret = kStatus_HAL_ValgoSuccess;
    OASIS_LOGI("++HAL_VisionAlgoDev_OasisLite_Run");

    vision_algo_result_t result;
    /* do inference run, derive meaningful information from the current frame data in dev private data */
    /* for example, oasisLite will inference according to two kinds of input frames: 
       void* frame1 = dev->data.frames[kVAlgoFrameID_IR].data
       void* frame2 = dev->data.frames[kVAlgoFrameID_Depth].data
       result = oasisLite_run(frame1, frame2, ......);
    */
    ...

    /* execute algorithm manager callback to inform algorithm manager the result */
    if (dev != NULL && result != NULL && dev->cap.callback != NULL)
    {
        dev->cap.callback(dev->id, kVAlgoEvent_VisionResultUpdate, result, sizeof(vision_algo_result_t), 0);
    }

    OASIS_LOGI("--HAL_VisionAlgoDev_OasisLite_Run");
    return ret;
}

/* vision algorithm device InputNotify function*/
static hal_valgo_status_t HAL_VisionAlgoDev_OasisLite_InputNotify(const vision_algo_dev_t *receiver, void *data)
{
    hal_valgo_status_t ret = kStatus_HAL_ValgoSuccess;
    OASIS_LOGI("++HAL_VisionAlgoDev_OasisLite_InputNotify");
    event_base_t eventBase = *(event_base_t *)data;

    /* do proess according to different input notify event */
    ...

    LOGI("--HAL_VisionAlgoDev_OasisLite_InputNotify");
    return ret;
}

/* register vision algorithm device to vision algorithm manager */
int HAL_VisionAlgoDev_OasisLite3D_Register()
{
    int error = 0;
    LOGD("HAL_VisionAlgoDev_OasisLite3D_Register");
    error = FWK_VisionAlgoManager_DeviceRegister(&s_VisionAlgoDev_OasisLite3D);

    return error;
}
```
