/*! *********************************************************************************
* \addtogroup BLESecurityManagerProtocol
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2020 NXP
* All rights reserved.
*
* \file
*
* This header file contains BLE Security Manager Protocol types and values defined
* by the Bluetooth specification
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

#ifndef SMP_TYPES_H
#define SMP_TYPES_H

#include "EmbeddedTypes.h"

#if (gBLE42_d == FALSE)
#define gSmpDefaultMtu_c        (23U)
#define gSmpMaxMtu_c            (23U)
#else  /* (gBLE42_d is FALSE) */
#define gSmpDefaultMtu_c        (65U)
#define gSmpMaxMtu_c            (65U)
#endif /* (gBLE42_d is FALSE) */

#define gSmpL2capChannel_c      (0x0006U)    /*!< L2CAP Channel reserved for the Security Manager Protocol */

#define gSmpTkSize_c            (16U)        /*!< SM Temporary Key size in octets */
#define gSmpStkSize_c           (16U)        /*!< SM Short Term Key size in octets */

#define gSmpLtkSize_c           (16U)        /*!< SM Long Term Key size in octets */
#define gSmpIrkSize_c           (16U)        /*!< SM Identity Resolving Key size in octets */
#define gSmpCsrkSize_c          (16U)        /*!< SM Connection Signature Resolving Key size in octets */

#define gSmpMinKeySize_c        (7U)         /*!< Minimum encryption key size accepted by the BLE SMP */
#define gSmpMaxKeySize_c        (16U)        /*!< Maximum encryption key size accepted by the BLE SMP */

#define gSmpConfirmValueSize_c  (16U)        /*!< Confirm value size - used during the SMP Pairing procedure.  */
#define gSmpPairingRandSize_c   (16U)        /*!< Random number size - used during the SMP Pairing procedure for the calculation of the SMP Pairing Confirm value by the Initiator and Responder. */

#define gSmpMaxPasskeyValue_c   (999999U)

#define gSmpLlEncryptionRandSize_c  (8U)     /*!< 8 byte random number used during the Link Layer Connection Encryption procedure */
#define gSmpLlEncryptionEdivSize_c  (2U)     /*!< 2 byte encryption diversified used during the Link Layer Connection Encryption procedure */
#define gSmpLlEncryptBlockSize_c    (16U)    /*!< AES block size used by the BLE Link Layer data encrypt command. */
#define gSmpLlRandSize_c            (8U)     /*!< Link Layer random number size - returned by LL random number generation procedure. */

#define gSmpAuthSignatureLength_c       (8U)

#if (gBLE42_d == TRUE)
#define gSmpEcdhPublicKeyCoordinateSize_c   (32U)    /*!< LE Secure Connections 256 bit X or Y coordinate of the ECC Public Key */
#define gSmpEcdhSecretKeySize_c             (32U)    /*!< LE Secure Connections 256 bit ECC Secret Key */
#define gSmpEcDHKeySize_c                   (32U)    /*!< LE Secure Connections 256 bit DHKey */

#define gSmpDhKeyCheckSize_c                (16U)    /*!< LE Secure Connections 128 bit Diffie Hellman key check */

#define gSmpMacKeySize_c                    (16U)    /*!< LE Secure Connections 128 bit MacKey */

#define gSmpLeScRandomValueSize_c           (16U)    /*!< LE Secure Connections 128 bit r value */
#define gSmpLeScRandomConfirmValueSize_c    (16U)    /*!< LE Secure Connections 128 bit Cr value */
#define gSmpLeScNonceSize_c                 (16U)    /*!< LE Secure Connections 128 bit N size */

#define gSmpLeSkPasskeyBitCheckRounds_c     (20U)    /*!< LE Secure Connections number of rounds of passkey bit checks.
                                                          There are k rounds for a k-bit Passkey -> k=20 for a 6-digit Passkey (999999=0xF423F). */
#endif /* (gBLE42_d is TRUE) */

