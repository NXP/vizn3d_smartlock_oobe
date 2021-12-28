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
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.smartlockmanager.service;

import android.annotation.TargetApi;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattServer;
import android.bluetooth.BluetoothGattServerCallback;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.text.TextUtils;
import android.util.Log;

import androidx.annotation.NonNull;

import com.smartlockmanager.AppConfig;
import com.smartlockmanager.event.BLEStateEvent;
import com.smartlockmanager.model.BLEAttributes;
import com.smartlockmanager.model.Task;
import com.smartlockmanager.utility.BLEConverter;
import com.smartlockmanager.utility.SdkUtils;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.UUID;
import java.util.zip.CRC32;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.jetbrains.annotations.NotNull;

public enum BLEService {

    /**
     * Please refer to http://stackoverflow.com/questions/70689.
     */
    INSTANCE;

    /**
     * Tag for the {@link Log}.
     */
    private static final String TAG = "SLM_BLS";

    /*
     * transmission protocol
     */
    public final byte AUTHENTICATION_REQ = 0;
    public final byte AUTHENTICATION_RES = 1;
    public final byte UPDATE_PASSWORD_REQ = 2;
    public final byte UPDATE_PASSWORD_RES = 3;
    public final byte REGISTRATION_REQ = 4;
    public final byte REGISTRATION_RES = 5;
    public final byte DELETE_USER_REQ = 6;
    public final byte DELETE_USER_RES = 7;
    public final byte GET_USER_COUNT_REQ = 8;
    public final byte GET_USER_COUNT_RES = 9;
    public final byte GET_USER_INFO_REQ = 10;
    public final byte GET_USER_INFO_RES = 11;
    public final byte UPDATE_USER_INFO_REQ = 12;
    public final byte UPDATE_USER_INFO_RES = 13;
    public final byte REGISTRATION_CMD_REQ = 14;
    public final byte REGISTRATION_CMD_RES = 15;
    public final byte DEREGISTRATION_CMD_REQ = 16;
    public final byte DEREGISTRATION_CMD_RES = 17;
    public final byte PREV_CAMERA_SWITCH_CMD_REQ = 18;
    public final byte PREV_CAMERA_SWITCH_CMD_RES = 19;
    public final byte GET_APP_TYPE_REQ = 20;
    public final byte GET_APP_TYPE_RES = 21;
    public final byte GET_ALGO_VERSION_REQ = 22;
    public final byte GET_ALGO_VERSION_RES = 23;

    public final byte INVALID_PACKET = -2;
    public static final int REGISTRATION_RESULT_DUPLICATE = 1;

    private final int packetLength[] = {
            6,      // AUTHENTICATION_REQ_PKT_LEN
            0,      // AUTHENTICATION_RES_PKT_LEN
            6,      // UPDATE_PASSWORD_REQ_PKT_LEN
            0,      // UPDATE_PASSWORD_RES_PKT_LEN
            432,    // REGISTRATION_REQ_PKT_LEN
            0,      // REGISTRATION_RES_PKT_LEN
            4,      // DELETE_USER_REQ_PKT_LEN
            0,      // DELETE_USER_RES_PKT_LEN
            0,      // GET_USER_COUNT_REQ_PKT_LEN
            0,      // GET_USER_COUNT_RES_PKT_LEN
            0,      // GET_USER_INFO_REQ_PKT_LEN
            0,     // GET_USER_INFO_RES_PKT_LEN
            36,     // UPDATE_USER_INFO_REQ_PKT_LEN
            0,      // UPDATE_USER_INFO_RES_PKT_LEN
            0,      // REGISTRATION_CMD_REQ_PKT_LEN
            0,      // REGISTRATION_CMD_RES_PKT_LEN
            0,      // DE-REGISTRATION_CMD_REQ_PKT_LEN
            0,      // DE-REGISTRATION_CMD_RES_PKT_LEN
            0,      // PREV_CAMERA_SWITCH_CMD_REQ_PKT_LEN
            0,      // PREV_CAMERA_SWITCH_CMD_RES_PKT_LEN
            0,      // GET_APP_TYPE_REQ
            0,      // GET_APP_TYPE_RES
            0,      // GET_ALGO_VERSION_REQ
            0,      // GET_ALGO_VERSION_RES
    };

    private static final byte PACKET_HEADER_LEN = 24;

    private int mPacketID = 0;
    private byte mReqPacketType;
    private int mReqPacketID;
    private byte mResPacketType;
    private int headerReserved = 0;

    private int packetDataLength = 0;
    private ByteArrayOutputStream packetData;
    private int packetReserved = 0;

    /**
     * States of BLE connection.
     */
    public interface State {

        int STATE_DISCONNECTED = 0;
        int STATE_CONNECTING = 1;
        int STATE_CONNECTED = 2;
    }

    /**
     * Request types which can be made to BLE devices.
     */
    public interface Request {

        int READ = 0;
        int WRITE = 1;
        int NOTIFY = 2;
        int INDICATE = 3;
        int WRITE_NO_RESPONSE = 4;
        int WRITE_WITH_AUTHEN = 5;
        int DISABLE_NOTIFY_INDICATE = 6;
    }

    private BluetoothManager mBluetoothManager; // centralized BluetoothManager
    private BluetoothAdapter mBluetoothAdapter; // corresponding Adapter
    private BluetoothGatt mBluetoothGatt; // GATT connection
    private BluetoothGattServer mBluetoothGattServer; // for UART, we need a server callback

    /**
     * Always use main thread to perform BLE operation to avoid issue with many Samsung devices.
     */
    private final Handler mMainLoop = new Handler(Looper.getMainLooper());

    /**
     * All awaiting operations need a timeout.
     */
    private final Runnable mTimeOutRunnable = new Runnable() {

        @Override
        public void run() {
            Log.v("ota","mTimeOutRunnable");
            continueTaskExecution(false);
        }
    };

    /**
     * Also, service discovery need a timeout too.
     */
    private final Runnable mDiscoveryTimeOut = new Runnable() {

        @Override
        public void run() {
            disconnect();
        }
    };

    /**
     * We hold reference to application-level context.
     */
    private Context mContext;

    /**
     * All registered tasks are stored and release when disconnect.
     */
    private List<Task> mTaskList;

    /**
     * Internal variable to track connection state.
     */
    private int mConnectionState;

    /**
     * Sometimes, connect to a BLE device need a few tries :(
     */
    private int tryTime;

    /**
     * Internal MAC address of BLE device.
     */
    private String address;

    /**
     * We need to create a Gatt Server with UART support (service and characteristic).
     */
    private boolean needUartSupport;


    private boolean isThroughput;

    private int mCurrentPhy;

