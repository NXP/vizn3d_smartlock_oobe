---

sidebar_position: 3
---

# Output Devices

The `Output` HAL devices is used to represent any device which produces output
(excluding specific devices which have their own specific device type like cameras and displays).

`Output` devices will respond to events passed by other HAL devices and produce corresponding output.
This includes changing the UI overlay in response to a "face recognized" event,
or changing the volume of the speaker in response to a specific shell command.

Multiple output devices can be registered at a time per the design of the framework.

## Subtypes

Currently output devices can be divided into three "subtypes" to better represent the specific nuances of a wider variety of output devices without creating entirely new HAL device types:

- ["General"](#general-devices) output devices
- ["Overlay/UI"](#ui-devices) output devices
- ["Audio"](#audio-devices) output devices

### General Devices

A "general"/generic output devices describes the majority of output devices,
and includes devices like LEDs.

### UI Devices

Overlay/UI output devices are used for output devices which act as an overlay which sits on top of a camera preview surface.

Overlay/UI devices require that a framebuffer be allocated when initializing a device of this subtype.

### Audio Devices

Audio output HAL devices represent devices which act as a recipients of audio data.
Audio output HAL devices typically process audio data so that they can play a sound in response to an event like a face being registered,
or sleep mode triggering.

## Device Definition

The HAL device definition for output devices can be found under `framework/hal_api/hal_output_dev.h` and is reproduced below:

```c title="output_dev_t"
/*! @brief definition of an output device */
typedef struct _output_dev
{
    /* unique id and assigned by Output Manager when this device register */
    int id;
    /* device name */
    char name[DEVICE_NAME_MAX_LENGTH];
    /* attributes */
    output_dev_attr_t attr;
    /* optional config for private configuration of special output device */
    hal_device_config configs[MAXIMUM_CONFIGS_PER_DEVICE];

    /* operations */
    const output_dev_operator_t *ops;
}output_dev_t;
```

The [operators](#operators) associated with output HAL devices are as shown below:

```c
/*! @brief Operation that needs to be implemented by an output device */
typedef struct _output_dev_operator
{
    /* initialize the dev */
    hal_output_status_t (*init)(const output_dev_t *dev);
    /* deinitialize the dev */
    hal_output_status_t (*deinit)(const output_dev_t *dev);
    /* start the dev */
    hal_output_status_t (*start)(const output_dev_t *dev);
    /* stop the dev */
    hal_output_status_t (*stop)(const output_dev_t *dev);

} output_dev_operator_t;
```

The device [attributes](#attributes) associated with output HAL devices are as shown below:

```c
/*! @brief Attributes of an output device */
typedef struct _output_dev_attr_t
{
    /* the type of output device */
    output_dev_type_t type;
    union
    {
        /* if the type of output device is OverlayUI, it need to allocate overlay surface */
        gfx_surface_t *pSurface;
        /* reserve for other type of output device*/
        void *reserve;
    };
} output_dev_attr_t;
```

## Operators

Operators are functions which "operate" on a HAL device itself.
Operators are akin to "public methods" in object oriented-languages,
and are used by the Output Manager to setup, start, etc. each of its registered output devices.

For more information about operators, see [Operators](overview.md#Operators)

### Init

```c
hal_output_status_t (*init)(const output_dev_t *dev);
```

The `Init` function is used to initialize the output device,
`Init` should initialize any hardware resources the output device requires (I/O ports, IRQs, etc.), turn on the hardware, and perform any other setup the device requires.

This operator will be called by the Output Manager when the Output Manager task first starts.

### DeInit

```c
hal_output_status_t (*deinit)(const output_dev_t *dev);
```

The `DeInit` function is used to initialize the output device,
`DeInit` should release any hardware resources the output device uses (I/O ports, IRQs, etc.), turn off the hardware, and perform any other shutdown the device requires.

This operator will be called by the Output Manager when the Output Manager task ends[^1].

:::note
[^1]The `DeInit` function generally will not be called under normal operation.
:::

### Start

```c
hal_output_status_t (*start)(const output_dev_t *dev);
```

Starts the output device.
The `Start` method will usually call `FWK_OutputManager_RegisterEventHandler` to register event handlers with the Output Manager
so that when the Output Manager receives an output event (like an "inference complete" event or an "input notify" event),
the corresponding event handler function will be executed.

This operator is called by the Output Manager when the Output Manager task first starts.

### Stop

```c
hal_output_status_t (*stop)(const output_dev_t *dev);
```

Stops the output device.  
The `Stop` method will usually call  `FWK_OutputManager_UnRegisterEventHandler` to unregister an event handler from the Output Manager.
This prevents the device's event handlers from executing when an event is triggered.

## Attributes

### type

The type of output device. If the type is `kOutputDevType_UI`,
the `pSurface` parameter will need to be set.
Otherwise `pSurface` can safely be ignored.

```c
output_dev_type_t type;
```

The type struct is shown below:

```c title="Attributes"
/*! @brief Types of output devices */
typedef enum _output_dev_type
{
    kOutputDevType_UI,     /* for Overlay UI */
    kOutputDevType_Audio,  /* for Audio output */
    kOutputDevType_Other,  /* for other general output, like LED, Console, etc */
} output_dev_type_t;
```

### pSurface

The `pSurface` variable is used by [`Overlay/UI` output devices](#ui-devices) to hold a frame buffer.

If the device type "subtype" is not a `kOuptutDevType_UI` device, then this parameter can be safely ignored.

```c title="Attributes"
gfx_surface_t * pSurface;
```

The `gfx_surface` struct is shown below:

```c title="Attributes"
typedef struct _gfx_surface
{
    int height;  /* the height of surface */
    int width;   /* the width of surface */
    int pitch;   /* the pitch of surface */
    int left;    /* the left coordinate of surface */
    int top;     /* the top coordinate of surface */
    int right;   /* the right coordinate of surface */
    int bottom;  /* the bottom coordinate of surface */
    int swapByte; /* For each 16 bit word of surface framebuffer, set true to swap the two bytes. */
    pixel_format_t format; /* the pixel format of surface, like kPixelFormat_RGB565 */
    void *buf;  /* the pointer for the framebuffer */
    void *lock; /* the mutex lock for the surface, is determined by hal and set to null if not use in hal*/
} gfx_surface_t;
```

## Example

The SLN-VIZN3D-IOT Smart Lock project has several output devices implemented for use as-is or for use as reference for implementing new output devices.
Source files for these output HAL devices can be found under `HAL/common/`.

Below is an example of the RGB LED HAL device driver `HAL/common/hal_output_rgb_led.c`:

```c title="HAL/common/hal_output_rgb_led.c"
static hal_output_status_t HAL_OutputDev_RgbLed_Init(output_dev_t *dev);
static hal_output_status_t HAL_OutputDev_RgbLed_Start(const output_dev_t *dev);
static hal_output_status_t HAL_OutputDev_RgbLed_InferComplete(const output_dev_t *dev,
                                                              output_algo_source_t source,
                                                              void *inferResult);

const static output_dev_event_handler_t s_OutputDev_RgbLedHandler = {
    .inferenceComplete = HAL_OutputDev_RgbLed_InferComplete,
    .inputNotify       = NULL,
};

/* output device operators*/
const static output_dev_operator_t s_OutputDev_RgbLedOps = {
    .init   = HAL_OutputDev_RgbLed_Init,
    .deinit = NULL,
    .start  = HAL_OutputDev_RgbLed_Start,
    .stop   = NULL,
};

/* output device */
static output_dev_t s_OutputDev_RgbLed = {
    .name         = "rgb_led",
    .attr.type    = kOutputDevType_Other,
    .attr.reserve = NULL,
    .ops          = &s_OutputDev_RgbLedOps,
};

/* RGB LED output device Init function*/
static hal_output_status_t HAL_OutputDev_RgbLed_Init(output_dev_t *dev)
{
    hal_output_status_t error = kStatus_HAL_OutputSuccess;
    /* put RGB LED hardware initialization here*/
    ...
    return error;
}

/* RGB LED output device start function*/
static hal_output_status_t HAL_OutputDev_RgbLed_Start(const output_dev_t *dev)
{
    hal_output_status_t error = kStatus_HAL_OutputSuccess;
    /* registered special event handler for this output device */
    if (FWK_OutputManager_RegisterEventHandler(dev, &s_OutputDev_RgbLedHandler) != 0)
    {
        error = kStatus_HAL_OutputError;
    }
    return error;
}

static hal_output_status_t HAL_OutputDev_RgbLed_InferComplete(const output_dev_t *dev,
                                                              output_algo_source_t source,
                                                              void *inferResult)
{
    hal_output_status_t error   = kStatus_HAL_OutputSuccess;
    /* algorithm_result_t is defined by special algorithm device registered into vision pipeline */
    algorithm_result_t *result = (algorithm_result_t *)inferResult;
    if (pResult != NULL)
    {
        /* do RGB LED hardware setting according to inference result from valgorithm manager*/
        ...
    }
    return error;
}

int HAL_OutputDev_RgbLed_Register()
{
    int error = 0;
    LOGD("output_dev_rgb_led_register");
    error = FWK_OutputManager_DeviceRegister(&s_OutputDev_RgbLed);
    return error;
}                
```

An example of an Overlay UI Output device can be found at `HAL/face_rec/hal_smart_lock_ui.c`.

```c title="HAL/face_rec/hal_smart_lock_ui.c"
static hal_output_status_t HAL_OutputDev_OverlayUi_Init(const output_dev_t *dev);
static hal_output_status_t HAL_OutputDev_OverlayUi_Start(const output_dev_t *dev);
static hal_output_status_t HAL_OutputDev_OverlayUi_InferComplete(const output_dev_t *dev,
                                                                 output_algo_source_t source,
                                                                 void *infer_result);
static hal_output_status_t HAL_OutputDev_OverlayUi_InputNotify(const output_dev_t *dev, void *data);

/* Overlay UI surface */
static gfx_surface_t s_UiSurface;
/* the framebuffer for Overlay UI surface */
SDK_ALIGN(static char s_AsBuffer[UI_BUFFER_WIDTH * UI_BUFFER_HEIGHT * UI_BUFFER_BPP], 32);
/* event handler */
const static output_dev_event_handler_t s_OutputDev_UiHandler = {
    .inferenceComplete = HAL_OutputDev_OverlayUi_InferComplete,
    .inputNotify       = HAL_OutputDev_OverlayUi_InputNotify,
};

/* output device operators */
const static output_dev_operator_t s_OutputDev_UiOps = {
    .init   = HAL_OutputDev_OverlayUi_Init,
    .deinit = NULL,
    .start  = HAL_OutputDev_OverlayUi_Start,
    .stop   = NULL,
};

/* output device */
static output_dev_t s_OutputDev_Ui = {
    .name          = "ui",
    .attr.type     = kOutputDevType_UI,
    .attr.pSurface = &s_UiSurface,
    .ops           = &s_OutputDev_UiOps,
};

/* Overlay UI output device Init function*/
static hal_output_status_t HAL_OutputDev_OverlayUi_Init(output_dev_t *dev)
{
    hal_output_status_t error = kStatus_HAL_OutputSuccess;
    /* init overlay ui surface */
    s_UiSurface.left   = 0;
    s_UiSurface.top    = 0;
    s_UiSurface.right  = UI_BUFFER_WIDTH - 1;
    s_UiSurface.bottom = UI_BUFFER_HEIGHT - 1;
    s_UiSurface.height = UI_BUFFER_HEIGHT;
    s_UiSurface.width  = UI_BUFFER_WIDTH;
    s_UiSurface.pitch  = UI_BUFFER_WIDTH * 2;
    s_UiSurface.format = kPixelFormat_RGB565;
    s_UiSurface.buf    = s_AsBuffer;
    s_UiSurface.lock   = xSemaphoreCreateMutex();

    return error;
}

/* Overlay UI output device start function*/
static hal_output_status_t HAL_OutputDev_OverlayUi_Start(const output_dev_t *dev)
{
    hal_output_status_t error = kStatus_HAL_OutputSuccess;
    /* registered special event handler for this output device */
    if (FWK_OutputManager_RegisterEventHandler(dev, &s_OutputDev_UiHandler) != 0)
        error = kStatus_HAL_OutputError;
    return error;
}

/* Overlay UI inferenceComplete event handler function*/
static hal_output_status_t HAL_OutputDev_OverlayUi_InferComplete(const output_dev_t *dev,
                                                                 output_algo_source_t source,
                                                                 void *infer_result)
{
    hal_output_status_t error    = kStatus_HAL_OutputSuccess;
    /* algorithm_result_t is defined by special algorithm device registered into vision pipeline */
    algorithm_result_t *pResult = (algorithm_result_t *)infer_result;

    if (pResult != NULL)
    {
        /* lock overlay surface to avoid conflict with PXP composing overlay surface */
        if (s_UiSurface.lock)
        {
            xSemaphoreTake(s_UiSurface.lock, portMAX_DELAY);
        }

        /* draw overlay surface here according to inference result from valgorithm manager */
        ...

        /* unlock */
        if (s_UiSurface.lock)
        {
            xSemaphoreGive(s_UiSurface.lock);
        }
    }
    return error;
}

/* Overlay UI inputNotify event handler function*/
static hal_output_status_t HAL_OutputDev_OverlayUi_InputNotify(const output_dev_t *dev, void *data)
{
    hal_output_status_t error = kStatus_HAL_OutputSuccess;
    event_base_t eventBase    = *(event_base_t *)data;

    if (eventBase != NULL)
    {
        /* lock overlay surface to avoid conflict with PXP composing overlay surface */
        if (s_UiSurface.lock)
        {
            xSemaphoreTake(s_UiSurface.lock, portMAX_DELAY);
        }

        /* draw overlay surface here according to input notify event from input manager*/
        ...

        /* unlock */
        if (s_UiSurface.lock)
        {
            xSemaphoreGive(s_UiSurface.lock);
        }
    }
    return error;
}

int HAL_OutputDev_UiSmartlock_Register()
{
    int error = 0;
    LOGD("output_dev_ui_smartlock_register");
    error = FWK_OutputManager_DeviceRegister(&s_OutputDev_Ui);
    return error;
}                
```
