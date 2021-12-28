---
sidebar_position: 2
---

# Vision Input Manager

<!-- TODO: Overview of Input Manager-->

## APIs

### FWK_InputManager_Init

```c
/**
 * @brief Init internal structures for input manager.
 * @return int Return 0 if the init process was successful
 */
int FWK_InputManager_Init();
```

### FWK_InputManager_DeviceRegister

```c
/**
 * @brief Register an input device. All input devices need to be registered before FWK_InputManager_Start is called.
 * @param dev Pointer to a display device structure
 * @return int Return 0 if registration was successful
 */
int FWK_InputManager_DeviceRegister(input_dev_t *dev);
```

### FWK_InputManager_Start

```c
/**
 * @brief Spawn Input manager task which will call init/start for all registered input devices
 * @return int Return 0 if the starting process was successful
 */
int FWK_InputManager_Start();
```

### FWK_InputManager_Deinit

```c
/**
 * @brief Denit internal structures for input manager.
 * @return int Return 0 if the deinit process was successful
 */
int FWK_InputManager_Deinit();
```

:::warning
Calling this function is unnecessary in most applications and should be used with caution.
:::
