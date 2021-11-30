/*! *********************************************************************************
* \addtogroup BLESecurityManager
* @{
********************************************************************************** */
/*! *********************************************************************************
* \file
*
* This is the header file contains BLE Security Manager configuration parameters.
*
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2019 NXP
*
* NXP Confidential Proprietary
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from NXP.
********************************************************************************** */

#ifndef SM_CONFIG_H
#define SM_CONFIG_H

#ifndef gSmpInitiatorSupported_d
    #define gSmpInitiatorSupported_d 1U
#endif

#ifndef gSmpResponderSupported_d
    #define gSmpResponderSupported_d 1U
#endif

#if !defined(gSmpInitiatorSupported_d) || !defined(gSmpResponderSupported_d)
    #error "The SM configuration is not completely defined. Please configure the SM Initiator and Responder support (enabled or disabled)."
#endif

#ifndef gSmTestHarnessEnabled_d
    #define gSmTestHarnessEnabled_d  0U
#endif

#if (gSmTestHarnessEnabled_d == 1U)
#pragma message("SM TEST HARNESS FUNCTIONALITY ENABLED! SM MAY BEHAVE IN AN UNEXPECTED WAY!")
#endif

#endif /* SM_CONFIG_H */

/*! *********************************************************************************
* @}
********************************************************************************** */
