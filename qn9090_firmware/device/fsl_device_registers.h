/*
 * Copyright 2014-2016 Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __FSL_DEVICE_REGISTERS_H__
#define __FSL_DEVICE_REGISTERS_H__

/*
 * Include the cpu specific register header files.
 *
 * The CPU macro should be declared in the project or makefile.
 */
#if (defined(CPU_QN9030HN) || defined(CPU_QN9030THN) || defined(CPU_QN9090HN) || \
    defined(CPU_QN9090THN) || defined(CPU_JN518X))

#define JN518X_SERIES

#define QN9090_SERIES

/* CMSIS-style register definitions */
#include "QN9090.h"
/* CPU specific feature definitions */
#include "QN9090_features.h"

#else
    #error "No valid CPU defined!"
#endif

#endif /* __FSL_DEVICE_REGISTERS_H__ */

/*******************************************************************************
 * EOF
 ******************************************************************************/
