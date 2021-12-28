---
sidebar_position: 2
---

# Event Handlers

Because events are the primary means by which the framework communicates between devices,
a mechanism to respond to those events is necessary for them to be useful.
Event handlers were created for this explicit purpose.

There are two kinds of event handler:

  - [Default Handlers](#default-handlers)
  - [App-specific Handlers](#app-specific-handlers)

Event handlers,
like other device operators,
are passed via the device's operator struct to its manager.

```c
const static display_dev_operator_t s_DisplayDev_LcdifOps = {
    .init        = HAL_DisplayDev_LcdifRk024hh2_Init,
    .deinit      = HAL_DisplayDev_LcdifRk024hh2_Uninit,
    .start       = HAL_DisplayDev_LcdifRk024hh2_Start,
    .blit        = HAL_DisplayDev_LcdifRk024hh2_Blit,
    .inputNotify = HAL_DisplayDev_LcdifRk024hh2_InputNotify,
};
```

Each HAL device may define its own handlers for any given event.
For example,
a developer may want the RGB LEDs to turn green when a face is recognized,
but have the UI display a specific overlay for that same event.
To do this,
the RGB Output HAL device and the UI Output HAL device can each implement an `InferComplete` handler which will be called by their manager
when an "InferComplete" event is received.

:::important
A HAL device does NOT have to implement an event handler for any specific event,
nor does it have to implement an `InputNotify` handler (applicable for most device types)
or an `InferComplete` handler (applicable only for output devices).
:::

## Default Handlers

Default event handlers are exactly what their name would suggest -- the default means by which a device handles events.
A HAL device's default event handlers (`InputNotify`, `InferComplete`, etc.) can be found in the HAL device driver itself.

Nearly every device has a default handler implemented[^1],
although most devices will only actually handle a few types of events.

<!-- TODO: Add default handler for each device -->
:::note
[^1] Devices which do not have a handler implemented can be extended to have one by using a similar device as an example.
:::

```c title="Example default handler"
static hal_display_status_t HAL_DisplayDev_LcdifRk024hh2_InputNotify(const display_dev_t *receiver, void *data)
{
    hal_display_status_t error           = kStatus_HAL_DisplaySuccess;
    event_base_t eventBase               = *(event_base_t *)data;
    event_status_t event_response_status = kEventStatus_Ok;

    if (eventBase.eventId == kEventID_SetDisplayOutputSource)
    {
        event_common_t event             = *(event_common_t *)data;
        s_DisplayDev_Lcdif.cap.srcFormat = event.displayOutput.displayOutputSource;
        s_NewBufferSet                   = true;
        if (eventBase.respond != NULL)
        {
            eventBase.respond(eventBase.eventId, &event.displayOutput, event_response_status, true);
        }
        LOGI("[display_dev_inputNotify]: kEventID_SetDisplayOutputSource devID %d, srcFormat %d", receiver->id,
             event.displayOutput.displayOutputSource);
    }
    else if (eventBase.eventId == kEventID_GetDisplayOutputSource)
    {
        display_output_event_t display;
        display.displayOutputSource = s_DisplayDev_Lcdif.cap.srcFormat;
        if (eventBase.respond != NULL)
        {
            eventBase.respond(eventBase.eventId, &display, event_response_status, true);
        }
        LOGI("[display_dev_inputNotify]: kEventID_GetDisplayOutputSource devID %d, srcFormat %d", receiver->id,
             display.displayOutputSource);
    }

    return error;
}
```

Some devices will not handle any events at all
and will instead return `0` after performing no action.

```c title="HAL/common/hal_camera_csi_gc0308.c"
hal_camera_status_t HAL_CameraDev_CsiGc0308_InputNotify(const camera_dev_t *dev, void *data)
{
    hal_camera_status_t ret = kStatus_HAL_CameraSuccess;

    return ret;
}
```

Alternatively,
some devices which do not require an event handler
may simply return a `NULL` pointer instead.

```c title="HAL/common/hal_display_lcdifv2_rk055ahd091.c"
const static display_dev_operator_t s_DisplayDev_LcdifOps = {
    .init        = HAL_DisplayDev_Lcdifv2Rk055ah_Init,
    .deinit      = HAL_DisplayDev_Lcdifv2Rk055ah_Deinit,
    .start       = HAL_DisplayDev_Lcdifv2Rk055ah_Start,
    .blit        = HAL_DisplayDev_Lcdifv2Rk055ah_Blit,
    .inputNotify = NULL,
};
```

Managers will know not to call the `InputNotify` or other handler if that handler points to `NULL`.

A device's default handler whether for `InputNotify` events or `InferComplete` or otherwise can be overridden by an ["app-specific"](#app-specific-handlers) handler.

## App-specific Handlers

App-specific handlers are device handlers which are defined for a specific "app"
such as the Smart Lock project or the upcoming Touchless HMI project.

Not every device will need to implement an app-specific handler,
but because default handlers are implemented using `WEAK` functions[^2], any device which has a default event handler can have that handler overridden.

:::note
Some devices may not have implemented their default handlers using `WEAK` functions,
but may be updated to do so in the future.
:::

For example,
the IR + White LEDs may not require project-specific handlers
because they will always react the same way to a `kEventID_SetConfig`/`kEventID_GetConfig` command.
Alternatively, an application may wish to override and/or extend that default event handling behavior so that,
for example,
the LEDs increase in brightness when an "Add Face" event is received.

To help denote an app-specific handler,
App-specific handlers will start with the `APP` prefix.
If an app-specific handler for a device exists,
it can be found in `source/event_handlers/{APP_NAME}_{DEV_TYPE}_{DEV_NAME}.c`