/*! These are the values for the BLE SMP Pairing Failed - Reason field. */
#define gSmpPairingFailedReasonReserved_c                               (0x00U)
#define gSmpPairingFailedReasonPasskeyEntryFailed_c                     (0x01U)
#define gSmpPairingFailedReasonOobNotAvailable_c                        (0x02U)
#define gSmpPairingFailedReasonAuthenticationRequirements_c             (0x03U)
#define gSmpPairingFailedReasonConfirmValueFailed_c                     (0x04U)
#define gSmpPairingFailedReasonPairingNotSupported_c                    (0x05U)
#define gSmpPairingFailedReasonEncryptionKeySize_c                      (0x06U)
#define gSmpPairingFailedReasonCommandNotSupported_c                    (0x07U)
#define gSmpPairingFailedReasonUnspecifiedReason_c                      (0x08U)
#define gSmpPairingFailedReasonRepeatedAttempts_c                       (0x09U)
#define gSmpPairingFailedReasonInvalidParameters_c                      (0x0AU)
#if (gBLE42_d == TRUE)
#define gSmpPairingFailedReasonDhKeyCheckFailed_c                       (0x0BU)
#define gSmpPairingFailedReasonNumericComparisonFailed_c                (0x0CU)
#define gSmpPairingFailedReasonBrEdrPairingInProgress_c                 (0x0DU)
#define gSmpPairingFailedReasonCrossTransportKeyDerivGenNotAllowed_c    (0x0EU)
#endif /* (gBLE42_d == TRUE) */

#define gSmpTimeoutSeconds_c        (30U)

/*! This is the enumeration for the BLE SMP command codes defined by the specification. */
typedef enum
{
    gSmpNoOpcode_c                          = 0x00,
    /* Pairing Methods */
    gSmpOpcodePairingRequest_c              = 0x01,
    gSmpOpcodePairingResponse_c             = 0x02,
    gSmpOpcodePairingConfirm_c              = 0x03,
    gSmpOpcodePairingRandom_c               = 0x04,
    gSmpOpcodePairingFailed_c               = 0x05,
    /* Key Distribution */
    gSmpOpcodeEncryptionInformation_c       = 0x06,
    gSmpOpcodeMasterIdentification_c        = 0x07,
    gSmpOpcodeIdentityInformation_c         = 0x08,
    gSmpOpcodeIdentityAddressInformation_c  = 0x09,
    gSmpOpcodeSigningInformation_c          = 0x0A,
    /* Slave Security Request */
    gSmpOpcodeSecurityRequest_c             = 0x0B,
#if (gBLE42_d == TRUE)
    /* Pairing Methods LE Secure Connections */
    gSmpOpcodePairingPublicKey_c            = 0x0C,
    gSmpOpcodePairingDhKeyCheck_c           = 0x0D,
    /* Keypress Notification */
    gSmpOpcodeKeypressNotification_c        = 0x0E,
#endif /* (gBLE42_d == TRUE) */
} smpOpcode_t;


/* Group of constants for SMP packet lengths in bytes */
/* Pairing Methods */
#define gSmpPairingRequestPacketLength_c                (7U)  /*!< Code[1] + IoCap[1] + OOB[1] + AuthReq[1] + MaxEncKeyS[1] + IKeyDist[1] + RKeyDist[1] */
#define gSmpPairingResponsePacketLength_c               (7U)  /*!< Code[1] + IoCap[1] + OOB[1] + AuthReq[1] + MaxEncKeyS[1] + IKeyDist[1] + RKeyDist[1] */
#define gSmpPairingConfirmPacketLength_c                (17U) /*!< Code[1] + ConfirmValue[16] */
#define gSmpPairingRandomPacketLength_c                 (17U) /*!< Code[1] + RandomValue[16] */
#define gSmpPairingFailedPacketLength_c                 (2U)  /*!< Code[1] + Reason[1] */
/* Key Distribution */
#define gSmpEncryptionInformationPacketLength_c         (17U) /*!< Code[1] + LTK[16] */
#define gSmpMasterIdentificationPacketLength_c          (11U) /*!< Code[1] + EDIV[2] + Rand[8] */
#define gSmpIdentityInformationPacketLength_c           (17U) /*!< Code[1] + IRK[16] */
#define gSmpIdentityAddressInformationPacketLength_c    (8U)  /*!< Code[1] + AddrType[1] + BD_ADDR[6] */
#define gSmpSigningInformationPacketLength_c            (17U) /*!< Code[1] + CSRK[16] */
/* Slave Security Request */
#define gSmpSecurityRequestPacketLength_c               (2U)  /*!< Code[1] + AuthReq[1] */
#if (gBLE42_d == TRUE)
/* Pairing Methods LE Secure Connections */
#define gSmpPairingPublicKeyPacketLength_c              (65U) /*!< Code[1] + PublicKeyX[32] + PublicKeyY[32] */
#define gSmpPairingDhKeyCheckPacketLength_c             (17U) /*!< Code[1] + PairingDhKeyCheck[16] */
/* Keypress Notification */
#define gSmpKeypressNotificationPacketLength_c          (2U)  /*!< Code[1] + NotificationType[1] */
#endif /* (gBLE42_d == TRUE) */

