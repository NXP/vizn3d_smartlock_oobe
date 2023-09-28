/*
 * Copyright 2022-2023 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

/*
 * @brief uart communication implementation.
 */

#include <FreeRTOS.h>
#include <queue.h>

#include "app_config.h"
#if ENABLE_UART_COMMUNICATION

#include "board.h"
#include "fsl_lpuart_freertos.h"
#include "fsl_lpuart.h"

#include "fwk_log.h"
#include "fwk_message.h"
#include "fwk_sln_task.h"
#include "fwk_input_manager.h"
#include "fwk_output_manager.h"
#include "fwk_lpm_manager.h"
#include "hal_event_descriptor_face_rec.h"
#include "hal_input_dev.h"
#include "hal_smart_lock_config.h"
#include "hal_vision_algo.h"

#include "fica_definition.h"

/*******************************************************************************
 * Defines
 ******************************************************************************/

#define PROJECT_VERSION_MAJOR  SMART_LOCK_VERSION_MAJOR
#define PROJECT_VERSION_MINOR  SMART_LOCK_VERSION_MINOR
#define PROJECT_VERSION_HOTFIX SMART_LOCK_VERSION_HOTFIX

#define PROTOCOL_VERSION_MAJOR  0x01
#define PROTOCOL_VERSION_MINOR  0x01
#define PROTOCOL_VERSION_HOTFIX 0x00

#define OASIS_VERSION_MAJOR  VERSION_MAJOR
#define OASIS_VERSION_MINOR  VERSION_MINOR
#define OASIS_VERSION_HOTFIX VERSION_HOTFIX

#define COMM_UART_BASEADDR   BOARD_COMM_UART_BASEADDR   // LPUART8
#define COMM_UART_IRQ        BOARD_COMM_UART_IRQ        // LPUART8_IRQn
#define COMM_UART_CLOCK_ROOT BOARD_COMM_UART_CLOCK_ROOT // kCLOCK_Root_Lpuart8

#define UART_COMM_NAME          "UART COMM"
#define UART_COMM_TASK_NAME     "UART_COMM_task"
#define UART_COMM_TASK_STACK    1024 * 2
#define UART_COMM_TASK_PRIORITY 7

#define UART_COMM_QUEUE_LENGTH    5
#define UART_COMM_HEADER_LENGTH   14
#define UART_COMM_MINI_TRANS_UINT 14

typedef enum _hal_status_t
{
    kHALStatus_Success,
    kHALStatus_TUMagicError,
    kHALStatus_TUCRC16Error,
    kHALStatus_PacketCRC16Error,
    kHALStatus_PacketValid,
    kHALStatus_PacketError,
    kHALStatus_PacketShort,
} hal_status_t;

typedef enum _hal_packet_t
{
    kHALPacket_Header,
    kHALPacket_Data,
} hal_packet_t;

typedef struct _hal_response_t
{
    union
    {
        oasis_lite_registration_result_t remoteRegResult;
    };
} hal_response_t;

typedef struct __attribute__((packed)) _hal_header_transfer_unit_t
{
    uint8_t tuMagic[3];
    uint8_t pktType;
    uint16_t pktLen;
    uint16_t pktId;
    uint16_t pktCrc;
    uint16_t reserved;
    uint16_t tuCrc;
} hal_header_transfer_unit_t;

typedef enum _hal_transfer_packet_type_t
{
    REGISTRATION_CMD_REQ = 0x80,
    REGISTRATION_CMD_RES,

    RECOGNITION_CMD_REQ, // 0x82
    RECOGNITION_CMD_RES,

    DEREGISTRATION_CMD_REQ, // 0x84
    DEREGISTRATION_CMD_RES,

    DELETE_USER_REQ, // 0x86
    DELETE_USER_RES,

    GET_USER_COUNT_REQ, // 0x88
    GET_USER_COUNT_RES,

    GET_USER_INFO_REQ, // 0x8a
    GET_USER_INFO_RES,

    GET_SYS_INFO_REQ, // 0x8c
    GET_SYS_INFO_RES,

    CAPTURE_IMAGE_REQ, // 0x8e
    CAPTURE_IMAGE_RES, // 0x8f

    FW_UPDATE_REQ, // 0x90
    FW_UPDATE_RES, // 0x91

    TMPL_UPDATE_REQ, // 0x92
    TMPL_UPDATE_RES,

    TMPL_RELOAD_REQ, // 0x94
    TMPL_RELOAD_RES,

    FEATURE_REG_REQ, // 0x96
    FEATURE_REG_RES,

    FEATURE_READ_REQ, // 0x98
    FEATURE_READ_RES,

    FIRMWARE_RESPONSE = 0xff,
} hal_transfer_packet_type_t;

static const uint8_t TU_MAGIC[] = {0x53, 0x79, 0x4c};

typedef enum _hal_ack_reserved_t
{
    kHAL_ACK_SUCCESS   = 0,
    kHAL_ACK_DUPLICATE = 1,
    kHAL_ACK_ERROR     = -1, // 0xffffffffff
} hal_ack_reserved_t;

typedef struct _hal_transfer_t
{
    uint8_t *nibbleBuf;
    uint8_t *packetBuf;
    uint16_t nibbleLen;
    uint16_t packetLen;
    uint16_t readOffset;
    uint8_t stage;
} hal_transfer_t;

