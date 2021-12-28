/*
 * Copyright 2020-2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in
 * accordance with the license terms that accompany it. By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms. If you do not agree to be
 * bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

/*
 * @brief H.264 recording vision algorithm HAL driver implementation.
 */

#include "fwk_log.h"
#include "fwk_sln_platform.h"
#include "fwk_vision_algo_manager.h"
#include "fwk_perf.h"
#include "openh264_enc.h"
#include "hal_event_descriptor_common.h"
#include "hal_sln_timer.h"
#include "hal_vision_algo.h"

#define H264_RECORDING_MAX_WIDTH      640
#define H264_RECORDING_MAX_HEIGHT     480
#define H264_RECORDING_MAX_FRAME_RATE 30
#define H264_RECORDING_TARGET_BITRATE 500000

/* the maximum recording duration in ms*/
#define RECORDING_MAX_DURATION 60000

/* H264 CLIP REGION START ADDRESS, need to align with the definition in the MCUXpresso MCU Setting */
extern void __base_BOARD_SDRAM_H264_CLIP(void);
extern void __top_BOARD_SDRAM_H264_CLIP(void);

#define H264_CLIP_REGION_START ((unsigned int)__base_BOARD_SDRAM_H264_CLIP)

/*
 * DUMP REGION SIZE, need to align with the definition in the MCUXpresso MCU Setting
 */
#define H264_CLIP_REGION_TOTAL_SIZE \
    (((unsigned int)__top_BOARD_SDRAM_H264_CLIP) - ((unsigned int)__base_BOARD_SDRAM_H264_CLIP))

typedef enum _h264_recording_state
{
    kH264RecordingState_Start = 0,
    kH264RecordingState_Recording,
    kH264RecordingState_Stop,
    kH264RecordingState_Invalid
} h264_recording_state_t;

typedef struct _h264_recording_param
{
    h264_recording_state_t state;
    unsigned int encoderState;

    unsigned int inputWidth;
    unsigned int inputHeight;

    unsigned int outputBitrate;
    unsigned int outputMaxFrameRate;

    recording_info_t info;

    sln_timer_t *pRecTimer; /* timer for recording */
    unsigned int recordDuration;
    vision_algo_dev_t *dev;

    vision_algo_result_t result;
} h264_recording_param_t;

static h264_recording_param_t s_H264RecordingParam;

static void _Recording_RequestFrame(const vision_algo_dev_t *dev)
{
    if ((dev != NULL) && (dev->cap.callback != NULL))
    {
        uint8_t fromISR = __get_IPSR();
        dev->cap.callback(dev->id, kVAlgoEvent_RequestFrame, NULL, 0, fromISR);
    }
}

static void _Recording_NotifyResult(const vision_algo_dev_t *dev, vision_algo_result_t *result)
{
    if ((dev != NULL) && (result != NULL) && (dev->cap.callback != NULL))
    {
        uint8_t fromISR = __get_IPSR();
        dev->cap.callback(dev->id, kVAlgoEvent_VisionResultUpdate, result, sizeof(vision_algo_result_t), fromISR);
    }
}

static void _Recording_NotifyStop(void)
{
    event_recording_t eventRecording;
    eventRecording.eventBase.eventId = kEventID_RecordingState;
    eventRecording.state             = kRecordingState_Stop;

    uint8_t fromISR = __get_IPSR();
    s_H264RecordingParam.dev->cap.callback(s_H264RecordingParam.dev->id, kVAlgoEvent_VisionRecordControl,
                                           &eventRecording, sizeof(event_recording_t), fromISR);
}

static void _Recording_SaveFrame(const void *pFrame, unsigned int size)
{
    if (s_H264RecordingParam.info.size + size < H264_CLIP_REGION_TOTAL_SIZE)
    {
        unsigned char *pStart = (unsigned char *)s_H264RecordingParam.info.start + s_H264RecordingParam.info.size;
        memcpy(pStart, pFrame, size);
        s_H264RecordingParam.info.size += size;
        LOGI("Recording:[Frame]:[0x%08x-%d:%d]", pStart, size, s_H264RecordingParam.info.size);
    }
    else
    {
        LOGD("Recording:[STOP]:out of space");
        _Recording_NotifyStop();
    }
}

static void _Recording_TimerCallback(void *arg)
{
    LOGD("Recording:[STOP]:timeout");
    _Recording_NotifyStop();
}

static hal_valgo_status_t _Recording_Record(const vision_algo_dev_t *dev)
{
    hal_valgo_status_t ret        = kStatus_HAL_ValgoSuccess;
    openh264_enc_error_t encError = OPENH264_ENC_ERROR_OK;
    unsigned int encodedSize;

    encError = OpenH264_EncodeFrame((unsigned char *)dev->data.frames[kVAlgoFrameID_RGB].data);
    if (encError)
    {
        LOGE("ERROR:OpenH264_EncodeFrame [%d]", encError);
        ret = kStatus_HAL_ValgoError;
        return ret;
    }
    else
    {
        encError = OpenH264_EncodeSave(_Recording_SaveFrame, &encodedSize);
        if (encError)
        {
            LOGE("ERROR:OpenH264_EncodeSave [%d]", encError);
            ret = kStatus_HAL_ValgoInitError;
        }
    }

    fwk_fps(kFWKFPSType_VAlgo, dev->id);

    return ret;
}

