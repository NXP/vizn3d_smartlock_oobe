/*
 * Copyright 2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

#ifndef FTP_CLIENT_API_H_
#define FTP_CLIENT_API_H_

#include "fwk_common.h"
#include "ftp_client.h"

typedef void *ftp_session_handle_t;

/**
 * @brief Set server info
 *
 * @param serverInfo ServerInfo contains IP and port
 * @return kStatus_Success if success
 */
status_t FTP_SetServerInfo(ftp_server_info_t serverInfo);

/**
 * @brief Set FTP server IP address
 *
 * @param ipv4 Ipv4 server address
 * @return kStatus_Success on success
 */
status_t FTP_SetServerIP(uint32_t ipv4);

/**
 * @brief Set FTP server IP port
 *
 * @param port
 * @return kStatus_Success on success
 */
status_t FTP_SetServerPort(uint16_t port);

/**
 * @brief Set FTP server info
 *
 * @param ftpInfo
 * @return kStatus_Success on success
 */

status_t FTP_SetInfo(ftp_info_t ftpInfo);

/**
 * @brief Used to get info from the FTP server
 *
 * @param ftpInfo Pointer to ftp_info_t structure. Valid info only if kStatus_Success returned
 * @return kStatus_Success on success
 */

status_t FTP_GetInfo(ftp_info_t *ftpInfo);

/**
 * @brief Fetches the Server Info from the flash, init internal structures.
 *
 * @return kStatus_Success on success
 */
status_t FTP_Init(void);

/**
 * @brief Connect to the server specified by saved server_info
 *
 * @return Return a handler which is to be used with the store/disconnect function. NULL if no connection was done
 */

ftp_session_handle_t FTP_ConnectBlocking(void);

/**
 * @brief Store the data at a specific remote location.
 *
 * @param sessionHandler Session handler obtain after the connect operation took place
 * @param remote_path Remote path at which to save the file. Should contain the name of the file
 * @param data_source   Data to be saved
 * @param len   Length of the file
 * @return kStatus_Success on success. If the status was fail, automatic disconnection takes place
 */
status_t FTP_StoreBlocking(ftp_session_handle_t sessionHandler, const char *remotePath, char *dataSource, uint32_t len);

/**
 * @brief Disconnect from a connected server
 *
 * @param sessionHandler
 * @return kStatus_Success on success
 */
status_t FTP_DisconnectBlocking(ftp_session_handle_t sessionHandler);

#endif /* FTP_CLIENT_API_H_ */