typedef struct __attribute__((packed)) _hal_face_result_t
{
    int8_t state;
    uint8_t result;
    uint8_t faceCount;
    uint32_t faceBox[4];
    uint8_t quality;
    uint8_t expectOri;
    float percentage;
    uint32_t sim;
    uint32_t faceID;
    char name[FACE_NAME_MAX_LEN];
} hal_face_result_t;

static oasis_lite_result_t s_LastOasisResult;
static hal_face_result_t faceResult;
static uint8_t lastFaceCmd = REGISTRATION_CMD_REQ;
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */
void BOARD_InitUartCommResource(void);
#if defined(__cplusplus)
}
#endif /* __cplusplus */

static hal_input_status_t HAL_InputDev_UartComm_Init(input_dev_t *dev, input_dev_callback_t callback);
static hal_input_status_t HAL_InputDev_UartComm_Deinit(const input_dev_t *dev);
static hal_input_status_t HAL_InputDev_UartComm_Start(const input_dev_t *dev);
static hal_input_status_t HAL_InputDev_UartComm_Stop(const input_dev_t *dev);
static hal_input_status_t HAL_InputDev_UartComm_InputNotify(const input_dev_t *dev, void *param);

static hal_output_status_t HAL_OutputDev_UartComm_Start(const output_dev_t *dev);
static hal_output_status_t HAL_OutputDev_UartComm_InferComplete(const output_dev_t *dev,
                                                                output_algo_source_t source,
                                                                void *inferResult);
static hal_output_status_t HAL_OutputDev_UartComm_InputNotify(const output_dev_t *dev, void *param);
static hal_status_t SLN_UartCommSendPacket(uint8_t *data, uint16_t len, uint8_t type, uint16_t pktId, uint16_t reserve);

static lpuart_rtos_handle_t s_LpuartRTOSHandle;
static lpuart_handle_t s_LpuartHandle;
static uint8_t s_LpuartRingBuffer[32];

static uint8_t s_UartCommNibbleBuffer[UART_COMM_MINI_TRANS_UINT];
static hal_transfer_t s_UartCommTransfer;
static hal_header_transfer_unit_t s_HeaderTransferUnit = {0};
static event_face_rec_t s_UartCommEvent;
static input_event_t s_InputEvent;

const static input_dev_operator_t s_InputDev_UartCommOps = {
    .init        = HAL_InputDev_UartComm_Init,
    .deinit      = HAL_InputDev_UartComm_Deinit,
    .start       = HAL_InputDev_UartComm_Start,
    .stop        = HAL_InputDev_UartComm_Stop,
    .inputNotify = HAL_InputDev_UartComm_InputNotify,
};

static input_dev_t s_InputDev_UartComm = {
    .id = 1, .name = UART_COMM_NAME, .ops = &s_InputDev_UartCommOps, .cap = {.callback = NULL}};

const static output_dev_event_handler_t s_OutputDev_UartCommHandler = {
    .inferenceComplete = HAL_OutputDev_UartComm_InferComplete,
    .inputNotify       = HAL_OutputDev_UartComm_InputNotify,
};

const static output_dev_operator_t s_OutputDev_UartCommOps = {
    .init   = NULL,
    .deinit = NULL,
    .start  = HAL_OutputDev_UartComm_Start,
    .stop   = NULL,
};

static output_dev_t s_OutputDev_UartComm = {
    .name         = UART_COMM_NAME,
    .attr.type    = kOutputDevType_Other,
    .attr.reserve = NULL,
    .ops          = &s_OutputDev_UartCommOps,
};

static uint16_t s_pckID;
static const uint16_t table16[0x100] = {
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241, 0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1,
    0xC481, 0x0440, 0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40, 0x0A00, 0xCAC1, 0xCB81, 0x0B40,
    0xC901, 0x09C0, 0x0880, 0xC841, 0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40, 0x1E00, 0xDEC1,
    0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41, 0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
    0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040, 0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1,
    0xF281, 0x3240, 0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441, 0x3C00, 0xFCC1, 0xFD81, 0x3D40,
    0xFF01, 0x3FC0, 0x3E80, 0xFE41, 0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840, 0x2800, 0xE8C1,
    0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41, 0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
    0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640, 0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0,
    0x2080, 0xE041, 0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240, 0x6600, 0xA6C1, 0xA781, 0x6740,
    0xA501, 0x65C0, 0x6480, 0xA441, 0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41, 0xAA01, 0x6AC0,
    0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840, 0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
    0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40, 0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1,
    0xB681, 0x7640, 0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041, 0x5000, 0x90C1, 0x9181, 0x5140,
    0x9301, 0x53C0, 0x5280, 0x9241, 0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440, 0x9C01, 0x5CC0,
    0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40, 0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
    0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40, 0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0,
    0x4C80, 0x8C41, 0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641, 0x8201, 0x42C0, 0x4380, 0x8341,
    0x4100, 0x81C1, 0x8081, 0x4040};

static uint16_t s_capState = 0;

