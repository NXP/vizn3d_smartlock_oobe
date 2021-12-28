/*
 * Copyright 2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

#include <limits.h>
#include "fwk_flash.h"
#include "fwk_log.h"
#include "ftp_client_api.h"
#include "lwftp.h"
#include "FreeRTOS.h"
#include "event_groups.h"

typedef enum _ftp_state
{
    kFTP_Connected,
    kFTP_ConnectedFail,
    kFTP_Disconected,
    kFTP_Store,
    kFTP_StoreFail,
    kFTP_Storing,
} ftp_state_t;

#define FTP_STORE_TIMEOUT_MS 20000
#define FTP_CONNECT_TIMEOUT_MS 30000
#define ENCRYPTED_FILE 0

#define FTP_INFO_DIR "wifi_info"

#define FTP_INFO_FILE \
    FTP_INFO_DIR      \
    "/"               \
    "ftp_info"

static ftp_info_t s_ftpInfo;
static EventGroupHandle_t s_ftpEvent;
static bool s_ftpInit = false;
static ftp_state_t s_state;
static char *s_pFileSource;
static uint32_t s_fileSize, s_fileOffset;

static uint32_t _FTP_DataSourceCallback(void *arg, const char **pptr, uint32_t maxlen)
{
    uint32_t len = 0;

    // Check for data request or data sent notice
    if (pptr)
    {
        LOGD("FTP store, offset %d", s_fileOffset);
        len = s_fileSize - s_fileOffset;
        if (len > maxlen)
        {
            len = maxlen;
        }

        *pptr = (s_pFileSource + s_fileOffset);

        s_fileOffset += len;
        if (len != 0)
        {
            xEventGroupSetBits(s_ftpEvent, 1 << kFTP_Storing);
        }
    }

    return len;
}

static void _FTP_StoreCallback(void *arg, int result)
{
    if (result >= LWFTP_RESULT_ERR_UNKNOWN)
    {
        LOGE("Store failed (%d)", result);
        xEventGroupSetBits(s_ftpEvent, 1 << kFTP_StoreFail);
    }
    else if ((result == LWFTP_RESULT_LOGGED) || (result == LWFTP_RESULT_OK))
    {
        LOGD("Store SUCCESS")
        xEventGroupSetBits(s_ftpEvent, 1 << kFTP_Store);
    }
    else if (result == LWFTP_RESULT_INPROGRESS)
    {
        s_fileOffset = 0;
    }
}

static void _FTP_ConnectCallback(void *arg, int result)
{
    if (result >= LWFTP_RESULT_ERR_UNKNOWN)
    {
        LOGE("Login failed (%d)", result);
        xEventGroupSetBits(s_ftpEvent, 1 << kFTP_ConnectedFail);
    }
    else if (result == LWFTP_RESULT_LOGGED)
    {
        xEventGroupSetBits(s_ftpEvent, 1 << kFTP_Connected);
    }
}

static status_t _FTP_SetDefaultInfo(ftp_info_t *ftpInfo)
{
    status_t status = kStatus_Success;
    if (ftpInfo != NULL)
    {
        memset(ftpInfo, 0, sizeof(ftp_info_t));
    }
    else
    {
        LOGE("NULL Pointer exception");
        status = kStatus_Fail;
    }
    return status;
}

static status_t _FTP_ReadServerInfo(ftp_info_t *ftpInfo)
{
    status_t status = kStatus_Success;
    if (ftpInfo != NULL)
    {
        sln_flash_status_t statusFlash;
        statusFlash = FWK_Flash_Read(FTP_INFO_FILE, ftpInfo, sizeof(ftp_info_t));
        if (statusFlash != kStatus_HAL_FlashSuccess)
        {
            LOGE("FTP file might be corrupted");
            status = kStatus_Fail;
        }
    }
    else
    {
        LOGE("NULL Pointer exception");
        status = kStatus_Fail;
    }

    return status;
}

static status_t _FTP_SaveServerInfo(ftp_info_t *ftpInfo)
{
    status_t status = kStatus_Success;
    if (ftpInfo != NULL)
    {
        sln_flash_status_t statusFlash;
        statusFlash = FWK_Flash_Save(FTP_INFO_FILE, ftpInfo, sizeof(ftp_info_t));
        if (statusFlash != kStatus_HAL_FlashSuccess)
        {
            LOGE("Failed to save server info status: %d", statusFlash);
            status = kStatus_Fail;
        }
    }
    else
    {
        LOGE("NULL Pointer exception");
        status = kStatus_Fail;
    }
    return status;
}

static status_t _FTP_SaveDefaultInfo(ftp_info_t *ftpInfo)
{
    status_t status = kStatus_Success;
    _FTP_SetDefaultInfo(ftpInfo);
    if (_FTP_SaveServerInfo(ftpInfo) != kStatus_Success)
    {
        LOGE("Failed to save default server info %d");
        status = kStatus_Fail;
    }
    return status;
}

status_t FTP_SetServerIP(uint32_t ipv4)
{
    status_t status               = kStatus_Success;
    s_ftpInfo.serverInfo.serverIP = ipv4;
    if (_FTP_SaveServerInfo(&s_ftpInfo) != kStatus_Success)
    {
        /* Load again what is in FS */
        _FTP_ReadServerInfo(&s_ftpInfo);
        status = kStatus_Fail;
    }
    return status;
}

