/**
 * Copyright 2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 *
 */

package com.smartlockmanager.activity;

import androidx.appcompat.widget.Toolbar;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.google.android.material.progressindicator.CircularProgressIndicator;
import com.smartlockmanager.R;
import com.smartlockmanager.event.BLEStateEvent;
import com.smartlockmanager.service.BLEService;
import com.smartlockmanager.utility.ForegroundObserver;
import com.smartlockmanager.utility.StatusPopUp;
import com.smartlockmanager.utility.UserInteractionTimer;

import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import static com.smartlockmanager.utility.SdkUtils.fullScreen;

public class AuthenticationActivity extends BaseServiceActivity {
    /**
     * Tag for the {@link Log}.
     */
    private static final String TAG = "SLM_RA";
    private static EditText password = null;

    public static final int PASSWORD_LENGTH = 6;
    private static final int REQUEST_CODE = 100;

    public static final int MTU_WUART = 247;
    private int currentMTU = 0;

    private static final int AUTHENTICATION_RESULT_OK = 0;
    private static final int AUTHENTICATION_RESULT_INVALID = -1;
    public final byte INVALID_PACKET = -2;

    private Intent pendingIntent;
    LinearLayout progressbar;

    private boolean onBackButtonPressed = false;

    @Override
    public void onEventMainThread(BLEStateEvent.Connecting e) {
        super.onEventMainThread(e);
        StatusPopUp.getStatusPopUpInstance().showProgress(
                this, findViewById(R.id.authentication_view), getString(R.string.state_connecting));
    }

    @Override
    public void onEventMainThread(BLEStateEvent.Connected e) {
        super.onEventMainThread(e);
        StatusPopUp.getStatusPopUpInstance().dismiss(this);
    }

    @Override
    public void onEventMainThread(BLEStateEvent.Disconnected e) {
        super.onEventMainThread(e);
        if(!onBackButtonPressed) {
            NoConnectionActivity.jumpToDisconnectActivity(this);
        }
    }

    @Override
    @Subscribe(threadMode =ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.DataAvailable e) {
        super.onEventMainThread(e);
        if (e == null) return;

        // request mtu start
        if (currentMTU == 0) {
            BLEService.INSTANCE.requestMTU(MTU_WUART);
            currentMTU = MTU_WUART;
        }
        // request mtu end
    }

    @Subscribe
    public void onEvent(BLEStateEvent.MTUUpdated mtuUpdated) {
        Log.d(TAG, "mtuUpdated = " + mtuUpdated.mtuSize + " success " + mtuUpdated.success);
        // request mtu start
        if (currentMTU != mtuUpdated.mtuSize) {
            BLEService.INSTANCE.requestMTU(mtuUpdated.mtuSize);
            currentMTU = mtuUpdated.mtuSize;
        }
        // request mtu end
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "+onCreate");

        super.onCreate(savedInstanceState);

        fullScreen(getWindow());

        setContentView(R.layout.activity_authentication);

        setSupportActionBar((Toolbar) findViewById(R.id.toolbar));
        getSupportActionBar().setDisplayShowTitleEnabled(false);
        ((TextView) findViewById(R.id.toolbar_title)).setText(getString(R.string.authentication));
        ((TextView) findViewById(R.id.toolbar_subtitle)).setText("");

        String login_username_text = getString(R.string.login_prefix) + " "
                + getIntent().getExtras().getString(BaseServiceActivity.INTENT_KEY_NAME) + " "
                + getString(R.string.login_suffix);

        ((TextView) findViewById(R.id.login_text)).setText(login_username_text);

        password = findViewById(R.id.authentication_password);

        progressbar = findViewById(R.id.progressbar);

        CircularProgressIndicator circularProgressIndicator = findViewById(R.id.toolbar_loading);
        circularProgressIndicator.setVisibility(View.GONE);

        TextView fab_name = findViewById(R.id.toolbar_loading_text);
        ImageView fab_image = findViewById(R.id.toolbar_loading_static);

        fab_image.setImageDrawable(getDrawable(R.drawable.ic_baseline_login_24));
        fab_image.setVisibility(View.VISIBLE);
        fab_name.setText(getString(R.string.login));

        // automatically perform connection request
        BLEService.INSTANCE.init(getApplicationContext());
        if (TextUtils.isEmpty(mDeviceAddress)) {
            //throw new NullPointerException("Invalid Bluetooth MAC Address");
            toggleState(false);
        } else {
            toggleState(true);
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        onBackButtonPressed = false;
        ForegroundObserver.getForegroundObserver().updateActivity(this);
        UserInteractionTimer.getTimerInstance().startTimer(this);
    }

    @Override
    public void onUserInteraction() {
        super.onUserInteraction();
        UserInteractionTimer.getTimerInstance().resetTimer();
    }

    @Override
    protected void onPause() {
        super.onPause();
        UserInteractionTimer.getTimerInstance().stopTimer();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if (requestCode == REQUEST_CODE) {
            if (resultCode == RESULT_OK) {
                onBackButtonPressed = true;
                releaseConnection();
                finish();
            }
        }
    }

    public void onBackFABPressed(View view) {
        onBackButtonPressed = true;
        releaseConnection();
        finish();
    }

    @Override
    public void onBackPressed() {
        onBackButtonPressed = true;
        releaseConnection();
        finish();
    }

    public void jumpToActivity(Context currentActivity, Class nextActivityClass){
        pendingIntent = new Intent();
        pendingIntent.setClass(currentActivity, nextActivityClass);

        pendingIntent.putExtra(BaseServiceActivity.INTENT_KEY_PHY, mPhy);
        pendingIntent.putExtra(BaseServiceActivity.INTENT_KEY_ADDRESS, mDeviceAddress);
        pendingIntent.putExtra(BaseServiceActivity.INTENT_KEY_NAME, mDeviceName);

        startActivityForResult(pendingIntent, REQUEST_CODE);
    }

    public void onPressFABCardView(View view) {
        Log.d(TAG, "+onClickOk");

        String s_password = password.getText().toString();
        if (s_password.length() != PASSWORD_LENGTH){
            StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                    this, findViewById(R.id.authentication_view), getString(R.string.error_incorrect_password));
        }
        else {
            s_password = password.getText().toString();
            BLEService.INSTANCE.sendAuthenticationReq(s_password);

            StatusPopUp.getStatusPopUpInstance().showProgress(
                    this, findViewById(R.id.authentication_view), getString(R.string.state_authenticating));
        }
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.AuthenticationRes e) {
        Log.d(TAG, "+AuthenticationRes");

        if (e == null) return;

        switch (e.mAuthenticationResult){
            case AUTHENTICATION_RESULT_OK:
                SharedPreferences sharedPref = AuthenticationActivity.
                        this.getSharedPreferences(getString(R.string.password_shared_pref), Context.MODE_PRIVATE);
                SharedPreferences.Editor editor = sharedPref.edit();
                editor.putString(getString(R.string.password), password.getText().toString());
                editor.apply();

                StatusPopUp.getStatusPopUpInstance().dismiss(this);

                jumpToActivity(AuthenticationActivity.this, SmartLockActivity.class);
                break;
            case AUTHENTICATION_RESULT_INVALID:
                StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                        this, findViewById(R.id.authentication_view), getString(R.string.error_wrong_password));
                break;
            case INVALID_PACKET:
                StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                        this, findViewById(R.id.authentication_view), getString(R.string.error_invalid_packet));
                break;
            default:
                StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                        this, findViewById(R.id.authentication_view), getString(R.string.error_general));
        }


        Log.d(TAG, "-AuthenticationRes:" + e.mAuthenticationResult);
    }
}