static int HAL_InputDev_UartComm_Respond(uint32_t eventId,
                                         void *response,
                                         event_status_t status,
                                         unsigned char isFinished)
{
    if (response == NULL)
    {
        return -1;
    }

    switch (eventId)
    {
        case kEventFaceRecID_GetUserList:
        {
            user_info_event_t usersInfo = *(user_info_event_t *)response;

            if (usersInfo.count == 0)
            {
                SLN_UartCommSendPacket((uint8_t *)&usersInfo.count, 2, GET_USER_INFO_RES, 0, 1);
            }
            else
            {
#define COUNT_STEP 5
                uint16_t num    = (usersInfo.count + COUNT_STEP - 1) / COUNT_STEP;
                uint16_t mod    = usersInfo.count % COUNT_STEP;
                uint16_t count  = COUNT_STEP;
                uint8_t *pData  = NULL;
                uint16_t offset = 0;

                for (int i = 0; i < num; i++)
                {
                    if ((i == (num - 1)) && (mod != 0))
                        count = mod;

                    if (i == 0)
                    {
                        pData = (uint8_t *)pvPortMalloc(2 + count * (4 + 32));
                        memcpy(pData, &usersInfo.count, 2);
                        offset = 2;
                    }
                    else
                    {
                        pData  = (uint8_t *)pvPortMalloc(count * (4 + 32));
                        offset = 0;
                    }
                    for (int j = 0; j < count; j++)
                    {
                        face_user_info_t userInfo = usersInfo.userInfo[i * COUNT_STEP + j];
                        memcpy(pData + offset + j * (4 + 32), &userInfo.id, 4);
                        memcpy(pData + offset + 4 + j * (4 + 32), userInfo.name, 32);
                    }
                    SLN_UartCommSendPacket(pData, offset + count * (4 + 32), GET_USER_INFO_RES, i, num);
                    vPortFree(pData);
                }
            }
        }
        break;

        case kEventFaceRecID_GetUserCount:
        {
            uint16_t count = *(uint16_t *)response;
            SLN_UartCommSendPacket((uint8_t *)&count, 2, GET_USER_COUNT_RES, s_pckID, 0);
        }
        break;

            //        case kEventFaceRecID_UpdateUserInfo:
            //        {
            //            if (status == kEventStatus_Ok)
            //            {
            //                SLN_UartCommSendPacket(NULL, 0, UPDATE_USER_INFO_RES, s_pckID, kHAL_ACK_SUCCESS);
            //            }
            //            else
            //            {
            //                SLN_UartCommSendPacket(NULL, 0, UPDATE_USER_INFO_RES, s_pckID, kHAL_ACK_ERROR);
            //            }
            //        }
            //        break;

        case kEventFaceRecID_DelUserAll:
        case kEventFaceRecID_DelUser:
        {
            uint8_t delResult;
            if (status == kEventStatus_Ok)
            {
                delResult = 0;
                SLN_UartCommSendPacket(&delResult, 1, DELETE_USER_RES, s_pckID, 0);
            }
            else
            {
                delResult = status;
                SLN_UartCommSendPacket(&delResult, 1, DELETE_USER_RES, s_pckID, 0);
            }
        }
        break;

        case kEventFaceRecID_AddUserRemote:
        {
            remote_reg_result_t res = *(remote_reg_result_t *)response;
            SLN_UartCommSendPacket(&res.result, 1, FEATURE_REG_RES, s_pckID, 0);
            vPortFree(s_UartCommEvent.remoteReg.regData);
        }
        break;

        case kEventFaceRecID_CapImage:
        {
            image_info_event_t imageInfo = *(image_info_event_t *)response;
            uint8_t ch                   = 3;
            if (imageInfo.format == IMAGE_FORMAT_GRAY888 || imageInfo.format == IMAGE_FORMAT_BGR888)
            {
                ch = 3;
            }
            else if (imageInfo.format == IMAGE_FORMAT_RAW16)
            {
                ch = 2;
            }
            uint32_t imageSize  = imageInfo.height * imageInfo.width * ch;
            uint32_t offset     = 0;
            const uint16_t step = 1000;
            uint32_t num        = (imageSize + step - 1) / step;
            uint32_t mod        = imageSize % step;
            uint16_t size       = step;

            for (uint32_t i = 0; i < num; i++)
            {
                uint8_t *pData = NULL;

                if (s_capState == 0)
                {
                    break;
                }

                if ((i == (num - 1)) && (mod != 0))
                {
                    size = mod;
                }

                if (i == 0)
                {
                    pData = (uint8_t *)pvPortMalloc(5 + size);
                    memcpy(pData, &imageInfo.width, 2);
                    memcpy(pData + 2, &imageInfo.height, 2);
                    memcpy(pData + 4, &imageInfo.format, 1);
                    memcpy(pData + 5, imageInfo.data + offset, size);
                    SLN_UartCommSendPacket(pData, 5 + size, CAPTURE_IMAGE_RES, i, num);
                }
                else
                {
                    pData = (uint8_t *)pvPortMalloc(size);
                    memcpy(pData, imageInfo.data + offset, size);
                    SLN_UartCommSendPacket(pData, size, CAPTURE_IMAGE_RES, i, num);
                }

                offset += size;
                vPortFree(pData);
            }
        }
        break;

        case kEventFaceRecID_GetUserFeature:
        {
            fea_read_event_t feaInfo = *(fea_read_event_t *)response;
            int faceItemSize         = OASISLT_getFaceItemSize();
            uint8_t *pData           = NULL;
            if (status == kEventStatus_Ok)
            {
                pData    = (uint8_t *)pvPortMalloc(1 + 4 + faceItemSize);
                pData[0] = status;
                memcpy(pData + 1, &feaInfo.id, 4);
                memcpy(pData + 1 + 4, feaInfo.face_data, faceItemSize);
                SLN_UartCommSendPacket(pData, 1 + 4 + faceItemSize, FEATURE_READ_RES, s_pckID, 0);
                vPortFree(pData);
            }
            else
            {
                SLN_UartCommSendPacket((uint8_t *)&status, 1, FEATURE_READ_RES, s_pckID, 0);
            }
        }
        break;
        default:
            break;
    }

    return 0;
}

