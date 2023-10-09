/**
* Copyright 2021 NXP.
* This software is owned or controlled by NXP and may only be used strictly in accordance with the
* license terms that accompany it. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you
* agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
* applicable license terms, then you may not retain, install, activate or otherwise use the software.
*
*/

#include "jni.h"

#include <math.h>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <vector>
#include <string>
#include <android/log.h>
#include <pthread.h>
#include <mutex>

#define TAG "FMD_AJNI"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

static int gOasisReady = 0;

#include "oasislite2D_runtime.h"

//#include "face_detect_test_data_480_640_bgr.h"

using namespace std;

#define OASIS_RGB_FRAME_WIDTH 640
#define OASIS_RGB_FRAME_HEIGHT 480

#define OASIS_DETECT_MIN_FACE 100

#define FACE_NAME_MAX_LEN 31

typedef enum _oasis_lite_mode
{
    OASIS_LITE_RECOGNITION,
    OASIS_LITE_REGISTRATION,
    OASIS_LITE_MODE_COUNT
} oasis_lite_mode_t;

typedef enum _oasis_lite_registration_result
{
    OASIS_LITE_REGISTRATION_INVALID,
    OASIS_LITE_REGISTRATION_SUCCESS,
    OASIS_LITE_REGISTRATION_DUPLICATED,
    OASIS_LITE_REGISTRATION_TIMEOUT,
    OASIS_LITE_REGISTRATION_RESULT_COUNT
} oasis_lite_registration_result_t;

// oasis lite algorithm result
typedef struct _oasis_lite_result
{
    oasis_lite_mode_t mode;

    // detect result
    int face_count;
    FaceBox_t face_box;

    // recognition result
    int face_recognized;
    int face_id;
    char name[FACE_NAME_MAX_LEN];

    // registration result
    oasis_lite_registration_result_t reg_result;
} oasis_lite_result_t;

typedef struct _oasis_lite_param
{
    OASISLTInitPara_t config;
    oasis_lite_result_t result;
    ImageFrame_t frames[OASISLT_INT_FRAME_IDX_LAST];
    ImageFrame_t *pframes[OASISLT_INT_FRAME_IDX_LAST];
    OASISRunFlag_t run_flag;

    // face data
    signed char *face_data;
    int face_data_size;
    int face_item_size;
} oasis_lite_param_t;

static oasis_lite_param_t gOasisLite;
static OASISLTHandler_t gOasisHandler;

static void _oasis_lite_EvtCb(ImageFrame_t *frames[], OASISLTEvt_t evt, OASISLTCbPara_t *para, void *user_data)
{
    LOGI("  OASIS_EVT:[%d]\r\n", evt);
    oasis_lite_param_t *pOasisLite = (oasis_lite_param_t *)user_data;

    switch (evt)
    {
        case OASISLT_EVT_DET_START: {
        }
        break;

        case OASISLT_EVT_DET_COMPLETE: {
            if (para->faceBoxRGB == NULL)
            {
                LOGI("DET:[0]\r\n");
                pOasisLite->result.face_count = 0;
            }
            else
            {
                LOGI("DET:[%d, %d, %d, %d]\r\n", para->faceBoxRGB->rect[0], para->faceBoxRGB->rect[1],
                     para->faceBoxRGB->rect[2], para->faceBoxRGB->rect[3]);
                pOasisLite->result.face_count = 1;
                pOasisLite->result.face_box   = (*(para->faceBoxRGB));
            }
        }
        break;

        case OASISLT_EVT_QUALITY_CHK_START: {
        }
        break;

        case OASISLT_EVT_QUALITY_CHK_COMPLETE: {
        }
        break;

        case OASISLT_EVT_REC_START: {
        }
        break;

        case OASISLT_EVT_REC_COMPLETE: {
#if 0
            // Recognition complete
            OASISLTRecognizeRes_t rec_result = para->recResult;
            int face_id                      = para->faceID;

            // recognized
            pOasisLite->result.face_recognized = 1;

            if (rec_result == OASIS_REC_RESULT_KNOWN_FACE)
            {
                // recognized
                pOasisLite->result.face_id = face_id;
                char *face_name            = facedb_get_name(face_id);

                if (face_name != NULL)
                {
                    LOGI("KNOWN_FACE:[%s]\r\n", face_name);
                    strcpy(pOasisLite->result.name, face_name);
                }
                else
                {
                    sprintf(pOasisLite->result.name, "user_%03d", face_id);
                    LOGD("ERROR:failed to get face name %d\r\n", face_id);
                }
            }
            else if (rec_result == OASIS_REC_RESULT_UNKNOWN_FACE)
            {
                pOasisLite->result.face_id = -1;
                // unknown face
                LOGI("UNKNOWN_FACE\r\n");
            }
            else
            {
            }
#endif
        }
        break;

        case OASISLT_EVT_REG_START: {
        }
        break;

        case OASISLT_EVT_REG_IN_PROGRESS: {
        }
        break;

        case OASISLT_EVT_REG_COMPLETE: {
            unsigned id              = para->faceID;
            OASISLTRegisterRes_t res = para->regResult;

            if (res == OASIS_REG_RESULT_OK)
            {
                gOasisLite.result.reg_result = OASIS_LITE_REGISTRATION_SUCCESS;
            }
            else if (res == OASIS_REG_RESULT_DUP)
            {
                gOasisLite.result.reg_result = OASIS_LITE_REGISTRATION_DUPLICATED;
            }
            else
            {
                gOasisLite.result.reg_result = OASIS_LITE_REGISTRATION_INVALID;
            }
        }
        break;

        default: {
        }
        break;
    }
}

