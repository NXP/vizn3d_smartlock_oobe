---
sidebar_position: 4
---

# Camera Manager

<!-- TODO: Overview of Camera Manager-->

## APIs

### FWK_CameraManager_Init

```c
/**
 * @brief Init internal structures for Camera manager.
 * @return int Return 0 if the init process was successful
 */
int FWK_CameraManager_Init();
```

### FWK_CameraManager_DeviceRegister

```c
/**
 * @brief Register a camera device. All camera devices need to be registered before FWK_CameraManager_Start is called
 * @param dev Pointer to a camera device structure
 * @return int Return 0 if registration was successful
 */
int FWK_CameraManager_DeviceRegister(camera_dev_t *dev);
```

### FWK_CameraManager_Start

```c
/**
 * @brief Spawn Camera manager task which will call init/start for all registered camera devices
 * @return int Return 0 if the starting process was successul
 */
int FWK_CameraManager_Start();
```

### FWK_CameraManager_Deinit

```c
/**
 * @brief Deinit CameraManager
 * @return int Return 0 if the deinit process was successful
 */
int FWK_CameraManager_Deinit();
```

:::warning
Calling this function is unnecessary in most applications and should be used with caution.
:::
