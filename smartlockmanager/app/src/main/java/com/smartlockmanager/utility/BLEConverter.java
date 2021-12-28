/**
 * Copyright 2016 Freescale Semiconductors, Inc.
 */

package com.smartlockmanager.utility;

import android.util.Log;

import androidx.annotation.NonNull;

import com.smartlockmanager.model.BLEAttributes;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Locale;
import java.util.UUID;

public class BLEConverter {

    private static final char[] HEX_ARRAY = "0123456789ABCDEF".toCharArray();

    private BLEConverter() {
    }

    /**
     * Utility method to convert from byte to string presentation.
     *
     * @param bytes
     * @return
     */
    public static String bytesToHex(byte[] bytes) {
        char[] hexChars = new char[bytes.length * 2];
        for (int j = 0; j < bytes.length; j++) {
            int v = bytes[j] & 0xFF;
            hexChars[j * 2] = HEX_ARRAY[v >>> 4];
            hexChars[j * 2 + 1] = HEX_ARRAY[v & 0x0F];
        }
        return new String(hexChars);
    }

    /**
     * Convert 16 bytes array to UUID.
     *
     * @param bytes
     * @return
     */
    public static String uuidFromByteArray(byte[] bytes) {
        ByteBuffer bb = ByteBuffer.wrap(bytes);
        UUID uuid = new UUID(bb.getLong(), bb.getLong());
        return uuid.toString();
    }

    /**
     * Converts from an UUID to its assigned number.
     *
     * @param uuid
     * @return
     */
    public static int getAssignedNumber(@NonNull UUID uuid) {
        return (int) ((uuid.getMostSignificantBits() & 0x0000FFFF00000000L) >> 32);
    }

    /**
     * Constructs an UUID string from an assigned number.
     *
     * @param number
     * @return
     */
    public static String stringFromAssignedNumber(int number) {
        if (number == BLEAttributes.WUART) {
            return BLEAttributes.SERVICE_WUART;
        }
        if (number == BLEAttributes.UART_STREAM) {
            return BLEAttributes.CHAR_WUART_STREAM;
        }
        if (number == BLEAttributes.UART_NOTIFY) {
            return BLEAttributes.CHAR_WUART_NOTIFY;
        }
        if (number == BLEAttributes.OTAP) {
            return BLEAttributes.SERVICE_OTAP;
        }
        if (number == BLEAttributes.OTAP_CONTROL) {
            return BLEAttributes.OTAP_CONTROL_POINT_CHARACTERISTIC;
        }
        if (number == BLEAttributes.OTAP_DATA) {
            return BLEAttributes.OTAP_DATA_CHARACTERISTIC;
        }
        if (number == BLEAttributes.FRDM) {
            return BLEAttributes.FRDM_UUID;
        }
        if (number == BLEAttributes.SHELL) {
            return BLEAttributes.SHELL_UUID;
        }
        if (number == BLEAttributes.SENSER){
            return  BLEAttributes.SENSOR_UUID;
        }
        if (number == BLEAttributes.SENSER_STREAM){
            return  BLEAttributes.SENSOR_CHARACTERISTIC;
        }
        if(number == BLEAttributes.QPP_UUID){
            return BLEAttributes.QPP_UUID_SERVICE;
        }

        String assignedNumber = String.format("%4x", number);
        return String.format(BLEAttributes.UUID_FORM, assignedNumber);
    }

    /**
     * Constructs an UUID from an assigned number.
     *
     * @param number
     * @return
     */
    public static UUID uuidFromAssignedNumber(int number) {
        return UUID.fromString(stringFromAssignedNumber(number));
    }

