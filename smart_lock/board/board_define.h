/*
 * Copyright 2020-2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

/*
 * @brief board-related macro definitions. Please place each required definition here before compiling.
 */

#ifndef BOARD_DEFINE_H_
#define BOARD_DEFINE_H_

/*----------------------------------------------------------------------------------------------
 * Put all RT117F/VIZN3D definitions here
 * ---------------------------------------------------------------------------------------------*/

#define QN9090
#define WIFI_IW416_BOARD_AW_AM457_USD
/* Display Chinese font definition */
#define ENABLE_CHINESE_FONT_DISPLAY 0

/* RK024HH298 Panel connected to Parallel LCDIF */
#define RK024HH298

/* Camera Definitions */

#if defined(SMART_LOCK_3D)
/* 3D Camera connected to the MIPI2CSI bridge */
#define ENABLE_CSI_3DCAMERA  1
#define ENABLE_MIPI_3DCAMERA 0

/* FlexIO camera */
#define ENABLE_FLEXIO_CAMERA 1
#define ENABLE_CSI_CAMERA    0

#elif defined(SMART_LOCK_2D) || defined(SMART_ACCESS_2D)
#define ENABLE_CSI_3DCAMERA  0
#define ENABLE_MIPI_3DCAMERA 0
/* FlexIO and CSI camera */
#define ENABLE_FLEXIO_CAMERA 1
#define ENABLE_CSI_CAMERA    1
/*FlexIO(IR)+CSI(IR) or FlexIO(RGB)+CSI(IR)*/
#define ENFORCE_FLEXIO_CAMERA_AS_IR 0
#else
#error "***Invalid demo APP definition! Supported definition [SMART_LOCK_3D | SMART_LOCK_2D | SMART_ACCESS_2D]***"
#endif

/*
 * Debug console definition
 */
#define DEBUG_CONSOLE_UART_INDEX 12

/*
 * Shell definition
 */
#define ENABLE_SHELL_USB  1
#define ENABLE_SHELL_UART 0

/*
 * Panel definition
 */
#if (defined(HEADLESS_ENABLE) && (HEADLESS_ENABLE==1))
#define ENABLE_DISPLAY_OVER_LPUART 0
#define ENABLE_DISPLAY_OVER_USBCDC 0
#define ENABLE_DISPLAY_UVC         0
#else
 /* Display over lpuart or usbcdc definition */
#define ENABLE_DISPLAY_OVER_LPUART 0
#define ENABLE_DISPLAY_OVER_USBCDC 0
#define ENABLE_DISPLAY_UVC         1
#if defined(RK024HH298)
/* RK024HH298 Panel connected to Parallel LCDIF */
#define ENABLE_LCDIF_RK024HH298 1
#elif defined(RK055AHD091)
/* RK055AHD091 Panel connected to MIPI LCDIFV2 */
#define ENABLE_LCDIFV2_RK055AHD091 0
#else
#warning "No panel has been enable. Use only default UVC."
#endif

#endif /* HEADLESS_ENABLE */

/* 3D camera simulator */
#define ENABLE_3D_SIMCAMERA 0

/* Connectivity definitions */

#if defined(WIFI_IW416_BOARD_AW_AM457_USD) || defined(WIFI_IW416_BOARD_AW_AM457MA)
#define ENABLE_WIFI 1
#define SD8978
#define SDMMCHOST_OPERATION_VOLTAGE_1V8
#include "wifi_config.h"
#endif

#if defined(QN9090)
#define ENABLE_QN9090 1
#endif

#endif /* BOARD_DEFINE_H_ */