static int _oasis_lite_GetFaces(uint16_t *face_ids, void **pFaces, unsigned int *face_num, void* userData)
{
    LOGI("++_oasis_lite_GetFaces\r\n");
    int ret            = 0;

    int db_count       = 0;
    *face_num = 0;

#if 0
    int face_item_size = OASISLT_getFaceItemSize();
    int db_count       = facedb_count();

    // query the face db count
    if (*face_num == 0)
    {
        *face_num = db_count;
        return ret;
    }

    for (uint32_t i = 0; i < *face_num; i++)
    {
        face_ids[i] = i;
        facedb_get_face(i, pFaces + i * sizeof(void *));
    }
#endif

    LOGI("--_oasis_lite_GetFaces [%d]\r\n", db_count);
    return ret;
}

static int _oasis_lite_AddFace(uint16_t* faceId, void* faceData, SnapshotItem_t* snapshotData, int snapshotNum, void* userData)
{
    int ret            = 1;
    int face_item_size = OASISLT_getFaceItemSize();
    LOGI("++_oasis_lite_AddFace %d\r\n", face_item_size);
    char* pFaceData = (char*)faceData;

    for (int i=0; i< face_item_size; i++){
        gOasisLite.face_data[i] = pFaceData[i];
        //LOGI("%02x ", pFaceData[i]);
    }

    gOasisLite.face_data_size = face_item_size;

    LOGI("--_oasis_lite_AddFace\r\n");
    return ret;
}

static int _oasis_lite_UpdateFace(uint16_t faceId, void* faceData, SnapshotItem_t* snapshotData, int snapshotNum, void* userData)
{
    //LOGD("++_oasis_lite_UpdateFace\r\n");
    //LOGD("--_oasis_lite_UpdateFace\r\n");
    return 0;
}

static void _oasis_lite_AdjustBrightness(uint8_t frame_idx, uint8_t direction, void* userData)
{
    //LOGI("++_oasis_lite_AdjustBrightness\r\n");
    //LOGI("--_oasis_lite_AdjustBrightness\r\n");
}

static int _oasis_lite_Log(const char *formatString)
{
    // todo
    LOGD("FMD_OASIS:%s", formatString);
    return 0;
}