    /**
     * This callback should be used whenever a GATT event is fired.
     * It will, in turn, broadcast EventBus events to other application components.
     */
    private final BluetoothGattCallback mGattCallback = new BluetoothGattCallback() {

        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            Log.e(TAG, "onConnectionStateChange:device state" + gatt.toString()+newState);
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                isThroughput = false;
                mConnectionState = State.STATE_CONNECTED;
                BLEStateEvent.Connected connectedEvent = new BLEStateEvent.Connected();
                connectedEvent.bondState = gatt.getDevice().getBondState();
                EventBus.getDefault().post(new BLEStateEvent.Connected());
                // after connected, ask executors to dicover BLE services
                registerTask(new Task() {

                    @Override
                    public void run() {
                        mMainLoop.removeCallbacks(mDiscoveryTimeOut);
                        if (mBluetoothGatt != null) {
                            mBluetoothGatt.discoverServices();
                            mMainLoop.postDelayed(mDiscoveryTimeOut, AppConfig.DEFAULT_REQUEST_TIMEOUT);
                        }
                    }
                });
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                mServiceDiscovered = null;
                if (status == 133 && !TextUtils.isEmpty(address) && tryTime < 10) {
                    mMainLoop.postDelayed(new Runnable() {

                        @Override
                        public void run() {
                            connect(address, needUartSupport,mCurrentPhy);
                        }
                    }, 1000);
                } else {
                    disconnect();
                    bluetoothReleaseResource();
                    EventBus.getDefault().post(new BLEStateEvent.Disconnected());
                }
            }
        }

        BLEStateEvent.ServiceDiscovered mServiceDiscovered;
        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                Log.e(TAG, "onServicesDiscovered:dicovery all service" + "SUCCESS");

                if (BLEService.this.needUartSupport == false) {
                    request(BLEAttributes.WUART, BLEAttributes.UART_NOTIFY, Request.NOTIFY);
                }

                mMainLoop.removeCallbacks(mDiscoveryTimeOut);
                if (mServiceDiscovered ==null){
                    mServiceDiscovered = new BLEStateEvent.ServiceDiscovered();
                    mServiceDiscovered.bondState = gatt.getDevice().getBondState();
                    EventBus.getDefault().post(mServiceDiscovered);
                }
                // post to get all FRDM's services
                //EventBus.getDefault().post(gatt.getServices());
                EventBus.getDefault().post(new BLEStateEvent.DataAvailableFRMD(gatt));
                continueTaskExecution(false);
            } else if (status == BluetoothGatt.GATT_INSUFFICIENT_AUTHENTICATION || status == BluetoothGatt.GATT_INSUFFICIENT_ENCRYPTION) {
                changeDevicePairing(gatt.getDevice(), true);
                Log.e(TAG, "onServicesDiscovered:dicovery all service" +"discovery failed and remove device");

            } else {
                Log.e(TAG, ":dicovery all service" +"discovery failed and remove device disconnect");

                disconnect();
            }
        }

        @Override
        public void onCharacteristicRead(BluetoothGatt gatt,
                                         BluetoothGattCharacteristic characteristic,
                                         int status) {
            Log.e(TAG, "onCharacteristicRead");
            if (status == BluetoothGatt.GATT_SUCCESS) {
                EventBus.getDefault().post(new BLEStateEvent.DataAvailable(characteristic));
                continueTaskExecution(false);
            } else if (status == BluetoothGatt.GATT_INSUFFICIENT_AUTHENTICATION || status == BluetoothGatt.GATT_INSUFFICIENT_ENCRYPTION) {
                changeDevicePairing(gatt.getDevice(), true);
            } else {
                disconnect();
            }
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt,
                                            BluetoothGattCharacteristic characteristic) {
            Log.d(TAG, "+onCharacteristicChanged");
        }

        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            Log.d(TAG, "onCharacteristicWrite");
            if (status == BluetoothGatt.GATT_SUCCESS) {
                if (isThroughput) {
                    Log.d(TAG, "  writeCharacteristic");
                    gatt.writeCharacteristic(characteristic);
                } else {
                    BLEStateEvent.DataAvailable e =new BLEStateEvent.DataAvailable(characteristic);
                    EventBus.getDefault().post(e);
                    continueTaskExecution(false);
                }

            }
        }

        @Override
        public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
            Log.d(TAG, "onDescriptorWrite");
            if (status == BluetoothGatt.GATT_SUCCESS) {
                continueTaskExecution(false);
            } else if (status == BluetoothGatt.GATT_INSUFFICIENT_AUTHENTICATION || status == BluetoothGatt.GATT_INSUFFICIENT_ENCRYPTION) {
                changeDevicePairing(gatt.getDevice(), true);
            } else {
                disconnect();
            }
        }

        @Override
        public void onReadRemoteRssi(BluetoothGatt gatt, int rssi, int status) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                EventBus.getDefault().post(new BLEStateEvent.DeviceRssiUpdated(rssi, address));
            }
        }

        @Override
        public void onMtuChanged(BluetoothGatt gatt, int mtuSize, int status) {
//            super.onMtuChanged(gatt, mtuSize, status);
            Log.e(TAG, "onMtuChanged : \nmtuSize : " + mtuSize + "\nstatus : " + status);
            EventBus.getDefault().post(new BLEStateEvent.MTUUpdated(address, mtuSize, (status == BluetoothGatt.GATT_SUCCESS)));
        }

        @TargetApi(Build.VERSION_CODES.O)
        @Override
        public void onPhyUpdate(BluetoothGatt gatt, int txPhy, int rxPhy, int status) {
            EventBus.getDefault().post(new BLEStateEvent.PHYUpdated(txPhy,rxPhy,status));
        }

        @TargetApi(Build.VERSION_CODES.O)
        @Override
        public void onPhyRead(BluetoothGatt gatt, int txPhy, int rxPhy, int status) {
            EventBus.getDefault().post(new BLEStateEvent.PHYReaded(txPhy,rxPhy));
        }

    };





    /**
     * This callback is used for Wireless UART custom profile.
     */
    private final BluetoothGattServerCallback mGattServerCallback = new BluetoothGattServerCallback() {

        @Override
        public void onConnectionStateChange(BluetoothDevice device, int status, int newState) {
            Log.d(TAG, "gatt server" + "gatt server state changed: "+status+" ==" + newState);
            EventBus.getDefault().post(new BLEStateEvent.BluetoothClientStateChanged(newState));
        }

        @Override
        public void onServiceAdded(int status, BluetoothGattService service) {
            Log.d(TAG, "gatt server" + "gatt server added: "+status);

        }

        @Override
        public void onCharacteristicWriteRequest(BluetoothDevice device, int requestId,
                                                 BluetoothGattCharacteristic characteristic,
                                                 boolean preparedWrite,
                                                 boolean responseNeeded,
                                                 int offset,
                                                 byte[] value) {

            Log.d(TAG, "gatt server" + "onCharacteristicWriteRequest");
            Log.d(TAG, "gatt_server: " + new String(value));

            final String charaterUuid = characteristic.getUuid().toString();
            Log.d(TAG, "  uuid = " + charaterUuid);

            if (BLEAttributes.CHAR_WUART_NOTIFY.equalsIgnoreCase(charaterUuid)) {
                Log.d(TAG, "  CHAR_WUART_NOTIFY uuid = " + charaterUuid);
                final byte[] data = value;
                if (null != data && 0 < data.length) {

                    final StringBuilder stringBuilder = new StringBuilder(data.length);
                    for (byte byteChar : data)
                        stringBuilder.append(String.format("%02x ", byteChar));
                    Log.d(TAG, "  Notify data len:" + data.length + ":[" + stringBuilder.toString() + "]");
                }else{
                    Log.d(TAG, "  Notify empty packet");
                }

                handleRes(data, mResPacketType);
            }
        }
    };

    /**
     * Init singleton instance using application-level context.
     * We ensure that BluetoothManager and Adapter are always available.
     *
     * @param context
     */
    public void init(@NonNull Context context) {
        if (this.mContext != null) {
            return;
        }
        this.mContext = context.getApplicationContext();
        if (mBluetoothManager == null) {
            mBluetoothManager = (BluetoothManager) mContext.getSystemService(Context.BLUETOOTH_SERVICE);
        }

        if (mBluetoothAdapter == null) {
            mBluetoothAdapter = mBluetoothManager.getAdapter();
        }
        EventBus.getDefault().register(this);

    }

    /**
     * This instance handles bonding state event to correctly execute last failed task.
     *
     * @param e
     */
    @Subscribe
    public void onEvent(BLEStateEvent.DeviceBondStateChanged e) {
        Log.e("Bond", e.device.getAddress() + " " + e.bondState);
        if (mBluetoothGatt != null && e.device.equals(mBluetoothGatt.getDevice())) {
            // if device has bonding removed, retry last task so it can request bonding again
            if (e.bondState == BluetoothDevice.BOND_NONE) {
                continueTaskExecution(true);
            } else if (e.bondState == BluetoothDevice.BOND_BONDING) {
                mMainLoop.removeCallbacks(mTimeOutRunnable);
            } else if (e.bondState == BluetoothDevice.BOND_BONDED) {
                // do nothing because request will return result to callback now
                continueTaskExecution(true);
            }
        }
    }

    /**
     * For display purpose only.
     *
     * @return
     */
    public int getConnectionState() {
        return mConnectionState;
    }

    /**
     * Check if Bluetooth of device is enabled.
     *
     * @return
     */
    public boolean isBluetoothAvailable() {
        return mBluetoothAdapter != null && mBluetoothAdapter.isEnabled();
    }

    BluetoothGattService mPhoneSideService = null;
    /**
     * Connects to the GATT server hosted on the Bluetooth LE device.
     *
     * @param address The device address of the destination device.
     * @return Return true DataWritenFromClientif the connection is initiated successfully. The connection result
     * is reported asynchronously through the
     * {@code BluetoothGattCallback#onConnectionStateChange(android.bluetooth.BluetoothGatt, int, int)}
     * callback.
     */
    public void connect(@NonNull final String address, final boolean needUartSupport,final int phy) {
        mCurrentPhy = phy;
        this.address = address;
        this.tryTime++;
        mMainLoop.post(new Runnable() {

            @Override
            public void run() {
                BluetoothDevice device = mBluetoothAdapter.getRemoteDevice(address);
                Log.e(TAG, "initialize: mBluetoothGatt isn't null");

                if(SdkUtils.hasO()){
                    if(mBluetoothGatt !=  null){
                        mBluetoothGatt.connect();
                    }else{
                        mBluetoothGatt = device.connectGatt(mContext, false, mGattCallback,BluetoothDevice.TRANSPORT_LE,phy);
                    }
                }else{
                    if(mBluetoothGatt !=  null){
                        mBluetoothGatt.connect();
                    }else{
                        mBluetoothGatt = device.connectGatt(mContext, false, mGattCallback);
                    }
                }
                Log.e(TAG, "initialize successfully: mBluetoothGatt isn't null");

                mConnectionState = State.STATE_CONNECTING;
                EventBus.getDefault().post(new BLEStateEvent.Connecting());
                BLEService.this.needUartSupport = needUartSupport;

                if (needUartSupport) {
                    Log.e(TAG, "needUartSupport");

                    mBluetoothGattServer = mBluetoothManager.openGattServer(mContext, mGattServerCallback);
                    final BluetoothGattCharacteristic characteristic = new BluetoothGattCharacteristic(
                            BLEConverter.uuidFromAssignedNumber(BLEAttributes.UART_STREAM),
                            BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE,
                            BluetoothGattCharacteristic.PERMISSION_WRITE);

                    mPhoneSideService = new BluetoothGattService(
                            BLEConverter.uuidFromAssignedNumber(BLEAttributes.WUART), BluetoothGattService.SERVICE_TYPE_PRIMARY);

                    mPhoneSideService.addCharacteristic(characteristic);
                    if(null != mBluetoothGattServer && null != mPhoneSideService){
                        mBluetoothGattServer.addService(mPhoneSideService);
                    }

                }
            }
        });
    }

    /**
     * Disconnects an existing connection or cancel a pending connection. The disconnection result
     * is reported asynchronously through the
     * {@code BluetoothGattCallback#onConnectionStateChange(android.bluetooth.BluetoothGatt, int, int)}
     * callback.
     */
    public void disconnect() {

        if (mBluetoothGatt != null) {
            try {

                mBluetoothGatt.disconnect();
//                        mBluetoothGatt.close();
            } catch (Throwable ignored) {
            }
            mBluetoothGatt = null;
        }
        if (mBluetoothGattServer != null) {
            try {
                mBluetoothGattServer.close();
            } catch (Throwable ignored) {
            }
            mBluetoothGattServer = null;
        }

        mConnectionState = State.STATE_DISCONNECTED;
        tryTime = 0;
    }
