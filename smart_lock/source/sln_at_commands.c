/*
 * Copyright 2019-2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

/*
 * @brief AT commands implementation.
 */

#include <FreeRTOS.h>
#include <queue.h>
#include <stdio.h>
#include <string.h>

#include "board_define.h"
#include "board.h"
#include "fsl_lpuart_freertos.h"
#include "fsl_lpuart.h"
#include "fwk_log.h"
#include "fwk_common.h"
#include "sln_at_commands.h"
#include "hal_event_descriptor_face_rec.h"
#include "app_config.h"

#define AT_COMMANDS_TASK_NAME           "at_commands_task"
#define AT_COMMANDS_TASK_STACK          2048
#define AT_COMMANDS_TASK_PRIORITY       5
#define AT_COMMANDS_BUFF_SIZE           1024
#define AT_COMMANDS_RSP_BUFF_SIZE       32
#define AT_COMMANDS_EMPTY_LIMIT         0
#define AT_COMMANDS_RREG_LIMIT          817
#define MAX_DUPLICATE_RESP_LEN          42

AT_NONCACHEABLE_SECTION_ALIGN_DTC(static uint8_t s_commands_buf[AT_COMMANDS_BUFF_SIZE], 8);
AT_NONCACHEABLE_SECTION_ALIGN_DTC(static char s_response_buff[AT_COMMANDS_RSP_BUFF_SIZE], 8);

static uint8_t s_lpuart_char = 0x00;
static lpuart_rtos_handle_t *s_LpuartRTOSHandle;
static input_dev_t *s_InputDev_ATCommands;
static event_face_rec_t s_ATCommandsEvent;
static input_event_t s_InputEvent;
static uint32_t s_current_idx;

#if HEADLESS_ENABLE
static oasis_lite_headless_reg_process_t s_HeadlessRegStatus = OASIS_LITE_HEADLESS_REG_START;
#endif

static sln_at_command_t Parse_ATCommand(uint8_t *rcv_buff, uint32_t buff_len)
{
    sln_at_command_t command = ATCOMMAND_ERROR;
    uint8_t *command_str;
    uint8_t *result_str;

    strupr((char *)rcv_buff);

    if ((result_str = (uint8_t *)strstr((const char *)rcv_buff, (const char *)"AT+")) != NULL)
    {
        command_str = (uint8_t *)strstr((const char *)result_str, (const char *)"FACEECHO=");
        if (command_str != NULL)
        {
            result_str = &command_str[strlen("FACEECHO=")];

            if (strncmp((const char *)result_str, "OK", 2) == 0)
            {
                return FACEECHO_OK;
            }

            if (strncmp((const char *)result_str, "FALSE", 5) == 0)
            {
                return FACEECHO_FALSE;
            }

            return ATCOMMAND_ERROR;
        }

        command_str = (uint8_t *)strstr((const char *)result_str, (const char *)"FACEREG=");
        if (command_str != NULL)
        {
            return FACEREG;
        }

        command_str = (uint8_t *)strstr((const char *)result_str, (const char *)"FACEDREG=");
        if (command_str != NULL)
        {
            return FACEDREG;
        }

        command_str = (uint8_t *)strstr((const char *)result_str, (const char *)"FACERREG=");
        if (command_str != NULL)
        {
            return FACERREG;
        }

        command_str = (uint8_t *)strstr((const char *)result_str, (const char *)"FACEDEL=");
        if (command_str != NULL)
        {
            return FACEDEL;
        }
    }

    return command;
}

