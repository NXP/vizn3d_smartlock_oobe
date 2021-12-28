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
import com.google.android.material.progressindicator.CircularProgressIndicator;
import com.google.android.material.textfield.TextInputEditText;
import com.google.android.material.textfield.TextInputLayout;
import com.smartlockmanager.R;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

import com.smartlockmanager.service.BLEService;
import com.smartlockmanager.event.BLEStateEvent;
import com.smartlockmanager.utility.StatusPopUp;
import com.smartlockmanager.utility.UserInteractionTimer;

import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.util.Base64;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.PopupWindow;
import android.widget.TextView;
import android.widget.Toast;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.regex.Pattern;

import butterknife.ButterKnife;

import static com.smartlockmanager.utility.SdkUtils.fullScreen;

public class RegistrationConfirmActivity extends AppCompatActivity {
    /**
     * Tag for the {@link Log}.
     */
    private static final String TAG = "SLM_RCA";
    private static final Pattern namePattern = Pattern.compile("[a-zA-Z0-9]+", Pattern.CASE_INSENSITIVE);

    private Bitmap mFaceImage;
    private byte[] mFaceFeature;

    private ImageView mFaceImageView;
    private EditText mNameEditText;
    private boolean reRegistration = false;
    private String duplicateFaceName = null;

    private static final int REGISTRATION_RESULT_OK = 0;
    private static final int REGISTRATION_RESULT_INVALID = -1;
    private static final int REGISTRATION_RESULT_DUPLICATE = 1;
    public final int REQUEST_CODE_REGISTRATION = 100;
    private static final int INVALID_PACKET = -2;
    private Intent pendingIntent;