private void bluetoothReleaseResource(){
    try {
        clearTasks();
        mBluetoothGatt.close();
    } catch (Throwable ignored) {
    }
    mBluetoothGatt = null;
    Log.e(TAG, "disconnect:BluetoothGatt is null");

    if (mBluetoothGattServer != null) {
        try {
            if(needUartSupport){
                mBluetoothGattServer.removeService(mPhoneSideService);
            }
            mBluetoothGattServer.close();
        } catch (Throwable ignored) {
        }
        mBluetoothGattServer = null;
    }
}
    /**
     * Check and get if a service with UUID is available within device.
     *
     * @param uuid assigned-number of a service
     * @return available service
     */
    public BluetoothGattService getService(int uuid) {
        return mBluetoothGatt == null ? null : mBluetoothGatt.getService(BLEConverter.uuidFromAssignedNumber(uuid));
    }


    public void readCustomCharacteristic() {
        if (mBluetoothAdapter == null || mBluetoothGatt == null) {

            return;
        }
        /*check if the service is available on the device*/
        BluetoothGattService mCustomService = mBluetoothGatt.getService(UUID.fromString("02ff5600-ba5e-f4ee-5ca1-eb1e5e4b1ce0"));
        if(mCustomService == null){

            return;
        }
        /*get the read characteristic from the service*/
        BluetoothGattCharacteristic mReadCharacteristic = mCustomService.getCharacteristic(UUID.fromString("02ff5700-ba5e-f4ee-5ca1-eb1e5e4b1ce0"));
        if(mBluetoothGatt.readCharacteristic(mReadCharacteristic) == false){

        }
    }




    public boolean request(String serviceUUID, int characteristicUUID, final int requestType) {
        if (mBluetoothGatt == null) {
            return false;
        }

        BluetoothGattService service = mBluetoothGatt.getService(UUID.fromString(serviceUUID));
        if (service == null) {
            return false;
        }
        final BluetoothGattCharacteristic characteristic = service.getCharacteristic(
                BLEConverter.uuidFromAssignedNumber(characteristicUUID));
        if (characteristic == null) {
            return false;
        }
        int charaProp = characteristic.getProperties();
        if ((charaProp | BluetoothGattCharacteristic.PROPERTY_READ) > 0) {
            if (requestType == Request.READ) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        readCharacteristic(characteristic);
                    }
                });
            }
        }
        if ((charaProp | BluetoothGattCharacteristic.PROPERTY_INDICATE) > 0) {
            if (requestType == Request.INDICATE) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        boolean waitForDescriptorWrite = setCharacteristicIndication(characteristic);
                        if (!waitForDescriptorWrite) {
                            continueTaskExecution(false);
                        }
                    }
                });
            }
        }
        if ((charaProp | BluetoothGattCharacteristic.PROPERTY_NOTIFY) > 0) {
            if (requestType == Request.NOTIFY) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        boolean waitForDescriptorWrite = setCharacteristicNotification(characteristic, true);
                        if (!waitForDescriptorWrite) {
                            continueTaskExecution(false);
                        }
                    }
                });
            }
        }
        if ((charaProp | BluetoothGattCharacteristic.PROPERTY_NOTIFY) > 0) {
            if (requestType == Request.DISABLE_NOTIFY_INDICATE) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        boolean waitForDescriptorWrite = setCharacteristicDisableNotification(characteristic, true);
                        if (!waitForDescriptorWrite) {
                            continueTaskExecution(false);
                        }
                    }
                });
            }
        }
        return true;
    }


    public boolean request(String serviceUUID, String characteristicUUID, final int requestType) {
        if (mBluetoothGatt == null) {
            return false;
        }
        BluetoothGattService service = mBluetoothGatt.getService(UUID.fromString(serviceUUID));
        if (service == null) {
            return false;
        }
       final BluetoothGattCharacteristic characteristic = service.getCharacteristic(UUID.fromString(characteristicUUID));
        if (characteristic == null) {
            return false;
        }
        int charaProp = characteristic.getProperties();
        if ((charaProp | BluetoothGattCharacteristic.PROPERTY_READ) > 0) {
            if (requestType == Request.READ) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        readCharacteristic(characteristic);
                    }
                });
            }
        }
        if ((charaProp | BluetoothGattCharacteristic.PROPERTY_INDICATE) > 0) {
            if (requestType == Request.INDICATE) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        boolean waitForDescriptorWrite = setCharacteristicIndication(characteristic);
                        if (!waitForDescriptorWrite) {
                            continueTaskExecution(false);
                        }
                    }
                });
            }
        }
        if ((charaProp | BluetoothGattCharacteristic.PROPERTY_NOTIFY) > 0) {
            if (requestType == Request.NOTIFY) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        boolean waitForDescriptorWrite = setCharacteristicNotification(characteristic, true);
                        if (!waitForDescriptorWrite) {
                            continueTaskExecution(false);
                        }
                    }
                });
            }
        }
        if ((charaProp | BluetoothGattCharacteristic.PROPERTY_NOTIFY) > 0) {
            if (requestType == Request.DISABLE_NOTIFY_INDICATE) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        boolean waitForDescriptorWrite = setCharacteristicDisableNotification(characteristic, true);
                        if (!waitForDescriptorWrite) {
                            continueTaskExecution(false);
                        }
                    }
                });
            }
        }
        return true;
    }


    /**
     * Request READ action using predefined service UUID and characteristic UUID, can allow notification if needed.
     *
     * @param serviceUUID
     * @param characteristicUUID
     * @param requestType        value must be of {@link com.freescale.bletoolbox.service.BLEService.Request}
     * @return true if both service and characteristic are found, false if otherwise.
     */
    public boolean request(int serviceUUID, int characteristicUUID, final int requestType) {
        Log.d(TAG, "+request:" + String.format("0x%x ", serviceUUID) + ":" + String.format("0x%x ", characteristicUUID) + ":" + requestType);
        if (mBluetoothGatt == null) {
            Log.d(TAG, "-request: mBluetoothGatt is null");
            return false;
        }
        BluetoothGattService service = getService(serviceUUID);
        if (service == null) {
            Log.d(TAG, "-request: service is null");
            return false;
        }
        final BluetoothGattCharacteristic characteristic = service.getCharacteristic(
                BLEConverter.uuidFromAssignedNumber(characteristicUUID));
        if (characteristic == null) {
            Log.d(TAG, "-request: characteristic is null");
            return false;
        }
        int charaProp = characteristic.getProperties();
        if ((charaProp | BluetoothGattCharacteristic.PROPERTY_READ) > 0) {
            Log.d(TAG, "  PROPERTY_READ");
            if (requestType == Request.READ) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        readCharacteristic(characteristic);
                    }
                });
            }
        }
        if ((charaProp | BluetoothGattCharacteristic.PROPERTY_INDICATE) > 0) {
            Log.d(TAG, "  PROPERTY_INDICATE");
            if (requestType == Request.INDICATE) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        boolean waitForDescriptorWrite = setCharacteristicIndication(characteristic);
                        if (!waitForDescriptorWrite) {
                            continueTaskExecution(false);
                        }
                    }
                });
            }
        }
        if ((charaProp | BluetoothGattCharacteristic.PROPERTY_NOTIFY) > 0) {
            Log.d(TAG, "  PROPERTY_NOTIFY");
            if (requestType == Request.NOTIFY) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        boolean waitForDescriptorWrite = setCharacteristicNotification(characteristic, true);
                        if (!waitForDescriptorWrite) {
                            continueTaskExecution(false);
                        }
                    }
                });
            }
        }
        if ((charaProp | BluetoothGattCharacteristic.PROPERTY_NOTIFY) > 0) {
            if (requestType == Request.DISABLE_NOTIFY_INDICATE) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        boolean waitForDescriptorWrite = setCharacteristicDisableNotification(characteristic, true);
                        if (!waitForDescriptorWrite) {
                            continueTaskExecution(false);
                        }
                    }
                });
            }
        }
        Log.d(TAG, "-request");
        return true;
    }



    /**
     * Specific request method to be used with Wireless UART.
     *
     * @param serviceUuid
     * @param characteristicUuid
     * @param data
     * @return
     */
    public boolean requestWrite(int serviceUuid, int characteristicUuid, final byte[] data) {
        Log.d(TAG, "+requestWrite");
        final boolean[] isSend = new boolean[1];
        if (mBluetoothGatt == null) {
            return false;
        }
        BluetoothGattService service = getService(serviceUuid);
        if (service == null) {
            return false;
        }
        final BluetoothGattCharacteristic characteristic = service.getCharacteristic(
                BLEConverter.uuidFromAssignedNumber(characteristicUuid));
        if (characteristic == null) {
            return false;
        }
        int charaProp = characteristic.getProperties();
        if ((charaProp | BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE) > 0) {
            registerTask(new Task() {

                @Override
                public void run() {
                    if (mBluetoothAdapter == null || mBluetoothGatt == null) {
                        Log.d(TAG, "  invalid state");
                        return;
                    }
                    characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
                    characteristic.setValue(data);
                    isSend[0] = mBluetoothGatt.writeCharacteristic(characteristic);
                    Log.d(TAG, "  writeCharacteristic:" + isSend[0]);
                }
            });
        }
        Log.d(TAG, "+requestWrite:" + isSend[0]);
        return true; //isSend[0];
    }


    public boolean requestWrite(String serviceUuid, String characteristicUuid, final byte[] data) {
        if (mBluetoothGatt == null) {
            Log.e("command send","step 1");

            return false;
        }
        BluetoothGattService service = mBluetoothGatt.getService(UUID.fromString(serviceUuid));
        //BluetoothGattService service = getService(serviceUuid);
        if (service == null) {
            Log.e("command send","step 2");

            return false;
        }
        final BluetoothGattCharacteristic characteristic = service.getCharacteristic(UUID.fromString(characteristicUuid));
       /* final BluetoothGattCharacteristic characteristic = service.getCharacteristic(
                BLEConverter.uuidFromAssignedNumber(characteristicUuid));*/
        if (characteristic == null) {
            Log.e("command send","step 3");

            return false;
        }
        int charaProp = characteristic.getProperties();
        if ((charaProp | BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE) > 0) {
            registerTask(new Task() {

                @Override
                public void run() {
                    if (mBluetoothAdapter == null || mBluetoothGatt == null) {
                        return;
                    }
                    characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
                    characteristic.setValue(data);
                    mBluetoothGatt.writeCharacteristic(characteristic);
                }
            });
        }
        return true;
    }

    /**
     * Request current RSSI value of connected BLE device.
     *
     * @return
     */
    public boolean requestRemoteRssi() {
        return mBluetoothGatt != null && mBluetoothGatt.readRemoteRssi();
    }

    /**
     * Add a task to single executor. This taks should be timed out after a few second.
     *
     * @param task
     */
    private void registerTask(Task task) {
        if (mTaskList == null) {
            mTaskList = new ArrayList<>();
        }
        mTaskList.add(task);
        Log.e("Task", "Registered with id " + task.id);
        if (mTaskList.size() == 1) {
            continueTaskExecution(true);
        }
    }

    /**
     * Unblock current execution, retry last task if needed or skip to next task.
     *
     * @param retryLastTask
     */
    private void continueTaskExecution(final boolean retryLastTask) {
        mMainLoop.removeCallbacks(mTimeOutRunnable);
        mMainLoop.post(new Runnable() {

            @Override
            public void run() {
                if (mTaskList == null || mTaskList.isEmpty()) {
                    return;
                }
                if (!retryLastTask) {
                    mTaskList.remove(0);
                }
                if (mTaskList.isEmpty()) {
                    return;
                }
                final Task task = mTaskList.get(0);
                if (task != null) {
                    Log.e("Task", "Schedule task " + task.id);
                    task.run();
                }
            }
        });
        mMainLoop.postDelayed(mTimeOutRunnable, AppConfig.DEFAULT_REQUEST_TIMEOUT);
    }

    /**
     * Clear all task variables.
     */
    private void clearTasks() {
        if (mTaskList != null) {
            mTaskList.clear();
            mTaskList = null;
        }
        Task.sInternalId = 0;
        mMainLoop.removeCallbacks(mTimeOutRunnable);
    }

    /**
     * Request a read on a given {@code BluetoothGattCharacteristic}. The read result is reported
     * asynchronously through the {@code BluetoothGattCallback#onCharacteristicRead(android.bluetooth.BluetoothGatt, android.bluetooth.BluetoothGattCharacteristic, int)}
     * callback. Always need to wait for callback.
     *
     * @param characteristic The characteristic to read from.
     */
    private void readCharacteristic(BluetoothGattCharacteristic characteristic) {
        if (mBluetoothAdapter == null || mBluetoothGatt == null || characteristic == null) {
            return;
        }
        mBluetoothGatt.readCharacteristic(characteristic);
    }

    /**
     * Enables or disables notification on a give characteristic.
     *
     * @param characteristic Characteristic to act on.
     * @param enabled        If true, enable notification.  False otherwise.
     * @return false if no need to perform any action, true if we need to wait from callback
     */
    private boolean setCharacteristicNotification(BluetoothGattCharacteristic characteristic, boolean enabled) {

        if (mBluetoothAdapter == null || mBluetoothGatt == null || characteristic == null) {
            return false;
        }
        mBluetoothGatt.setCharacteristicNotification(characteristic, enabled);
        BluetoothGattDescriptor descriptor = characteristic.getDescriptor(
                BLEConverter.uuidFromAssignedNumber(BLEAttributes.CLIENT_CHARACTERISTIC_CONFIG));
        if (descriptor != null) {
            descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
            boolean writeS = mBluetoothGatt.writeDescriptor(descriptor);

            return true;
        }
        return false;
    }

    /**
     * send Disable Notification to the remove device
     * Enables or disables notification on a give characteristic.
     *
     * @param characteristic Characteristic to act on.
     * @param enabled        If true, enable notification.  False otherwise.
     * @return false if no need to perform any action, true if we need to wait from callback
     */
    private boolean setCharacteristicDisableNotification(BluetoothGattCharacteristic characteristic, boolean enabled) {
        if (mBluetoothAdapter == null || mBluetoothGatt == null || characteristic == null) {
            return false;
        }
        mBluetoothGatt.setCharacteristicNotification(characteristic, enabled);
        BluetoothGattDescriptor descriptor = characteristic.getDescriptor(
                BLEConverter.uuidFromAssignedNumber(BLEAttributes.CLIENT_CHARACTERISTIC_CONFIG));
        if (descriptor != null) {
            descriptor.setValue(BluetoothGattDescriptor.DISABLE_NOTIFICATION_VALUE);
            mBluetoothGatt.writeDescriptor(descriptor);
            return true;
        }
        return false;
    }

    /**
     * @param serviceUUID
     * @param characteristicUUID
     * @param data
     */
    public boolean writeDataWithAuthen(int serviceUUID, int characteristicUUID, final byte[] data) {
        //remove write type with authentication ( because this type only work on Sony devices)
        return writeData(serviceUUID, characteristicUUID, Request.WRITE, data);
    }


    public boolean writeData(String serviceUUID, int characteristicUUID, final int writeType, final byte[] data) {
        if (mBluetoothGatt == null) {
            return false;
        }
        BluetoothGattService service = mBluetoothGatt.getService(UUID.fromString(serviceUUID));
        if (service == null) {
            return false;
        }
        final BluetoothGattCharacteristic characteristic = service.getCharacteristic(
               BLEConverter.uuidFromAssignedNumber(characteristicUUID));
      //  final BluetoothGattCharacteristic characteristic = service.getCharacteristic(UUID.fromString(characteristicUUID));
        if (characteristic == null) {
            return false;
        }
        int charaProp = characteristic.getProperties();
        if (Request.WRITE_WITH_AUTHEN == writeType) {
            if ((charaProp | BluetoothGattCharacteristic.WRITE_TYPE_SIGNED) > 0) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        if (mBluetoothAdapter == null || mBluetoothGatt == null) {
                            return;
                        }
                        characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_SIGNED);
                        characteristic.setValue(data);
                        mBluetoothGatt.writeCharacteristic(characteristic);
                    }
                });
            }
        } else if (Request.WRITE_NO_RESPONSE == writeType) {
            if ((charaProp | BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE) > 0) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        if (mBluetoothAdapter == null || mBluetoothGatt == null) {
                            return;
                        }
                        characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
                        characteristic.setValue(data);
                        mBluetoothGatt.writeCharacteristic(characteristic);
                    }
                });
            }
        } else if (Request.WRITE == writeType) {
            if ((charaProp | BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT) > 0) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        if (mBluetoothAdapter == null || mBluetoothGatt == null) {
                            return;
                        }
                        characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT);
                        characteristic.setValue(data);
                        mBluetoothGatt.writeCharacteristic(characteristic);
                    }
                });
            }
        }
        return true;
    }


    public boolean writeData(String serviceUUID, String characteristicUUID, final int writeType, final byte[] data) {
        if (mBluetoothGatt == null) {
            return false;
        }
        BluetoothGattService service = mBluetoothGatt.getService(UUID.fromString(serviceUUID));
        if (service == null) {
            return false;
        }
        final BluetoothGattCharacteristic characteristic = service.getCharacteristic(UUID.fromString(characteristicUUID));
        if (characteristic == null) {
            return false;
        }
        int charaProp = characteristic.getProperties();
        if (Request.WRITE_WITH_AUTHEN == writeType) {
            if ((charaProp | BluetoothGattCharacteristic.WRITE_TYPE_SIGNED) > 0) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        if (mBluetoothAdapter == null || mBluetoothGatt == null) {
                            return;
                        }
                        characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_SIGNED);
                        characteristic.setValue(data);
                        mBluetoothGatt.writeCharacteristic(characteristic);
                    }
                });
            }
        } else if (Request.WRITE_NO_RESPONSE == writeType) {
            if ((charaProp | BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE) > 0) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        if (mBluetoothAdapter == null || mBluetoothGatt == null) {
                            return;
                        }
                        characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
                        characteristic.setValue(data);
                        mBluetoothGatt.writeCharacteristic(characteristic);
                    }
                });
            }
        } else if (Request.WRITE == writeType) {
            if ((charaProp | BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT) > 0) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        if (mBluetoothAdapter == null || mBluetoothGatt == null) {
                            return;
                        }
                        characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT);
                        characteristic.setValue(data);
                        mBluetoothGatt.writeCharacteristic(characteristic);
                    }
                });
            }
        }
        return true;
    }


    public boolean throughputWriteData(String serviceUUID, String characteristicUUID, byte[] value, boolean repeate){
        if (mBluetoothGatt == null) {
            return false;
        }
        BluetoothGattService service = mBluetoothGatt.getService(UUID.fromString(serviceUUID));
        if (service == null) {
            return false;
        }

        final BluetoothGattCharacteristic characteristic = service.getCharacteristic(UUID.fromString(characteristicUUID));
        if (characteristic == null) {
            return false;
        }
        isThroughput = repeate;

        characteristic.setValue(value);
        return mBluetoothGatt.writeCharacteristic(characteristic);
    }


    public boolean requestMTU(final int mtuSize) {
        if (Build.VERSION.SDK_INT >= 21 && mBluetoothGatt != null) {
            registerTask(new Task() {
                @Override
                public void run() {
                    mBluetoothGatt.requestMtu(mtuSize);
                }
            });
            return true;
        } else {
            return false;
        }
    }


    public boolean requestMTUInTaskWay(final int mtuSize) {
        if (Build.VERSION.SDK_INT >= 21 && mBluetoothGatt != null) {
            registerTask(new Task() {
                @Override
                public void run() {
                    mBluetoothGatt.requestMtu(mtuSize);
                }
            });
            return true;
        } else {
            return false;
        }
    }

    public boolean requestConnectPrioity(int priority) {
        if (Build.VERSION.SDK_INT >= 21) {
            return mBluetoothGatt.requestConnectionPriority(priority);
        } else {
            return false;
        }

    }

    /**
     * @param serviceUUID
     * @param characteristicUUID
     * @param writeType
     * @param data
     */
    public boolean writeData(int serviceUUID, int characteristicUUID, final int writeType, final byte[] data) {
        if (mBluetoothGatt == null) {
            return false;
        }
        BluetoothGattService service = getService(serviceUUID);
        if (service == null) {
            return false;
        }
        final BluetoothGattCharacteristic characteristic = service.getCharacteristic(
                BLEConverter.uuidFromAssignedNumber(characteristicUUID));
        if (characteristic == null) {
            return false;
        }
//        Log.d("OTA========", "send :" + BLEConverter.bytesToHex(data));

        int charaProp = characteristic.getProperties();
        if (Request.WRITE_WITH_AUTHEN == writeType) {
            if ((charaProp | BluetoothGattCharacteristic.WRITE_TYPE_SIGNED) > 0) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        if (mBluetoothAdapter == null || mBluetoothGatt == null) {
                            return;
                        }
                        characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_SIGNED);
                        characteristic.setValue(data);
                        mBluetoothGatt.writeCharacteristic(characteristic);
                    }
                });
            }
        } else if (Request.WRITE_NO_RESPONSE == writeType) {
            if ((charaProp | BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE) > 0) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        if (mBluetoothAdapter == null || mBluetoothGatt == null) {
                            return;
                        }
                        characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
                        characteristic.setValue(data);
                        mBluetoothGatt.writeCharacteristic(characteristic);

                    }
                });
            }
        } else if (Request.WRITE == writeType) {
            if ((charaProp | BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT) > 0) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        if (mBluetoothAdapter == null || mBluetoothGatt == null) {
                            return;
                        }
                        characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT);
                        characteristic.setValue(data);
                        mBluetoothGatt.writeCharacteristic(characteristic);
                        Log.d("ota", "SEND NewImageInfoResponse " + BLEConverter.bytesToHexWithSpace(data));

                    }
                });
            }
        }
        return true;
    }



    public boolean writeCharacteristic(String serviceUUID, String characteristicUUID, final int requestType, final int value, final int format, final int offset) {
        if (mBluetoothGatt == null) {
            return false;
        }
        BluetoothGattService service = mBluetoothGatt.getService(UUID.fromString(serviceUUID));
        if (service == null) {
            return false;
        }
        final BluetoothGattCharacteristic characteristic = service.getCharacteristic(UUID.fromString(characteristicUUID));
        if (characteristic == null) {
            return false;
        }
        int charaProp = characteristic.getProperties();
        if ((charaProp | BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE) > 0) {
            if (requestType == Request.WRITE_NO_RESPONSE) {
                registerTask(new Task() {
                    @Override
                    public void run() {
                        boolean wrote = setCharacteristic(characteristic, value, format, offset);
                        continueTaskExecution(false);
                    }
                });
            }
        }
        if ((charaProp | BluetoothGattCharacteristic.PROPERTY_WRITE) > 0) {
            if (requestType == Request.WRITE) {
                registerTask(new Task() {
                    @Override
                    public void run() {
                        boolean wrote = setCharacteristic(characteristic, value, format, offset);
                    }
                });
            }
        }
        return true;
    }



    public boolean writeCharacteristic(int serviceUUID, int characteristicUUID, final int requestType, final int value, final int format, final int offset) {
        if (mBluetoothGatt == null) {
            return false;
        }
        BluetoothGattService service = getService(serviceUUID);
        if (service == null) {
            return false;
        }
        final BluetoothGattCharacteristic characteristic = service.getCharacteristic(
                BLEConverter.uuidFromAssignedNumber(characteristicUUID));
        if (characteristic == null) {
            return false;
        }
        int charaProp = characteristic.getProperties();
        if ((charaProp | BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE) > 0) {
            if (requestType == Request.WRITE_NO_RESPONSE) {
                registerTask(new Task() {
                    @Override
                    public void run() {
                        boolean wrote = setCharacteristic(characteristic, value, format, offset);
                        continueTaskExecution(false);
                    }
                });
            }
        }
        if ((charaProp | BluetoothGattCharacteristic.PROPERTY_WRITE) > 0) {
            if (requestType == Request.WRITE) {
                registerTask(new Task() {
                    @Override
                    public void run() {
                        boolean wrote = setCharacteristic(characteristic, value, format, offset);
                    }
                });
            }
        }
        return true;
    }


    /**
     * @param characteristic
     * @param value
     * @param format
     * @param offset
     * @return false if no need to perform any action, true if we need to wait from callback
     */
    private boolean setCharacteristic(BluetoothGattCharacteristic characteristic, int value, int format, int offset) {
        if (mBluetoothAdapter == null || mBluetoothGatt == null || characteristic == null) {
            return false;
        }
        characteristic.setValue(value, format, offset);
        return mBluetoothGatt.writeCharacteristic(characteristic);
    }

    /**
     * Enables or disables indication on a give characteris
     *tic.
     *
     * @param characteristic Characteristic to act on.
     * @return false if no need to perform any action, true if we need to wait from callback
     */
    private boolean setCharacteristicIndication(BluetoothGattCharacteristic characteristic) {
        if (mBluetoothAdapter == null || mBluetoothGatt == null || characteristic == null) {
            return false;
        }
        mBluetoothGatt.setCharacteristicNotification(characteristic, true);
        BluetoothGattDescriptor descriptor = characteristic.getDescriptor(
                BLEConverter.uuidFromAssignedNumber(BLEAttributes.CLIENT_CHARACTERISTIC_CONFIG));
        if (descriptor != null) {
            descriptor.setValue(BluetoothGattDescriptor.ENABLE_INDICATION_VALUE);
            mBluetoothGatt.writeDescriptor(descriptor);
            return true;
        }
        return false;
    }

    private boolean changeDevicePairing(@NonNull BluetoothDevice device, boolean remove) {
        try {
            Method m = device.getClass()
                    .getMethod(remove ? "removeBond" : "createBond", (Class[]) null);
            m.invoke(device, (Object[]) null);
            return true;
        } catch (Exception ignored) {
            return false;
        }
    }

    public void updatePreferredPhy(int txPhy,int rxPhy,int phyOptions){
        if(SdkUtils.hasO()){
        mBluetoothGatt.setPreferredPhy(txPhy,rxPhy,phyOptions);
    }
}

    public void readPhy(){
        if(SdkUtils.hasO()){
            mBluetoothGatt.readPhy();
        }
    }

    private void dumpBytes(byte[] bytes, int len) {
        Log.d(TAG, "bytes:[" + bytes.length + "]");
        int line_len = 16;
        //int bytes_len = bytes.length;
        // only 16 header + one 128 bytes face feature as the other two 128 face feature are same as the first one
        //if (face_feature_len > (128 + 16)) {
        //face_feature_len = 128 + 16;
        //}
        int lines = len / line_len;
        int remains = len % line_len;
        int i = 0;
        for (; i < lines; i++) {
            //Log.d(TAG, bytes2HexString(faceFeature + i * line_len, line_len));
            Log.d(TAG, String.format("0x%02x ",bytes[i * line_len]) + String.format("0x%02x ",bytes[i * line_len +1]) +
                    String.format("0x%02x ",bytes[i * line_len+2]) + String.format("0x%02x ",bytes[i * line_len+3]) +
                    String.format("0x%02x ",bytes[i * line_len+4]) + String.format("0x%02x ",bytes[i * line_len+5]) +
                    String.format("0x%02x ",bytes[i * line_len+6]) + String.format("0x%02x ",bytes[i * line_len+7]) +
                    String.format("0x%02x ",bytes[i * line_len+8]) + String.format("0x%02x ",bytes[i * line_len+9]) +
                    String.format("0x%02x ",bytes[i * line_len+10]) + String.format("0x%02x ",bytes[i * line_len+11]) +
                    String.format("0x%02x ",bytes[i * line_len+12]) + String.format("0x%02x ",bytes[i * line_len+13]) +
                    String.format("0x%02x ",bytes[i * line_len+14]) + String.format("0x%02x ",bytes[i * line_len+15]));
        }

        String strRemains = "";
        for (int j = 0; j < remains; j++) {
            strRemains += String.format("0x%02x ",bytes[i * line_len + j]);
        }
        if (remains > 0) {
            Log.d(TAG, strRemains);
        }
    }

    private boolean chunkSend(byte[] data, int length)
    {
        int chunkSize = 20;
        int chunkCount = length / chunkSize;
        int remain = length % chunkSize;
        boolean ret = true;

        for (int i = 0; i < chunkCount; i++) {
            byte[] chunk = Arrays.copyOfRange(data, i * chunkSize, (i + 1)*chunkSize);
            ret = requestWrite(BLEAttributes.WUART, BLEAttributes.UART_STREAM, chunk);

            if (ret == false) {
                Log.e(TAG, "  send chunk:[" + i + "] failed");
                mReqPacketType = INVALID_PACKET;
                mReqPacketID = 0;
                return ret;
            }

            Log.e(TAG, "  send chunk:[" + i + "] ok");
            dumpBytes(chunk, chunk.length);
        }
        if (remain > 0) {
            byte[] chunk = new byte[remain];
            System.arraycopy(data, chunkCount * chunkSize, chunk, 0, remain);
            ret = requestWrite(BLEAttributes.WUART, BLEAttributes.UART_STREAM, chunk);

            if (ret == false) {
                Log.e(TAG, "  send remain failed");
                mReqPacketType = INVALID_PACKET;
                mReqPacketID = 0;
                return ret;
            }
            Log.e(TAG, "  send remain ok");
            dumpBytes(chunk, chunk.length);
        }

        return ret;
    }

    private boolean sendHeader(byte pkt_type, int pkt_len, long pkt_crc) {
        Log.d(TAG, "+sendHeader");
        byte[] header = new byte[PACKET_HEADER_LEN];
        // tu_type
        header[0] = (byte)0x53;
        header[1] = (byte)0x79;
        header[2] = (byte)0x4c;
        // pkt_type
        header[3] = pkt_type;
        // pkt_len
        header[4] = (byte)(pkt_len & 0xff);
        header[5] = (byte)((pkt_len >> 8) & 0xff);
        header[6] = (byte)((pkt_len >> 16) & 0xff);
        header[7] = (byte)((pkt_len >> 24) & 0xff);

        mReqPacketID = mPacketID;
        // pkt_id
        header[8] = (byte)(mPacketID & 0xff);
        header[9] = (byte)((mPacketID >> 8) & 0xff);
        header[10] = (byte)((mPacketID >> 16) & 0xff);
        header[11] = (byte)((mPacketID >> 24) & 0xff);
        // increase the ID

        // pkt_crc
        header[12] = (byte)(pkt_crc & 0xff);
        header[13] = (byte)((pkt_crc >> 8) & 0xff);
        header[14] = (byte)((pkt_crc >> 16) & 0xff);
        header[15] = (byte)((pkt_crc >> 24) & 0xff);

        // reserved
        header[16] = (byte)(headerReserved & 0xff);
        header[17] = (byte)((headerReserved >> 8) & 0xff);
        header[18] = (byte)((headerReserved >> 16) & 0xff);
        header[19] = (byte)((headerReserved >> 24) & 0xff);

        // tu crc
        CRC32 crc32 = new CRC32();
        crc32.reset();
        crc32.update(header, 0, 20);
        long header_crc = crc32.getValue();
        header[20] = (byte)(header_crc & 0xff);
        header[21] = (byte)((header_crc >> 8) & 0xff);
        header[22] = (byte)((header_crc >> 16) & 0xff);
        header[23] = (byte)((header_crc >> 24) & 0xff);

        Log.d(TAG, "Header:[" +
                String.format("0x%02x",header[0]) + ":" + String.format("0x%02x",header[1]) + ":" +
                String.format("0x%02x",header[2]) + ":" + String.format("0x%02x",header[3]) + ":" +
                String.format("0x%02x",header[4]) + ":" + String.format("0x%02x",header[5]) + ":" +
                String.format("0x%02x",header[6]) + ":" + String.format("0x%02x",header[7]) + ":" +
                String.format("0x%02x",header[8]) + ":" + String.format("0x%02x",header[9]) + ":" +
                String.format("0x%02x",header[10]) + ":" + String.format("0x%02x",header[11]) + ":" +
                String.format("0x%02x",header[12]) + ":" + String.format("0x%02x",header[13]) + ":" +
                String.format("0x%02x",header[14]) + ":" + String.format("0x%02x",header[15]) + ":" +
                String.format("0x%02x",header[16]) + ":" + String.format("0x%02x",header[17]) + ":" +
                String.format("0x%02x",header[18]) + ":" + String.format("0x%02x",header[19]) + ":" +
                String.format("0x%02x",header[20]) + ":" + String.format("0x%02x",header[21]) + ":" +
                String.format("0x%02x",header[22]) + ":" + String.format("0x%02x",header[23]) + ":" +
                "]");

        chunkSend(header, PACKET_HEADER_LEN);
        Log.d(TAG, "-sendHeader:pkt_type:"+pkt_type+":pkt_len:"+pkt_len+":pkt_id:"+mPacketID+":pkt_crc:"+pkt_crc+":header_crc:"+header_crc);
        mPacketID++;
        return true;
    }

    public boolean sendPacket(byte[] data, int length, byte type) {
        boolean ret = false;

        mReqPacketType = type;
        mResPacketType = INVALID_PACKET;
        CRC32 crc32 = new CRC32();
        crc32.reset();

        if (data != null) {
            crc32.update(data);
        }

        long pkt_crc = crc32.getValue();
        Log.d(TAG, "+sendPacket:" + type + ":" + length + ":crc:" + pkt_crc);

        ret = sendHeader(type, length, pkt_crc);

        if (ret == false) {
            Log.d(TAG, "ERROR:send header");
            mReqPacketType = INVALID_PACKET;
            mReqPacketID = 0;
            return ret;
        }

        if (data != null) {
            ret = chunkSend(data, length);
            if (ret == false) {
                Log.d(TAG, "ERROR:send pkt");
                mReqPacketType = INVALID_PACKET;
                mReqPacketID = 0;
                return ret;
            }
        }

        mResPacketType = (byte) (type + 1);
        Log.d(TAG, "-sendPacket:");
        return true;
    }


    /* send authentication Req */
    public boolean sendAuthenticationReq(String password){
        byte pkt_type = AUTHENTICATION_REQ;
        int pkt_len = password.getBytes().length;
        byte[] pkt_data = password.getBytes();
        return sendPacket(pkt_data, pkt_len, pkt_type);
    }

    /* send update password Req */
    public boolean sendUpdatePasswordReq(String newPassword){
        byte pkt_type = UPDATE_PASSWORD_REQ;
        int pkt_len = newPassword.getBytes().length;
        byte[] pkt_data = newPassword.getBytes();
        return sendPacket(pkt_data, pkt_len, pkt_type);
    }

    /* send registration Req */
    public boolean sendRegistrationReq(String faceName, byte[] faceFeature, boolean reRegister) {
        Log.d(TAG, "+sendRegistrationReq");
        byte pkt_type = REGISTRATION_REQ;
        int name_size = 32;
        int pkt_len = name_size + faceFeature.length;

        if (reRegister){
            headerReserved = 1;
        } else {
            headerReserved = 0;
        }

        int faceName_len = faceName.length();
        if (faceName_len > 31) {
            faceName_len = 31;
        }
        byte[] faceName_bytes = faceName.getBytes();
        byte[] pkt_data = new byte[pkt_len];

        // fill the name
        int index = 0;
        for (; index <= 31; index++) {
            if (index < faceName_len) {
                pkt_data[index] = faceName_bytes[index];
            } else {
                pkt_data[index] = 0;
            }
        }

        // fill the face feature
        for (index = 0; index < faceFeature.length; index++) {
            pkt_data[index + 32] = faceFeature[index];
        }

        // send the pkt
        boolean ret = sendPacket(pkt_data, pkt_len, pkt_type);

        Log.d(TAG, "-sendRegistrationReq:" + ret);
        return ret;
    }

    /* send delete user Req */
    public boolean sendDeleteUserReq(int faceId){
        byte pkt_type = DELETE_USER_REQ;
        int pkt_len = 4;
        byte[] pkt_data = new byte[4];
        pkt_data[0] = (byte)(faceId & 0xff);
        pkt_data[1] = (byte)((faceId >> 8) & 0xff);
        pkt_data[2] = (byte)((faceId >> 16) & 0xff);
        pkt_data[3] = (byte)((faceId >> 24) & 0xff);

        return sendPacket(pkt_data, pkt_len, pkt_type);
    }

    /* send get user count Req */
    public boolean sendGetUserCountReq(){
        byte pkt_type = GET_USER_COUNT_REQ;
        int pkt_len = 0;
        byte[] pkt_data = new byte[0];
        return sendPacket(pkt_data, pkt_len, pkt_type);
    }

    /* send user infomation Req */
    public boolean sendGetUserInfoReq(){
        byte pkt_type = GET_USER_INFO_REQ;
        int pkt_len = 0;
        byte[] pkt_data = new byte[0];
        return sendPacket(pkt_data, pkt_len, pkt_type);
    }

    /* send update user information Req */
    public boolean sendUpdateUserInfoReq(int faceId, String userName){
        byte pkt_type = UPDATE_USER_INFO_REQ;
        int pkt_len = 36;
        byte[] pkt_data = new byte[36];
        pkt_data[0] = (byte)(faceId & 0xff);
        pkt_data[1] = (byte)((faceId >> 8) & 0xff);
        pkt_data[2] = (byte)((faceId >> 16) & 0xff);
        pkt_data[3] = (byte)((faceId >> 24) & 0xff);

        System.arraycopy(userName.getBytes(),0, pkt_data, 4, userName.getBytes().length);

        return sendPacket(pkt_data, pkt_len, pkt_type);
    }

    /* send registration command Req */
    public boolean sendRegistrationCmdReq(){
        byte pkt_type = REGISTRATION_CMD_REQ;
        int pkt_len = 0;
        byte[] pkt_data = new byte[0];
        return sendPacket(pkt_data, pkt_len, pkt_type);
    }

    /* send deregistration command Req */
    public boolean sendDeregistrationCmdReq(){
        byte pkt_type = DEREGISTRATION_CMD_REQ;
        int pkt_len = 0;
        byte[] pkt_data = new byte[0];
        return sendPacket(pkt_data, pkt_len, pkt_type);
    }

    /* send preview camera switch command Req */
    public boolean sendPreviewCameraSwitchReq(){
        byte pkt_type = PREV_CAMERA_SWITCH_CMD_REQ;
        int pkt_len = 0;
        byte[] pkt_data = new byte[0];
        return sendPacket(pkt_data, pkt_len, pkt_type);
    }

    /* send get app type Req */
    public boolean sendGetAppTypeReq(){
        byte pkt_type = GET_APP_TYPE_REQ;
        int pkt_len = 0;
        byte[] pkt_data = new byte[0];
        return sendPacket(pkt_data, pkt_len, pkt_type);
    }

    /* send get algorithm version Req */
    public boolean sendGetAlgoVersionReq(){
        byte pkt_type = GET_ALGO_VERSION_REQ;
        int pkt_len = 0;
        byte[] pkt_data = new byte[0];
        return sendPacket(pkt_data, pkt_len, pkt_type);
    }

    private int CheckResPacket(@NotNull byte[] data, byte packet_type, int packet_len) {
        long tu_crc = 0;

        CRC32 crc32 = new CRC32();
        crc32.reset();

        for (int i = 0; i < (PACKET_HEADER_LEN - 4); i++) {
            crc32.update(data[i]);
        }

        tu_crc = crc32.getValue();

        /* pkt magic check */
        if ((data[0] != 0x53) || (data[1] !=  0x79) || (data[2] != 0x4c)) {
            Log.d(TAG, "  ERROR:invalid tu_type:" + data[0]);
            return -2;
        }

        byte pkt_type =   data[3];
        long pkt_len =    (long)((((data[7] & 0xff) << 24)  | ((data[6] & 0xff) << 16)  | ((data[5] & 0xff) << 8)  | (data[4] & 0xff)) & 0xFFFFFFFFL);
        long pkt_id =     (long)((((data[11] & 0xff) << 24) | ((data[10] & 0xff) << 16) | ((data[9] & 0xff) << 8)  | (data[8] & 0xff)) & 0xFFFFFFFFL);
        long pkt_crc =    (long)((((data[15] & 0xff) << 24) | ((data[14] & 0xff) << 16) | ((data[13] & 0xff) << 8) | (data[12] & 0xff)) & 0xFFFFFFFFL);
        long reserved =   (long)((((data[19] & 0xff) << 24) | ((data[18] & 0xff) << 16) | ((data[17] & 0xff) << 8) | (data[16] & 0xff)) & 0xFFFFFFFFL);
        long header_crc = (long)((((data[23] & 0xff) << 24) | ((data[22] & 0xff) << 16) | ((data[21] & 0xff) << 8) | (data[20] & 0xff)) & 0xFFFFFFFFL);

        if (pkt_type != packet_type) {
            Log.d(TAG, "  ERROR:invalid pkt_type:" + data[1]);
            return -3;
        }

        if (pkt_len != packet_len) {
            Log.d(TAG, "  ERROR:invalid pkt_len:" + pkt_len);
            return -4;
        }

        if (pkt_id != mReqPacketID) {
            Log.d(TAG, "  ERROR:invalid pkt_id:" + pkt_id + ":" + mReqPacketID);
            return -5;
        }

        if (header_crc != tu_crc) {
            Log.d(TAG, "  ERROR:invalid header_crc:" + header_crc + ":" + String.format("0x%08x", header_crc) + ":" + tu_crc);
            return -7;
        }

        return 0;
    }

    private void notifyRes(int packet_type, int result) {
        /* The received packets is OK */
        switch (packet_type) {
            case AUTHENTICATION_RES:
                EventBus.getDefault().post(new BLEStateEvent.AuthenticationRes(result));
                break;
            case UPDATE_PASSWORD_RES:
                EventBus.getDefault().post(new BLEStateEvent.UpdatePasswordRes(result));
                break;
            case REGISTRATION_RES:
                EventBus.getDefault().post(new BLEStateEvent.RegistrationRes(
                        result, Arrays.copyOfRange(packetData.toByteArray(), PACKET_HEADER_LEN, packetData.toByteArray().length)));
                break;
            case DELETE_USER_RES:
                EventBus.getDefault().post(new BLEStateEvent.DeleteUserRes(result));
                break;
            case GET_USER_COUNT_RES:
                EventBus.getDefault().post(new BLEStateEvent.GetUserCountRes(result));
                break;
            case GET_USER_INFO_RES:
                EventBus.getDefault().post(new BLEStateEvent.GetUserInfoRes(
                        result, Arrays.copyOfRange(packetData.toByteArray(), PACKET_HEADER_LEN, packetData.toByteArray().length)));
                break;
            case UPDATE_USER_INFO_RES:
                EventBus.getDefault().post(new BLEStateEvent.UpdateUserInfoRes(result));
                break;
            case REGISTRATION_CMD_RES:
                EventBus.getDefault().post(new BLEStateEvent.RegistrationCMDRes(result));
                break;
            case DEREGISTRATION_CMD_RES:
                EventBus.getDefault().post(new BLEStateEvent.DeregistrationCMDRes(result));
                break;
            case PREV_CAMERA_SWITCH_CMD_RES:
                EventBus.getDefault().post(new BLEStateEvent.PrevCameraSwitchCMDRes(result));
                break;
            case GET_APP_TYPE_RES:
                EventBus.getDefault().post(new BLEStateEvent.GetAppTypeRes(result));
                break;
            case GET_ALGO_VERSION_RES:
                EventBus.getDefault().post(new BLEStateEvent.GetAlgoVersionRes(result));
                break;
            default:
                break;
        }
    }

    private void handleRes(byte[] data, byte packet_type) {
        Log.d(TAG, "+handleRes:[" + packet_type+ "]");

        // This has to be long, but due to the switch case in Java not supporting the type long,
        // will leave it as int, with the mention that in case the package result is bigger than
        // INT max size, which it won't, it will be a mess :)
        int result = 0;
        int pkt_len = 0;

        if (packetDataLength == 0) {
            if (data.length < PACKET_HEADER_LEN) {
                Log.d(TAG, "-handleRes:invalid packet:" + result);
                result = INVALID_PACKET;
                notifyRes(packet_type, result);
                return;
            }

            pkt_len = ((int) ((((data[7] & 0xff) << 24) | ((data[6] & 0xff) << 16) | ((data[5] & 0xff) << 8) | (data[4] & 0xff)) & 0xFFFFFFFFL));

            /* check packet length */
            if (pkt_len == 0) {
                result = CheckResPacket(data, packet_type, packetLength[packet_type]);
            } else {
                result = CheckResPacket(data, packet_type, packetLength[packet_type] + pkt_len);
            }

            if (result == 0) {
                packetDataLength = PACKET_HEADER_LEN + pkt_len;
                packetData = new ByteArrayOutputStream();
                packetReserved = (int) ((((data[19] & 0xff) << 24) | ((data[18] & 0xff) << 16) | ((data[17] & 0xff) << 8) | (data[16] & 0xff)) & 0xFFFFFFFFL);
            } else {
                Log.d(TAG, "-handleRes:check packet error:" + result);
                result = INVALID_PACKET;
                notifyRes(packet_type, result);
                return;
            }
        }

        /* save data into buffer */
        try {
            packetData.write(data);
            packetDataLength -= data.length;
        } catch (IOException e) {
            packetDataLength = -1;
            Log.e(TAG,"extra packets data write error: " + e.getMessage());
        }

        /* The received packets don't match with the sent ones */
        if (packetDataLength < 0)
        {
            packetDataLength = 0;
            packetReserved = 0;
            Log.d(TAG, "-handleRes:packets data length mismatch: " + packetDataLength);
            result = INVALID_PACKET;
            notifyRes(packet_type, result);
            return;
        }

        if (packetDataLength > 0)
        {
            Log.d(TAG, "-handleRes:" + packetDataLength);
            return;
        }

        notifyRes(packet_type, packetReserved);
        Log.d(TAG, "-handleRes:[" + packetReserved + "]");
    }
}
