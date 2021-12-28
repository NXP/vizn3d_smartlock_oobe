---
sidebar_position: 9
---

# Audio Processing Manager

<!-- TODO: Overview of Audio Processing Manager-->

## APIs

### FWK_AudioProcessing_Init

```c
/**
 * @brief Init Audio Processing manager
 *
 * @return int Return 0 if the init process was successful
 */
int FWK_AudioProcessing_Init(void);
```

### FWK_AudioProcessing_DeviceRegister

```c
/**
 * @brief Register an audio processing device
 *
 * @param dev Pointer to an Audio Processing device
 * @return int Return 0 if the register was successful
 */
int FWK_AudioProcessing_DeviceRegister(audio_processing_dev_t *dev);
```

### FWK_AudioProcessing_Start

```c
/**
 * @brief Start Audio Processing manager
 *
 * @return int Return 0 if the starting process was successful
 */
int FWK_AudioProcessing_Start(void);
```

### FWK_AudioProcessing_Deinit

```c
/**
 * @brief Deinit Audio Processing manager
 *
 * @return int Return 0 if the deit process was successful
 */
int FWK_AudioProcessing_Deinit(void);
```

:::warning
Calling this function is unnecessary in most applications and should be used with caution.
:::