/*! This is the enumeration for the BLE SMP Pairing Failed - Reason field. */
typedef uint8_t smpPairingFailedReason_t;

typedef uint8_t smpPairingIoCapability_t;
typedef enum
{
    gSmpPairingIoCapabDisplayOnly_c         = 0x00,
    gSmpPairingIoCapabDisplayyesNo_c        = 0x01,
    gSmpPairingIoCapabKeyboardOnly_c        = 0x02,
    gSmpPairingIoCapabNoInputNoOutput_c     = 0x03,
    gSmpPairingIoCapabKeyboardDisplay_c     = 0x04,
} smpPairingIoCapability_tag;

typedef enum
{
    gSmpPairingOobDataFlagAuthDataNotPresent_c      = 0x00,
    gSmpPairingOobDataFlagAuthDataPresent_c         = 0x01,
} smpPairingOobDataFlag_t;

/*! Authentication Request field, bonding flags subfield. */
typedef enum
{
    gSmpAuthReqBondingFlagsNoBonding_c      = 0x00,
    gSmpAuthReqBondingFlagsBonding_c        = 0x01,
    gSmpAuthReqBondingFlagsReserved10_c     = 0x02,
    gSmpAuthReqBondingFlagsReserved11_c     = 0x03,
} smpAuthReqBondingFlags_t;

/*! Authentication Request field, Man In The Middle Protection subfield. */
typedef enum
{
    gSmpAuthReqMitmReqOff_c         = 0x00,
    gSmpAuthReqMitmReqOn_c          = 0x01,
} smpAuthReqMitm_t;

#if (gBLE42_d == TRUE)
/*! Authentication Request field, SC subfield (LE Secure Connections). */
typedef enum
{
    gSmpAuthReqScReqOff_c           = 0x00,
    gSmpAuthReqScReqOn_c            = 0x01,
} smpAuthReqSc_t;
/*! Authentication Request field, Man In The Middle Protection flag subfield. */
typedef enum
{
    gSmpAuthReqKeypressNotificationsReqOff_c    = 0x00,
    gSmpAuthReqKeypressNotificationsReqOn_c     = 0x01,
} smpAuthReqKeypress_t;
#endif /* (gBLE42_d == TRUE) */

#if (gBLE42_d == TRUE)
/*! Notification Type field of the SMP Keypress Notification packet. */
typedef uint8_t smpKeypressNotificationType_t;
typedef enum
{
    gSmpKeypressNotificationPasskeyEntryStarted_c   = 0x00,
    gSmpKeypressNotificationPasskeyDigitEntered_c   = 0x01,
    gSmpKeypressNotificationPasskeyDigitErased_c    = 0x02,
    gSmpKeypressNotificationPasskeyCleared_c        = 0x03,
    gSmpKeypressNotificationPasskeyEntryCompleted_c = 0x04,
} smpKeypressNotificationType_tag;
#endif /* (gBLE42_d == TRUE) */

#endif /* SMP_TYPES_H */

/*! *********************************************************************************
* @}
********************************************************************************** */