    private PopupWindow mPopupWindow;

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            setResultToActivity(RegistrationActivity.RESULT_CANCELED);
            finish();
            return true;
        }

        return super.onOptionsItemSelected(item);
    }
    /**
     * Convert String to bitmap.
     *
     * @param encodedString image String
     * @return android.graphics.Bitmap
     */
    public Bitmap stringToBitMap(String encodedString) {
        try {
            byte[] encodeByte = Base64.decode(encodedString, Base64.DEFAULT);
            return BitmapFactory.decodeByteArray(encodeByte, 0, encodeByte.length);
        } catch (Exception e) {
            e.getMessage();
            return null;
        }
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.Disconnected e) {
        NoConnectionActivity.jumpToDisconnectActivity(this);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        fullScreen(getWindow());

        setContentView(R.layout.activity_registration_confirm);

        setSupportActionBar((Toolbar) findViewById(R.id.toolbar));
        getSupportActionBar().setDisplayShowTitleEnabled(false);
        ((TextView) findViewById(R.id.toolbar_title)).setText(getString(R.string.app_registration_confirm));
        ((TextView) findViewById(R.id.toolbar_subtitle)).setText("");

        String strBase64Image = getIntent().getStringExtra(BaseServiceActivity.INTENT_KEY_IMAG);
        mFaceImage = stringToBitMap(strBase64Image);
        mFaceImageView = (ImageView)findViewById(R.id.imageview);
        if (mFaceImageView != null){
            mFaceImageView.setImageBitmap(mFaceImage);
        }

        mFaceFeature = getIntent().getByteArrayExtra(BaseServiceActivity.INTENT_KEY_FEATURE);

        mNameEditText = (EditText)findViewById(R.id.registration_name);

        if (getIntent().getExtras().getString(getString(R.string.intent_user_name)) != null){
            reRegistration = true;

            findViewById(R.id.outlinedTextField).setVisibility(View.GONE);
            findViewById(R.id.outlinedTextFieldFixed).setVisibility(View.VISIBLE);

            mNameEditText = (EditText)findViewById(R.id.registration_name_fixed);
            mNameEditText.setText(getIntent().getExtras().getString(getString(R.string.intent_user_name)));
        }

        CircularProgressIndicator circularProgressIndicator = findViewById(R.id.toolbar_loading);
        circularProgressIndicator.setVisibility(View.GONE);

        TextView fab_name = findViewById(R.id.toolbar_loading_text);
        ImageView fab_image = findViewById(R.id.toolbar_loading_static);

        fab_image.setImageDrawable(getDrawable(R.drawable.ic_baseline_how_to_reg_24));
        fab_image.setVisibility(View.VISIBLE);
        fab_name.setText(getString(R.string.button_register));
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

    public void onPressFABCardView(View view) {
        Log.d(TAG, "+onClickOk");

        /* Text manipulation to trim and to eliminate additional whitespaces */
        String nameText = mNameEditText.getText().toString();
        nameText = nameText.replaceAll("( +)"," ").trim();

        if (!namePattern.matcher(nameText).find()) {
            StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                    this, findViewById(R.id.registration_confirm_view), getString(R.string.error_registration_no_name));
            mNameEditText.getText().clear();
        } else {
            StatusPopUp.getStatusPopUpInstance().showProgress(
                    this, findViewById(R.id.registration_confirm_view), getString(R.string.state_registering));
            BLEService.INSTANCE.sendRegistrationReq(mNameEditText.getText().toString(), mFaceFeature, reRegistration);
        }
    }

    private void setResultToActivity(int result) {
        Intent intent = new Intent();
        if (getParent() == null) {
            setResult(result, intent);
        } else {
            getParent().setResult(result, intent);
        }
    }

    @Override
    public void onBackPressed() {
        setResultToActivity(RegistrationActivity.RESULT_CANCELED);
        finish();
    }

    public void onBackFABPressed(View view) {
        Log.d(TAG, "+onClickCancel");
        setResultToActivity(RegistrationActivity.RESULT_CANCELED);
        finish();
    }

    public void setImage(Bitmap image){
        this.mFaceImage = image;
    }

    private void duplicateHandle(BLEStateEvent.RegistrationRes e) {
        // inflate the layout of the popup window
        duplicateFaceName = e.mRegistrationDuplicateName;

        LayoutInflater inflater = (LayoutInflater)
                getSystemService(LAYOUT_INFLATER_SERVICE);
        View popupView = inflater.inflate(R.layout.popup, null);

        mPopupWindow = new PopupWindow(popupView, LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT, true);

        TextView popup_title = mPopupWindow.getContentView().findViewById(R.id.title_popup);
        TextInputEditText user_name_view = mPopupWindow.getContentView().findViewById(R.id.popup_user_detail_name);
        TextInputLayout user_name_layout = mPopupWindow.getContentView().findViewById(R.id.outlinedTextField);
        TextView popup_description = mPopupWindow.getContentView().findViewById(R.id.description_popup);
        TextView popup_question = mPopupWindow.getContentView().findViewById(R.id.question_popup);
        Button buttonOK = mPopupWindow.getContentView().findViewById(R.id.button_confirm_popup);

        user_name_view.setVisibility(View.GONE);
        user_name_layout.setVisibility(View.GONE);
        user_name_layout.setVisibility(View.GONE);
        popup_description.setVisibility(View.VISIBLE);
        popup_question.setVisibility(View.VISIBLE);

        popup_title.setText(String.format(getString(R.string.registration_duplicate_title), duplicateFaceName));
        popup_description.setText(R.string.registration_duplicate_description);
        popup_question.setText(R.string.registration_duplicate_question);
        buttonOK.setText(R.string.button_update);

        mPopupWindow.setOutsideTouchable(false);
        mPopupWindow.setFocusable(false);
        mPopupWindow.setTouchable(true);
        mPopupWindow.showAtLocation(findViewById(R.id.imageview), Gravity.CENTER, 0, 0);
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.RegistrationRes e) {
        Log.d(TAG, "+RegistrationRes");

        if (e == null) return;

        StatusPopUp.getStatusPopUpInstance().dismiss(this);

        switch (e.mRegistrationResult) {
            case REGISTRATION_RESULT_OK:
                setResultToActivity(SmartLockActivity.RESULT_OK);
                finish();
                break;
            case REGISTRATION_RESULT_INVALID:
                setResultToActivity(SmartLockActivity.REGISTRATION_RESULT_INVALID);
                finish();
                break;
            case REGISTRATION_RESULT_DUPLICATE:
                setResultToActivity(SmartLockActivity.REGISTRATION_RESULT_DUPLICATE);
                duplicateHandle(e);
                break;
            case INVALID_PACKET:
                setResultToActivity(SmartLockActivity.INVALID_PACKET);
                finish();
                break;
            default:
                setResultToActivity(SmartLockActivity.GENERAL_ERROR);
                finish();
        }

        Log.d(TAG, "-RegistrationRes:" + e.mRegistrationResult);
    }

    public void onClickConfirm(View view) {
        Log.d(TAG, "+onClickConfirm");

        TextView popup_description= mPopupWindow.getContentView().findViewById(R.id.description_popup);
        StatusPopUp.getStatusPopUpInstance().showProgress(
                this, findViewById(R.id.registration_confirm_view), getString(R.string.state_registering));
        BLEService.INSTANCE.sendRegistrationReq(duplicateFaceName, mFaceFeature, true);

        mPopupWindow.dismiss();
    }

    public void onClickCancel(View view) {
        Log.d(TAG, "+onClickCancel");
        mPopupWindow.dismiss();
        finish();
    }
}
