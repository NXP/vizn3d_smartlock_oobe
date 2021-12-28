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
import android.graphics.Color;
import android.os.Handler;
import android.os.Looper;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.widget.LinearLayout;
import android.widget.PopupWindow;
import android.widget.TextView;

import com.google.android.material.snackbar.Snackbar;
import com.smartlockmanager.R;


import static android.content.Context.LAYOUT_INFLATER_SERVICE;

public class StatusPopUp {

    private final float DEFOCUS_STRENGTH = 0.8f;
    private final long TOTAL_TIME = 1000;
    private final long TOTAL_TIME_PROGRESS = 20 * 1000;
    private final int TEXT_VIEW_PADDING = 3;

    private PopupWindow popupWindow = null;

    private Handler handler = new Handler(Looper.getMainLooper());

    private static volatile StatusPopUp STATUSPOPUP_INSTANCE;

    public static StatusPopUp getStatusPopUpInstance() {
        if (STATUSPOPUP_INSTANCE == null) {
            synchronized (StatusPopUp.class) {
                if (STATUSPOPUP_INSTANCE == null) {
                    STATUSPOPUP_INSTANCE = new StatusPopUp();
                }
            }
        }
        return STATUSPOPUP_INSTANCE;
    }

    public static void hideKeyboard(Activity activity) {
        InputMethodManager imm = (InputMethodManager) activity.getSystemService(Activity.INPUT_METHOD_SERVICE);
        //Find the currently focused view, so we can grab the correct window token from it.
        View view = activity.getCurrentFocus();
        //If no view currently has focus, create a new one, just so we can grab a window token from it
        if (view == null) {
            view = new View(activity);
        }
        imm.hideSoftInputFromWindow(view.getWindowToken(), 0);
    }

    private void defocusEffect(Activity activity, float alpha){
        WindowManager.LayoutParams lp = activity.getWindow().getAttributes();
        lp.alpha = alpha;
        activity.getWindow().addFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
        activity.getWindow().setAttributes(lp);
    }

    private int inDP(Activity activity, int value){
        float scale = activity.getResources().getDisplayMetrics().density;
        return (int) (value*scale + 0.5f);
    }


    private void cleanScreen(Activity activity){
        dismiss(activity);
        hideKeyboard(activity);
    }

    public void showErrorSnackBar(Activity activity, View parentView, String text){
        TextView textView;

        cleanScreen(activity);

        Snackbar snackbar = Snackbar.make(parentView, "", Snackbar.LENGTH_LONG);
        //inflate view
        View snackView = activity.getLayoutInflater().inflate(R.layout.progress_bar, null);

        snackbar.getView().setBackgroundColor(Color.WHITE);

        ((ViewGroup) snackbar.getView()).removeAllViews();
        ((ViewGroup) snackbar.getView()).addView(snackView);

        snackView.findViewById(R.id.progress_indicator).setVisibility(View.GONE);

        textView = ((TextView) snackView.findViewById(R.id.text_progressBar));
        textView.setTextColor(activity.getResources().getColor(R.color.md_edittext_error));
        textView.setText(text);

        snackbar.show();
    }

    public void showSuccessPopUp(Activity activity, View parentView, String text) {
        TextView textView;

        if (popupWindow == null) {
            cleanScreen(activity);

            // inflate the layout of the popup window
            LayoutInflater inflater = (LayoutInflater)
                    activity.getSystemService(LAYOUT_INFLATER_SERVICE);
            View popupView = inflater.inflate(R.layout.progress_bar, null);

            popupWindow = new PopupWindow(popupView, LinearLayout.LayoutParams.MATCH_PARENT,
                    LinearLayout.LayoutParams.WRAP_CONTENT, true);
            popupWindow.setAnimationStyle(R.style.popup_window_animation);

            popupWindow.setElevation(6);
        }

        defocusEffect(activity, 1.0f);

        popupWindow.getContentView().findViewById(R.id.progress_indicator).setVisibility(View.GONE);

        textView = ((TextView) popupWindow.getContentView().findViewById(R.id.text_progressBar));
        textView.setPadding(0,inDP(activity, TEXT_VIEW_PADDING),0,inDP(activity, TEXT_VIEW_PADDING));
        textView.setTextColor(activity.getResources().getColor(R.color.green));
        textView.setText(text);

        popupWindow.setOnDismissListener(() -> defocusEffect(activity, 1.0f));

        parentView.post(new Runnable() {
            public void run() {
                // show the popup window
                if (popupWindow != null) {
                    popupWindow.showAtLocation(
                            parentView,
                            Gravity.BOTTOM,
                            0,
                            0);

                    handler.postDelayed(new Runnable() {
                        @Override
                        public void run() {
                            dismiss(activity);
                        }
                    }, TOTAL_TIME);
                }
            }
        });
    }