static int Send_ATCommnd_Response(sln_at_command_t command, char *value)
{
    uint32_t size;

    switch (command)
    {
        case FACEREG_RSP:
            size = snprintf(s_response_buff, AT_COMMANDS_RSP_BUFF_SIZE, "AT+FACEREG=%s\r\n", value);
            break;

        case FACERREG_RSP:
            size = snprintf(s_response_buff, AT_COMMANDS_RSP_BUFF_SIZE, "AT+FACERREG=%s\r\n", value);
            break;

        case FACEDEL_RSP:
            size = snprintf(s_response_buff, AT_COMMANDS_RSP_BUFF_SIZE, "AT+FACEDEL=%s\r\n", value);
            break;

        case FACERES:
            size = snprintf(s_response_buff, AT_COMMANDS_RSP_BUFF_SIZE, "AT+FACERES=%s\r\n", value);
            break;

        case FACEMODE:
            size = snprintf(s_response_buff, AT_COMMANDS_RSP_BUFF_SIZE, "AT+FACEMODE=%s\r\n", value);
            break;
        default:
            size = -1;
            break;
    }

    if (size > 0)
    {
        LPUART_RTOS_Send(s_LpuartRTOSHandle, (uint8_t *)s_response_buff, size);
    }
    else
    {
        return -1;
    }

    return 0;
}

static int ATCommands_InputDev_Respons(uint32_t eventId,
                                       void *response,
                                       event_status_t status,
                                       unsigned char isFinished)
{
    char ID[3];
    remote_reg_result_t *res;

    if (response == NULL)
    {
        return -1;
    }

    switch (eventId) {
        case kEventFaceRecID_DelUser:
            if (status == kEventStatus_Ok)
            {
                Send_ATCommnd_Response(FACEDEL_RSP, "SUCCESS");
            }
            else
            {
                Send_ATCommnd_Response(FACEDEL_RSP, "FAIL");
            }

            break;
        case kEventFaceRecID_AddUserRemote:
            if (status == kEventStatus_Ok)
            {
                res = response;
                if (res->result == OASIS_REG_RESULT_OK)
                {
                    itoa(res->id, ID, 10);
                    Send_ATCommnd_Response(FACERREG_RSP, ID);
                }
                else if (res->result == OASIS_REG_RESULT_DUP)
                {
                    char duplicate_response[MAX_DUPLICATE_RESP_LEN] = "DUPLICATE";

                    Send_ATCommnd_Response(FACERREG_RSP, strncat(duplicate_response, res->name, strlen(res->name)));
                }
                else
                {
                    Send_ATCommnd_Response(FACERREG_RSP, "FAIL");
                }
            }
            else
            {
                Send_ATCommnd_Response(FACERREG_RSP, "FAIL");
            }

            vPortFree(s_ATCommandsEvent.remoteReg.regData);

            s_ATCommandsEvent.remoteReg.regData      = NULL;
            s_ATCommandsEvent.remoteReg.dataLen      = 0;
            s_ATCommandsEvent.remoteReg.isReRegister = -1;
        default:
            break;
    }
    return 0;
}

static void read_command(int limit, char limit_char){

    size_t n = 0;
    s_current_idx = 0;

    while (1)
    {
        LPUART_RTOS_Receive(s_LpuartRTOSHandle, &s_lpuart_char, 1, &n);

        s_commands_buf[s_current_idx] = s_lpuart_char;
        s_current_idx++;

        if (s_current_idx >= AT_COMMANDS_BUFF_SIZE)
        {
            LOGE("Command buffer overflow\r\n");
            s_current_idx = 0;
        }

        if (limit == 0 && s_lpuart_char == limit_char)
        {
            break;
        }

        if (s_current_idx == limit)
        {
            break;
        }

        /* When the LPC is reset we receive a \0 so we know we can reset our buffer */
        if (s_lpuart_char == '\0' && limit == 0)
        {
            s_current_idx = 0;
        }
    }
}

