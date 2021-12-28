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

public interface BLEAttributes {

    String UUID_FORM = "0000%s-0000-1000-8000-00805f9b34fb";

    /***
     * Services
     ***/
    int HEART_RATE_SERVICE = 0x180D;
    int DEVICE_INFORMATION_SERVICE = 0x180A;
    int BATTERY_SERVICE = 0x180F;
    int CYCLING_SPEED = 0x1816;
    int THERMOMETER = 0x1809;
    int RUNNING_SPEED = 0x1814;

    // custom constant to keep current flow
    int WUART = 0xffff;
    int OTAP = 0xfff2;
    int FRDM = 0xfff4;
    int FRDM_INPUT_REPORT = 0xfff5;
    int SHELL = 0xfff6;

    int BLOOD_PRESSURE_SERVICE = 0x1810;
    // PROXIMITY
    int LINK_LOSS_SERVICE = 0x1803; // mandatory
    int IMMEDIATE_ALERT_SERVICE = 0x1802; // optional
    int TX_POWER_SERVICE = 0x1804; // optional

    int GLUCOSE_SERVICE = 0x1808;

    /**
     * Characteristics
     ***/
    // Heart Rate
    int HEART_RATE_MEASUREMENT = 0x2A37;
    int BODY_SENSOR_LOCATION = 0x2A38;
    int BATTERY_LEVEL = 0x2A19;

    int TEMPERATURE_MEASUREMENT = 0x2A1C;
    int TEMPERATURE_TYPE = 0x2A1D;
    int INTERMEDIATE_TEMPERATURE = 0x2A1E;

    int RSC_MEASUREMENT = 0x2A53;
    int CSC_MEASUREMENT = 0x2A5B;
    int SENSOR_LOCATION = 0x2A5D;

    // Blood Pressure
    int BLOOD_PRESSURE_MEASUREMENT = 0x2A35;
    int INTERMEDIATE_CUFF_PRESSURE = 0x2A36;
    // PROXIMITY
    int TX_POWER_LEVEL = 0x2A07;
    int ALERT_LEVEL = 0x2A06;

    // GLUCOSE
    int GLUCOSE_MEASUREMENT = 0x2A18;
    int GLUCOSE_MEASUREMENT_CONTEXT = 0x2A34;
    int GLUCOSE_RECORD_ACCESS_CONTROL_POINT = 0x2A52;

    // Wireless UART, using fake UUID
    int UART_STREAM = 0xfffe;
    int UART_NOTIFY = 0xffffe;

    int OTAP_CONTROL = 0xffee;
    int OTAP_DATA = 0xffef;
    int FRDM_CONTROL = 0xfeff;

    // OTAP custom UUID characteristic
    /**
     * can
     * be written and indicated to exchange OTAP Commands between the OTAP Server and the OTAP
     * Client. Data chunks are not transferred using this characteristic.
     */
    String OTAP_CONTROL_POINT_CHARACTERISTIC = "01ff5551-ba5e-f4ee-5ca1-eb1e5e4b1ce0";
    /**
     * can be
     * written without response by the OTAP Server to transfer image file data chunks to the OTAP
     * Client when an image block transfer is requested via the ATT transfer method. Data chunks can
     * also be transferred via the L2CAP credit based PSM method.
     */
    String OTAP_DATA_CHARACTERISTIC = "01ff5552-ba5e-f4ee-5ca1-eb1e5e4b1ce0";

    /*** ***/
    int MANUFACTURER_NAME = 0x2A29;
    int MODEL_NUMBER_STRING = 0x2A24;
    int SERIAL_NUMBER_STRING = 0x2A25;
    int HARDWARE_REV_STRING = 0x2A27;
    int SOFTWARE_REV_STRING = 0x2A28;
    int FIRMWARE_REV_STRING = 0x2A26;
    int SYSTEM_ID = 0x2A23;
    int REGULAR_CERT = 0x2A2A;
    int PNP_ID = 0x2A50;

    // Descriptors
    int CLIENT_CHARACTERISTIC_CONFIG = 0x2902;

    /**
     * Custom profiles and services
     */
    String CHAR_WUART_NOTIFY = "01FF0101-BA5E-F4EE-5CA1-EB1E5E4B1CE0";
    String SERVICE_WUART = "01FF0100-BA5E-F4EE-5CA1-EB1E5E4B1CE0";
    String CHAR_WUART_STREAM = "01FF0101-BA5E-F4EE-5CA1-EB1E5E4B1CE0";
    String SERVICE_OTAP = "01ff5550-ba5e-f4ee-5ca1-eb1e5e4b1ce0";
    String FRDM_UUID = "02ff5600-ba5e-f4ee-5ca1-eb1e5e4b1ce0";