    /**
     * Refer to http://stackoverflow.com/questions/18019161/ for supporting BLE scanning with 128-bit UUID.
     *
     * @param advertisedData
     * @return
     */
    public static List<UUID> uuidsFromScanRecord(byte[] advertisedData) {
        if (advertisedData == null) {
            return Collections.emptyList();
        }
        List<UUID> uuids = new ArrayList<>();

        ByteBuffer buffer = ByteBuffer.wrap(advertisedData).order(ByteOrder.LITTLE_ENDIAN);
        while (buffer.remaining() > 2) {
            byte length = buffer.get();
            if (length == 0) break;

            byte type = buffer.get();
            switch (type) {
                case 0x02: // Partial list of 16-bit UUIDs
                case 0x03: // Complete list of 16-bit UUIDs
                    while (length >= 2) {
                        uuids.add(UUID.fromString(String.format(
                                "%08x-0000-1000-8000-00805f9b34fb", buffer.getShort())));
                        length -= 2;
                    }
                    break;

                case 0x06: // Partial list of 128-bit UUIDs
                case 0x07: // Complete list of 128-bit UUIDs
                    while (length >= 16) {
                        long lsb = buffer.getLong();
                        long msb = buffer.getLong();
                        uuids.add(new UUID(msb, lsb));
                        length -= 16;
                    }
                    break;

                default:
                    try{
                    buffer.position(buffer.position() + length - 1);
                    }catch (Exception e){
                        Log.e("data",bytes2HexStr(advertisedData)+"/position: / "+buffer.position()+"/length: /"+length);
                        //2019-07-26 13:47:50.889 6003-6003/com.freescale.kinetisbletoolbox E/data: 02011009534E42432D563534304C2D30383630/position: / 15/length: /76
                        e.printStackTrace();
//                        System.exit(2);
                    }
                    break;
            }
        }
        
        return uuids;
    }

    /**
     * Converts bodySensorLocation constants to their counterpart values.
     *
     * @param bodySensorLocation
     * @return
     */
    public static String fromBodySensorLocation(int bodySensorLocation) {
        switch (bodySensorLocation) {
            case 0:
                return "Other";
            case 1:
                return "Chest";
            case 2:
                return "Wrist";
            case 3:
                return "Finger";
            case 4:
                return "Hand";
            case 5:
                return "Ear Lobe";
            case 6:
                return "Foot";
            default:
                return "Unknown";
        }
    }

    /**
     * Converts type constants to their counterpart values.
     *
     * @param type
     * @return
     */
    public static String fromTemperatureType(int type) {
        switch (type) {
            case 1:
                return "Armpit";
            case 2:
                return "Body";
            case 3:
                return "Ear";
            case 4:
                return "Finger";
            case 5:
                return "Gastro-Intestinal";
            case 6:
                return "Mouth";
            case 7:
                return "Rectum";
            case 8:
                return "Toe";
            case 9:
                return "Tympanum";
            default:
                return "Unknown";
        }
    }

    /**
     * Converts sensorLocation constants to their counterpart values.
     *
     * @param sensorLocation
     * @return
     */
    public static String fromSensorLocation(int sensorLocation) {
        switch (sensorLocation) {
            case 0:
                return "Other";
            case 1:
                return "Top Of Shoe";
            case 2:
                return "In Shoe";
            case 3:
                return "Hip";
            case 4:
                return "Front Wheel";
            case 5:
                return "Left Crank";
            case 6:
                return "Right Crank";
            case 7:
                return "Left Pedal";
            case 8:
                return "Right Pedal";
            case 9:
                return "Front Hub";
            case 10:
                return "Rear Dropout";
            case 11:
                return "Chainstay";
            case 12:
                return "Rear Wheel";
            case 13:
                return "Rear Hub";
            case 14:
                return "Chest";
            default:
                return "Unknown";
        }
    }

    public static byte[] convertLittleEndian(byte[] byteArray) {
        byte[] result = new byte[byteArray.length];
        for (int i = 0; i < byteArray.length; i++) {
            result[byteArray.length - 1 - i] = byteArray[i];
        }
        return result;
    }

    public static String readStringFromByte(byte[] charBytes) {
        StringBuilder sb = new StringBuilder();
        for (byte charB : charBytes) {
            // 0x00 ==> null
            if (0x00 != charB) sb.append((char) charB);
        }
        return sb.toString();
    }

    public static void hexStrToByteArr(@NonNull String hexStr, @NonNull byte[] dst, int dstStart) {
        StringBuilder sb = new StringBuilder(hexStr);
        if (0 != (hexStr.length() % 2)) {
            sb.insert(0, 0);
        }
        for (int i = 0; i < sb.length(); i += 2) {
            int j = i / 2;
            dst[j + dstStart] = (byte) Short.parseShort(sb.substring(i, i + 2), 16);
        }
    }

