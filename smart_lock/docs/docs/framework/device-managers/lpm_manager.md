---
sidebar_position: 8
---

# Low Power Manager

The Low Power Device Manager is unique amongst the managers
because it does not have the typical `Init` and `Start` functions that the other managers do.
Instead,
the Low Power Manager has APIs to register a device (only one at a time), configure how deep a sleep the board should enter, enable sleep mode, and more.

:::note
Due to the unique nature of the low power devices being an abstract "virtual" device,
only one LPM device can be registered to the LPM manager at a time.
However,
there should be no need for more than one LPM device because other devices can configure the current low power mode states by using the Low Power Manager APIs.
:::

## APIs

### FWK_LpmManager_DeviceRegister

```c
/**
 * @brief Register a low power mode device. Currently, only one low power mode device can be registered at a time.
 * @param dev Pointer to a low power mode device structure
 * @return int Return 0 if registration was successful
 */
int FWK_LpmManager_DeviceRegister(lpm_dev_t *dev);
```

### FWK_LpmManager_RegisterRequestHandler

```c
int FWK_LpmManager_RegisterRequestHandler(hal_lpm_request_t *req);
```

### FWK_LpmManager_UnregisterRequestHandler

```c
int FWK_LpmManager_UnregisterRequestHandler(hal_lpm_request_t *req);
```

### FWK_LpmManager_RuntimeGet

```c
int FWK_LpmManager_RuntimeGet(hal_lpm_request_t *req);
```

### FWK_LpmManager_RuntimePut

```c
int FWK_LpmManager_RuntimePut(hal_lpm_request_t *req);
```

### FWK_LpmManager_RuntimeSet

```c
int FWK_LpmManager_RuntimeSet(hal_lpm_request_t *req, int8_t count);
```

### FWK_LpmManager_RequestStatus

```c
int FWK_LpmManager_RequestStatus(unsigned int *totalUsageCount);
```

### FWK_LpmManager_SetSleepMode

```c
/**
 * @brief Configure the sleep mode to use when entering sleep
 * @param sleepMode sleep mode to use when entering sleep. Examples include SNVS and other "lighter" sleep modes
 * @return int Return 0 if successful
 */
int FWK_LpmManager_SetSleepMode(hal_lpm_mode_t sleepMode);
```

### FWK_LpmManager_EnableSleepMode

```c
/**
 * @brief Configure sleep mode on/off status
 * @param enable used to set sleep mode on/off; true is enable, false is disable
 * @return int Return 0 if successful
 */
int FWK_LpmManager_EnableSleepMode(hal_lpm_manager_status_t enable);
```