status_t FTP_SetServerPort(uint16_t port)
{
    status_t status                 = kStatus_Success;
    s_ftpInfo.serverInfo.serverPort = port;
    if (_FTP_SaveServerInfo(&s_ftpInfo) != kStatus_Success)
    {
        /* Load again what is in FS */
        _FTP_ReadServerInfo(&s_ftpInfo);
        status = kStatus_Fail;
    }
    return status;
}

status_t FTP_SetServerInfo(ftp_server_info_t serverInfo)
{
    status_t status = kStatus_Success;
    memcpy(&s_ftpInfo.serverInfo, &serverInfo, sizeof(ftp_server_info_t));
    if (_FTP_SaveServerInfo(&s_ftpInfo) != kStatus_Success)
    {
        /* Load again what is in FS */
        _FTP_ReadServerInfo(&s_ftpInfo);
        status = kStatus_Fail;
    }
    return status;
}

status_t FTP_SetInfo(ftp_info_t ftpInfo)
{
    status_t status = kStatus_Success;
    memcpy(&s_ftpInfo, &ftpInfo, sizeof(ftp_info_t));
    if (_FTP_SaveServerInfo(&s_ftpInfo) != kStatus_Success)
    {
        /* Load again what is in FS */
        _FTP_ReadServerInfo(&s_ftpInfo);
        status = kStatus_Fail;
    }
    return status;
}

status_t FTP_GetInfo(ftp_info_t *ftpInfo)
{
    status_t status = kStatus_Success;
    if (ftpInfo != NULL)
    {
        if (_FTP_ReadServerInfo(&s_ftpInfo) == kStatus_Success)
        {
            memcpy(ftpInfo, &s_ftpInfo, sizeof(ftp_info_t));
        }
        else
        {
            status = kStatus_Fail;
        }
    }
    else
    {
        status = kStatus_Fail;
    }

    return status;
}

status_t FTP_Init(void)
{
    status_t status = kStatus_Success;
    sln_flash_status_t statusFlash;
    s_state    = kFTP_Disconected;
    s_ftpEvent = xEventGroupCreate();
    if (s_ftpEvent == NULL)
    {
        LOGE("Failed to create event handler for FTP");
        return kStatus_Fail;
    }

    statusFlash = FWK_Flash_Mkdir(FTP_INFO_DIR);
    if (statusFlash == kStatus_HAL_FlashSuccess)
    {
        status = _FTP_SaveDefaultInfo(&s_ftpInfo);
    }
    else if (statusFlash == kStatus_HAL_FlashDirExist)
    {
        /* Exist try to read */
        statusFlash = FWK_Flash_Read(FTP_INFO_FILE, &s_ftpInfo, UINT_MAX);
        if (statusFlash == kStatus_HAL_FlashFileNotExist)
        {
            status = _FTP_SaveDefaultInfo(&s_ftpInfo);
        }
        else if (status != kStatus_HAL_FlashSuccess)
        {
            LOGE("Failed to read FTP info status %d");
            status = kStatus_Fail;
        }
        else
        {
            LOGD("FTP info loaded from Flash");
        }
    }
    else
    {
        LOGE("Failed to create directory for FTP file status %d", statusFlash);
        status = kStatus_Fail;
    }

    if (status == kStatus_Fail)
    {
        vEventGroupDelete(s_ftpEvent);
        s_ftpEvent = NULL;
    }
    else
    {
        s_ftpInit = true;
    }

    return status;
}

