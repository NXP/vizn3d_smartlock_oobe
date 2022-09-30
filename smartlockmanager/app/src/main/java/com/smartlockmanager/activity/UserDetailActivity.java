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

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.PopupWindow;
import android.widget.TextView;
import android.widget.Toast;

import com.google.android.material.textfield.TextInputEditText;
import com.google.android.material.textfield.TextInputLayout;
import com.smartlockmanager.R;
import com.smartlockmanager.database.AppDatabase;
import com.smartlockmanager.database.UserDao;
import com.smartlockmanager.event.BLEStateEvent;
import com.smartlockmanager.service.BLEService;
import com.smartlockmanager.utility.StatusPopUp;
import com.smartlockmanager.utility.UserInteractionTimer;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import static com.smartlockmanager.database.AppDatabase.databaseWriteExecutor;
import static com.smartlockmanager.utility.SdkUtils.fullScreen;

import java.util.regex.Pattern;

public class UserDetailActivity extends AppCompatActivity {
    private static final String TAG = "SLM_RA";

    private static final Pattern namePattern = Pattern.compile("[a-zA-Z0-9]+", Pattern.CASE_INSENSITIVE);

    private static final int REQUEST_CODE_REGISTRATION = 100;
    public static final int REGISTRATION_RESULT_DUPLICATE = 1;

    private static final int USER_DETAIL_RESULT_OK = 0;
    private static final int USER_DETAIL_RESULT_INVALID = -1;
    public final byte INVALID_PACKET = -2;

    private static final int BUTTON_DELETE = 0;
    private static final int BUTTON_UPDATE = 1;
    private static final int BUTTON_REREGISTER = 2;

    private AppDatabase db;
    private UserDao userDao;
    private PopupWindow popupWindow;
    private int pressedButton;
    private Intent pendingIntent;