    // FRDM LED
    String LED_SERVICE = "02FF5600-BA5E-F4EE-5CA1-EB1E5E4B1CE0";
    String LED_CHARACTERISTIC_STATUS = "02ff5700-ba5e-f4ee-5ca1-eb1e5e4b1ce0";
    String LED_CHARACTERISTIC_CONTROL = "02FF5701-BA5E-F4EE-5CA1-EB1E5E4B1CE0";

    // FRDM INPUT REPORT
    String INPUT_SERVICE = "02ff5601-ba5e-f4ee-5ca1-eb1e5e4b1ce0";
    String INPUT_CHARACTERISTIC = "02ff5702-ba5e-f4ee-5ca1-eb1e5e4b1ce0";
    int INPUT_CHARACTERISTIC_CONTROL = 0x2902;

    // FRDM BUZZER
    String BUZZER_SERVICE = "02ff5602-ba5e-f4ee-5ca1-eb1e5e4b1ce0";
    String BUZZER_CHARACTERISTIC = "02ff5703-ba5e-f4ee-5ca1-eb1e5e4b1ce0";

    // FRDM TEMPERATURE
    String TEMPERATURE_SERVICE = "02ff5603-ba5e-f4ee-5ca1-eb1e5e4b1ce0";
    int TEMPERATURE_CHARACTERISTIC = 0x2A6E;

    // FRDM TEMPERATURE

    String POTENTIONMETER_SERVICE = "02ff5604-ba5e-f4ee-5ca1-eb1e5e4b1ce0";
    String POTENTIONMETER_CHARACTERISTIC = "02ff5704-ba5e-f4ee-5ca1-eb1e5e4b1ce0";

    // FRDM ACCELEROMETER
    String ACCELEROMETER_SERVICE = "02FF5605-ba5e-f4ee-5ca1-eb1e5e4b1ce0";
    String ACCELEROMETER_CHARACTERISTIC_SCALE = "02ff5705-ba5e-f4ee-5ca1-eb1e5e4b1ce0";
    String ACCELEROMETER_CHARACTERISTIC_READING = "02ff5706-ba5e-f4ee-5ca1-eb1e5e4b1ce0";

    // FRDM COMPASS
    String COMPASS_SERVICE = "02ff5606-ba5e-f4ee-5ca1-eb1e5e4b1ce0";
    String COMPASS_CHARACTERISTIC = "02ff5707-ba5e-f4ee-5ca1-eb1e5e4b1ce0";

    // FRDM CONTROLLER
    String CONTROLLER_SERVICE = "02ff5607-ba5e-f4ee-5ca1-eb1e5e4b1ce0";
    String CONTROLLER_CHARACTERISTIC_COMMAND = "02ff5708-ba5e-f4ee-5ca1-eb1e5e4b1ce0";
    String CONTROLLER_CHARACTERISTIC_CONFIGURATION = "02ff5709-ba5e-f4ee-5ca1-eb1e5e4b1ce0";

    // SHELL
    String SHELL_UUID = "01FF0100-BA5E-F4EE-5CA1-EB1E5E4B1CE0";
    String SHELL_CHARACTERISTIC = "01FF0101-BA5E-F4EE-5CA1-EB1E5E4B1CE0";


    //QPP
    int QPP_UUID = 0xFEE9;
    String QPP_UUID_SERVICE = "0000FEE9-0000-1000-8000-00805F9B34FB";
    String QPP_WRITE_CHARACTERISTIC = "D44BC439-ABFD-45A2-B575-925416129600";
    String QPP_NOTIFY_CHARACTERISTIC = "D44BC439-ABFD-45A2-B575-925416129601";


    //Sensor
    int SENSER_STREAM = 0x1E10;
    int SENSER = 0x1EE9;
    String SENSOR_UUID = "01FF0100-BA5E-F4EE-5CA1-EB1E5E4B1CE0";
    String SENSOR_CHARACTERISTIC = "01FF0101-BA5E-F4EE-5CA1-EB1E5E4B1CE0";

    //    Zigbee
    int ZIGBEE = 0xffff;
}
