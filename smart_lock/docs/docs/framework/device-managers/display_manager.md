---
sidebar_position: 5
---

# Display Manager

<!-- TODO: Overview of Display Manager-->

## APIs

### FWK_DisplayManager_Init

```c
/**
 * @brief Init internal structures for display manager.
 * @return int Return 0 if the init process was successful
 */
int FWK_DisplayManager_Init();
```

### FWK_DisplayManager_DeviceRegister

```c
/**
 * @brief Register a display device. All display devices need to be registered before FWK_DisplayManager_Start is
 * called.
 * @param dev Pointer to a display device structure
 * @return int Return 0 if registration was successful
 */
int FWK_DisplayManager_DeviceRegister(display_dev_t *dev);
```

### FWK_DisplayManager_Start

```c
/**
 * @brief Spawn Display manager task which will call init/start for all registered display devices. Will start the flow
 * to recive frames from the camera.
 * @return int Return 0 if starting was successful
 */
int FWK_DisplayManager_Start();
```

### FWK_DisplayManager_Deinit

```c
/**
 * @brief Init internal structures for display manager.
 * @return int Return 0 if the init process was successful
 */
int FWK_DisplayManager_Deinit();
```

:::warning
Calling this function is unnecessary in most applications and should be used with caution.
:::
