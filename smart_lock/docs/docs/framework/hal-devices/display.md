---
sidebar_position: 5
---

# Display Devices

The `Display` HAL device provides an abstraction to represent many different display panels which may have different controllers, resolutions, color formats, and event connection interfaces.

For example, in the VIZN3D kit, the "rk024hh298" panel is connected via the eLCDIF interface
and the rk055ahd091 panel is connected via the LCDIF v2 interface.

:::info
A display HAL devices represents a display panel + interface.

For example, the `hal_display_lcdif_rk024hh298.c` is the display HAL device driver for the rk024hh298 panel with eLCDIF interface.

This means a separate device driver is required for the same display using different interfaces.
:::

As with other device types,
display devices are controlled via their manager.
The Display Manager is responsible for managing all registered display HAL devices,
and invoking display device operators (`init`, `start`, etc.) as necessary.

## Device Definition

The HAL device definition for display devices can be found under `framework/hal_api/hal_display_dev.h` and is reproduced below:

```c title="display_dev_t"
typedef struct _display_dev display_dev_t;
/*! @brief Attributes of a display device. */
struct _display_dev
{
    /* unique id which is assigned by Display Manager during the registration */
    int id;
    /* name of the device */
    char name[DEVICE_NAME_MAX_LENGTH];
    /* operations */
    const display_dev_operator_t *ops;
    /* private capability */
    display_dev_private_capability_t cap;
};
```