static void crc16(const uint8_t *data, size_t numBytes, uint16_t *crc)
{
    uint16_t val   = 0;
    uint16_t index = 0;
    for (size_t i = 0; i < numBytes; ++i)
    {
        index = (val & 0xff) ^ (data[i] & 0xff);
        val   = ((val >> 8) & 0xff) ^ table16[index];
    }
    *crc = val;
}

static void PRINTF_HEX(uint8_t *data, int len)
{
    for (int i = 0; i < len; i++)
    {
        LOGD("0x%2x ", data[i]);
    }
}

static void SLN_UartCommCreateHeader(
    uint8_t *htUnit, uint8_t *data, uint16_t len, uint8_t type, uint16_t pktId, uint16_t reserve)
{
    uint16_t crc_res = 0;

    /* transfer magic */
    memcpy(htUnit, TU_MAGIC, sizeof(TU_MAGIC));

    /* packet type */
    htUnit[3] = type;

    /* packet length */
    *((uint16_t *)(htUnit + 4)) = len;

    /* packet id */
    *((uint16_t *)(htUnit + 6)) = pktId;

    if (len > 0)
    {
        /* packet crc */
        crc16(data, len, &crc_res);
        *((uint16_t *)(htUnit + 8)) = crc_res;
    }
    else
    {
        *((uint16_t *)(htUnit + 8)) = 0;
    }

    /* packet reserved */
    *((uint16_t *)(htUnit + 10)) = reserve;

    /* transfer unit crc */
    crc_res = 0;
    crc16(htUnit, UART_COMM_HEADER_LENGTH - 2, &crc_res);
    *((uint16_t *)(htUnit + 12)) = crc_res;
}

static hal_status_t SLN_UartCommSendPacket(uint8_t *data, uint16_t len, uint8_t type, uint16_t pktId, uint16_t reserve)
{
    hal_status_t status = kHALStatus_Success;
    uint8_t *pUnit      = (uint8_t *)pvPortMalloc(UART_COMM_HEADER_LENGTH + len);

    if (pUnit != NULL)
    {
        SLN_UartCommCreateHeader(pUnit, data, len, type, pktId, reserve);
        if (len > 0)
        {
            memcpy(pUnit + UART_COMM_HEADER_LENGTH, data, len);
        }
        //
        // LPUART_RTOS_Send
        LPUART_RTOS_SendTimeout(&s_LpuartRTOSHandle, pUnit, UART_COMM_HEADER_LENGTH + len, 1000);

        vPortFree(pUnit);
    }
    else
    {
        LOGE("[ERROR]: pvPortMalloc for pUnit \r\n");
    }

    return status;
}

static hal_status_t SLN_UartCommParseHeader(uint8_t *headerBuf, hal_header_transfer_unit_t *pHtUnit)
{
    hal_status_t status = kHALStatus_Success;
    uint16_t crc_res    = 0;

    /* transfer magic */
    if (memcmp(headerBuf, TU_MAGIC, sizeof(TU_MAGIC)) != 0)
    {
        return kHALStatus_TUMagicError;
    }
    memcpy(pHtUnit->tuMagic, headerBuf, 3);

    /* packet type */
    pHtUnit->pktType = headerBuf[3];

    /* packet length */
    pHtUnit->pktLen = *((uint16_t *)(headerBuf + 4));

    /* packet id */
    pHtUnit->pktId = *((uint16_t *)(headerBuf + 6));

    /* packet crc */
    pHtUnit->pktCrc = *((uint16_t *)(headerBuf + 8));

    /* packet reserved */
    pHtUnit->reserved = *((uint16_t *)(headerBuf + 10));

    /* transfer unit crc */
    pHtUnit->tuCrc = *((uint16_t *)(headerBuf + 12));
    crc16(headerBuf, UART_COMM_HEADER_LENGTH - 2, &crc_res);
    if (crc_res != pHtUnit->tuCrc)
    {
        return kHALStatus_TUCRC16Error;
    }

    return status;
}

