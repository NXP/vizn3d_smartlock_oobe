---
sidebar_position: 7
---

# Voice Algorithm Manager

<!-- TODO: Add overview of Voice Algo Manager-->

## APIs

### FWK_VoiceAlgoManager_Init

```c
/**
 * @brief Init internal structures for VisionAlgo manager.
 * @return int Return 0 if the init process was successful
 */
int FWK_VoiceAlgoManager_Init();
```

### FWK_VoiceAlgoManager_DeviceRegister

```c
/**
 * @brief Register a voice algorithm device. All algorithm devices need to be registered before
 * FWK_VoiceAlgoManager_Start is called
 * @param dev Pointer to a vision algo device structure
 * @return int Return 0 if registration was successful
 */
int FWK_VoiceAlgoManager_DeviceRegister(voice_algo_dev_t *dev);
```

### FWK_VoiceAlgoManager_Start

```c
/**
 * @brief Spawn VisionAlgo manager task which will call init/start for all registered VisionAlgo devices
 * @return int Return 0 if the starting process was successful
 */
int FWK_VoiceAlgoManager_Start();
```

### FWK_VoiceAlgoManager_Deinit

```c
/**
 * @brief Deinit VisionAlgoManager
 * @return int Return 0 if the deinit process was successful
 */
int FWK_VoiceAlgoManager_Deinit();
```

:::warning
Calling this function is unnecessary in most applications and should be used with caution.
:::
