/*
 * Copyright 2020-2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

/*
 * @brief app_config macro definitions. Please place each required definition here before compiling.
 */

#ifndef APP_CONFIG_H_
#define APP_CONFIG_H_

#include "board_define.h"
#include "fwk_common.h"
#include "hal_camera_dev.h"

#define AT_NONCACHEABLE_SECTION_ALIGN_DTC(var, alignbytes) \
    __attribute__((section("NonCacheable2,\"aw\",%nobits @"))) var __attribute__((aligned(alignbytes)))

#define AT_CACHEABLE_SECTION_ALIGN_OCRAM(var, alignbytes) \
    __attribute__((section(".bss.$SRAM_OCRAM_CACHED,\"aw\",%nobits @"))) var __attribute__((aligned(alignbytes)))
#define AT_NONCACHEABLE_SECTION_ALIGN_OCRAM(var, alignbytes) \
    __attribute__((section(".bss.$SRAM_OCRAM_NCACHED,\"aw\",%nobits @"))) var __attribute__((aligned(alignbytes)))

#define APP_TASK_ID(n) (kFWKTaskID_APPStart + n)

/*----------------------------------------------------------------------------------------------
 * Put all RT117F/VIZN3D definitions here
 * ---------------------------------------------------------------------------------------------*/
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

int HAL_GfxDev_Pxp_Register();
int HAL_DisplayDev_UsbUvc_Register();
int HAL_CameraDev_CsiObU1s_Register();
int HAL_CameraDev_MipiOrbbecU1s_Register();
int HAL_CameraDev_3DSim_Register();
int SLN_CameraDevRGBSimRegister();
int HAL_Dev_BleWuartQn9090_Register();
int HAL_LpmDev_Register();
int HAL_OutputDev_UiSmartlock_Register();
int HAL_OutputDev_UiFfi_Register();
int HAL_OutputDev_Console_Register();
int HAL_OutputDev_RgbLed_Register();
int HAL_OutputDev_IrWhiteLeds_Register();
int HAL_InputDev_PushButtons_Register();

int HAL_OutputDev_SmartLockConfig_Register();

/* App task ID offset definition here. App task ID will start from kFWKTaskID_APPStart */

#define MQS_AUDIO_TASK_ID APP_TASK_ID(1)
#define kAppTaskID_WiFi   APP_TASK_ID(2)


int HAL_OutputDev_MqsAudio_Register();

// for vision_algo_oasis_lite2D device
// please include oasis2D/oasislite_runtime.h
// please link oasis2D/liboasis_lite2D_DEFAULT_117f_ae.a
int HAL_VisionAlgo_OasisLite2D_Register(int mode); // mode=0 smartlock; mode=1 ffi

// for vision_algo_oasis_lite3D device
// please include oasis3D/oasislite_runtime.h
// please link oasis3D/liboasis_lite3D_DEFAULT_117f_ae.a
int HAL_VisionAlgoDev_OasisLite3D_Register(int mode); // mode=0 smartlock; mode=1 ffi

/*
 * @brief H264 recording HAL device register.
 *
 * @param recordDuration [in]  the recording duraiton in ms, 0 to use the default recording duration 60000.
 * @param inputWidth [in]  the width of the input YUV420P frame, 0 to use the default input width 640.
 * @param inputHeight [in]  the height of the input YUV420P frame, 0 to use the default input height 480.
 * @returns 0 for the success
 */
int HAL_VisionAlgoDev_H264Recording_Register(unsigned int recordDuration,
                                             unsigned int inputWidth,
                                             unsigned int inputHeight);
                                             
#if ENABLE_3D_SIMCAMERA
int HAL_CameraDev_3DSim_Register();
#endif

#if ENABLE_QN9090
int HAL_Dev_BleWuartQn9090_Register();
#endif

#if ENABLE_WIFI
int HAL_WiFiAWAM510_Register();
#endif

#if ENABLE_CSI_CAMERA
#define CAMERA_PIXEL_FORMAT_CSI_GC0308    kPixelFormat_UYVY1P422_Gray
#define CAMERA_WIDTH_CSI_GC0308           640
#define CAMERA_HEIGHT_CSI_GC0308          480
#define CAMERA_BYTES_PER_PIXEL_CSI_GC0308 2
#define CAMERA_BUFFER_COUNT_CSI_GC0308    4
#define CAMERA_ROTATION_CSI_GC0308 kCWRotateDegree_270
#define CAMERA_FLIP_CSI_GC0308     kFlipMode_None
#define CAMERA_SWAPBYTE_CSI_GC0308 1

int HAL_CameraDev_CsiGc0308_Register(camera_dev_static_config_t *format);
#endif

#if ENABLE_SHELL_USB
int HAL_InputDev_ShellUsb_Register();
#endif
#if ENABLE_SHELL_UART
int HAL_InputDev_ShellUart_Register();
#endif

#if ENABLE_FLEXIO_CAMERA
#define CAMERA_PIXEL_FORMAT_FLEXIO_GC0308    kPixelFormat_UYVY1P422_RGB
#define CAMERA_WIDTH_FLEXIO_GC0308           640
#define CAMERA_HEIGHT_FLEXIO_GC0308          480
#define CAMERA_BYTES_PER_PIXEL_FLEXIO_GC0308 2
#define CAMERA_BUFFER_COUNT_FLEXIO_GC0308    4
#define CAMERA_ROTATION_FLEXIO_GC0308 kCWRotateDegree_270
#define CAMERA_FLIP_FLEXIO_GC0308     kFlipMode_None
#define CAMERA_SWAPBYTE_FLEXIO_GC0308 1

int HAL_CameraDev_FlexioGc0308_Register(camera_dev_static_config_t *config);
#endif

#if ENABLE_LCDIF_RK024HH298
int HAL_DisplayDev_LcdifRk024hh298_Register();
#endif

#if ENABLE_LCDIFV2_RK055AHD091
int HAL_DisplayDev_Lcdifv2Rk055ahd091_Register();
#endif

#if ENABLE_DISPLAY_OVER_USBCDC
int HAL_DisplayDev_UsbCdc3D_Register();
int HAL_DisplayDev_UsbCdc2D_Register();
#endif

/*
 * The UI will dependent on the display resolution
 */
#if ENABLE_LCDIFV2_RK055AHD091
#define UI_BUFFER_WIDTH  480
#define UI_BUFFER_HEIGHT 640
#else
#define UI_BUFFER_WIDTH  240
#define UI_BUFFER_HEIGHT 320
#endif

int HAL_FlashDev_Littlefs_Init();

#if ENABLE_DISPLAY_OVER_LPUART
int HAL_DisplayDev_Lpuart3D_Register();
int HAL_DisplayDev_Lpuart2D_Register();
#endif

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* APP_CONFIG_H_ */