static int Oasis_Init(int appType)
{
    LOGD("++Oasis_Init = %d ", appType);
    OASISLTResult_t oasis_ret = OASISLT_OK;

    memset(&gOasisLite, 0, sizeof(gOasisLite));

    // init the oasis lite config
    gOasisLite.config.imgType              = (appType == 0) ? OASIS_IMG_TYPE_IR_RGB_DUAL : OASIS_IMG_TYPE_RGB_IR_DUAL;
    gOasisLite.config.minFace              = OASIS_DETECT_MIN_FACE;
    gOasisLite.config.cbs.EvtCb            = _oasis_lite_EvtCb;
    gOasisLite.config.cbs.GetFaces         = _oasis_lite_GetFaces;
    gOasisLite.config.cbs.AddFace          = _oasis_lite_AddFace;
    gOasisLite.config.cbs.UpdateFace       = _oasis_lite_UpdateFace;
    gOasisLite.config.cbs.AdjustBrightness = _oasis_lite_AdjustBrightness;
    gOasisLite.config.cbs.reserved         = (void*)_oasis_lite_Log;
    gOasisLite.config.cbs.lock             = NULL;
    gOasisLite.config.cbs.unlock           = NULL;
    gOasisLite.config.enableFlags          = 0;//OASIS_ENABLE_MULTI_VIEW;
    gOasisLite.config.falseAcceptRate      = OASIS_FAR_1_1000000;
    gOasisLite.config.height               = OASIS_RGB_FRAME_HEIGHT;
    gOasisLite.config.width                = OASIS_RGB_FRAME_WIDTH;
    gOasisLite.config.size                 = 0;
    gOasisLite.config.memPool             = NULL;
    gOasisLite.config.fastMemBuf         = NULL;
    gOasisLite.config.fastMemSize        = 0;
    gOasisLite.config.runtimePara.brightnessTH[0] = 80;
    gOasisLite.config.runtimePara.brightnessTH[1] = 180;

    gOasisLite.run_flag = OASIS_DET_REC_REG_REMOTE;

    oasis_ret = OASISLT_init(&gOasisLite.config);

    if (oasis_ret == OASIS_INIT_INVALID_MEMORYPOOL)
    {
        gOasisLite.config.memPool = new char[gOasisLite.config.size]; //(char *)pvPortMalloc(gOasisLite.config.size);
        LOGE("OASIS LITE MEM POOL %d\r\n", gOasisLite.config.size);

        if (gOasisLite.config.memPool == NULL)
        {
            LOGE("[ERROR]: Unable to allocate memory for oasis mem pool %d\r\n", gOasisLite.config.size);

            while (1)
                ;
        }

        oasis_ret = OASISLT_init(&gOasisLite.config);
    }

    // allocate the face_data
    gOasisLite.face_item_size = OASISLT_getFaceItemSize();
    LOGD("Face data size %d\r\n", gOasisLite.face_item_size);
    gOasisLite.face_data = new signed char[gOasisLite.face_item_size];
    if (gOasisLite.face_data == NULL)
    {
        LOGE("[ERROR]: Unable to allocate memory for face data %d\r\n", gOasisLite.face_data_size);

        while (1)
            ;
    }

    if (oasis_ret != OASISLT_OK)
    {
        LOGE("[ERROR]:OASISLT_init ret %d\r\n", oasis_ret);
        return oasis_ret;
    }

    oasis_ret =  OASISLT_CreateInstance(&gOasisHandler);
    if (oasis_ret != OASISLT_OK)
    {
        LOGE("[ERROR]:OASISLT_CreateInstance ret %d\r\n", oasis_ret);
        return oasis_ret;
    }

    LOGD("--Oasis_Init");

    return 0;
}

static int Oasis_Exit()
{
    int ret = 0;

    OASISLT_DeleteInstance(gOasisHandler);

    ret = OASISLT_uninit();

    if (ret)
    {
        LOGE("[ERROR]:OASISLT_uninit failed\n");
    }

    if (gOasisLite.config.memPool != NULL)
    {
        delete gOasisLite.config.memPool;
    }

    if (gOasisLite.face_data != NULL)
    {
        delete gOasisLite.face_data;
        gOasisLite.face_data_size = 0;
    }

    return ret;
}


extern "C" JNIEXPORT jint JNICALL Java_com_smartlockmanager_utility_Algorithm_Init(JNIEnv *env, jobject obj, jint appType)
{
    LOGE("++Init:%d", gOasisReady);
    jint ret = 0;
    if (gOasisReady == 0)
    {
        ret = Oasis_Init(appType);

        if (ret)
        {
            LOGE("[ERROR]:oasis init %d", ret);
            return ret;
        }
    }
    gOasisReady = 1;
    LOGE("--Init");
    return ret;
}

extern "C" JNIEXPORT jint JNICALL Java_com_smartlockmanager_utility_Algorithm_Exit(JNIEnv *env, jobject obj)
{
    LOGD("++Exit:%d", gOasisReady);
    jint ret = 0;
    gOasisReady = 0;
    ret = Oasis_Exit();
    LOGD("--Exit");
    return ret;
}

extern "C" JNIEXPORT jint JNICALL Java_com_smartlockmanager_utility_Algorithm_GetFaceSize(JNIEnv *env, jobject instance)
{
    LOGD("++GetFaceSize:%d", gOasisReady);
    if (gOasisReady) {
        return OASISLT_getFaceItemSize();
    } else {
        return 0;
    }
    LOGD("--GetFaceSize");
}