static void SLN_ATCommands_Task()
{
    sln_at_command_t command;
    uint32_t receiverList = 0;
    uint32_t dataLen = 0;

    while(1)
    {
        read_command(AT_COMMANDS_EMPTY_LIMIT, '=');

        /* Add '\0' at the end of the string since it ends with '=' */
        s_commands_buf[s_current_idx] = '\0';
        command = Parse_ATCommand(s_commands_buf, s_current_idx);

        switch (command)
        {
            case FACEREG:

                read_command(AT_COMMANDS_EMPTY_LIMIT, '\r');

                /* Replace the "\r" with \0 so we can use strlen to calculate the size of the data */
                s_commands_buf[s_current_idx - 1] = '\0';

                receiverList = 1 << kFWKTaskID_VisionAlgo;
                dataLen = (uint32_t)strlen(s_commands_buf);

                if (s_InputDev_ATCommands->cap.callback != NULL)
                {
                    s_ATCommandsEvent.eventBase.eventId = kEventFaceRecID_AddUser;
                    s_ATCommandsEvent.eventBase.respond = ATCommands_InputDev_Respons;

                    if (dataLen > 0)
                    {
                        s_ATCommandsEvent.addFace.hasName = true;
                        strncpy(s_ATCommandsEvent.addFace.name,(const char *) s_commands_buf, dataLen);
                    }
                    else
                    {
                        s_ATCommandsEvent.addFace.hasName = false;
                    }

                    uint8_t fromISR                   = __get_IPSR();
                    s_InputEvent.inputData            = &s_ATCommandsEvent;
                    s_InputDev_ATCommands->cap.callback(s_InputDev_ATCommands, kInputEventID_Recv, receiverList,
                                                           &s_InputEvent, 0, fromISR);
                }
                break;

            case FACEDREG:
            receiverList = 1 << kFWKTaskID_VisionAlgo;

            if (s_InputDev_ATCommands->cap.callback != NULL)
            {
                s_ATCommandsEvent.eventBase.eventId = kEventFaceRecID_DelUser;
                s_ATCommandsEvent.eventBase.respond = ATCommands_InputDev_Respons;
                s_ATCommandsEvent.delFace.hasName   = false;
                s_ATCommandsEvent.delFace.hasID     = false;
                uint8_t fromISR                   = __get_IPSR();
                s_InputEvent.inputData            = &s_ATCommandsEvent;
                s_InputDev_ATCommands->cap.callback(s_InputDev_ATCommands, kInputEventID_Recv, receiverList,
                                                       &s_InputEvent, 0, fromISR);
            }
            break;

            case FACERREG:
                read_command(AT_COMMANDS_RREG_LIMIT, '\r');

                receiverList = 1 << kFWKTaskID_VisionAlgo;
                dataLen = AT_COMMANDS_RREG_LIMIT - 1;

                if (dataLen > 0)
                {
                    s_ATCommandsEvent.eventBase.eventId = kEventFaceRecID_AddUserRemote;
                    s_ATCommandsEvent.eventBase.respond = ATCommands_InputDev_Respons;

                   if (s_commands_buf[0] == 0)
                   {
                       s_ATCommandsEvent.remoteReg.isReRegister = 0;
                   }
                   else
                   {
                       s_ATCommandsEvent.remoteReg.isReRegister = 1;
                   }

                    s_ATCommandsEvent.remoteReg.dataLen = dataLen;
                    s_ATCommandsEvent.remoteReg.regData = pvPortMalloc(dataLen);

                    if (s_ATCommandsEvent.remoteReg.regData != NULL)
                    {
                        memcpy(s_ATCommandsEvent.remoteReg.regData, s_commands_buf + 5, dataLen);

                        uint8_t fromISR = __get_IPSR();
                        if (s_InputDev_ATCommands->cap.callback != NULL)
                        {
                            s_InputEvent.inputData = &s_ATCommandsEvent;
                            s_InputDev_ATCommands->cap.callback(s_InputDev_ATCommands, kInputEventID_Recv, receiverList,
                                                                   &s_InputEvent, 0, fromISR);
                        }
                    }
                    else
                    {
                        LOGE("[ERROR]: ATCommandsTask FACERREG malloc buffer error.\r\n");
                        Send_ATCommnd_Response(FACERREG_RSP, "FAIL");
                    }
                }
                else
                {
                    Send_ATCommnd_Response(FACERREG_RSP, "FAIL");
                }
                break;

            case FACEDEL:
                read_command(AT_COMMANDS_EMPTY_LIMIT, '\r');

                /* Replace the "\r" with \0 so we can use strlen to calculate the size of the data */
                s_commands_buf[s_current_idx - 1] = '\0';

                receiverList = 1 << kFWKTaskID_VisionAlgo;

                if (s_InputDev_ATCommands->cap.callback != NULL)
                {
                    s_ATCommandsEvent.eventBase.eventId = kEventFaceRecID_DelUser;
                    s_ATCommandsEvent.eventBase.respond = ATCommands_InputDev_Respons;
                    s_ATCommandsEvent.delFace.hasName   = false;
                    s_ATCommandsEvent.delFace.hasID     = true;
                    s_ATCommandsEvent.delFace.id        = atoi((const char*)s_commands_buf);
                    uint8_t fromISR                   = __get_IPSR();
                    s_InputEvent.inputData            = &s_ATCommandsEvent;
                    s_InputDev_ATCommands->cap.callback(s_InputDev_ATCommands, kInputEventID_Recv, receiverList,
                                                           &s_InputEvent, 0, fromISR);
                }
                break;

            default:
                break;
        }

        s_lpuart_char = 0;
        s_current_idx = 0;
    }
}