    public void showErrorPopUp(Activity activity, View parentView, String text) {
        TextView textView;

        if (popupWindow == null) {
            cleanScreen(activity);

            // inflate the layout of the popup window
            LayoutInflater inflater = (LayoutInflater)
                    activity.getSystemService(LAYOUT_INFLATER_SERVICE);
            View popupView = inflater.inflate(R.layout.progress_bar, null);

            popupWindow = new PopupWindow(popupView, LinearLayout.LayoutParams.MATCH_PARENT,
                    LinearLayout.LayoutParams.WRAP_CONTENT, true);
            popupWindow.setAnimationStyle(R.style.popup_window_animation);

            popupWindow.setElevation(6);
        }

        defocusEffect(activity, 1.0f);

        popupWindow.getContentView().findViewById(R.id.progress_indicator).setVisibility(View.GONE);

        textView = ((TextView) popupWindow.getContentView().findViewById(R.id.text_progressBar));
        textView.setPadding(0,inDP(activity, TEXT_VIEW_PADDING),0,inDP(activity, TEXT_VIEW_PADDING));
        textView.setTextColor(activity.getResources().getColor(R.color.red));
        textView.setText(text);

        popupWindow.setOnDismissListener(() -> defocusEffect(activity, 1.0f));

        parentView.post(new Runnable() {
            public void run() {
                // show the popup window
                if (popupWindow != null) {
                    popupWindow.showAtLocation(
                            parentView,
                            Gravity.BOTTOM,
                            0,
                            0);

                    handler.postDelayed(new Runnable() {
                        @Override
                        public void run() {
                            dismiss(activity);
                        }
                    }, TOTAL_TIME);
                }
            }
        });
    }

    public void showProgress(Activity activity, View parentView, String text){
        cleanScreen(activity);

        // inflate the layout of the popup window
        LayoutInflater inflater = (LayoutInflater)
                activity.getSystemService(LAYOUT_INFLATER_SERVICE);
        View popupView = inflater.inflate(R.layout.progress_bar, null);

        popupWindow = new PopupWindow(popupView, LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT, true);
        popupWindow.setAnimationStyle(R.style.popup_window_animation);

        ((TextView) popupWindow.getContentView().findViewById(R.id.text_progressBar)).setText(text);
        ((TextView) popupWindow.getContentView().findViewById(R.id.text_progressBar)).setPadding(0,0,0,0);

        popupWindow.setElevation(6);
        popupWindow.setFocusable(false);
        popupWindow.setOutsideTouchable(false);

        defocusEffect(activity, DEFOCUS_STRENGTH);

        popupWindow.setOnDismissListener(() -> defocusEffect(activity, 1.0f));

        parentView.post(new Runnable() {
            public void run() {
                // show the popup window
                if (popupWindow != null) {
                    popupWindow.showAtLocation(
                            parentView,
                            Gravity.BOTTOM,
                            0,
                            0);

                    handler.postDelayed(new Runnable() {
                        @Override
                        public void run() {
                            if (popupWindow != null) {
                               showErrorPopUp(activity, parentView, activity.getString(R.string.error_timeout));
                            }
                        }
                    }, TOTAL_TIME_PROGRESS);
                }
            }
        });
    }

    public void dismiss(Activity activity){
        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (popupWindow != null && popupWindow.isShowing()) {
                    handler.removeCallbacksAndMessages(null);
                    popupWindow.dismiss();
                    popupWindow = null;
                }
            }
        });

    }
}
