/*
 * Copyright 2019 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 *
 */

#include <ui_resources/fonts/cambo.h>
#include <ui_resources/fonts/font.h>
#include <ui_resources/fonts/open_sans.h>
#include <ui_resources/fonts/raleway.h>
#include <ui_resources/fonts/source_han_serif.h>
#include "stdint.h"
#include "fwk_log.h"
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define PAGE_SIZE_DESC 256
#define UNICODE_FIRST  (0x4E00)
#define UNICODE_LAST   (0X9FB0)

/*******************************************************************************
 * Variables
 *******************************************************************************/
static D4D_FONT_DESCRIPTOR s_d4dFontDesc;
static D4D_FONT_DESCRIPTOR s_d4dFontDescChinese;

/*******************************************************************************
 * Code
 *******************************************************************************/

static void set_d4dfnt_desc(font_t type)
{
    if (type == kFont_OpenSans8)
    {
        s_d4dFontDesc = d4dfnt_OpenSans8_desc;
    }
    else if (type == kFont_OpenSans16)
    {
        s_d4dFontDesc = d4dfnt_OpenSans16_desc;
    }
    else if (type == kFront_Cambo10)
    {
        s_d4dFontDesc = d4dfnt_Cambo10_desc;
    }
    else if (type == kFront_Cambo18)
    {
        s_d4dFontDesc = d4dfnt_Cambo18_desc;
    }
    else if (type == kFront_RaleWay10)
    {
        s_d4dFontDesc = d4dfnt_Raleway10_desc;
    }
    else if (type == kFront_RaleWay16)
    {
        s_d4dFontDesc = d4dfnt_Raleway16_desc;
    }
    else if (type == kFront_SourceHanSerif11)
    {
        s_d4dFontDesc = d4dfnt_SourceHanSerif11_desc;
    }
}

static void put_char(int x, int y, int c, int fontColor16b, int bgColor16b, uint16_t *buf, int pitch)
{
    int i, j, k, bits, width, height, offset;
    k      = 0;
    height = s_d4dFontDesc.charFullSize.height;
    width  = s_d4dFontDesc.pSizeTable[c - s_d4dFontDesc.startChar];
    offset = s_d4dFontDesc.pOffTable[c - s_d4dFontDesc.startChar];
    bits   = s_d4dFontDesc.pFontData[offset++];

    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++, bits <<= 1, k++)
        {
            if (k == 8 * sizeof(char))
            {
                bits = s_d4dFontDesc.pFontData[offset++];
                k    = 0;
            }

            if (bits & 0x80)
            {
                *(buf + (y + i) * pitch + x + j) = fontColor16b;
            }
            else if (bgColor16b != -1)
            {
                *(buf + (y + i) * pitch + x + j) = bgColor16b;
            }
        }
    }
}

void put_string(int x, int y, char *s, int fontColor16b, int bgColor16b, font_t type, uint16_t *buf, int pitch)
{
    int width;
    set_d4dfnt_desc(type);

    for (; *s; s++)
    {
        put_char(x, y, *s, fontColor16b, bgColor16b, buf, pitch);
        width = s_d4dFontDesc.pSizeTable[(int)*s - s_d4dFontDesc.startChar];
        x += width;
    }
}

int get_stringwidth(char *s, font_t type)
{
    int x = 0;
    set_d4dfnt_desc(type);

    for (; *s; s++)
    {
        x += s_d4dFontDesc.pSizeTable[(int)*s - s_d4dFontDesc.startChar];
    }

    return x;
}

int get_fontheight(font_t type)
{
    set_d4dfnt_desc(type);

    return s_d4dFontDesc.charFullSize.height;
}

static void set_d4dfnt_desc_chinese(font_t type, uint16_t unicode)
{
    D4D_FONT_DESCRIPTOR *pDescNow, *pDescNext;
    uint32_t whichPage = 0;
    uint32_t i         = 0;

    whichPage = (unicode - UNICODE_FIRST) / PAGE_SIZE_DESC;

    if (type == kFront_SourceHanSerifSC11)
    {
        pDescNow = (D4D_FONT_DESCRIPTOR *)&d4dfnt_SourceHanSerifSC11_desc;
    }
    else
    {
        LOGE("Error,unsupported font!");
    }

    for (i = 0; i < whichPage; i++)
    {
        pDescNext = (D4D_FONT_DESCRIPTOR *)pDescNow->pNext;
        pDescNow  = pDescNext;
    }
    s_d4dFontDescChinese = *pDescNow;
}

static void put_char_chinese(int x, int y, uint16_t c, int fontColor16b, int bgColor16b, uint16_t *buf, int pitch)
{
    int i, j, k, bits, width, height, offset, start;
    k      = 0;
    height = s_d4dFontDescChinese.charFullSize.height;
    width  = s_d4dFontDescChinese.charFullSize.width;
    start  = s_d4dFontDescChinese.startChar;
    offset = (s_d4dFontDescChinese.charBmpSize) * ((c - start) % PAGE_SIZE_DESC);
    bits   = s_d4dFontDescChinese.pFontData[offset++];

    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++, bits <<= 1, k++)
        {
            if (k == 8 * sizeof(char))
            {
                bits = s_d4dFontDescChinese.pFontData[offset++];
                k    = 0;
            }
            if (bits & 0x80)
            {
                *(buf + (y + i) * pitch + x + j) = fontColor16b;
            }
            else if (bgColor16b != -1)
            {
                *(buf + (y + i) * pitch + x + j) = bgColor16b;
            }
        }
    }
}

void put_string_chinese(
    int x, int y, uint16_t *s, int fontColor16b, int bgColor16b, font_t type, uint16_t *buf, int pitch)
{
    int width;
    for (; *s; s++)
    {
        set_d4dfnt_desc_chinese(type, *s);
        put_char_chinese(x, y, *s, fontColor16b, bgColor16b, buf, pitch);
        width = s_d4dFontDescChinese.charFullSize.width;
        x += width;
    }
}

void put_string_utf8(int x, int y, char *s, int fontColor16b, int bgColor16b, font_t type, uint16_t *buf, int pitch)
{
    int width;
    int codeLen = 0;
    while (*s)
    {
        if (0 == (s[0] & 0x80))
        {
            // One byte for AscII font
            codeLen = 1;
            set_d4dfnt_desc(type);
            put_char(x, y, *s, fontColor16b, bgColor16b, buf, pitch);
            width = s_d4dFontDesc.pSizeTable[(int)*s - s_d4dFontDesc.startChar];

            x += width;
            s += codeLen;
        }
#if ENABLE_CHINESE_FONT_DISPLAY
        else if (0xE0 == (s[0] & 0xF0) && 0x80 == (s[1] & 0xC0) && 0x80 == (s[2] & 0xC0))
        {
            // Three bytes for Chinese font
            codeLen          = 3;
            uint16_t unicode = (uint16_t)((((uint16_t)s[0] & 0x000F) << 12) | (((uint16_t)s[1] & 0x003F) << 6) |
                                          ((uint16_t)s[2] & 0x003F));
            set_d4dfnt_desc_chinese(type, unicode);
            put_char_chinese(x, y, unicode, fontColor16b, bgColor16b, buf, pitch);
            width = s_d4dFontDescChinese.charFullSize.width;
            x += width;
            s += codeLen;
        }
#endif
    }
}
