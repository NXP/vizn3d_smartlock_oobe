/*
 * Copyright 2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

#ifndef WIFI_CREDENTIALS_API_H_
#define WIFI_CREDENTIALS_API_H_

#include "fwk_common.h"
#include <stdbool.h>
#include "wpl.h"
#include "wifi_credentials.h"

/**
 * @brief Check if WiFi credentials are present in memory
 *
 * @return                true if wifi_credentials are present
 */
bool WiFi_CredentialsPresent(void);

/**
 * @brief Reads WiFi credentials from flash memory
 *
 * @param cred Pointer to a pre-allocated wifi_cred_t structure where the credentials will be stored
 * @return 0 on success, 1 otherwise
 */
status_t WiFi_GetCredentials(wifi_cred_t *cred);

/**
 * @brief Writes WiFi credentials in flash memory
 *
 * @param cred Pointer to a wifi_cred_t structure containing the data that will be written
 * @return 0 on success, 1 otherwise
 */
status_t WiFi_SetCredentials(wifi_cred_t *cred);

/**
 * @brief Erase the wifi_credentials
 *
 * @return                0 on success, 1 otherwise
 */
status_t WiFi_EraseCredentials(void);

/**
 * @brief Check if the provided WiFi credentials are valid
 *
 * @param wifi_cred Pointer to a wifi_cred_t structure containing the data that will be validated
 * @return true if wifi_credentials are valid
 */
bool WiFi_CheckCredentials(wifi_cred_t *cred);

/**
 * @brief Initialize the WiFi information structure based on the information found in the file system
 *
 * @return 0 on success, 1 otherwise
 */
status_t WiFi_LoadCredentials(void);

/**
 * @brief Get the current state of the WiFi
 *
 * @return The current WiFi state
 */
wifi_state_t WiFi_GetState(void);

/**
 * @brief Set the WiFi state
 *
 * @param state On or OFF
 * @return 0 if the state was set properly, 1 otherwise
 */
status_t WiFi_SetState(wifi_state_t state);

#endif /* WIFI_CREDENTIALS_API_H_ */
