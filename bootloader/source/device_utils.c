/*
 * Copyright 2019-2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

#include <FreeRTOS.h>

#include "mbedtls/base64.h"
#include "board.h"
#include "device_utils.h"

/*!
 * @brief Converts an uint64 number to a hex string.
 *
 * @param u64UniqueIDRaw    64bits integer to be converted to hex string
 * @param *uniqueID         Pointer to pre-allocated buffer for the converted string
 * @param uniqueIdBufferLen Length of the pre-allocated buffer
 *
 */
static void UniqueID_ToHexString(uint64_t u64UniqueIDRaw, char *uniqueID, uint32_t uniqueIdBufferLen)
{
    const char hex_chars[] = "0123456789abcdef";
    uint32_t index         = uniqueIdBufferLen - 1;

    /* Make sure the buffer len is big enough to hold the ID */
    if (u64UniqueIDRaw >> (4 * index))
    {
        return;
    }

    /* Add string terminator */
    uniqueID[index] = '\0';

    while (u64UniqueIDRaw || index)
    {
        index--;
        uniqueID[index] = hex_chars[u64UniqueIDRaw & 0xF];
        u64UniqueIDRaw >>= 4;
    }
}

__attribute__((weak)) void APP_GetHexUniqueID(char **uniqueID)
{
    uint64_t u64UniqueIDRaw = 0;
    uint32_t bufferLen      = 0;

/* Generate unique identifier for the device. */
#if (defined(MIMXRT106A_SERIES) || defined(MIMXRT106L_SERIES) || defined(MIMXRT106F_SERIES)) || \
    defined(MIMXRT106S_SERIES)
    /* Get the unique ID from the registers */
    u64UniqueIDRaw = (uint64_t)((uint64_t)OCOTP->CFG1 << 32ULL) | OCOTP->CFG0;
#elif (defined(MIMXRT1176_cm7_SERIES) || defined(MIMXRT117F_cm7_SERIES) || defined(MIMXRT117H_cm7_SERIES))
    /* OCOTP Must be initialized first before calling this function */
    u64UniqueIDRaw = (uint64_t)((uint64_t)OCOTP_ReadFuseShadowRegister(OCOTP, 0x900) << 32ULL) |
                     OCOTP_ReadFuseShadowRegister(OCOTP, 0x910);
#else
#error "UNSUPPORTED PLATFORM"
#endif
    /* Leave space for string terminator */
    bufferLen = INT64_UNIQUE_ID_HEX_STRING_LEN + 1;

    *uniqueID = (char *)pvPortMalloc(bufferLen);
    if (*uniqueID == NULL)
    {
        return;
    }

    UniqueID_ToHexString(u64UniqueIDRaw, *uniqueID, bufferLen);
}

__attribute__((weak)) void APP_GetUniqueID(char **uniqueID, bool cloudSafe)
{
    uint32_t outputLen      = 0;
    uint32_t cIdLen         = 0;
    uint64_t u64UniqueIDRaw = 0;

    if (uniqueID == NULL)
    {
        return;
    }

/* Generate unique identifier for the device. */
#if (defined(MIMXRT106A_SERIES) || defined(MIMXRT106L_SERIES) || defined(MIMXRT106F_SERIES) || \
     defined(MIMXRT106S_SERIES))
    /* Get the unique ID from the registers */
    u64UniqueIDRaw = (uint64_t)((uint64_t)OCOTP->CFG1 << 32ULL) | OCOTP->CFG0;
#elif (defined(MIMXRT1176_cm7_SERIES) || defined(MIMXRT117F_cm7_SERIES) || defined(MIMXRT117H_cm7_SERIES))
    /* OCOTP Must be initialized first before calling this function */
    u64UniqueIDRaw = (uint64_t)((uint64_t)OCOTP_ReadFuseShadowRegister(OCOTP, 0x900) << 32ULL) |
                     OCOTP_ReadFuseShadowRegister(OCOTP, 0x910);
#else
#error "UNSUPPORTED PLATFORM"
#endif
    mbedtls_base64_encode(NULL, 0, (size_t *)&cIdLen, (const unsigned char *)&u64UniqueIDRaw, sizeof(uint64_t));

    *uniqueID = (char *)pvPortMalloc(cIdLen + 1);
    if (*uniqueID == NULL)
    {
        return;
    }

    memset(*uniqueID, 0, cIdLen + 1);
    mbedtls_base64_encode((unsigned char *)*uniqueID, cIdLen, (size_t *)&outputLen,
                          (const unsigned char *)&u64UniqueIDRaw, sizeof(uint64_t));

    /* Encode for cloud safe usage, satisfying [a-zA-Z0-9:_-]+ */
    if (cloudSafe)
    {
        for (uint32_t idx = 0; idx < outputLen; idx++)
        {
            switch ((*uniqueID)[idx])
            {
                case '+':
                    (*uniqueID)[idx] = '_';
                    break;
                case '/':
                    (*uniqueID)[idx] = '-';
                    break;
                case '=':
                    (*uniqueID)[idx] = '\0';
                    break;
                default:
                    /* Any other base64 encoded character should satisfy [a-zA-Z0-9:_-]+ */
                    break;
            }
        }
    }
}

__attribute__((weak)) void PKCS11_KeyGen(char **keyPhrase)
{
    /* Enter unique pass phrase generation routine here. */
}