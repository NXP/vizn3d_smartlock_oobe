---
sidebar_position: 3
---

# Output Manager

<!-- TODO: Overview of Output Manager-->

## APIs

### FWK_OutputManager_Init

```c
/**
 * @brief Init internal structures for output manager.
 * @return int Return 0 if the init process was successful
 */
int FWK_OutputManager_Init();
```

### FWK_OutputManager_DeviceRegister

```c
/**
 * @brief Register a display device. All display devices need to be registered before FWK_OutputManager_Start is called.
 * @param dev Pointer to an output device structure
 * @return int Return 0 if registration was successful
 */
int FWK_OutputManager_DeviceRegister(output_dev_t *dev);
```

### FWK_OutputManager_Start

```c
/**
 * @brief Spawn output manager task which will call init/start for all registered output devices.
 * @return int Return 0 if starting was successful
 */
int FWK_OutputManager_Start();
```

### FWK_OutputManager_Deinit

```c
/**
 * @brief DeInit internal structures for output manager.
 * @return int Return 0 if the deinit process was successful
 */
int FWK_OutputManager_Deinit();
```

:::warning
Calling this function is unnecessary in most applications and should be used with caution.
:::

```c
/**
 * @brief A registered output device doesn't need to be also active. After the start procedure, the output device
 *          can register a handler of capabilities to receive events.
 * @param dev Device that register the handler
 * @param handler Pointer to a handler
 * @return int Return 0 if the registration of the event handler was successful
 */
int FWK_OutputManager_RegisterEventHandler(const output_dev_t *dev, const output_dev_event_handler_t *handler);
```

### FWK_OutputManager_UnregisterEventHandler

```c
/**
 * @brief A registered output device doesn't need to be also active. A device can call this function to unsubscribe
 * from receiving events
 * @param dev Device that unregister the handler
 * @return int Return 0 if the deregistration of the event handler was successful
 */
int FWK_OutputManager_UnregisterEventHandler(const output_dev_t *dev);
```