static hal_status_t SLN_UartCommParseData(uint8_t *dataBuf, hal_header_transfer_unit_t *pHtUnit)
{
    hal_status_t status = kHALStatus_Success;
    uint16_t crc_res    = 0;

    if ((dataBuf != NULL) && (pHtUnit->pktLen > 0))
    {
        crc16(dataBuf, pHtUnit->pktLen, &crc_res);
        if (crc_res != pHtUnit->pktCrc)
        {
            return kHALStatus_PacketCRC16Error;
        }
    }

    s_pckID = pHtUnit->pktId;
    /* handle all kinds of packet type */
    switch (pHtUnit->pktType)
    {
        case REGISTRATION_CMD_REQ:
        {
            lastFaceCmd           = REGISTRATION_CMD_REQ;
            uint32_t receiverList = 1 << kFWKTaskID_VisionAlgo;
            uint8_t start         = *dataBuf;
            if (s_InputDev_UartComm.cap.callback != NULL)
            {
                if (start)
                {
                    s_UartCommEvent.eventBase.eventId = kEventFaceRecID_AddUser;
                    s_UartCommEvent.addFace.hasName   = *(dataBuf + 1);
                    if (s_UartCommEvent.addFace.hasName)
                    {
                        memcpy(s_UartCommEvent.addFace.name, dataBuf + 2, FACE_NAME_MAX_LEN);
                    }
                }
                else
                {
                    s_UartCommEvent.eventBase.eventId = kEventFaceRecID_AddUserStop;
                }
                s_UartCommEvent.eventBase.respond = NULL;
                uint8_t fromISR                   = __get_IPSR();
                s_InputEvent.inputData            = &s_UartCommEvent;
                s_InputDev_UartComm.cap.callback(&s_InputDev_UartComm, kInputEventID_Recv, receiverList, &s_InputEvent,
                                                 0, fromISR);
            }
        }
        break;

        case RECOGNITION_CMD_REQ:
        {
            lastFaceCmd           = RECOGNITION_CMD_REQ;
            uint32_t receiverList = 1 << kFWKTaskID_VisionAlgo;
            uint8_t start         = *dataBuf;
            if (s_InputDev_UartComm.cap.callback != NULL)
            {
                if (start)
                {
                    s_UartCommEvent.eventBase.eventId = kEventFaceRecID_StartRec;
                }
                else
                {
                    s_UartCommEvent.eventBase.eventId = kEventFaceRecID_StopRec;
                }
                s_UartCommEvent.eventBase.respond = NULL;
                uint8_t fromISR                   = __get_IPSR();
                s_InputEvent.inputData            = &s_UartCommEvent;
                s_InputDev_UartComm.cap.callback(&s_InputDev_UartComm, kInputEventID_Recv, receiverList, &s_InputEvent,
                                                 0, fromISR);
            }
        }
        break;

        case DEREGISTRATION_CMD_REQ:
        {
            lastFaceCmd           = DEREGISTRATION_CMD_REQ;
            uint32_t receiverList = 1 << kFWKTaskID_VisionAlgo;
            uint8_t start         = *dataBuf;
            if (s_InputDev_UartComm.cap.callback != NULL)
            {
                if (start)
                {
                    s_UartCommEvent.eventBase.eventId = kEventFaceRecID_DelUser;
                    s_UartCommEvent.delFace.hasName   = false;
                    s_UartCommEvent.delFace.hasID     = false;
                }
                else
                {
                    s_UartCommEvent.eventBase.eventId = kEventFaceRecID_DelUserStop;
                }
                s_UartCommEvent.eventBase.respond = NULL;
                uint8_t fromISR                   = __get_IPSR();
                s_InputEvent.inputData            = &s_UartCommEvent;
                s_InputDev_UartComm.cap.callback(&s_InputDev_UartComm, kInputEventID_Recv, receiverList, &s_InputEvent,
                                                 0, fromISR);
            }
        }
        break;

        case FEATURE_REG_REQ:
        {
            uint32_t receiverList = 1 << kFWKTaskID_VisionAlgo;

            s_UartCommEvent.eventBase.eventId = kEventFaceRecID_AddUserRemote;
            s_UartCommEvent.eventBase.respond = HAL_InputDev_UartComm_Respond;

            s_UartCommEvent.remoteReg.flag    = *dataBuf;
            s_UartCommEvent.remoteReg.dataLen = pHtUnit->pktLen - 1;
            s_UartCommEvent.remoteReg.regData = pvPortMalloc(pHtUnit->pktLen - 1);

            if (s_UartCommEvent.remoteReg.regData != NULL)
            {
                memcpy(s_UartCommEvent.remoteReg.regData, dataBuf + 1, pHtUnit->pktLen - 1);

                uint8_t fromISR = __get_IPSR();
                if (s_InputDev_UartComm.cap.callback != NULL)
                {
                    s_InputEvent.inputData = &s_UartCommEvent;
                    s_InputDev_UartComm.cap.callback(&s_InputDev_UartComm, kInputEventID_Recv, receiverList,
                                                     &s_InputEvent, 0, fromISR);
                }
            }
            else
            {
                LOGE("[ERROR]: UartCommTask REMOTE_REGISTRATION_REQ malloc buffer error.\r\n");
            }
        }
        break;

        case DELETE_USER_REQ:
        {
            uint32_t receiverList = 1 << kFWKTaskID_VisionAlgo;
            if (s_InputDev_UartComm.cap.callback != NULL)
            {
                if (dataBuf[0] == 0x00) // del -a
                {
                    s_UartCommEvent.eventBase.eventId = kEventFaceRecID_DelUserAll;
                }
                else if (dataBuf[0] == 0x01) // del id
                {
                    s_UartCommEvent.eventBase.eventId = kEventFaceRecID_DelUser;
                    s_UartCommEvent.delFace.hasName   = false;
                    s_UartCommEvent.delFace.hasID     = true;
                    s_UartCommEvent.delFace.id        = *(uint32_t *)(dataBuf + 1);
                }
                else if (dataBuf[0] == 0x02) // del name
                {
                    s_UartCommEvent.eventBase.eventId = kEventFaceRecID_DelUser;
                    s_UartCommEvent.delFace.hasName   = true;
                    memcpy(s_UartCommEvent.delFace.name, dataBuf + 1 + 4, FACE_NAME_MAX_LEN);
                    s_UartCommEvent.delFace.hasID = false;
                }
                else
                {
                    break;
                }
                s_UartCommEvent.eventBase.respond = HAL_InputDev_UartComm_Respond;
                uint8_t fromISR                   = __get_IPSR();
                s_InputEvent.inputData            = &s_UartCommEvent;
                s_InputDev_UartComm.cap.callback(&s_InputDev_UartComm, kInputEventID_Recv, receiverList, &s_InputEvent,
                                                 0, fromISR);
            }
        }
        break;

        case GET_USER_COUNT_REQ:
        {
            uint32_t receiverList = 1 << kFWKTaskID_VisionAlgo;

            if (s_InputDev_UartComm.cap.callback != NULL)
            {
                s_UartCommEvent.eventBase.eventId = kEventFaceRecID_GetUserCount;
                s_UartCommEvent.eventBase.respond = HAL_InputDev_UartComm_Respond;
                uint8_t fromISR                   = __get_IPSR();
                s_InputEvent.inputData            = &s_UartCommEvent;
                s_InputDev_UartComm.cap.callback(&s_InputDev_UartComm, kInputEventID_Recv, receiverList, &s_InputEvent,
                                                 0, fromISR);
            }
        }
        break;

        case FEATURE_READ_REQ:
        {
            uint32_t receiverList = 1 << kFWKTaskID_VisionAlgo;

            if (s_InputDev_UartComm.cap.callback != NULL)
            {
                s_UartCommEvent.eventBase.eventId = kEventFaceRecID_GetUserFeature;
                s_UartCommEvent.feaRead.id        = *(uint32_t *)dataBuf;
                s_UartCommEvent.feaRead.face_data = NULL;
                s_UartCommEvent.eventBase.respond = HAL_InputDev_UartComm_Respond;
                uint8_t fromISR                   = __get_IPSR();
                s_InputEvent.inputData            = &s_UartCommEvent;
                s_InputDev_UartComm.cap.callback(&s_InputDev_UartComm, kInputEventID_Recv, receiverList, &s_InputEvent,
                                                 0, fromISR);
            }
        }
        break;

        case GET_USER_INFO_REQ:
        {
            uint32_t receiverList = 1 << kFWKTaskID_VisionAlgo;

            if (s_InputDev_UartComm.cap.callback != NULL)
            {
                s_UartCommEvent.eventBase.eventId = kEventFaceRecID_GetUserList;
                s_UartCommEvent.eventBase.respond = HAL_InputDev_UartComm_Respond;
                uint8_t fromISR                   = __get_IPSR();
                s_InputEvent.inputData            = &s_UartCommEvent;
                s_InputDev_UartComm.cap.callback(&s_InputDev_UartComm, kInputEventID_Recv, receiverList, &s_InputEvent,
                                                 0, fromISR);
            }
        }
        break;

        case GET_SYS_INFO_REQ:
        {
            uint32_t runningFromBankA = (((*(uint32_t *)(APPLICATION_RESET_ISR_ADDRESS)-FLEXSPI_AMBA_BASE) &
                                          (FICA_IMG_BANK_APP_MASK)) == FICA_IMG_APP_A_ADDR);

            uint32_t runningFromBankB = (((*(uint32_t *)(APPLICATION_RESET_ISR_ADDRESS)-FLEXSPI_AMBA_BASE) &
                                          (FICA_IMG_BANK_APP_MASK)) == FICA_IMG_APP_B_ADDR);
            uint8_t bank;

            if (runningFromBankB)
            {
                LOGD("App running in bankB\r\n");
                bank = 1;
            }
            else if (runningFromBankA)
            {
                LOGD("App running in bankA\r\n");
                bank = 0;
            }

            uint8_t system_info[] = {
                PROJECT_VERSION_MAJOR,
                PROJECT_VERSION_MINOR,
                PROJECT_VERSION_HOTFIX,
                OASIS_VERSION_MAJOR,
                OASIS_VERSION_MINOR,
                OASIS_VERSION_HOTFIX,
                PROTOCOL_VERSION_MAJOR,
                PROTOCOL_VERSION_MINOR,
                PROTOCOL_VERSION_HOTFIX,
                // Bank;
                0,
            // 3D or 2D
#if defined(SMART_LOCK_3D)
                0x01,
#else
                0x00,
#endif
            // RGB camera
#if ENABLE_FLEXIO_CAMERA
                0x01,
#else
                0x00,
#endif
            // LCD
#if ENABLE_LCDIFV2_KD024HVFID076 || ENABLE_LCDIF_RK024HH298 || ENABLE_LCDIFV2_RK055AHD091
                0x01,
#else
                0x00,
#endif
#if ENABLE_DISPLAY_UVC
                0x01,
#else
                0x00
#endif

            };
            system_info[9] = bank;
            SLN_UartCommSendPacket(system_info, sizeof(system_info), GET_SYS_INFO_RES, pHtUnit->pktId, 0);
        }
        break;

        case CAPTURE_IMAGE_REQ:
        {
            s_capState            = dataBuf[0];
            uint32_t receiverList = 1 << kFWKTaskID_VisionAlgo;

            if (s_InputDev_UartComm.cap.callback != NULL && s_capState)
            {
                s_UartCommEvent.eventBase.eventId = kEventFaceRecID_CapImage;
                s_UartCommEvent.eventBase.respond = HAL_InputDev_UartComm_Respond;
                uint8_t fromISR                   = __get_IPSR();
                s_InputEvent.inputData            = &s_UartCommEvent;
                s_InputDev_UartComm.cap.callback(&s_InputDev_UartComm, kInputEventID_Recv, receiverList, &s_InputEvent,
                                                 0, fromISR);
            }
        }
        break;

        default:
            break;
    }

    return status;
}

