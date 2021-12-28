/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "sln_device_utils.h"
#include "stdint.h"
#include "string.h"

uint32_t mergeParameters(char *destination, uint32_t destination_size, char **source, uint32_t parameterCount)
{
    uint32_t size = 0;
    if ((destination == NULL) || (destination_size == 0))
    {
        return 0;
    }

    for (int i = 0; i < parameterCount; i++)
    {
        size += strlen(source[i]);

        if ((i < (parameterCount - 1)))
        {
            /* add space */
            size++;
        }

        if (destination_size > size)
        {
            strcat(destination, source[i]);
            if ((i < (parameterCount - 1)))
            {
                /* add space */
                strcat(destination, " ");
            }
        }
        else
        {
            return 0;
        }
    }
    /* Change with real value */
    return size;
}


bool hasSpecialCharacters(const char str[])
{
    uint32_t length;

    if (str != NULL)
    {
        length = strlen(str);
        for (int i = 0; i < length; i++)
        {
            if ((!isdigit((int)str[i])) && (!isalpha((int)str[i])) && (str[i] != '-') && (str[i] != '_'))
                return true;
        }
    }
    return false;
}
