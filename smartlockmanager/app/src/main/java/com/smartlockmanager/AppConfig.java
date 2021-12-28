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

package com.smartlockmanager;

import java.util.LinkedHashMap;

public interface AppConfig {
    /**
     * Time interval before a request is considered timed out.
     */
    int DEFAULT_REQUEST_TIMEOUT = 10 * 1000;

    /**
     * Time interval between battery updates (ms).
     */
    int BATTERY_UPDATE_INTERVAL = 50 * 1000;

    /**
     * Scanning period of BLE devices (ms)
     */
    int DEVICE_SCAN_PERIOD = 30 * 1000;

    /**
     * Some devices filter BLE packets by default. We need to continuously turn on/off scanning to keep RSSI updated.
     */
    int DEVICE_CONTINUOUSLY_SCAN = 2 * 1000;

    /**
     * If a beacon can not be seen in this period, we consider that beacon is out of range.
     */
    int BEACON_OUT_OF_RANGE_PERIOD = 10 * 1000;

    /**
     * Layout for all supported beacon types inside app.
     */
    String[] SUPPORTED_BEACON_LAYOUTS = {"m:2-3=0215,i:4-19,i:20-21,i:22-23,p:24-24"};

    /**
     * Bluetooth Assigned Number for FSL.
     */
    int FSL_MANUFACTURER_ID = 0x01ff;

    /**
     * Time interval between PROXIMITY TX POWER update scheduler (ms).
     */
    int PROXIMITY_TX_POWER_UPDATE_INTERVAL = 3 * 1000;
    int PROXIMITY_RSSI_UPDATE_INTERVAL = 1 * 1000;

    /**
     * For Wireless UART message, we need to define a maximum value of buffered string to avoid overflow.
     */
    int MAX_WUART_BUFFER = 1000;

    public static final int qppServerBufferSize = 20;
}