static void SLN_UartCommMsgHandle(void *param)
{
    // LOGD("[UartCommTask] start.");

    hal_status_t status = kHALStatus_PacketValid;

    int error;
    size_t n = 0;

    hal_transfer_t *transfer = param;

    while (1)
    {
        // Rev Header
        error = LPUART_RTOS_ReceiveTimeout(&s_LpuartRTOSHandle, transfer->nibbleBuf, 1, &n, 100);
        if ((error != kStatus_Success) || (n != 1) || (transfer->nibbleBuf[0] != TU_MAGIC[0]))
        {
            continue;
        }
        error = LPUART_RTOS_ReceiveTimeout(&s_LpuartRTOSHandle, transfer->nibbleBuf + 1, 1, &n, 100);
        if ((error != kStatus_Success) || (n != 1) || (transfer->nibbleBuf[1] != TU_MAGIC[1]))
        {
            continue;
        }
        error = LPUART_RTOS_ReceiveTimeout(&s_LpuartRTOSHandle, transfer->nibbleBuf + 2, 1, &n, 100);
        if ((error != kStatus_Success) || (n != 1) || (transfer->nibbleBuf[2] != TU_MAGIC[2]))
        {
            continue;
        }
        error = LPUART_RTOS_ReceiveTimeout(&s_LpuartRTOSHandle, transfer->nibbleBuf + 3, UART_COMM_HEADER_LENGTH - 3,
                                           &n, 100);
        if ((error != kStatus_Success) || (n != (UART_COMM_HEADER_LENGTH - 3)))
        {
            continue;
        }
        // Parse header
        transfer->stage = kHALPacket_Header;
        status          = SLN_UartCommParseHeader(transfer->nibbleBuf, &s_HeaderTransferUnit);
        LOGD("HEADER_PACKET: ");
        if (status != kHALStatus_Success)
        {
            LOGD("HEADER PACKET PARSE FAILED[%d].", transfer->nibbleLen);
            PRINTF_HEX(transfer->nibbleBuf, UART_COMM_HEADER_LENGTH);
            continue;
        }

        transfer->readOffset = 0;
        transfer->packetLen  = s_HeaderTransferUnit.pktLen;
        transfer->stage      = kHALPacket_Data;
        LOGD("HEADER PACKET PARSE SUCCESSFUL[%d].", transfer->packetLen);
        // Rev Data
        if (transfer->packetLen > 0)
        {
            transfer->packetBuf = pvPortMalloc(s_HeaderTransferUnit.pktLen);
            if (transfer->packetBuf != NULL)
            {
                error =
                    LPUART_RTOS_ReceiveTimeout(&s_LpuartRTOSHandle, transfer->packetBuf, transfer->packetLen, &n, 100);
                if ((error != kStatus_Success) || (n != transfer->packetLen))
                {
                    continue;
                }
            }
            else
            {
                LOGE("Error: pvPortMalloc for packetBuf");
                continue;
            }
        }

        // Parse Data
        if (transfer->stage == kHALPacket_Data)
        {
            status = SLN_UartCommParseData(transfer->packetBuf, &s_HeaderTransferUnit);
            LOGD("DATA_PACKET: ");

            if (status == kHALStatus_Success)
            {
                LOGD("DATA PACKET PARSE SUCCESSFUL[%d].", transfer->packetLen);
            }
            else
            {
                LOGD("DATA PACKET PARSE FAILED[%d].", transfer->packetLen);
                PRINTF_HEX(transfer->packetBuf, s_HeaderTransferUnit.pktLen);
            }
            if (transfer->packetBuf != NULL)
            {
                vPortFree(transfer->packetBuf);
                transfer->packetBuf = NULL;
            }
            memset(&s_HeaderTransferUnit, 0x00, sizeof(s_HeaderTransferUnit));
            transfer->stage = kHALPacket_Header;
        }
    }
}

