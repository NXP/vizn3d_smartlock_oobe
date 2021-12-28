/**
 * Copyright 2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 *
 */

/**
 * Copyright 2016 Freescale Semiconductors, Inc.
 */

package com.smartlockmanager.event;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import androidx.annotation.NonNull;

import com.smartlockmanager.database.User;

import java.util.ArrayList;
import java.util.Arrays;

public class BLEStateEvent {

    /**
     * This event will be fired in global-level to inform if Bluetooth state has been changed.
     */
    public static class BluetoothStateChanged extends BaseEvent {

        public final int newState;

        public BluetoothStateChanged(int newState) {
            super();
            this.newState = newState;
        }
    }

    public static class BluetoothClientStateChanged extends BaseEvent {

        public final int newState;

        public BluetoothClientStateChanged(int newState) {
            super();
            this.newState = newState;
        }
    }
    /**
     * This event will be fired whenever a BLE device is connected.
     */
    public static class Connected extends BaseEvent {
        public int bondState;
    }

    /**
     * This event will be fired whenever phone is trying to connect to a BLE device.
     */
    public static class Connecting extends BaseEvent {
    }

    /**
     * This event will be fired whenever a current connection to BLE device has been lost.
     */
    public static class Disconnected extends BaseEvent {
    }

    /**
     * This event will be fired whenever all services of a BLE device have been discovered.
     */
    public static class ServiceDiscovered extends BaseEvent {
        public int bondState;
    }

    /**
     * This event will be fired whenever a piece of data is available through a {@link BluetoothGattCharacteristic}.
     */
    public static class DataAvailable extends BaseEvent {

        public final BluetoothGattCharacteristic characteristic;
        public boolean isNotify = false;

        public DataAvailable(@NonNull BluetoothGattCharacteristic characteristic) {
            super();
            this.characteristic = characteristic;
        }
    }

    /*
     * AUTHENTICATION_RES
     */
    public static class AuthenticationRes extends BaseEvent {
        public int mAuthenticationResult = 1;

        public AuthenticationRes(int authenticationResult) {
            super();
            this.mAuthenticationResult = authenticationResult;
        }
    }

    /*
     * UPDATE_PASSWORD_RES
     */
    public static class UpdatePasswordRes extends BaseEvent {
        public int mUpdatePasswordResult = 1;

        public UpdatePasswordRes(int updatePasswordResult) {
            super();
            this.mUpdatePasswordResult = updatePasswordResult;
        }
    }

    /*
     * REGISTRATION_RES
     */
    public static class RegistrationRes extends BaseEvent {
        public int mRegistrationResult = 1;
        public String mRegistrationDuplicateName = null;

        public RegistrationRes(int registrationResult, byte[]registrationData) {
            super();
            this.mRegistrationResult = registrationResult;
            this.mRegistrationDuplicateName = new String(registrationData);
        }
    }

    /*
     * DELETE_USER_RES
     */
    public static class DeleteUserRes extends BaseEvent {
        public int mDeleteUserResult = 1;

        public DeleteUserRes(int deleteUserResult) {
            super();
            this.mDeleteUserResult = deleteUserResult;
        }
    }

    /*
     * GET_USER_COUNT_RES
     */
    public static class GetUserCountRes extends BaseEvent {
        public int mGetUserCountResult = 1;

        public GetUserCountRes(int getUserCountResult) {
            super();
            this.mGetUserCountResult = getUserCountResult;
        }
    }

    /*
     * GET_USER_INFO_RES
     */
    public static class GetUserInfoRes extends BaseEvent {

        private final int USER_ID_SIZE = 4;
        private final int USER_STRUCT_SIZE = 40;
        private final int USER_NAME_SIZE = 32;

        public int mGetUserInfoResult = 1;
        public ArrayList<User> mGetUserInfoData = null;

        private void parseUserData(int userCount, byte[] userData){
            mGetUserInfoData = new ArrayList<>();

            for (int i = 0; i<(userCount*USER_STRUCT_SIZE); i+=USER_STRUCT_SIZE){

                int id = (int) (((userData[i + 3] & 0xff) << 24) |
                                ((userData[i + 2] & 0xff) << 16) |
                                ((userData[i + 1] & 0xff) << 8)  |
                                (userData[i] & 0xff));

                String name = new String(
                        Arrays.copyOfRange(userData, i + USER_ID_SIZE, i + (USER_ID_SIZE + USER_NAME_SIZE))).trim();

                boolean is_saved = userData[i + (USER_ID_SIZE + USER_NAME_SIZE)] == 1;

                User user = new User(id, name, null);
                mGetUserInfoData.add(user);
            }
        }

