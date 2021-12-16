/*! *********************************************************************************
* \defgroup BLESecurityManagerProtocol BLE Security Manager Protocol
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2017 NXP
* All rights reserved.
*
* \file
*
* This header file contains BLE Security Manager Protocol packet structures
* defined by the Bluetooth specification.
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

#ifndef SMP_PACKETS_H
#define SMP_PACKETS_H

#include "smp_types.h"

typedef PACKED_STRUCT
{
    uint8_t         bondingFlags   :2; /*< \ref smpAuthReqBondingFlags_t */
    uint8_t         mitm           :1; /*< \ref smpAuthReqMitm_t */
#if (gBLE42_d == FALSE)
    uint8_t         reserved       :5;
#else  /* (gBLE42_d is FALSE) */
    uint8_t         sc             :1; /*< \ref smpAuthReqSc_t */
    uint8_t         keypress       :1; /*< \ref smpAuthReqKeypress_t */
    uint8_t         reserved       :3;
#endif /* (gBLE42_d == FALSE) */
} smpAuthReq_t;

#define smpAuthReq_Mask_c       0x07U

typedef PACKED_STRUCT
{
    uint8_t         encKey         :1;  /* LTK */
    uint8_t         idKey          :1;  /* IRK */
    uint8_t         sign           :1;  /* CSRK */
#if (gBLE42_d == FALSE)
    uint8_t         reserved       :5;
#else  /* (gBLE42_d is FALSE) */
    uint8_t         linkKey        :1;  /* LinkKey */
    uint8_t         reserved       :4;
#endif /* (gBLE42_d == FALSE) */
} smpKeyDistribution_t;

#define smpPairingKeyDistribution_Mask_c    0x07U


/******************************************************
** SMP Pairing Methods
******************************************************/

/*! Pairing Request : Code : 0x01 */
typedef PACKED_STRUCT
{
    uint8_t                     ioCapability;               /*!< \ref smpPairingIoCapability_t */
    uint8_t                     oobDataFlag;                /*!< \ref smpPairingOobDataFlag_t */
    smpAuthReq_t                authReq;                    /*!< Structure of size uint8_t */
    uint8_t                     maximumEncryptionkeySize;
    smpKeyDistribution_t        initiatorKeyDistribution;   /*!< Structure of size uint8_t */
    smpKeyDistribution_t        responderKeyDistribution;   /*!< Structure of size uint8_t */
} smpPairingRequestResponse_t;

/*! Pairing Response : Code : 0x02 */
/*! Same parameter structure as the SMP Pairing Request Packet */

/*! Pairing Confirm : Code : 0x03 */
typedef PACKED_STRUCT
{
    uint8_t         confirmValue[gSmpConfirmValueSize_c]; /*!< Mconfirm or Sconfirm */
} smpPairingConfirm_t;

/*! Pairing Random : Code : 0x04 */
typedef PACKED_STRUCT
{
    uint8_t         randomValue[gSmpConfirmValueSize_c]; /*!< Mrand or Srand */
} smpPairingRandom_t;

/*! Pairing Failed : Code : 0x05 */
typedef PACKED_STRUCT
{
    uint8_t         reason; /*!< \ref smpPairingFailedReason_t */
} smpPairingFailed_t;


/******************************************************
** SMP Key Distribution Commands
******************************************************/

/*! Encryption Information (Long Term Key) : Code : 0x06 */
typedef PACKED_STRUCT
{
    uint8_t         longTermKey[gSmpLtkSize_c];
} smpEncryptionInformation_t;

/*! Master Identification (EDIV and Rand) : Code : 0x07 */
typedef PACKED_STRUCT
{
    uint8_t         ediv[gSmpLlEncryptionEdivSize_c];
    uint8_t         rand[gSmpLlEncryptionRandSize_c];
} smpMasterIdentification_t;

/*! Identity Information (Identity Resolving Key) : Code : 0x08 */
typedef PACKED_STRUCT
{
    uint8_t         identityResolvingKey[gSmpIrkSize_c];
} smpIdentityInformation_t;

/*! Identity Address Information (BD_ADDR and it's type) : Code : 0x09 */
typedef PACKED_STRUCT
{
    uint8_t         addrType; /*!< \ref bleAddressType_t */
    uint8_t         bdAddr[6];
} smpIdentityAddressInformation_t;

/*! Signing Information (CSRK - Connection Signature Resolving Key) : Code : 0x0A */
typedef PACKED_STRUCT
{
    uint8_t         signatureKey[gSmpCsrkSize_c];
} smpSigningInformation_t;


/******************************************************
** SMP Slave Security Request
******************************************************/

/*! Security Request : Code : 0x0B */
typedef PACKED_STRUCT
{
    smpAuthReq_t    authReq;
} smpSecurityRequest_t;


/******************************************************
** SMP LE Secure Connections Commands
******************************************************/
#if (gBLE42_d == TRUE)
/*! Pairing Public Key : Code : 0x0C */
typedef PACKED_STRUCT
{
    uint8_t         publicKeyX[gSmpEcdhPublicKeyCoordinateSize_c];
    uint8_t         publicKeyY[gSmpEcdhPublicKeyCoordinateSize_c];
} smpPairingPublicKey_t;

/*! Pairing DHKey Check : Code : 0x0D */
typedef PACKED_STRUCT
{
    uint8_t         dhKeyCheck[gSmpEcdhPublicKeyCoordinateSize_c];
} smpPairingDhKeyCheck_t;
#endif /* (gBLE42_d == TRUE) */


/******************************************************
** SMP Slave Security Request
******************************************************/
#if (gBLE42_d == TRUE)
/*! Keypress Notification : Code : 0x0E */
typedef PACKED_STRUCT
{
    uint8_t         notificationType;   /*!< \ref smpKeypressNotificationType_t */
} smpKeypressNotification_t;
#endif /* (gBLE42_d == TRUE) */


/******************************************************
** SMP General Command Packet Structure
******************************************************/
typedef PACKED_STRUCT
{
    uint8_t            code; /*< \ref smpOpcode_t */
    PACKED_UNION
    {
        /* Pairing Methods */
        smpPairingRequestResponse_t         smpPairingRequest;
        smpPairingRequestResponse_t         smpPairingResponse;
        smpPairingConfirm_t                 smpPairingConfirm;
        smpPairingRandom_t                  smpPairingRandom;
        smpPairingFailed_t                  smpPairingFailed;
        /* Key Distribution */
        smpEncryptionInformation_t          smpEncryptionInformation;       /*!< LTK - Long Term Key */
        smpMasterIdentification_t           smpMasterIdentification;        /*!< EDIV and Rand */
        smpIdentityInformation_t            smpIdentityInformation;         /*!< IRK - Identity Resolving Key */
        smpIdentityAddressInformation_t     smpIdentityAddressInformation;  /*!< BD_ADDR and it's type */
        smpSigningInformation_t             smpSigningInformation;          /*!< CSRK - Connection Signature Resolvin Key */
        /* Slave Security Request */
        smpSecurityRequest_t                smpSecurityRequest;
#if (gBLE42_d == TRUE)
        /* Pairing Methods LE Secure Connections */
        smpPairingPublicKey_t               smpPairingPublicKey;
        smpPairingDhKeyCheck_t              smpPairingDhKeyCheck;
        /* Keypress Notification */
        smpKeypressNotification_t           smpKeypressNotification;
#endif /* (gBLE42_d == TRUE) */
    } data;
} smpPacket_t;

#endif /* SMP_PACKETS_H */

/*! *********************************************************************************
* @}
********************************************************************************** */