static hal_input_status_t HAL_InputDev_UartComm_Init(input_dev_t *dev, input_dev_callback_t callback)
{
    hal_input_status_t status = kStatus_HAL_InputSuccess;

    lpuart_rtos_config_t config = {
        .base        = COMM_UART_BASEADDR,
        .baudrate    = 115200,
        .parity      = kLPUART_ParityDisabled,
        .stopbits    = kLPUART_OneStopBit,
        .buffer      = s_LpuartRingBuffer,
        .buffer_size = sizeof(s_LpuartRingBuffer),
#if defined(FSL_FEATURE_LPUART_HAS_MODEM_SUPPORT) && FSL_FEATURE_LPUART_HAS_MODEM_SUPPORT
        .enableRxRTS = 0,
        .enableTxCTS = 0,
        .txCtsSource = 0,
        .txCtsConfig = 0,
#endif
    };

    config.srcclk = CLOCK_GetRootClockFreq(COMM_UART_CLOCK_ROOT);
    BOARD_InitUartCommResource();
    NVIC_SetPriority(COMM_UART_IRQ, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1);
    if (kStatus_Success != LPUART_RTOS_Init(&s_LpuartRTOSHandle, &s_LpuartHandle, &config))
    {
        vTaskSuspend(NULL);
    }

    dev->cap.callback = callback;

    return status;
}

