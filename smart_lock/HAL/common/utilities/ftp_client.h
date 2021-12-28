/*
 * Copyright 2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

#ifndef FTP_CLIENT_H_
#define FTP_CLIENT_H_


#define FTP_USER_LENGTH     63
#define FTP_PASSWORD_LENGTH 63

/**
 * @brief The FTP USER structure
 */
typedef struct _ftp_user
{
    uint8_t length;                    /**< user length */
    uint8_t value[FTP_USER_LENGTH];    /**< actual user value */
} ftp_user_t;

/**
 * @brief The FTP Password structure
 */
typedef struct _ftp_password
{
    uint8_t length;                         /**< pass length */
    uint8_t value[FTP_PASSWORD_LENGTH];    /**< actual pass value */
} ftp_password;

/**
 * @brief The WiFi credentials
 */
typedef struct _ftp_cred
{
    ftp_user_t user;          /**< The user name */
    ftp_password password;    /**< The user password, can be \0 */
} ftp_cred_t;

typedef struct _ftp_server_info
{
    uint32_t    serverIP;
    uint16_t    serverPort;
    uint8_t     serverReserved[8];
} ftp_server_info_t;

typedef struct _ftp_info
{
    ftp_server_info_t serverInfo;
#if 0
    /* Not supported for now. Use only not secure FTP. */
*    ftp_cred_t ftp_cred;
#endif
} ftp_info_t;


#endif /* FTP_CLIENT_H_ */