    public static String bytesToHexWithSpace(byte[] data) {
        StringBuilder result = new StringBuilder();
        for (byte each : data) {
            result.append(String.format("%02X ", each));
        }
        return result.toString();
    }

    public static String bytes2HexStr(byte[] buf) {
        if (buf == null)
            return "";
        StringBuffer result = new StringBuffer(2 * buf.length);
        for (int i = 0; i < buf.length; i++) {
            appendHex(result, buf[i]);
        }
        return result.toString();
    }
    private static void appendHex(StringBuffer sb, byte b) {
        sb.append(String.valueOf(HEX_ARRAY).charAt((b >> 4) & 0x0f)).append(String.valueOf(HEX_ARRAY).charAt(b & 0x0f));
    }

    public static String shortTo4HexStrLitleEndial(short value) {
        byte[] byteData = shortToByteArray(value);
        byteData = convertLittleEndian(byteData);
        String data = bytesToHex(byteData);
        StringBuilder result = new StringBuilder("0000");
        result.replace(4 - data.length(), data.length(), data);
        return result.toString().toUpperCase();
    }

    public static String intTo6HexStrLitleEndial(int value) {
        if (0xFFFFFF < value | 0x000000 > value) throw new IllegalArgumentException("value out of bound");
        byte[] byteData = intToByteArray(value);
        byteData = convertLittleEndian(byteData);
        String data = bytesToHex(byteData);
        StringBuilder result = new StringBuilder("000000");
        result.replace(8 - data.length(), data.length(), data);
        return result.toString().toUpperCase();
    }

    public static String intTo8HexStrLitleEndial(int value) {
        byte[] byteData = intToByteArray(value);
        byteData = convertLittleEndian(byteData);
        String data = bytesToHex(byteData);
        StringBuilder result = new StringBuilder("00000000");
        result.replace(8 - data.length(), data.length(), data);
        return result.toString().toUpperCase();
    }

    public static String longTo16HexStrLitleEndial(long value) {
        byte[] byteData = longToByteArray(value);
        byteData = convertLittleEndian(byteData);
        String data = bytesToHex(byteData);
        StringBuilder result = new StringBuilder("0000000000000000");
        result.replace(16 - data.length(), data.length(), data);
        return result.toString().toUpperCase();
    }

    public static final byte[] shortToByteArray(short value) {
        return new byte[]{
                (byte) (value >>> 8),
                (byte) value};
    }

    public static final byte[] intToByteArray(int value) {
        return new byte[]{
                (byte) (value >>> 24),
                (byte) (value >>> 16),
                (byte) (value >>> 8),
                (byte) value};
    }

    public static final byte[] longToByteArray(long value) {
        return new byte[]{
                (byte) (value >>> 56),
                (byte) (value >>> 48),
                (byte) (value >>> 40),
                (byte) (value >>> 32),
                (byte) (value >>> 24),
                (byte) (value >>> 16),
                (byte) (value >>> 8),
                (byte) value};
    }

    public static String convertByteToBinaryString(byte b) {
        return String.format("%8s", Integer.toBinaryString(b & 0xFF)).replace(' ', '0');
    }

    public static byte[] hexStr2Bytes(String hexString) {
		/*
		 * int len = paramString.length()/2; byte[] mbytes = new byte[len]; for(int i=0;i<len;i++){ mbytes[i] =
		 * (byte)Integer.parseInt(paramString.substring(i*2, i*2+2), 16); } return mbytes;
		 */

        if (hexString == null || hexString.isEmpty()) {
            return null;
        }

        hexString = hexString.toUpperCase(Locale.US);

        int length = hexString.length() >> 1;
        char[] hexChars = hexString.toCharArray();

        int i = 0;

//        do {
//            int checkChar = String.valueOf(HEX_ARRAY).indexOf(hexChars[i]);
//
//            if (checkChar == -1)
//                return null;
//            i++;
//        } while (i < hexString.length());

        byte[] dataArr = new byte[length];

        for (i = 0; i < length; i++) {
            int strPos = i * 2;

            dataArr[i] = (byte) (charToByte(hexChars[strPos]) << 4 | charToByte(hexChars[strPos + 1]));
        }

        return dataArr;
    }
    private static byte charToByte(char c) {
        return (byte) String.valueOf(HEX_ARRAY).indexOf(c);
    }
}