ftp_session_handle_t FTP_ConnectBlocking(void)
{
    static lwftp_session_t ftpSession; // static content for the whole FTP session
    err_t error;
    status_t status             = kStatus_Success;
    ftp_session_handle_t handle = NULL;

    if (s_ftpInit)
    {
        memset(&ftpSession, 0, sizeof(ftpSession));
        ftpSession.server_ip.addr = s_ftpInfo.serverInfo.serverIP;
        ftpSession.server_port    = s_ftpInfo.serverInfo.serverPort;
        ftpSession.done_fn        = _FTP_ConnectCallback;

        /* Not supported for now */
        ftpSession.user = "anonymous";
        ftpSession.pass = "anonymous@domain.com";

        /* We have no extra user data, simply use the session structure */
        ftpSession.handle = &ftpSession;
        /* Start the connection state machine */
        xEventGroupClearBits(s_ftpEvent, (1 << kFTP_Connected) | (1 << kFTP_ConnectedFail));
        error = lwftp_connect(&ftpSession);
        if (error != LWFTP_RESULT_INPROGRESS)
        {
            LOGE("lwftp_connect failed (%d)", error);
            status = kStatus_Fail;
        }
        else
        {
            /* Wait for the callback */
            EventBits_t bits = xEventGroupWaitBits(s_ftpEvent, ((1 << kFTP_Connected) | (1 << kFTP_ConnectedFail)),
                                                   pdTRUE, pdFALSE, pdMS_TO_TICKS(FTP_CONNECT_TIMEOUT_MS));
            if ((bits & (1 << kFTP_ConnectedFail)) != 0)
            {
                status = kStatus_Fail;
            }
            else if ((bits & (1 << kFTP_Connected)) == 0)
            {
                LOGE("Failed to connect FTP. Timeout");
                status = kStatus_Fail;
            }
        }

        if (status == kStatus_Success)
        {
            handle  = (ftp_session_handle_t)&ftpSession;
            s_state = kFTP_Connected;
        }
        else
        {
            ftpSession.done_fn = NULL;
            lwftp_close(&ftpSession);
        }
    }

    return handle;
}

status_t FTP_StoreBlocking(ftp_session_handle_t sessionHandler, const char *remotePath, char *dataSource, uint32_t len)
{
    lwftp_session_t *ftpSession;
    status_t status = kStatus_Success;

    if ((sessionHandler == NULL) || (remotePath == NULL))
    {
        status = kStatus_InvalidArgument;
    }

    if (status == kStatus_Success)
    {
        ftpSession = (lwftp_session_t *)sessionHandler;
        if (s_state == kFTP_Connected)
        {
            err_t error;
            s_state                 = kFTP_Store;
            s_pFileSource           = dataSource;
            s_fileSize              = len;
            ftpSession->data_source = _FTP_DataSourceCallback;
            ftpSession->done_fn     = _FTP_StoreCallback;
            ftpSession->remote_path = remotePath;
            xEventGroupClearBits(s_ftpEvent, (1 << kFTP_Store) | (1 << kFTP_StoreFail) | (1 << kFTP_Storing));
            error = lwftp_store(ftpSession);
            if (error != LWFTP_RESULT_INPROGRESS)
            {
                LOGE("lwftp_store failed (%d)", error);
                status = kStatus_Fail;
            }
            else
            {
                while (status != kStatus_Fail)
                {
                    EventBits_t bits = xEventGroupWaitBits(
                        s_ftpEvent, ((1 << kFTP_Store) | (1 << kFTP_StoreFail) | (1 << kFTP_Storing)), pdTRUE, pdFALSE,
                        pdMS_TO_TICKS(FTP_STORE_TIMEOUT_MS));

                    if ((bits & (1 << kFTP_StoreFail)) != 0)
                    {
                        status = kStatus_Fail;
                    }
                    else if ((bits & (1 << kFTP_Storing)) != 0)
                    {
                        LOGI("Still sending..")
                        s_state = kFTP_Storing;
                    }
                    else if ((bits & (1 << kFTP_Store)) != 0)
                    {
                        break;
                    }
                    else
                    {
                        /* If no activity detected. Abort */
                        LOGE("FTP store timeout");
                        status = kStatus_Fail;
                    }
                }
            }

            if (status == kStatus_Fail)
            {
                ftpSession->done_fn = NULL;
                lwftp_close(ftpSession);
                s_state = kFTP_Disconected;
            }
            else
            {
                s_state = kFTP_Connected;
            }
        }
    }
    return status;
}

status_t FTP_DisconnectBlocking(ftp_session_handle_t sessionHandler)
{
    lwftp_session_t *ftpSession;
    status_t status = kStatus_Success;
    if (sessionHandler == NULL)
    {
        status = kStatus_InvalidArgument;
    }

    if (status == kStatus_Success)
    {
        ftpSession = (lwftp_session_t *)sessionHandler;
        if (s_state == kFTP_Connected)
        {
            ftpSession->done_fn = NULL;
            lwftp_close(ftpSession);
            s_state = kFTP_Disconected;
        }
    }

    return status;
}