    private String user_id;
    private String user_name;

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.Disconnected e) {
        NoConnectionActivity.jumpToDisconnectActivity(this);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        fullScreen(getWindow());

        setContentView(R.layout.activity_user_detail);

        setSupportActionBar((Toolbar) findViewById(R.id.toolbar));
        getSupportActionBar().setDisplayShowTitleEnabled(false);
        ((TextView) findViewById(R.id.toolbar_title)).setText(getIntent().getExtras().getString(getString(R.string.intent_user_name)));

        String subtitle = "User ID is " + String.valueOf(getIntent().getExtras().getInt(getString(R.string.intent_user_id)));

        ((TextView) findViewById(R.id.toolbar_subtitle)).setText(subtitle);

        user_id = String.valueOf(getIntent().getExtras().getInt(getString(R.string.intent_user_id)));
        user_name = getIntent().getExtras().getString(getString(R.string.intent_user_name));
        
        db = AppDatabase.getDatabase(this);
        userDao = db.userDao();
    }

    @Override
    protected void onResume() {
        super.onResume();
        EventBus.getDefault().register(this);
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

        switch (requestCode) {
            case REQUEST_CODE_REGISTRATION:
                if (resultCode == RESULT_OK) {
                    StatusPopUp.getStatusPopUpInstance().showSuccessPopUp(
                            this, findViewById(R.id.user_detail_view), getString(R.string.success_reregistration));
                } else if (resultCode == RESULT_CANCELED) {
//                    StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
//                            this, findViewById(R.id.user_detail_view), getString(R.string.success_reregistration_cancelled));
                } else if (resultCode == REGISTRATION_RESULT_DUPLICATE) {
                    StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                            this, findViewById(R.id.user_detail_view), getString(R.string.error_duplicate));
                } else if (resultCode == INVALID_PACKET) {
                    StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                            this, findViewById(R.id.user_detail_view), getString(R.string.error_invalid_packet));
                } else {
                    StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                            this, findViewById(R.id.user_detail_view), getString(R.string.error_general));
                }
                break;
        }
    }

    private void setResultToActivity(int result, int type, String name) {
        Intent intent = new Intent();

        intent.putExtra(UserManagementFragment.USER_DETAIL_RETURN, type);
        if (type == UserManagementFragment.UPDATE_USER) {
            intent.putExtra(UserManagementFragment.NEW_USER_NAME, name);
        }

        if (getParent() == null) {
            setResult(result, intent);
        } else {
            getParent().setResult(result, intent);
        }
    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();
        finish();
    }

    public void onBackFABPressed(View view) {
        super.onBackPressed();
        finish();
    }

    private void defocusEffect(float alpha){
        WindowManager.LayoutParams lp = this.getWindow().getAttributes();
        lp.alpha = alpha;
        this.getWindow().addFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
        this.getWindow().setAttributes(lp);
    }

    private void openPopUp(int buttonType){

        pressedButton = buttonType;

        // inflate the layout of the popup window
        LayoutInflater inflater = (LayoutInflater)
                getSystemService(LAYOUT_INFLATER_SERVICE);
        View popupView = inflater.inflate(R.layout.popup, null);

        popupWindow = new PopupWindow(popupView, LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT, true);

        TextView popup_title = popupWindow.getContentView().findViewById(R.id.title_popup);
        TextInputEditText user_name_view = popupWindow.getContentView().findViewById(R.id.popup_user_detail_name);
        TextInputLayout user_name_layout = popupWindow.getContentView().findViewById(R.id.outlinedTextField);
        TextView popup_description = popupWindow.getContentView().findViewById(R.id.description_popup);
        TextView popup_question = popupWindow.getContentView().findViewById(R.id.question_popup);
        Button buttonOK = popupWindow.getContentView().findViewById(R.id.button_confirm_popup);

        user_name_view.setText(user_name);

        defocusEffect(0.7f);

        popupWindow.setOnDismissListener(() -> defocusEffect(1.0f));

        findViewById(R.id.user_detail_view).post(new Runnable() {
            public void run() {
                // show the popup window
                popupWindow.showAtLocation(findViewById(R.id.user_detail_view), Gravity.CENTER, 0, 0);
            }
        });

        switch(buttonType){
            case BUTTON_UPDATE:
                user_name_layout.setVisibility(View.VISIBLE);
                popup_description.setVisibility(View.GONE);
                popup_question.setVisibility(View.GONE);

                popup_title.setText(getString(R.string.update_title_popup));
                buttonOK.setBackgroundColor(getResources().getColor(R.color.button_blue));
                buttonOK.setText(getString(R.string.button_update_name));
                break;
            case BUTTON_DELETE:
                user_name_layout.setVisibility(View.GONE);
                popup_description.setVisibility(View.VISIBLE);
                popup_question.setVisibility(View.VISIBLE);

                popup_title.setText(getString(R.string.delete_title_popup));
                buttonOK.setBackgroundColor(getResources().getColor(R.color.red));
                buttonOK.setText(getString(R.string.button_delete));
                break;
        }
    }

    public void onClickConfirm(View view) {
        Log.d(TAG, "+onClickConfirm");

        TextInputEditText popup_name_view = popupWindow.getContentView().findViewById(R.id.popup_user_detail_name);
        user_name = popup_name_view.getText().toString();
        user_name = user_name.replaceAll("( +)"," ").trim();

        switch(pressedButton){
            case BUTTON_UPDATE:
                if (!namePattern.matcher(user_name).find()) {
                    StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                            this, findViewById(R.id.user_detail_view), getString(R.string.error_registration_no_name));
                    popup_name_view.getText().clear();
                } else {
                    StatusPopUp.getStatusPopUpInstance().showProgress(
                            this, findViewById(R.id.user_detail_view), getString(R.string.state_updating_name));
                    BLEService.INSTANCE.sendUpdateUserInfoReq(Integer.parseInt(user_id), user_name);
                }
                break;
            case BUTTON_DELETE:
                StatusPopUp.getStatusPopUpInstance().showProgress(
                        this, findViewById(R.id.user_detail_view), getString(R.string.state_deleting_user));
                BLEService.INSTANCE.sendDeleteUserReq(Integer.parseInt(user_id));
                break;
        }
    }

    public void onClickCancel(View view) {
        Log.d(TAG, "+onClickCancel");
        popupWindow.dismiss();
    }

    public void onClickDelete(View view) {
        Log.d(TAG, "+onClickDelete");
        openPopUp(BUTTON_DELETE);
    }

    public void onClickUpdateName(View view) {
        Log.d(TAG, "+onClickUpdateName");
        openPopUp(BUTTON_UPDATE);
    }

    public void onClickReRegister(View view) {
        Log.d(TAG, "+onClickReRegister");
        pendingIntent = new Intent();
        pendingIntent.setClass(this, RegistrationActivity.class);

        pendingIntent.putExtra(BaseServiceActivity.INTENT_KEY_PHY, getIntent().getExtras().getInt(BaseServiceActivity.INTENT_KEY_PHY));
        pendingIntent.putExtra(BaseServiceActivity.INTENT_KEY_ADDRESS, getIntent().getExtras().getString(BaseServiceActivity.INTENT_KEY_ADDRESS));
        pendingIntent.putExtra(BaseServiceActivity.INTENT_KEY_NAME, getIntent().getExtras().getString(BaseServiceActivity.INTENT_KEY_NAME));

        TextView name_view = findViewById(R.id.toolbar_title);
        pendingIntent.putExtra(getString(R.string.intent_user_name), name_view.getText());

        startActivityForResult(pendingIntent, REQUEST_CODE_REGISTRATION);
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.UpdateUserInfoRes e) {
        Log.d(TAG, "+UpdateUserInfoRes");
        if (e == null) return;

        switch (e.mUpdateUserInfoResult){
            case USER_DETAIL_RESULT_OK:
                ((TextView) findViewById(R.id.toolbar_title)).setText(user_name);

                databaseWriteExecutor.execute(() -> {
                    userDao.updateName(user_name, Integer.parseInt(user_id));
                });

                if (popupWindow !=  null) {
                    popupWindow.dismiss();
                }

                setResultToActivity(getIntent().getExtras().getInt(getString(R.string.intent_user_position)),
                        UserManagementFragment.UPDATE_USER, user_name);

                StatusPopUp.getStatusPopUpInstance().showSuccessPopUp(
                        this, findViewById(R.id.user_detail_view), getString(R.string.success_update));
                break;
            case USER_DETAIL_RESULT_INVALID:
                StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                        this, findViewById(R.id.user_detail_view), getString(R.string.error_wrong_password));
                break;
            case INVALID_PACKET:
                StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                        this, findViewById(R.id.user_detail_view), getString(R.string.error_invalid_packet));
                break;
            default:
                StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                        this, findViewById(R.id.user_detail_view), getString(R.string.error_general));
        }


        Log.d(TAG, "-UpdateUserInfoRes:" + e.mUpdateUserInfoResult);
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.DeleteUserRes e) {
        Log.d(TAG, "+DeleteUserInfoRes");
        if (e == null) return;

        switch (e.mDeleteUserResult){
            case USER_DETAIL_RESULT_OK:
                int id = Integer.parseInt(user_id);

                databaseWriteExecutor.execute(() -> {
                    userDao.deleteByID(id);
                });

                popupWindow.setOnDismissListener(new PopupWindow.OnDismissListener() {
                    @Override
                    public void onDismiss() {
                        setResultToActivity(getIntent().getExtras().getInt(getString(R.string.intent_user_position)),
                                UserManagementFragment.DELETE_USER, null);
                        UserDetailActivity.this.finish();
                    }
                });

                StatusPopUp.getStatusPopUpInstance().dismiss(this);

                if (popupWindow !=  null) {
                    popupWindow.dismiss();
                }

                break;
            case USER_DETAIL_RESULT_INVALID:
                StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                        this, findViewById(R.id.user_detail_view), getString(R.string.error_wrong_password));
                break;
            case INVALID_PACKET:
                StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                        this, findViewById(R.id.user_detail_view), getString(R.string.error_invalid_packet));
                break;
            default:
                StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                        this, findViewById(R.id.user_detail_view), getString(R.string.error_general));
        }


        Log.d(TAG, "-DeleteUserInfoRes:" + e.mDeleteUserResult);
    }
}