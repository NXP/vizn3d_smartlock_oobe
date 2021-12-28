/*
 * Copyright 2018, 2020 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

#include "FreeRTOS.h"
#include "task.h"

#include "common_connection_handler.h"

#include "lwip/sockets.h"
#include <string.h>
#include "lwip/def.h"
#include "lwip/ip_addr.h"
#include "fsl_debug_console.h"
#include "network_connection.h"
#include "tcp_connection_handler_private.h"

#if COMMS_MESSAGE_HANDLER_FWUPDATE_METHOD_OTA_TCP
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define OTA_TIMEOUT_S  30
#define OTA_TIMEOUT_US 0
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void SLN_TCP_COMMS_ServerTask(void *args);
static sln_common_connection_message_status_t SLN_TCP_COMMS_ReadFd(int32_t fd);
static sln_common_connection_message_status_t SLN_TCP_COMMS_ProcessConnection(int32_t fd, int32_t *client_fd);
static sln_common_connection_message_status_t SLN_TCP_COMMS_Write(sln_common_connection_write_context_t *writeContext);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static TaskHandle_t tcpCommsTaskHandle    = NULL;
static sln_common_connection_desc_t *desc = NULL;
static uint8_t *tmpBuff                   = NULL;

/*******************************************************************************
 * Private Functions
 ******************************************************************************/
/**
 * @brief The TCP Socket handler Task
 *
 * @param args: Task arguments
 *
 */
