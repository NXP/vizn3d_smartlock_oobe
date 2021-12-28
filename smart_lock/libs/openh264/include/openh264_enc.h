/*
 * Copyright 2020-2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

/*
 * @brief OpenH264 encode export API declaration..
 */

#ifndef _OPEN_H_264_ENC_H_
#define _OPEN_H_264_ENC_H_

#if defined(__cplusplus)
extern "C" {
#endif

#define VERSION_MAJOR 0
#define VERSION_MINOR 1
/*this version number only used for hot fix on frozen release or branch*/
#define VERSION_HOTFIX 0

/*
 * @brief the OpenH264 encode error definition.
 */
typedef enum _openh264_enc_error {
    OPENH264_ENC_ERROR_OK = 0, /* success */
    OPENH264_ENC_ERROR, /* generic error */
    OPENH264_ENC_ERROR_AUTHENTICATE_FAILED, /* authenticate failed */
    OPENH264_ENC_ERROR_INIT = 0x10, /* init error start */
    OPENH264_ENC_ERROR_ENC_FRAME = 0x20, /* encode error start */
    OPENH264_ENC_ERROR_SAVE_FRAME = 0x30, /* save frame error start */
} openh264_enc_error_t;

/*
 * @brief the OpenH264 encoded frame save call back function.
 *
 * @param frame [in]  the encoded H264 frame.
 * @param size [in]  the size in bytes of the encoded H264 frame.
 */
typedef void (*SaveFrameCallback) (const void* frame, unsigned int size);

/*
 * @brief OpenH264 encode init.
 *
 * @param width [in]  the width of the input YUV420P frame.
 * @param height [in]  the height of the input YUV420P frame.
 * @returns OPENH264_ENC_ERROR_OK for the success
 */
openh264_enc_error_t OpenH264_EncodeInit(int width, int height, int maxFrameRate, int targetBitrate);

/*
 * @brief OpenH264 encode exit.
 *
 * @returns none
 */
void OpenH264_EncodeExit(void);

/*
 * @brief OpenH264 encode one input YUV420P frame.
 *
 * @param data [in]  point to the YUV raw data of the input YUV420P frame.
 * @returns OPENH264_ENC_ERROR_OK for the success
 */
openh264_enc_error_t OpenH264_EncodeFrame(unsigned char* data);

/*
 * @brief OpenH264 save one encoded output frame.
 *
 * This function will prepare the encoded output frame of the previously OpenH264_EncodeFrame call
 * and invoke the SaveFrame callback to save the encoded output frame.
 *
 * @param saveframe [in]  point to encoded frame save callback.
 * @param size [io]  point to encoded bytes of the previous encoded frame.
 * @returns OPENH264_ENC_ERROR_OK for the success
 */
openh264_enc_error_t OpenH264_EncodeSave(SaveFrameCallback saveframe, unsigned int* size);

/*
 * @brief OpenH264 version information.
 *
 * This function will get the version information
 *
 * @param major [out] major version.
 * @param minor [out] minor version.
 * @param hotfix [out] hotfix version.
 */
void OpenH264_Version(unsigned int *major, unsigned int *minor, unsigned int *hotfix);

#if defined(__cplusplus)
}
#endif

#endif /* _OPEN_H_264_ENC_H_ */