static hal_input_status_t HAL_InputDev_UartComm_Deinit(const input_dev_t *dev)
{
    hal_input_status_t status = kStatus_HAL_InputSuccess;
    return status;
}

static hal_input_status_t HAL_InputDev_UartComm_Start(const input_dev_t *dev)
{
    hal_input_status_t status = kStatus_HAL_InputSuccess;

    s_UartCommTransfer.nibbleBuf  = s_UartCommNibbleBuffer;
    s_UartCommTransfer.nibbleLen  = sizeof(s_UartCommNibbleBuffer);
    s_UartCommTransfer.packetBuf  = NULL;
    s_UartCommTransfer.packetLen  = 0;
    s_UartCommTransfer.readOffset = 0;
    s_UartCommTransfer.stage      = kHALPacket_Header;

    s_UartCommEvent.eventBase.respond = NULL;

    if (xTaskCreate(SLN_UartCommMsgHandle, UART_COMM_TASK_NAME, UART_COMM_TASK_STACK, &s_UartCommTransfer,
                    UART_COMM_TASK_PRIORITY, NULL) != pdPASS)
    {
        LOGE("[UartComm] Task creation failed!.");
        while (1)
            ;
    }

    return status;
}

static hal_input_status_t HAL_InputDev_UartComm_Stop(const input_dev_t *dev)
{
    hal_input_status_t status = kStatus_HAL_InputSuccess;
    return status;
}

static hal_input_status_t HAL_InputDev_UartComm_InputNotify(const input_dev_t *dev, void *param)
{
    hal_input_status_t status = kStatus_HAL_InputSuccess;
    return status;
}

static hal_output_status_t HAL_OutputDev_UartComm_Start(const output_dev_t *dev)
{
    hal_output_status_t status = kStatus_HAL_OutputSuccess;
    if (FWK_OutputManager_RegisterEventHandler(dev, &s_OutputDev_UartCommHandler) != 0)
        status = kStatus_HAL_OutputError;
    return status;
}

static hal_output_status_t HAL_OutputDev_UartComm_InferComplete(const output_dev_t *dev,
                                                                output_algo_source_t source,
                                                                void *inferResult)
{
    hal_output_status_t error              = kStatus_HAL_OutputSuccess;
    vision_algo_result_t *visionAlgoResult = (vision_algo_result_t *)inferResult;
    oasis_lite_result_t *pResult           = NULL;

    if (visionAlgoResult != NULL)
    {
        if (visionAlgoResult->id == kVisionAlgoID_OasisLite)
        {
            pResult = (oasis_lite_result_t *)&(visionAlgoResult->oasisLite);
        }
    }

    if (pResult != NULL)
    {
        // update algo result begin====>
        uint8_t new_results = memcmp(&s_LastOasisResult, pResult, sizeof(oasis_lite_result_t));
        uint8_t type        = RECOGNITION_CMD_RES;
        if (new_results != 0)
        {
            switch (pResult->state)
            {
                case kOASISLiteState_Invalid:
                {
                    switch (lastFaceCmd)
                    {
                        case REGISTRATION_CMD_REQ:
                        {
                            type = REGISTRATION_CMD_RES;
                        }
                        break;
                        case RECOGNITION_CMD_REQ:
                        {
                            type = RECOGNITION_CMD_RES;
                        }
                        break;
                        case DEREGISTRATION_CMD_REQ:
                        {
                            type = DEREGISTRATION_CMD_RES;
                        }
                        break;
                        default:
                            break;
                    }
                }
                break;

                case kOASISLiteState_Recognition:
                {
                    type = RECOGNITION_CMD_RES;
                }
                break;

                case kOASISLiteState_Registration:
                {
                    type = REGISTRATION_CMD_RES;
                }
                break;

                case kOASISLiteState_DeRegistration:
                {
                    type = DEREGISTRATION_CMD_RES;
                }
                break;

                default:
                    break;
            }

            faceResult.state     = pResult->state;
            faceResult.result    = pResult->result;
            faceResult.faceCount = pResult->face_count;
            memcpy(faceResult.faceBox, pResult->face_box.rect, 16);
            faceResult.quality    = pResult->qualityCheck;
            faceResult.expectOri  = pResult->debug_info.OriExpected;
            faceResult.percentage = pResult->process;
            faceResult.sim        = pResult->debug_info.sim;
            faceResult.faceID     = pResult->face_id;
            memcpy(faceResult.name, pResult->name, sizeof(faceResult.name));

            SLN_UartCommSendPacket((uint8_t *)&faceResult, sizeof(faceResult), type, 0, kHAL_ACK_SUCCESS);
        }
        //<====update algo result end
        memcpy(&s_LastOasisResult, pResult, sizeof(oasis_lite_result_t));
    }

    return error;
}

static hal_output_status_t HAL_OutputDev_UartComm_InputNotify(const output_dev_t *dev, void *param)
{
    hal_output_status_t status = kStatus_HAL_OutputSuccess;
    // event_base_t eventBase     = *(event_base_t *)param;
    return status;
}

int HAL_Dev_UartComm_Register()
{
    hal_input_status_t status = kStatus_HAL_InputSuccess;
    LOGD("HAL_Dev_UartComm_Register");

    status = FWK_InputManager_DeviceRegister(&s_InputDev_UartComm);
    if (status)
    {
        return status;
    }

    status = FWK_OutputManager_DeviceRegister(&s_OutputDev_UartComm);
    if (status)
    {
        return status;
    }

    return status;
}
#endif
