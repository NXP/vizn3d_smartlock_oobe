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

package com.smartlockmanager.model;

import android.bluetooth.BluetoothDevice;

public class BLEDevice {

    private BluetoothDevice internalDevice;
    private int rssi;
    private long lastScannedTime;

    public BLEDevice(BluetoothDevice internalDevice) {
        if (internalDevice == null) {
            throw new NullPointerException();
        }
        this.internalDevice = internalDevice;
    }

    public BluetoothDevice getInternalDevice() {
        return internalDevice;
    }

    public int getRssi() {
        return rssi;
    }

    public void setRssi(int rssi) {
        this.rssi = rssi;
    }

    public long getLastScannedTime() {
        return lastScannedTime;
    }

    public void setLastScannedTime(long lastScannedTime) {
        this.lastScannedTime = lastScannedTime;
    }

    @Override
    public int hashCode() {
        return internalDevice.hashCode();
    }

    @Override
    public boolean equals(Object o) {
        if (o == null) {
            return false;
        }
        if (!(o instanceof BLEDevice)) {
            return false;
        }
        if (((BLEDevice) o).internalDevice == null) {
            return false;
        }
        return internalDevice.equals(((BLEDevice) o).internalDevice);
    }
}
