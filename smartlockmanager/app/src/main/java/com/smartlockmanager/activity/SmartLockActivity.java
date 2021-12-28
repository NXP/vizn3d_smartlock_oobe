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

import android.Manifest;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;
import androidx.cardview.widget.CardView;
import androidx.fragment.app.Fragment;
import androidx.viewpager2.widget.ViewPager2;

import com.google.android.material.floatingactionbutton.ExtendedFloatingActionButton;
import com.google.android.material.tabs.TabLayout;
import com.google.android.material.tabs.TabLayoutMediator;
import com.smartlockmanager.R;
import com.smartlockmanager.event.BLEStateEvent;
import com.smartlockmanager.service.BLEService;
import com.smartlockmanager.utility.Algorithm;
import com.smartlockmanager.utility.SdkUtils;
import com.smartlockmanager.utility.StatusPopUp;
import com.smartlockmanager.utility.UserInteractionTimer;
import com.smartlockmanager.view.AboutDialog;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.Objects;

import butterknife.OnClick;

import static android.content.pm.PackageManager.PERMISSION_GRANTED;
import static androidx.viewpager2.widget.ViewPager2.ORIENTATION_HORIZONTAL;
import static com.smartlockmanager.utility.SdkUtils.fullScreen;

public class SmartLockActivity extends AppCompatActivity {
    /**
     * Tag for the {@link Log}.
     */
    private static final String TAG = "SLM_SLA";

    private final int REQUEST_PERMISSION_CODE = 2710;
    public final int REQUEST_CODE_REGISTRATION = 100;
    public final int REQUEST_CODE_CHANGING_PASSWORD = 200;
    public final int NO_REQUEST_CODE = 0;

    public static final int REGISTRATION_RESULT_DUPLICATE = 1;
    public static final int REGISTRATION_RESULT_INVALID = 2;
    public static final int INVALID_PACKET = -2;
    public static final int GENERAL_ERROR = -3;

    private final int resultCode = RESULT_CANCELED;
    private final int requestCode = -1;

