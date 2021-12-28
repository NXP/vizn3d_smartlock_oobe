/*
 * Copyright 2019 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 *
 */

#ifndef _VIDEO_FONT_H
#define _VIDEO_FONT_H

#include "stdint.h"
/*******************************************************************************
 * Definitions
 *******************************************************************************/

typedef enum _font
{
    kFont_OpenSans8           = 1, // OpenSans8 font height=15 widthMax=10, AscII[0x20~0x7F] for Western
    kFont_OpenSans16          = 2, // OpenSans16 font height=28 widthMax=20, AscII[0x20~0x7F] for Western
    kFront_Cambo10            = 3,
    kFront_Cambo18            = 4,
    kFront_RaleWay10          = 5,
    kFront_RaleWay16          = 6,
    kFront_SourceHanSerif11   = 7, // SourceHanSerif11 font height=17 widthMax=15, AscII[0x20~0x7F] for Western
    kFront_SourceHanSerifSC11 = kFront_SourceHanSerif11 // SourceHanSerifSC11 font heightMax=17 widthMax=15,
                                                        // unicode16[0x4E00~0x9FB0] for Chinese
} font_t;

/*******************************************************************************
 * API
 *******************************************************************************/
int get_stringwidth(char *s, font_t type);
int get_fontheight(font_t type);
void put_string(int x, int y, char *s, int fontColor16b, int bgColor16b, font_t type, uint16_t *buf, int pitch);
void put_string_utf8(int x, int y, char *s, int fontColor16b, int bgColor16b, font_t type, uint16_t *buf, int pitch);
#endif /* _VIDEO_FONT_H */