static void SLN_TCP_COMMS_ServerTask(void *args)
{
    sln_common_connection_message_status_t status = kCommon_Success;
    int32_t socket_status;
    fd_set current_fd_set, online_fd_set;
    int32_t socket;
    struct sockaddr_in server_addr;

    configPRINTF(("[%s] Starting TCP Connection handler processor\r\n", __FUNCTION__));
    /* Create the socket server */
    socket = lwip_socket(PF_INET, SOCK_STREAM, 0);

    if (socket < 0)
    {
        configPRINTF(("Failed to create socket\r\n"));
        status = kCommon_Failed;
    }

    configPRINTF(("[%s] Binding Socket\r\n", __FUNCTION__));

    /* Bind to the socket server */
    if (kCommon_Success == status)
    {
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family      = PF_INET;
        server_addr.sin_port        = lwip_htons(OTA_TCP_COMMS_PORT);
        server_addr.sin_addr.s_addr = lwip_htonl(*(u32_t *)IP_ADDR_ANY);

        socket_status = lwip_bind(socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
        if (socket_status < 0)
        {
            configPRINTF(("[%s] Failed to Bind\r\n", __FUNCTION__));
            status = kCommon_Failed;
        }
    }

    configPRINTF(("[%s] Listening\r\n", __FUNCTION__));
    /* Create the socket listener to wait for incoming connections */
    if (kCommon_Success == status)
    {
        socket_status = lwip_listen(socket, 1);
        if (socket_status == -1)
        {
            configPRINTF(("[%s] Failed to Listen\r\n", __FUNCTION__));
            status = kCommon_Failed;
        }
    }

    /* Memset the fd set */
    if (kCommon_Success == status)
    {
        FD_ZERO(&online_fd_set);
        FD_SET(socket, &online_fd_set);
    }

    configPRINTF(("[%s] Entering While Loop %d\r\n", __FUNCTION__, status));
    while (status == kCommon_Success)
    {
        bool activity;
        current_fd_set         = online_fd_set;
        struct timeval timeout = {OTA_TIMEOUT_S, OTA_TIMEOUT_US};
        configPRINTF(("[%s] Waiting for activity\r\n", __FUNCTION__));
        /* Wait for activity on the socket */
        if (lwip_select(FD_SETSIZE, &current_fd_set, NULL, NULL, &timeout) < 0)
        {
            configPRINTF(("[%s] Failed Activity\r\n", __FUNCTION__));
            break;
        }

        /* We either have activity or timeout expired. Mark activity false */
        activity = false;
        /* Loop over all the socket inputs */
        for (int32_t i = 0; i < FD_SETSIZE; i++)
        {
            /* Check to see if there are any activity on the fd */
            if (FD_ISSET(i, &current_fd_set))
            {
                activity = true;
                /* If the fd is set to the socket, it's a new connection and needs to be assigned */
                if (i == socket)
                {
                    int32_t client_fd;

                    /* Accept the incomming connection */
                    status = SLN_TCP_COMMS_ProcessConnection(socket, &client_fd);
                    if (status == kCommon_Success)
                    {
                        FD_SET(client_fd, &online_fd_set);
                    }
                }
                else
                {
                    status = SLN_TCP_COMMS_ReadFd(i);

                    /* If there was no data read, then we don't want to clear the flag
                     *  because we need to check again if it's disconnected
                     */
                    if (kCommon_ConnectionLost == status)
                    {
                        FD_CLR(i, &online_fd_set);
                    }
                    /* Arbitrary fail instead of catastrophic. Set back to success to continue */
                    status = kCommon_Success;
                }

                if (kCommon_Failed == status)
                {
                    break;
                }
            }
        }

        if (activity == false)
        {
            /* There was no activity. False alarm. Reset and go into main app */
            configPRINTF(("[%s] Failed Activity for timeout reason \r\n", __FUNCTION__));
            status = kCommon_ConnectionLost;
        }
        else
        {
            /* There was activity. Continue to listen */
            configPRINTF(("[%s] Activity Detected\r\n", __FUNCTION__));
        }
    }
    /* It should not reached here unless an error occurred. Reset the board. */

    /* Give time to logging task to print last logs */
    vTaskDelay(300);

    /* The Bootloader Task is suspended. Reset the board */
    NVIC_SystemReset();
}

/**
 * @brief Write to the socket via the given fd instance.
 *
 * @param sln_common_connection_write_context_t: Data and client context
 *
 * @return          sln_comms_message_status
 */

static sln_common_connection_message_status_t SLN_TCP_COMMS_Write(sln_common_connection_write_context_t *writeContext)
{
    sln_common_connection_message_status_t status = kCommon_Success;
    uint32_t bytes_sent                           = 0;

    /* Send the length of the message */
    bytes_sent = lwip_send(writeContext->connContext.sTcpContext.fd, &writeContext->len, 4, 0);

    /* If four bytes weren't sent, abort */
    if (bytes_sent != 4)
    {
        status = kCommon_Failed;
    }

    /* Send the data */
    if (status == kCommon_Success)
    {
        bytes_sent = lwip_send(writeContext->connContext.sTcpContext.fd, writeContext->data, writeContext->len, 0);
    }

    /* TODO: Could possibly loop as the message could end up being large */
    if (bytes_sent != writeContext->len)
    {
        status = kCommon_Failed;
    }

    return status;
}

/**
 * @brief The function to read from a file descriptor and call read callback
 *
 * @param fd [in]: file descriptor id to read from
 *
 */
static sln_common_connection_message_status_t SLN_TCP_COMMS_ReadFd(int32_t fd)
{
    sln_common_connection_message_status_t status = kCommon_Success;
    uint8_t *buff                                 = NULL;

    configPRINTF(("[%s] Received Data from connected device\r\n", __FUNCTION__));
    int32_t packet_size = 0;

    /* Receive the header length information */
    int32_t len = lwip_recv(fd, (void *)&packet_size, 4, 0);

    /* If the len is negative, connection was dropped */
    if (len < 0)
    {
        configPRINTF(("[%s] Connection Loss of FD: %d\r\n", __FUNCTION__, fd));
        lwip_close(fd);
        status = kCommon_ConnectionLost;
    }
    /* If the len is then there was nothing to read, could be a connection loss */
    else if (len == 0)
    {
        /* Could be connection issue, will read again later to check */
        configPRINTF(("[%s] Received no bytes\r\n", __FUNCTION__));
        status = kCommon_NoDataRead;
    }

    if (packet_size > OTA_MAX_BUFFER_SIZE)
    {
        /* Bounds checking to ensure we didn't get a bad length */
        configPRINTF(("[%s] Data too large\r\n", __FUNCTION__));
        status = kCommon_ToManyBytes;
    }

    if (kCommon_Success == status)
    {
        /* Malloc the size of the expected message */
        buff = (uint8_t *)pvPortMalloc(packet_size);
        if (buff == NULL)
        {
            configPRINTF(("[%s] Failed to malloc\r\n", __FUNCTION__));
            status = kCommon_Failed;
        }
        else
        {
            /* Set the data to 0 to ensure potentially weird things don't occur */
            memset(buff, 0, packet_size);
        }
    }

    if (kCommon_ToManyBytes == status)
    {
        /* Need to flush the incomming message so it's not incorrectly read for the next message */
        configPRINTF(("[%s] Dumping socket data\r\n", __FUNCTION__));
        int32_t bytesToRead = 0;
        while (len)
        {
            if (OTA_MAX_BUFFER_SIZE < len)
            {
                bytesToRead = len;
            }
            else
            {
                bytesToRead = OTA_MAX_BUFFER_SIZE;
            }
            len -= lwip_recv(fd, tmpBuff, OTA_MAX_BUFFER_SIZE, bytesToRead);
        }
    }

    if (kCommon_Success == status)
    {
        configPRINTF(("[%s] Receiving data\r\n", __FUNCTION__));
        int32_t bytes_read       = 0;
        int32_t total_bytes_read = 0;
        /* Read the data until we hit the size that is expected */
        do
        {
            bytes_read = lwip_recv(fd, (void *)buff + total_bytes_read, packet_size - total_bytes_read, 0);
            if (bytes_read < 0)
            {
                configPRINTF(("[%s] Connection Loss\r\n", __FUNCTION__));
                lwip_close(fd);
                status = kCommon_ConnectionLost;
                break;
            }
            else
            {
                total_bytes_read += bytes_read;
            }
        } while (total_bytes_read != packet_size);

        /* If the callback is valid, pass it */
        /* TODO: Change to event and seperate task */
        if ((kCommon_Success == status) && (desc->recv_cb))
        {
            sln_common_connection_recv_context_t rxContext;

            rxContext.data                       = buff;
            rxContext.packet_size                = total_bytes_read;
            rxContext.connContext.sTcpContext.fd = fd;
            desc->recv_cb(&rxContext);
        }
        vPortFree(buff);
    }

    return status;
}

/**
 * @brief The function to process a new connection request
 *
 * @param fd [in]: file descriptor of the server which had an event
 *
 */
static sln_common_connection_message_status_t SLN_TCP_COMMS_ProcessConnection(int32_t fd, int32_t *client_fd)
{
    struct sockaddr_in client_addr;
    size_t size;
    sln_common_connection_message_status_t status = kCommon_Success;

    configPRINTF(("[%s] New Connection Established\r\n", __FUNCTION__));

    /* Allow the incomming connection to connect */
    size       = sizeof(client_addr);
    *client_fd = lwip_accept(fd, (struct sockaddr *)&client_addr, &size);

    /* If the fd is negative, then something went wrong with the incomming connection */
    if (*client_fd < 0)
    {
        status = kCommon_Failed;
    }

    return status;
}

/*******************************************************************************
 * Public Functions
 ******************************************************************************/

sln_common_connection_message_status_t SLN_TCP_COMMS_Init(sln_common_connection_desc_t *descriptor)
{
    sln_common_connection_message_status_t status = kStatus_Success;
    BaseType_t xReturned;
    uint8_t retryCount = 0;
#if USE_ETHERNET_CONNECTION
    APP_NETWORK_Init(true);
#elif USE_WIFI_CONNECTION
    APP_NETWORK_Init();
    while (retryCount < 3)
    {
        status_t statusConnect;

        statusConnect = APP_NETWORK_Wifi_Connect(true, true);
        if (WIFI_CONNECT_SUCCESS == statusConnect)
        {
            status = kCommon_Success;
            break;
        }
        else if (WIFI_CONNECT_NO_CRED == statusConnect)
        {
            status = kCommon_Failed;
            break;
        }
        else
        {
            retryCount++;
            status = kCommon_Failed;
        }
    }
#endif

    if (status == kCommon_Success)
    {
        if (descriptor->context.sTcpContext.max_rx_buff_size <= 0)
        {
            status = kCommon_Failed;
        }
        else
        {
            tmpBuff = (uint8_t *)pvPortMalloc(descriptor->context.sTcpContext.max_rx_buff_size);

            if (NULL == tmpBuff)
            {
                status = kCommon_Failed;
            }
        }
    }

    if (status == kCommon_Success)
    {
        if (descriptor->context.sTcpContext.port <= 0)
        {
            status = kCommon_Failed;
        }
    }

    if (status == kCommon_Success)
    {
        desc        = descriptor;
        desc->write = &SLN_TCP_COMMS_Write;

        xReturned = xTaskCreate(SLN_TCP_COMMS_ServerTask, "TCP_Comms_Server", 1024, &tcpCommsTaskHandle,
                                configMAX_PRIORITIES - 3, &tcpCommsTaskHandle);
        if (xReturned == pdPASS)
        {
            status = kCommon_Success;
        }
        else
        {
            status = kCommon_Failed;
        }
    }

    return status;
}

#endif /* COMMS_MESSAGE_HANDLER_FWUPDATE_METHOD_OTA_TCP */
