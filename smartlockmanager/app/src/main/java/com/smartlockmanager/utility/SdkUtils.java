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

import android.app.Activity;
import android.os.Build;
import android.view.View;
import android.view.Window;
import android.view.WindowInsets;
import android.view.WindowInsetsController;
import android.widget.ImageView;
import android.widget.TextView;

import com.google.android.material.progressindicator.CircularProgressIndicator;
import com.smartlockmanager.R;

public class SdkUtils {

    private SdkUtils() {
    }

    public static boolean hasLollipop() {
        return android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.LOLLIPOP;
    }

    public static boolean hasMarshmallow() {
        return Build.VERSION.SDK_INT >= Build.VERSION_CODES.M;
    }

    public static boolean hasO() {
        return Build.VERSION.SDK_INT >= Build.VERSION_CODES.O;
    }

    public static void fullScreen(Window window){
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            window.getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN);

//            window.setDecorFitsSystemWindows(false);
//            WindowInsetsController controller = window.getInsetsController();
//            if (controller != null) {
//                controller.hide(WindowInsets.Type.statusBars() | WindowInsets.Type.navigationBars());
//                controller.setSystemBarsBehavior(WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
//            }
        } else{
            window.getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN);
        }
    }

    public static void changeToolbarFABButtonState(Activity activity, int loading_text, int static_text, boolean isLoading){
        CircularProgressIndicator progressWheel = activity.findViewById(R.id.toolbar_loading);
        ImageView imageView = activity.findViewById(R.id.toolbar_loading_static);
        TextView textView = activity.findViewById(R.id.toolbar_loading_text);

        if (isLoading){
            textView.setText(loading_text);
            imageView.setVisibility(View.GONE);
            progressWheel.setVisibility(View.VISIBLE);
        }else{
            textView.setText(static_text);
            progressWheel.setVisibility(View.GONE);
            imageView.setVisibility(View.VISIBLE);
        }
    }
}

