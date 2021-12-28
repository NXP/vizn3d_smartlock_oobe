/*
 * Copyright 2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

#include <limits.h>
#include <wifi_credentials_api.h>
#include "fwk_flash.h"
#include "fwk_log.h"

static wifi_info_t s_wifiInfo;
static wifi_cred_t *s_wifiCredRef;

#define ENCRYPTED_CREDENTIALS 0

#define WIFI_INFO_DIR "wifi_info"

#define WIFI_INFO_FILE \
    WIFI_INFO_DIR      \
    "/"                \
    "wifi_info"

bool WiFi_CredentialsPresent(void)
{
    return (s_wifiCredRef == NULL) ? false : true;
}

status_t WiFi_GetCredentials(wifi_cred_t *cred)
{
    status_t status = kStatus_Success;
    if (cred == NULL)
    {
        LOGE("WiFi credentials NULL pointer");
        status = kStatus_Fail;
    }

    if (status == kStatus_Success)
    {
        if (s_wifiCredRef)
        {
            memcpy(cred, s_wifiCredRef, sizeof(wifi_cred_t));
        }
        else
        {
            LOGD("No WiFi credentials found")
            memset(cred, 0, sizeof(wifi_cred_t));
            status = kStatus_Fail;
        }
    }
    return status;
}

status_t WiFi_SetCredentials(wifi_cred_t *cred)
{
    status_t status = kStatus_Success;
    if (cred == NULL)
    {
        LOGE("WiFi credentials NULL pointer");
        status = kStatus_Fail;
    }

    if (status == kStatus_Success)
    {
        sln_flash_status_t statusFlash;
        if ((cred->ssid.length == 0) && (cred->password.length != 0))
        {
            memcpy(&(s_wifiInfo.wifiCred.password), &cred->password, sizeof(cred->password));
            LOGD("WiFi Credentials - Password set");
        }
        else if ((cred->ssid.length != 0) && (cred->password.length == 0))
        {
            memcpy(&(s_wifiInfo.wifiCred.ssid), &cred->ssid, sizeof(cred->ssid));
            LOGD("WiFi Credentials - SSID set");
        }
        else if (WiFi_CheckCredentials(cred))
        {
            memcpy(&(s_wifiInfo.wifiCred), cred, sizeof(wifi_cred_t));
            LOGD("WiFi Credentials - Complete set");
        }
        else
        {
            LOGE("WiFi credentials are not valid");
            status = kStatus_Fail;
        }

        if (status == kStatus_Success)
        {
            statusFlash = FWK_Flash_Save(WIFI_INFO_FILE, &s_wifiInfo, sizeof(wifi_info_t));
            if (statusFlash != kStatus_HAL_FlashSuccess)
            {
                /* Write has failed, bring what was in Flash before */
                statusFlash = FWK_Flash_Read(WIFI_INFO_FILE, &s_wifiInfo, sizeof(wifi_cred_t));
                if (statusFlash != kStatus_HAL_FlashSuccess)
                {
                    LOGE("WiFi file might be corrupted");
                }
                status = kStatus_Fail;
            }
            else
            {
                if ((s_wifiInfo.wifiCred.password.length != 0) && (s_wifiInfo.wifiCred.ssid.length != 0))
                {
                    s_wifiCredRef = &(s_wifiInfo.wifiCred);
                }
            }
        }
    }
    return status;
}

status_t WiFi_EraseCredentials(void)
{
    status_t status = kStatus_Success;
    sln_flash_status_t statusFlash;
    memset(&(s_wifiInfo.wifiCred), 0, sizeof(wifi_cred_t));
    statusFlash = FWK_Flash_Save(WIFI_INFO_FILE, &s_wifiInfo, sizeof(wifi_info_t));
    if (statusFlash != kStatus_HAL_FlashSuccess)
    {
        status = kStatus_Fail;
    }
    else
    {
        s_wifiCredRef = NULL;
    }

    return status;
}

status_t WiFi_LoadCredentials(void)
{
    status_t status = kStatus_Success;
    sln_flash_status_t statusFlash;
    statusFlash = FWK_Flash_Mkdir(WIFI_INFO_DIR);
    if ((statusFlash == kStatus_HAL_FlashSuccess) || (statusFlash == kStatus_HAL_FlashDirExist))
    {
        statusFlash   = FWK_Flash_Read(WIFI_INFO_FILE, &s_wifiInfo, sizeof(wifi_info_t));
        s_wifiCredRef = NULL;
        if (statusFlash != kStatus_HAL_FlashSuccess)
        {
            LOGD("WiFi Info File doesn't exist. Create it with default values");
            /* Create the file with default values */
            memset(&s_wifiInfo, 0, sizeof(wifi_info_t));
            s_wifiInfo.state = kWiFi_On;
            statusFlash      = FWK_Flash_Save(WIFI_INFO_FILE, &s_wifiInfo, sizeof(wifi_info_t));
            if (statusFlash != kStatus_HAL_FlashSuccess)
            {
                status = kStatus_Fail;
            }
        }
        else
        {
            LOGD("WiFi credentials loaded from flash SSID <%s> Password <%s>.", s_wifiInfo.wifiCred.ssid.length,
                 s_wifiInfo.wifiCred.password.value);
            if ((s_wifiInfo.wifiCred.password.length != 0) && (s_wifiInfo.wifiCred.ssid.length != 0))
            {
                s_wifiCredRef = &(s_wifiInfo.wifiCred);
            }
        }
    }
    else
    {
        status = kStatus_Fail;
    }

    return status;
}

bool WiFi_CheckCredentials(wifi_cred_t *cred)
{
    return true;
}

wifi_state_t WiFi_GetState(void)
{
    return s_wifiInfo.state;
}

status_t WiFi_SetState(wifi_state_t state)
{
    status_t status = kStatus_Success;
    if (s_wifiInfo.state != state)
    {
        sln_flash_status_t statusFlash;
        wifi_state_t oldState = s_wifiInfo.state;
        s_wifiInfo.state      = state;
        statusFlash           = FWK_Flash_Save(WIFI_INFO_FILE, &s_wifiInfo, sizeof(wifi_info_t));
        if (statusFlash != kStatus_HAL_FlashSuccess)
        {
            status           = kStatus_Fail;
            s_wifiInfo.state = oldState;
        }
    }
    return status;
}
