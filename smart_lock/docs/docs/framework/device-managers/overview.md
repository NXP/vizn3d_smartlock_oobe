---
sidebar_position: 1
---

# Overview

As the name would imply,
device managers are responsible for "managing" devices used by the system.
Each device type (input, output, etc.) has its own type-specific device manager.

A device manager serves two primary purposes:

* Initializing and starting each device registered to that manager
* Sending data to and receiving data from each device registered to that manager

This section will avoid low-level implementation details of the device managers
and instead focus on the device manager APIs and the startup flow for the device managers.
The device managers themselves are provided as a library binary file to,
in part,
help abstract the underlying implementation details and encourage developers to focus on the HAL devices being managed instead.

:::info
The device managers themselves are provided as a library binary file in the `framework` folder,
while the APIs for each manager can be found in the `framework/inc` folder.
:::

## Initialization Flow

Before a device manager can properly manage devices, it must follow a specific startup process.
The startup process for device managers is summarized as follows:

1. Initialize managers
2. Register each device to their respective manager
3. Start managers

This process is clearly demonstrated in the `main` function found in `source/main.cpp`

```c title="source/main.cpp" {9-16}
/*
 * @brief   Application entry point.
 */
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

As part of a manager's `start` routine,
the manager will call the `init` and `start` functions of each of its registered devices.

:::note
In general,
developers should only be concerned with adding/removing devices from the `APP_RegisterHalDevices()` function as the `Init` and `Start` functions for each manager is already called by default inside the `APP_InitFramework()` and `APP_StartFramework()` functions in `main()`.
:::