The [operators](#operators) associated with display HAL devices are as shown below:

```c title="Operators"
/*! @brief Operation that needs to be implemented by a display device */
typedef struct _display_dev_operator
{
    /* initialize the dev */
    hal_display_status_t (*init)(
        display_dev_t *dev,
        int width, int height,
        display_dev_callback_t callback,
        void *param);
    /* deinitialize the dev */
    hal_display_status_t (*deinit)(const display_dev_t *dev);
    /* start the dev */
    hal_display_status_t (*start)(const display_dev_t *dev);
    /* blit a buffer to the dev */
    hal_display_status_t (*blit)(const display_dev_t *dev,
                                void *frame,
                                int width,
                                int height);
    /* input notify */
    hal_display_status_t (*inputNotify)(const display_dev_t *dev, void *data);
} display_dev_operator_t;
```

The [capabilities](#capabilities) associated with display HAL devices are as shown below:

```c title="capability"
/*! @brief Structure that characterize the display device. */
typedef struct _display_dev_private_capability
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
    /* pixel format */
    pixel_format_t format;
    /* the source pixel format of the requested frame */
    pixel_format_t srcFormat;
    void *frameBuffer;
    /* callback */
    display_dev_callback_t callback;
    /* param for the callback */
    void *param;
} display_dev_private_capability_t;
```

## Operators

Operators are functions which "operate" on a HAL device itself.
Operators are akin to "public methods" in object oriented-languages,
and are used by the Display Manager to setup, start, etc. each of its registered display devices.

For more information about operators, see [Operators](overview.md#Operators)

### Init

```c
hal_display_status_t (*init)(display_dev_t *dev,
                            int width,
                            int height,
                            display_dev_callback_t callback,
                            void *param);
```

Initialize the display device.

`Init` should initialize any hardware resources the display device requires (I/O ports, IRQs, etc.), turn on the hardware, and perform any other setup the device requires.

The [callback function](#callback) to the device's manager is typically installed as part of the `Init` function as well.

This operator will be called by the Display Manager when the Display Manager task first starts.

### Deinit

```c
hal_display_status_t (*deinit)(const display_dev_t *dev);
```

"Deinitialize" the display device.

`DeInit` should release any hardware resources the display device uses (I/O ports, IRQs, etc.), turn off the hardware, and perform any other shutdown the device requires.

This operator will be called by the Display Manager when the Display Manager task ends[^1].

:::note
[^1]The `DeInit` function generally will not be called under normal operation.
:::

### Start

```c
hal_display_status_t (*start)(const display_dev_t *dev);
```

Start the display device.

The `Start` operator will be called in the initialization stage of the Display Manager's task after the call to the `Init` operator.
The startup of the display sensor and interface should be implemented in this operator.
This includes, for example, starting the interface and enabling the IRQ of the DMA used by the interface.

### Blit

```c
hal_display_status_t (*blit)(const display_dev_t *dev,
                            void *frame,
                            int width,
                            int height);
```

Sends a frame to the display panel and "blits" the frame with any additional required components (UI overlay, etc.).

`Blit` is called by the Display Manager once a previously requested frame of the matching [srcFormat](#srcformat) has been sent by a camera device.
The sending of the frame from the Display Manager to the display panel should be take place in this operator.  

`kStatus_HAL_DisplaySuccess` should be returned if the frame was successfully sent to the display panel.
After calling this operator, the Display Manager will request a new frame.

:::note
If the `Blit` operator is working in asynchronous mode, the hardware will continue sending the frame buffer even after the return of the `Blit` function call.
In this case, `kStatus_HAL_DisplayNonBlocking` should be returned instead,
and the Display Manager will not issue a new display frame request after this `Blit` call.

To request a new frame, the device should invoke the Display Manager's callback using a `kDisplayEvent_RequestFrame` event to notify the completion of the sending of the previous frame.
Once the Display Manager sees this new request, it will requesting a new frame.
:::

### InputNotify

```c
    hal_display_status_t (*inputNotify)(const display_dev_t *dev, void *data);
```

Handle input events.

The `InputNotify` operator is called by the Display Manager whenever a `kFWKMessageID_InputNotify` message is received by and forwarded from the Display Manager's message queue.

For more information regarding events and event handling, see [Events](../events/overview.md).

## Capabilities

```c title="display_dev_private_capability_t"
/*! @brief Structure that characterizes the display device. */
typedef struct _display_dev_private_capability
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
    /* pixel format */
    pixel_format_t format;
    /* the source pixel format of the requested frame */
    pixel_format_t srcFormat;
    void *frameBuffer;
    /* callback */
    display_dev_callback_t callback;
    /* param for the callback */
    void *param;
} display_dev_private_capability_t;
```

The `capabilities` struct is primarily used for storing a callback to communicate information from the device back to the Display Manager.
This callback function is typically installed via a device's `init` operator.

Display devices also maintain information regarding the size of the display, pixel format, and other information pertinent to the display.

### height

```c
int height;    
```

The height of the display buffer.

### width

```c
int width;    
```

The width of the display buffer.

### pitch

```c
int pitch;
```

The total number of bytes in one row of the display buffer.

### left

```c
int left;    
```

The left edge of the active area[^1] in the display frame buffer.

:::info
[^1]The active area indicates the area of the display frame buffer that will be utilized.
:::

### top

```c
int top;    
```

The top edge of the active area in the display frame buffer.

### right

```c
int right;    
```

The right edge of the active area in the display frame buffer.

### bottom

```c
int bottom;
```

The bottom edge of the active area in the display frame buffer.

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

The rotate degree of the display frame buffer.

### format

```c
typedef enum _pixel_format
{
    /* 2d frame format */
    kPixelFormat_RGB,
    kPixelFormat_RGB565,
    kPixelFormat_BGR,
    kPixelFormat_Gray888,
    kPixelFormat_Gray888X,
    kPixelFormat_Gray,
    kPixelFormat_Gray16,
    kPixelFormat_YUV1P444_RGB,   /* color display sensor */
    kPixelFormat_YUV1P444_Gray,  /* ir display sensor */
    kPixelFormat_UYVY1P422_RGB,  /* color display sensor */
    kPixelFormat_UYVY1P422_Gray, /* ir display sensor */
    kPixelFormat_VYUY1P422,

    /* 3d frame format */
    kPixelFormat_Depth16,
    kPixelFormat_Depth8,

    kPixelFormat_YUV420P,

    kPixelFormat_Invalid
} pixel_format_t;
```

The format of the display frame buffer.

### srcFormat

The source format of the requested display frame buffer.

Because there may be multiple display devices operating at a time,
the display will check the `srcFormat` property of the frame to determine whether it is from the display device it is expecting.
This prevents the display from displaying a 3D depth image when the user expects an RGB image, for example.

### frameBuffer

Pointer to the display frame buffer.

### callback

```c title="display_dev_callback_t"
/**
 * @brief callback function to notify Display Manager that an async event took place
 * @param dev Device structure of the display device calling this function
 * @param event id of the event that took place
 * @param param Parameters
 * @param fromISR True if this operation takes place in an irq, 0 otherwise
 * @return 0 if the operation was successfully
 */
typedef int (*display_dev_callback_t)(const display_dev_t *dev,
                                    display_event_t event,
                                    void *param,
                                    uint8_t fromISR);
```

```c
display_dev_callback_t callback;   
```

Callback to the Display Manager.
The HAL device invokes this callback to notify the Display Manager of specific events.

:::note
Currently, only the `kDisplayEvent_RequestFrame` event callback is implemented in the Display Manager.
:::

The Display Manager will provide this callback to the device when the `init` operator is called.
As a result, the HAL device should make sure to store the callback in the `init` operator's implementation.

```c title="Example Display Device Init" {10}
hal_display_status_t HAL_DisplayDev_ExampleDev_Init(
    display_dev_t *dev, int width, int height, display_dev_callback_t callback, void *param)
{
    hal_display_status_t ret = kStatus_HAL_DisplaySuccess;

    /* PERFORM INIT FUNCTIONALITY HERE */

    ...
    
    /* Installing callback function from manager... */
    dev->cap.callback    = callback;
    
    return ret;
}
```

The HAL device invokes this callback to notify the Display Manager of specific events.

### param

```c
void *param;
```

The parameter of the Display Manager callback.[^2]

:::note
The `param` field is not currently used by the framework in any way.
::::

## Example

The SLN-VIZN3D-IOT Smart Lock project has several display devices implemented for use as-is or as reference for implementing new display devices.
The source files for these display HAL devices can be found under `HAL/common/`.

Below is an example of the "rk024hh298" display HAL device driver `HAL/common/hal_display_lcdif_rk024hh298.c`.

```c title="HAL/common/hal_display_lcdif_rk024hh298.c"

hal_display_status_t HAL_DisplayDev_LcdifRk024hh2_Init(display_dev_t *dev,
                                                        int width,
                                                        int height,
                                                        display_dev_callback_t callback,
                                                        void *param);
hal_display_status_t HAL_DisplayDev_LcdifRk024hh2_Uninit(const display_dev_t *dev);
hal_display_status_t HAL_DisplayDev_LcdifRk024hh2_Start(const display_dev_t *dev);
hal_display_status_t HAL_DisplayDev_LcdifRk024hh2_Blit(const display_dev_t *dev,
                                                        void *frame,
                                                        int width,
                                                        int height);
static hal_display_status_t HAL_DisplayDev_LcdifRk024hh2_InputNotify(const display_dev_t *receiver,
                                                                    void *data);

/* The operators of the rk024hh298 Display HAL Device */
const static display_dev_operator_t s_DisplayDev_LcdifOps = {
    .init        = HAL_DisplayDev_LcdifRk024hh2_Init,
    .deinit      = HAL_DisplayDev_LcdifRk024hh2_Uninit,
    .start       = HAL_DisplayDev_LcdifRk024hh2_Start,
    .blit        = HAL_DisplayDev_LcdifRk024hh2_Blit,
    .inputNotify = HAL_DisplayDev_LcdifRk024hh2_InputNotify,
};

/* rk024hh298 Display HAL Device */
static display_dev_t s_DisplayDev_Lcdif = {
    .id   = 0,
    .name = DISPLAY_NAME,
    .ops  = &s_DisplayDev_LcdifOps,
    .cap  = {
        .width       = DISPLAY_WIDTH,
        .height      = DISPLAY_HEIGHT,
        .pitch       = DISPLAY_WIDTH * DISPLAY_BYTES_PER_PIXEL,
        .left        = 0,
        .top         = 0,
        .right       = DISPLAY_WIDTH - 1,
        .bottom      = DISPLAY_HEIGHT - 1,
        .rotate      = kCWRotateDegree_0,
        .format      = kPixelFormat_RGB565,
        .srcFormat   = kPixelFormat_UYVY1P422_RGB,
        .frameBuffer = NULL,
        .callback    = NULL,
        .param       = NULL
        }
    };

hal_display_status_t HAL_DisplayDev_LcdifRk024hh2_Init(display_dev_t *dev,
                                                        int width,
                                                        int height,
                                                        display_dev_callback_t callback,
                                                        void *param)
{
    hal_display_status_t ret = kStatus_HAL_DisplaySuccess;

    /* init the capability */
    dev->cap.width       = width;
    dev->cap.height      = height;
    dev->cap.frameBuffer = (void *)&s_FrameBuffers[1];

    /* store the callback and param for late using */
    dev->cap.callback    = callback;

    /* init the low level display panel and interface */

    return ret;
}

hal_display_status_t HAL_DisplayDev_LcdifRk024hh2_Uninit(const display_dev_t *dev)
{
    hal_display_status_t ret = kStatus_HAL_DisplaySuccess;
    /* Currently do nothing for the Deinit as we didn't support the runtime de-registraion of the device */
    return ret;
}

hal_display_status_t HAL_DisplayDev_LcdifRk024hh2_Start(const display_dev_t *dev)
{
    hal_display_status_t ret = kStatus_HAL_DisplaySuccess;

    /* start the display pannel and the interface */
    
    return ret;
}

hal_display_status_t HAL_DisplayDev_LcdifRk024hh2_Blit(const display_dev_t *dev, void *frame, int width, int height)
{
    hal_display_status_t ret = kStatus_HAL_DisplayNonBlocking;
    
    /* blit the frame to the real display pannel */

    return ret;
}

static hal_display_status_t HAL_DisplayDev_LcdifRk024hh2_InputNotify(const display_dev_t *receiver, void *data)
{
    hal_display_status_t error           = kStatus_HAL_DisplaySuccess;
    event_base_t eventBase               = *(event_base_t *)data;
    event_status_t event_response_status = kEventStatus_Ok;

    /* handle the events which are interested in */
    if (eventBase.eventId == kEventID_SetDisplayOutputSource)
    {
        
    }
    
    return error;
}
```