static hal_valgo_status_t _Recording_Start(const vision_algo_dev_t *dev)
{
    openh264_enc_error_t encError = OPENH264_ENC_ERROR_OK;
    hal_valgo_status_t ret        = kStatus_HAL_ValgoSuccess;

    encError = OpenH264_EncodeInit(s_H264RecordingParam.inputWidth, s_H264RecordingParam.inputHeight,
                                   H264_RECORDING_MAX_FRAME_RATE, H264_RECORDING_TARGET_BITRATE);
    if (encError)
    {
        LOGE("ERROR:OpenH264_EncodeInit [%d]", encError);
        ret = kStatus_HAL_ValgoInitError;
        return ret;
    }

    if (sln_timer_start("RecTimer", s_H264RecordingParam.recordDuration, 0, _Recording_TimerCallback,
                        &s_H264RecordingParam, &s_H264RecordingParam.pRecTimer))
    {
        LOGE("Failed to start \"RecTimer\" timer.");
        ret = kStatus_HAL_ValgoInitError;
        return ret;
    }

    s_H264RecordingParam.encoderState = kH264RecordingState_Start;
    s_H264RecordingParam.state        = kH264RecordingState_Recording;
    s_H264RecordingParam.info.size    = 0;

    /* Notify the recording result */
    s_H264RecordingParam.result.h264Recording.state = kRecordingState_Start;
    _Recording_NotifyResult(dev, &(s_H264RecordingParam.result));

    /* calculate the fps */
    fwk_fps_reset(kFWKFPSType_VAlgo, dev->id);
    fwk_fps(kFWKFPSType_VAlgo, dev->id);

    ret = _Recording_Record(dev);

    return ret;
}

static hal_valgo_status_t _Recording_Stop(const vision_algo_dev_t *dev)
{
    hal_valgo_status_t ret = kStatus_HAL_ValgoStop;
    if (s_H264RecordingParam.encoderState != kH264RecordingState_Stop)
    {
        vision_algo_result_t *result = &s_H264RecordingParam.result;
        if (s_H264RecordingParam.pRecTimer)
        {
            sln_timer_stop(&s_H264RecordingParam.pRecTimer);
        }
        s_H264RecordingParam.encoderState = kH264RecordingState_Stop;

        /* Notify the recording result */
        result->h264Recording.recordedDataSize      = s_H264RecordingParam.info.size;
        result->h264Recording.recordedDataAddress   = (uint8_t*)s_H264RecordingParam.info.start;
        result->h264Recording.state                 = kRecordingState_Stop;

        _Recording_NotifyResult(dev, &(s_H264RecordingParam.result));

        OpenH264_EncodeExit();
    }
    return ret;
}

static hal_valgo_status_t HAL_VisionAlgoDev_H264Recording_Init(vision_algo_dev_t *dev,
                                                               valgo_dev_callback_t callback,
                                                               void *param)
{
    hal_valgo_status_t ret = kStatus_HAL_ValgoSuccess;

    /* init the device */
    dev->cap.callback = callback;

    /* init the h264 recording parameter */
    s_H264RecordingParam.dev   = dev;
    s_H264RecordingParam.state = kH264RecordingState_Stop;

    s_H264RecordingParam.encoderState = kH264RecordingState_Stop;

    s_H264RecordingParam.info.start           = H264_CLIP_REGION_START;
    s_H264RecordingParam.info.size            = 0;
    s_H264RecordingParam.result.id            = kVisionAlgoID_H264Recording;
    s_H264RecordingParam.result.h264Recording.state = kRecordingState_Stop;

    dev->data.autoStart                              = 0;
    dev->data.frames[kVAlgoFrameID_RGB].height       = s_H264RecordingParam.inputHeight;
    dev->data.frames[kVAlgoFrameID_RGB].width        = s_H264RecordingParam.inputWidth;
    dev->data.frames[kVAlgoFrameID_RGB].pitch        = s_H264RecordingParam.inputWidth * 3 / 2;
    dev->data.frames[kVAlgoFrameID_RGB].is_supported = 1;
    dev->data.frames[kVAlgoFrameID_RGB].rotate       = kCWRotateDegree_0;
    dev->data.frames[kVAlgoFrameID_RGB].flip         = kFlipMode_None;

    dev->data.frames[kVAlgoFrameID_RGB].format    = kPixelFormat_YUV420P;
    dev->data.frames[kVAlgoFrameID_RGB].srcFormat = kPixelFormat_UYVY1P422_RGB;
    int h264_yuv420p_frame_aligned_size           = SDK_SIZEALIGN(
        s_H264RecordingParam.inputWidth * s_H264RecordingParam.inputHeight * 3 / 2, FSL_FEATURE_L1DCACHE_LINESIZE_BYTE);
    dev->data.frames[kVAlgoFrameID_RGB].data = pvPortMalloc(h264_yuv420p_frame_aligned_size);

    if (dev->data.frames[kVAlgoFrameID_RGB].data == NULL)
    {
        LOGE("Unable to allocate memory for kVAlgoFrameID_RGB.");
        ret = kStatus_HAL_ValgoMallocError;
    }

    return ret;
}

