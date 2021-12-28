/*
 * Copyright 2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

#ifndef WIFI_CREDENTIALS_H_
#define WIFI_CREDENTIALS_H_

#define WIFI_SSID_LENGTH     32
#define WIFI_PASSWORD_LENGTH 63

typedef enum _wifi_state
{
    kWiFi_On,
    kWiFi_Off,
} wifi_state_t;
/**
 * @brief The WiFi SSID structure
 */
typedef struct _wpl_ssid
{
    uint8_t length;                     /**< SSID length */
    uint8_t value[WIFI_SSID_LENGTH];    /**< actual SSID value */
} wpl_ssid_t;

/**
 * @brief The WiFi PSK structure
 */
typedef struct _wpl_psk
{
    uint8_t length;                         /**< psk length */
    uint8_t value[WIFI_PASSWORD_LENGTH];    /**< actual psk value */
} wpl_psk_t;

/**
 * @brief The WiFi credentials
 */
typedef struct _wifi_cred
{

    wpl_ssid_t ssid;        /**< The network name */
    wpl_psk_t password;    /**< The network password, can be \0 */
} wifi_cred_t;

typedef struct _wifi_info
{
    wifi_state_t state;
    wifi_cred_t wifiCred;
} wifi_info_t;

#endif /* WIFI_CREDENTIALS_H_ */