    public boolean isSyncing = true;
    public boolean userChange = false;

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.Disconnected e) {
        NoConnectionActivity.jumpToDisconnectActivity(this);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        fullScreen(getWindow());

        setContentView(R.layout.activity_smart_lock);

        setSupportActionBar((Toolbar) findViewById(R.id.toolbar));
        Objects.requireNonNull(getSupportActionBar()).setDisplayShowTitleEnabled(false);

        String activity_title = getIntent().getExtras().getString(BaseServiceActivity.INTENT_KEY_NAME)
                + " " + getString(R.string.upper_board_suffix);

        ((TextView) findViewById(R.id.toolbar_title))
                .setText(activity_title);
        ((TextView) findViewById(R.id.toolbar_subtitle)).setText("");

        String[] titles = { "Users", "Commands"};

        TabLayout tabLayout = findViewById(R.id.tab_layout);

        ViewPager2 viewPager = findViewById(R.id.view_pager);
        viewPager.setUserInputEnabled(true);
        viewPager.setOrientation(ORIENTATION_HORIZONTAL);
        viewPager.setAdapter(new MainPagerAdapter(this));
        new TabLayoutMediator(tabLayout, viewPager,
                (tab, position) -> tab.setText(titles[position])
        ).attach();

        viewPager.registerOnPageChangeCallback(new ViewPager2.OnPageChangeCallback() {
            @Override
            public void onPageSelected(int position) {
                super.onPageSelected(position);

                CardView sync_fab = findViewById(R.id.sync_fab_cardview);
                ExtendedFloatingActionButton add_user_fab = findViewById(R.id.add_user_fab);

                if (position == 0) {
                    add_user_fab.animate().scaleX(1.0f).scaleY(1.0f).setDuration(300).start();
                    sync_fab.animate().scaleX(1.0f).scaleY(1.0f).setDuration(300).start();
                    add_user_fab.setVisibility(View.VISIBLE);
                    sync_fab.setVisibility(View.VISIBLE);
                }else if (position == 1) {
                    sync_fab.animate().scaleX(0.0f).scaleY(0.0f).setDuration(300).start();
                    add_user_fab.animate().scaleX(0.0f).scaleY(0.0f).setDuration(300).start();
                    add_user_fab.setVisibility(View.GONE);
                    sync_fab.setVisibility(View.GONE);
                }
            }
        });

        EventBus.getDefault().register(this);

        BLEService.INSTANCE.sendGetAlgoVersionReq();
    }

    @Override
    protected void onResume() {
        super.onResume();
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
        EventBus.getDefault().unregister(this);
        UserInteractionTimer.getTimerInstance().stopTimer();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        Fragment user_mgmt_fragment = getSupportFragmentManager().findFragmentByTag("f0");
        if (user_mgmt_fragment != null) {
            user_mgmt_fragment.onActivityResult(requestCode, resultCode, data);
        }

        switch (requestCode) {
            case REQUEST_CODE_REGISTRATION:
                if (resultCode == RESULT_OK) {
                    StatusPopUp.getStatusPopUpInstance().showSuccessPopUp(
                            this, findViewById(R.id.smartlock_view), getString(R.string.success_registration));
                } else if (resultCode == RESULT_CANCELED) {
//                    StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
//                            this, findViewById(R.id.smartlock_view), getString(R.string.success_registration_cancelled));
                } else if (resultCode == REGISTRATION_RESULT_DUPLICATE) {
                    StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                            this, findViewById(R.id.smartlock_view), getString(R.string.error_duplicate));
                } else if (resultCode == INVALID_PACKET) {
                    StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                            this, findViewById(R.id.smartlock_view), getString(R.string.error_invalid_packet));
                } else {
                    StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                            this, findViewById(R.id.smartlock_view), getString(R.string.error_general));
                }
                break;
            case REQUEST_CODE_CHANGING_PASSWORD:
                if (resultCode == RESULT_OK) {
                    StatusPopUp.getStatusPopUpInstance().showSuccessPopUp(
                            this, findViewById(R.id.smartlock_view), getString(R.string.success_password));
                }
                break;
        }
    }

    @OnClick(R.id.status_view_info)
    public void viewAboutInfo() {
        AboutDialog.newInstance(this).show();
    }

    private void setResultToSkipAuthActivity() {
        Intent intent = new Intent();
        if (getParent() == null) {
            setResult(AuthenticationActivity.RESULT_OK, intent);
        } else {
            getParent().setResult(AuthenticationActivity.RESULT_OK, intent);
        }
    }

    @Override
    public void onBackPressed() {
        setResultToSkipAuthActivity();
        finish();
    }

    public void onBackFABPressed(View view) {
        setResultToSkipAuthActivity();
        finish();
    }

    public void onPressFABCardView(View view) {
        Log.d(TAG, "+onClickOk");

        if(isSyncing){
            isSyncing = false;
            SdkUtils.changeToolbarFABButtonState(this, R.string.menu_scan_stop, R.string.sync, isSyncing);
        }
        else {
            isSyncing = true;
            BLEService.INSTANCE.sendGetUserInfoReq();
            SdkUtils.changeToolbarFABButtonState(this, R.string.menu_scan_stop, R.string.sync, isSyncing);
        }
    }


    public void onClickAddUser(View v) {
        // if OS >= 6 -> need ask permission access device's location
        if (SdkUtils.hasMarshmallow()) {
            if (PERMISSION_GRANTED != checkSelfPermission(Manifest.permission.ACCESS_FINE_LOCATION)) {
                // handle "Never ask again"
                if (!shouldShowRequestPermissionRationale(Manifest.permission.ACCESS_FINE_LOCATION)) {
                    showMessageOKCancel(getString(R.string.grant_permission),
                            new DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    String[] permissions = {Manifest.permission.ACCESS_FINE_LOCATION};
                                    requestPermissions(permissions, REQUEST_PERMISSION_CODE);
                                }
                            });
                    return;
                }

                String[] permissions = {Manifest.permission.ACCESS_FINE_LOCATION};
                requestPermissions(permissions, REQUEST_PERMISSION_CODE);
            } else {
                jumpToActivity(SmartLockActivity.this, RegistrationActivity.class, REQUEST_CODE_REGISTRATION);
            }
        } else {
            jumpToActivity(SmartLockActivity.this, RegistrationActivity.class, REQUEST_CODE_REGISTRATION);
        }
    }

    private void showMessageOKCancel(String message, DialogInterface.OnClickListener okListener) {
        new AlertDialog.Builder(SmartLockActivity.this)
                .setMessage(message)
                .setPositiveButton("OK", okListener)
                .setNegativeButton("Cancel", null)
                .create()
                .show();
    }

    private void showMessageOK(String message) {
        new AlertDialog.Builder(SmartLockActivity.this)
                .setMessage(message)
                .setPositiveButton("OK", null)
                .create()
                .show();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);

        Log.e("- - - - "," requestCode   :" + requestCode + " permissions   :" +  String.valueOf(permissions) + " grantResults   :" + String.valueOf(grantResults));

        for (int i = 0 ; i < grantResults.length; i++){
            Log.e("- - - - "," = = = = =  = =    :" + grantResults[i]);
        }
        if (REQUEST_PERMISSION_CODE == requestCode) {
            boolean grantedAccessCoarseLocation = true;
            for (int i = 0; i < permissions.length; i++) {
                String permission = permissions[i];
                if (Manifest.permission.ACCESS_COARSE_LOCATION.equals(permission)) {
                    grantedAccessCoarseLocation = grantResults[i] == PERMISSION_GRANTED;
                }
            }

            if (!grantedAccessCoarseLocation) {
                 Toast.makeText(this, "Please grant permissions", Toast.LENGTH_SHORT).show();
            }
        }
    }

    public void jumpToActivity(Context currentActivity, Class nextActivityClass, int requestCode){
        Intent pendingIntent = new Intent();
        pendingIntent.setClass(currentActivity, nextActivityClass);

        pendingIntent.putExtra(BaseServiceActivity.INTENT_KEY_PHY, getIntent().getExtras().getInt(BaseServiceActivity.INTENT_KEY_PHY));
        pendingIntent.putExtra(BaseServiceActivity.INTENT_KEY_ADDRESS, getIntent().getExtras().getString(BaseServiceActivity.INTENT_KEY_ADDRESS));
        pendingIntent.putExtra(BaseServiceActivity.INTENT_KEY_NAME, getIntent().getExtras().getString(BaseServiceActivity.INTENT_KEY_NAME));

        if (requestCode == NO_REQUEST_CODE)
            startActivity(pendingIntent);
        else
            startActivityForResult(pendingIntent, requestCode);
    }

    private void treatReturnValue(int Result) {
        int algoVersion = new Algorithm().GetVersion();
        EventBus.getDefault().unregister(this);

        if (Result != algoVersion) {
            StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                    this, findViewById(R.id.smartlock_view), getString(R.string.error_algo_version_mismatch));
        }

        BLEService.INSTANCE.sendGetUserInfoReq();
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.GetAlgoVersionRes e) {
        Log.d(TAG, "+GetAlgoVersionRes");

        if (e == null) return;

        treatReturnValue(e.mGetAlgoVersionResult);

        Log.d(TAG, "-GetAlgoVersionRes:" + e.mGetAlgoVersionResult);
    }
}
