---
sidebar_position: 6
---

# Vision Algorithm Manager

<!-- TODO: Overview of Vision Algorithm Manager-->

## APIs

### FWK_VisionAlgoManager_Init

```c
/**
 * @brief Init internal structures for VisionAlgo manager.
 * @return int Return 0 if the init process was successful
 */
int FWK_VisionAlgoManager_Init();
```

### FWK_VisionAlgoManager_DeviceRegister

```c
/**
 * @brief Register a vision algorithm device. All algorithm devices need to be registered before
 * FWK_VisionAlgoManager_Start is called
 * @param dev Pointer to a vision algo device structure
 * @return int Return 0 if registration was successful
 */
int FWK_VisionAlgoManager_DeviceRegister(vision_algo_dev_t *dev);
```

### FWK_VisionAlgoManager_Start

```c
/**
 * @brief Spawn VisionAlgo manager task which will call init/start for all registered VisionAlgo devices
 * @return int Return 0 if the starting process was successul
 */
int FWK_VisionAlgoManager_Start();
```

### FWK_VisionAlgoManager_Deinit

```c
/**
 * @brief Deinit VisionAlgoManager
 * @return int Return 0 if the deinit process was successful
 */
int FWK_VisionAlgoManager_Deinit();
```

:::warning
Calling this function is unnecessary in most applications and should be used with caution.
:::