static hal_valgo_status_t HAL_VisionAlgoDev_H264Recording_Deinit(vision_algo_dev_t *dev)
{
    hal_valgo_status_t ret = kStatus_HAL_ValgoSuccess;
    return ret;
}

static hal_valgo_status_t HAL_VisionAlgoDev_H264Recording_Run(const vision_algo_dev_t *dev, void *data)
{
    hal_valgo_status_t ret = kStatus_HAL_ValgoSuccess;
    LOGI("Recording run: %d", s_H264RecordingParam.state);

    switch (s_H264RecordingParam.state)
    {
        case kH264RecordingState_Start:
        {
            ret = _Recording_Start(dev);
        }
        break;

        case kH264RecordingState_Stop:
        {
            ret = _Recording_Stop(dev);
        }
        break;

        case kH264RecordingState_Recording:
        {
            ret = _Recording_Record(dev);
        }
        break;

        default:
            break;
    }

    return ret;
}

static hal_valgo_status_t HAL_VisionAlgoDev_H264Recording_InputNotify(const vision_algo_dev_t *receiver, void *param)
{
    hal_valgo_status_t ret = kStatus_HAL_ValgoSuccess;
    LOGI("++HAL_VisionAlgoDev_OasisLite_InputNotify");

    event_recording_t *pEventRecording = (event_recording_t *)param;
    if (pEventRecording->eventBase.eventId == kEventID_RecordingState)
    {
        LOGD("Recording:[State]:%d", pEventRecording->state);

        if (pEventRecording->state == kRecordingState_Start)
        {
            if (s_H264RecordingParam.state == kH264RecordingState_Stop)
            {
                /* Start the recording */
                s_H264RecordingParam.state = kH264RecordingState_Start;
                _Recording_RequestFrame(receiver);
            }
        }
        else if (pEventRecording->state == kRecordingState_Stop)
        {
            if (s_H264RecordingParam.state != kH264RecordingState_Stop)
            {
                /* Stop the recording */
                s_H264RecordingParam.state = kH264RecordingState_Stop;
            }
        }
    }
    else if (pEventRecording->eventBase.eventId == kEventID_RecordingInfo)
    {
        if (pEventRecording->eventBase.respond != NULL)
        {
            s_H264RecordingParam.info.state = s_H264RecordingParam.state;
            pEventRecording->eventBase.respond(kEventID_RecordingInfo, &s_H264RecordingParam.info, kEventStatus_Ok,
                                               true);
        }
    }

    return ret;
}

const static vision_algo_dev_operator_t s_VisionAlgoDev_H264RecordingOps = {
    .init        = HAL_VisionAlgoDev_H264Recording_Init,
    .deinit      = HAL_VisionAlgoDev_H264Recording_Deinit,
    .run         = HAL_VisionAlgoDev_H264Recording_Run,
    .inputNotify = HAL_VisionAlgoDev_H264Recording_InputNotify,
};

static vision_algo_dev_t s_VisionAlgoDev_H264Recording = {
    .id   = 0,
    .name = "H264_Recording",
    .ops  = (vision_algo_dev_operator_t *)&s_VisionAlgoDev_H264RecordingOps,
    .cap  = {.param = NULL},
};

/*
 * @brief H264 recording HAL device register.
 *
 * @param recordDuration [in]  the recording duration in ms, 0 to use the default recording duration 60000.
 * @param inputWidth [in]  the width of the input YUV420P frame, 0 to use the default input width 640.
 * @param inputHeight [in]  the height of the input YUV420P frame, 0 to use the default input height 480.
 * @returns 0 for the success
 */
int HAL_VisionAlgoDev_H264Recording_Register(unsigned int recordDuration,
                                             unsigned int inputWidth,
                                             unsigned int inputHeight)
{
    int error = 0;

    s_H264RecordingParam.recordDuration = RECORDING_MAX_DURATION;
    if ((recordDuration > 0) && (recordDuration < RECORDING_MAX_DURATION))
    {
        s_H264RecordingParam.recordDuration = recordDuration;
    }
    s_H264RecordingParam.inputWidth = H264_RECORDING_MAX_WIDTH;
    if ((inputWidth > 0) && (inputWidth < H264_RECORDING_MAX_WIDTH))
    {
        s_H264RecordingParam.inputWidth = inputWidth;
    }
    s_H264RecordingParam.inputHeight = H264_RECORDING_MAX_HEIGHT;
    if ((inputHeight > 0) && (inputHeight < H264_RECORDING_MAX_HEIGHT))
    {
        s_H264RecordingParam.inputHeight = inputHeight;
    }
    error = FWK_VisionAlgoManager_DeviceRegister(&s_VisionAlgoDev_H264Recording);
    return error;
}