extern "C" JNIEXPORT jint JNICALL Java_com_smartlockmanager_utility_Algorithm_GetVersion(JNIEnv *env, jobject instance)
{
    return  VERSION_MAJOR << 16 | VERSION_MINOR << 8 | VERSION_HOTFIX;
}

extern "C" JNIEXPORT jint JNICALL Java_com_smartlockmanager_utility_Algorithm_Registration(JNIEnv *env, jobject obj,
                                                                                           jbyteArray data, jint width,
                                                                                           jint height, jintArray box,
                                                                                           jbyteArray feature)
{
    LOGE("++Registration:%d", gOasisReady);
    if (gOasisReady)
    {
        jbyte *frameData = env->GetByteArrayElements(data, NULL);
        if (NULL == frameData) {
            env->ReleaseByteArrayElements(data, frameData, 0);
            return -3;
        }

        gOasisLite.frames[OASISLT_INT_FRAME_IDX_RGB].height = height;
        gOasisLite.frames[OASISLT_INT_FRAME_IDX_RGB].width  = width;
        gOasisLite.frames[OASISLT_INT_FRAME_IDX_RGB].fmt    = OASIS_IMG_FORMAT_BGR888;
//        gOasisLite.frames[OASISLT_INT_FRAME_IDX_RGB].data   = (unsigned char*)face_det_test_data_480_640_hwc_bgr[0].data;
        gOasisLite.frames[OASISLT_INT_FRAME_IDX_RGB].data   = (unsigned char *)frameData;
        gOasisLite.pframes[OASISLT_INT_FRAME_IDX_RGB]       = &gOasisLite.frames[OASISLT_INT_FRAME_IDX_RGB];
        gOasisLite.pframes[OASISLT_INT_FRAME_IDX_IR]        = &gOasisLite.frames[OASISLT_INT_FRAME_IDX_RGB];

        if (gOasisLite.run_flag != OASIS_RUN_FLAG_NUM) {
            // clear the result
            memset(&gOasisLite.result, 0, sizeof(gOasisLite.result));

            // clear the face data size
            gOasisLite.face_data_size = 0;

            if (gOasisLite.run_flag == OASIS_DET_REC) {
                gOasisLite.result.mode = OASIS_LITE_RECOGNITION;
            } else if (gOasisLite.run_flag == OASIS_DET_REC_REG || gOasisLite.run_flag == OASIS_DET_REC_REG_REMOTE) {
                gOasisLite.result.mode = OASIS_LITE_REGISTRATION;
            }

            int oasis_ret = OASISLT_MT_run_extend(gOasisHandler, gOasisLite.pframes, gOasisLite.run_flag, gOasisLite.config.minFace, &gOasisLite);
            if (oasis_ret) {
                LOGE("ERROR:OASISLT_run_extend %d\r\n", oasis_ret);
                return -4;
            }

            env->ReleaseByteArrayElements(data, frameData, 0);

            // update the result
            if (gOasisLite.result.face_count == 1) {
                LOGD("DET one face");
                jint *boxData = env->GetIntArrayElements(box, NULL);
                if (NULL == boxData) {
                    env->ReleaseIntArrayElements(box, boxData, 0);
                    LOGD("NULL boxData");
                    return -5;
                }

                int *pBox = (int *)boxData;
                pBox[0] = gOasisLite.result.face_box.rect[0];
                pBox[1] = gOasisLite.result.face_box.rect[1];
                pBox[2] = gOasisLite.result.face_box.rect[2];
                pBox[3] = gOasisLite.result.face_box.rect[3];
                env->ReleaseIntArrayElements(box, boxData, 0);
                LOGD("Update box");

                if (gOasisLite.face_data_size == gOasisLite.face_item_size) {
                    // fill the face feature
                    jbyte *jFaceFeature = env->GetByteArrayElements(feature, NULL);
                    if (NULL == jFaceFeature) {
                        env->ReleaseByteArrayElements(feature, jFaceFeature, 0);
                        return -6;
                    }
                    signed char *pFaceFeature = (signed char *)jFaceFeature;
                    for (int i = 0; i < gOasisLite.face_data_size; i++) {
                        pFaceFeature[i] = gOasisLite.face_data[i];
                    }
                    env->ReleaseByteArrayElements(feature, jFaceFeature, 0);

                    LOGE("--Registration");

                    return 0;
                } else {
                    return -7;
                }

            } else {
                return -8;
            }

        } else {
            return -9;
        }
    } else {
        return -10;
    }
}


