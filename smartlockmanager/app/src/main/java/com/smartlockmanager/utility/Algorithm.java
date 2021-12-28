/**
 * Copyright 2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 *
 */

package com.smartlockmanager.utility;

import android.util.Log;

public class Algorithm {
    /**
     * Tag for the {@link Log}.
     */
    private static final String TAG = "FMD_AG";

    static {
        System.loadLibrary("JniAlgorithm");
    }

    public native int Init(int appType);
    public native int Exit();
    public native int GetFaceSize();
    public native int GetVersion();
    public native int Registration(byte[] data, int width , int height, int[] box, byte[] feature);
}
