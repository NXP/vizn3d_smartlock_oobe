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

#ifndef _SLN_AT_COMMANDS_H_
#define _SLN_AT_COMMANDS_H_

#include "hal_input_dev.h"
#include "hal_output_dev.h"
#include "hal_vision_algo.h"

typedef enum _sln_at_commands_type
{
    FACEECHO_OK = 0,    /* LPC55 echo to RT117F as CMD received success​ */
    FACEECHO_FALSE,     /* LPC55 echo to RT117F as CMD received failed​ */
    FACEREG,            /* Starting Registration / Enroll Face ID 0-n */
    FACEDREG,           /* Starting Deregistration */
    FACERREG,           /* Remote face registration */
    FACEREG_RSP,        /* Response to local face registration​ */
    FACERREG_RSP,       /* Response to remote face registration */
    FACEDEL,            /* Starting Deregistration face id 0~n​ */
    FACEDEL_RSP,        /* Deregistration response */
    FACERES,            /* Face Recognition response​ */
    FACEMODE,        /* Face module enter to sleep mode​ */
    FACEMODE_CONFIRM,   /* Backup to be used in future​ */
    ATCOMMAND_ERROR     /* Error in parsing the command */
} sln_at_command_t;

void APP_OutputDev_ATCommands_InferCompleteDecode(output_algo_source_t source, oasis_lite_result_t inferResult);

void Start_ATCommands_Processing(lpuart_rtos_handle_t *LpuartRTOSHandle, input_dev_t *input_dev);

#endif //_SLN_AT_COMMANDS_H_