void APP_OutputDev_ATCommands_InferCompleteDecode(output_algo_source_t source, oasis_lite_result_t inferResult)
{
    uint32_t result = inferResult.result;
    char ID[3];

    if (source == kOutputAlgoSource_LPM)
    {
        Send_ATCommnd_Response(FACEMODE, "LP");
    }

    if (inferResult.qualityCheck == kOasisLiteQualityCheck_FakeFace)
    {
        Send_ATCommnd_Response(FACEREG_RSP, "FAKE");
    }

    switch (inferResult.state)
    {
        case kOASISLiteState_Recognition:
            switch (result)
            {
                case kOASISLiteRecognitionResult_Success:
                    itoa(inferResult.face_id, ID, 10);
                    Send_ATCommnd_Response(FACERES, ID);
                    break;
                default:
                    break;
            }

            break;
        case kOASISLiteState_Registration:
            switch (result)
            {
                case kOASISLiteRegistrationResult_Success:
                    itoa(inferResult.face_id, ID, 10);
                    Send_ATCommnd_Response(FACEREG_RSP, ID);
                    break;

                case kOASISLiteRegistrationResult_Timeout:
                    Send_ATCommnd_Response(FACEREG_RSP, "FAIL");
                    break;

                case kOASISLiteRegistrationResult_Duplicated:
                    Send_ATCommnd_Response(FACEREG_RSP, "DUPLICATE");
                    break;

                default:
                    break;
            }

#if HEADLESS_ENABLE
            if(s_HeadlessRegStatus != inferResult.headless_reg_status)
            {
                switch (inferResult.headless_reg_status)
                {
                    case OASIS_LITE_HEADLESS_REG_FRONT_FACE:
                        Send_ATCommnd_Response(FACEREG_RSP, "FRONT");
                        break;

                    case OASIS_LITE_HEADLESS_REG_LEFT_FACE:
                        Send_ATCommnd_Response(FACEREG_RSP, "LEFT");
                        break;

                    case OASIS_LITE_HEADLESS_REG_RIGHT_FACE:
                        Send_ATCommnd_Response(FACEREG_RSP, "RIGHT");
                        break;

                    default:
                        break;
                }
            }
#endif
            break;

        case kOASISLiteState_DeRegistration:
            switch (result)
            {
                case kOASISLiteDeregistrationResult_Success:
                    Send_ATCommnd_Response(FACEDEL_RSP, "SUCCESS");
                    break;

                case kOASISLiteDeregistrationResult_Timeout:
                    Send_ATCommnd_Response(FACEDEL_RSP, "FAIL");
                    break;

                default:
                    break;
            }
            break;

        default:
            break;
    }

#if HEADLESS_ENABLE
    s_HeadlessRegStatus = inferResult.headless_reg_status;
#endif

}

void Start_ATCommands_Processing(lpuart_rtos_handle_t *LpuartRTOSHandle, input_dev_t *input_dev)
{
    s_LpuartRTOSHandle = LpuartRTOSHandle;
    s_InputDev_ATCommands = input_dev;

    if (xTaskCreate(SLN_ATCommands_Task, AT_COMMANDS_TASK_NAME, AT_COMMANDS_TASK_STACK, NULL,
            AT_COMMANDS_TASK_PRIORITY, NULL) != pdPASS)
    {
        LOGE("[ATCommands] Task creation failed!.");
        while (1)
            ;
    }
}
