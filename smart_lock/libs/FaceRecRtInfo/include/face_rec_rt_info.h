/*
 * Copyright 2020-2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

#ifndef _FACE_REC_RT_INFO_H_
#define _FACE_REC_RT_INFO_H_

#ifdef __cplusplus
extern "C" {
#endif

#define FACE_REC_RT_INFO_VERSION_MAJOR 1
#define FACE_REC_RT_INFO_VERSION_MINOR 0
/*this version number only used for hot fix on frozen release or branch*/
#define FACE_REC_RT_INFO_VERSION_HOTFIX 1

/*
 * face rec runtime info id
 */
typedef enum _face_rec_rt_info_id
{
    kFaceRecRtInfoId_Global = 0x56, /* enable: global switch 0: off 1: on; filer: 0: singleshot 1: continously*/
    kFaceRecRtInfoId_Detect,
    kFaceRecRtInfoId_Quality,
    kFaceRecRtInfoId_Fake,
    kFaceRecRtInfoId_MaskDetection,
    kFaceRecRtInfoId_GlassesDetection,
    kFaceRecRtInfoId_FaceAlign,
    kFaceRecRtInfoId_FaceFecognize,
    kFaceRecRtInfoId_Count
} face_rec_rt_info_id_t;

typedef void (*log_printf_t)(const char *);

int FaceRecRtInfo_Init(unsigned char *pStart, unsigned int size, log_printf_t logPrintf);
void FaceRecRtInfo_Size(unsigned char** pStart, unsigned int *pSize);
void FaceRecRtInfo_Clean();
int FaceRecRtInfo_Filter(face_rec_rt_info_id_t id, unsigned char enable, unsigned char filter);
void FaceRecRtInfo_Disable();

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* _FACE_REC_RT_INFO_H_ */