        public GetUserInfoRes(int getUserInfoResult, byte[] getUserInfoData) {
            super();
            this.mGetUserInfoResult = getUserInfoResult;
            if (mGetUserInfoResult >= 0)
                parseUserData(getUserInfoResult, getUserInfoData);
        }
    }

    /*
     * UPDATE_USER_INFO_RES
     */
    public static class UpdateUserInfoRes extends BaseEvent {
        public int mUpdateUserInfoResult = 1;

        public UpdateUserInfoRes(int updateUserInfoResult) {
            super();
            this.mUpdateUserInfoResult = updateUserInfoResult;
        }
    }

    /*
     * REGISTRATION_CMD_RES
     */
    public static class RegistrationCMDRes extends BaseEvent {
        public int mRegistrationCMDResult = 1;

        public RegistrationCMDRes(int registrationCMDResult) {
            super();
            this.mRegistrationCMDResult = registrationCMDResult;
        }
    }

    /*
     * DEREGISTRATION_CMD_RES
     */
    public static class DeregistrationCMDRes extends BaseEvent {
        public int mDeregistrationCMDResult = 1;

        public DeregistrationCMDRes(int deregistrationCMDResult) {
            super();
            this.mDeregistrationCMDResult = deregistrationCMDResult;
        }
    }

    /*
     * PREV_CAMERA_SWITCH_CMD_RES
     */
    public static class PrevCameraSwitchCMDRes extends BaseEvent {
        public int mPrevCameraSwitchCMDResult = 1;

        public PrevCameraSwitchCMDRes(int prevCameraSwitchCMDResult) {
            super();
            this.mPrevCameraSwitchCMDResult = prevCameraSwitchCMDResult;
        }
    }

    /*
     * GET_APP_TYPE_RES
     */
    public static class GetAppTypeRes extends BaseEvent {
        public int mGetAppTypeResult = 1;

        public GetAppTypeRes(int getAppTypeResult) {
            super();
            this.mGetAppTypeResult = getAppTypeResult;
        }
    }

    /*
     * GET_ALGO_VERSION_RES
     */
    public static class GetAlgoVersionRes extends BaseEvent {
        public int mGetAlgoVersionResult = 1;

        public GetAlgoVersionRes(int getAlgoVersionResult) {
            super();
            this.mGetAlgoVersionResult = getAlgoVersionResult;
        }
    }

    /**
     * This event will be fired whenever a piece of data is available through a {@link BluetoothGattCharacteristic}.
            */
    public static class DataAvailableFRMD extends BaseEvent {

        public final BluetoothGatt characteristic;

        public DataAvailableFRMD(@NonNull BluetoothGatt characteristic) {
            super();
            this.characteristic = characteristic;
        }
    }


    public static class DataWritenFromClient extends BaseEvent {

        public final BluetoothDevice device;
        public final int requestId;
        public final BluetoothGattCharacteristic characteristic;
        public final boolean preparedWrite;
        public final boolean responseNeeded;
        public final int offset;
        public final byte[] value;

        public DataWritenFromClient(BluetoothDevice device, int requestId,
                                    BluetoothGattCharacteristic characteristic, boolean preparedWrite,
                                    boolean responseNeeded, int offset, byte[] value) {
            this.device = device;
            this.requestId = requestId;
            this.characteristic = characteristic;
            this.preparedWrite = preparedWrite;
            this.responseNeeded = responseNeeded;
            this.offset = offset;
            this.value = value;
        }
    }

    /**
     * Global-level event, fired whenever bonding state of a bluetooth device is changed.
     */
    public static class DeviceBondStateChanged extends BaseEvent {

        public final BluetoothDevice device;
        public final int bondState;

        public DeviceBondStateChanged(BluetoothDevice device, int bondState) {
            super();
            this.device = device;
            this.bondState = bondState;
        }
    }

    public static class DeviceRssiUpdated extends BaseEvent {

        public final String device;
        public final int rssi;

        public DeviceRssiUpdated(int rssi, String device) {
            super();
            this.rssi = rssi;
            this.device = device;
        }
    }

    public static class MTUUpdated extends BaseEvent {

        public final String device;
        public final int mtuSize;
        public final boolean success;

        public MTUUpdated(String device, int mtuSize, boolean success) {
            super();
            this.device = device;
            this.mtuSize = mtuSize;
            this.success = success;
        }
    }

    public static class PHYUpdated extends BaseEvent {

        public final int txPhy;
        public final int rxPhy;
        public final int status;

        public PHYUpdated(int tx, int rx, int sts) {
            super();
            this.txPhy = tx;
            this.rxPhy = rx;
            this.status = sts;
        }
    }

    public static  class PHYReaded extends BaseEvent{
        public  final  int txPhy;
        public final  int rxPhy;
        public PHYReaded(int tx,int rx){
            this.txPhy=tx;
            this.rxPhy=rx;
        }
    }
}
