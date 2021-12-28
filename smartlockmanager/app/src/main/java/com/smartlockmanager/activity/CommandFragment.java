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

import androidx.cardview.widget.CardView;
import androidx.fragment.app.Fragment;

import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;

import com.smartlockmanager.R;
import com.smartlockmanager.event.BLEStateEvent;
import com.smartlockmanager.service.BLEService;
import com.smartlockmanager.utility.StatusPopUp;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

public class CommandFragment extends Fragment implements View.OnClickListener {
    private static final String TAG = "SLM_RA";

    private static final byte CHANGE_PASSWORD = 0;
    private static final byte REGISTRATION = 1;
    private static final byte DEREGISTRATION = 2;
    private static final byte PREVIEW_CAMERA_SWITCH= 3;

    private static final int CMD_RESULT_OK = 0;
    private static final int CMD_RESULT_INVALID = 1;
    private static final byte INVALID_PACKET = -2;

    public CommandFragment(){}

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        View view = inflater.inflate(R.layout.fragment_remote_command, container, false);

        CardView cardview_change_password = view.findViewById(R.id.change_password_card_view);
        cardview_change_password.setOnClickListener(this);

        CardView cardview_register = view.findViewById(R.id.registration_card_view);
        cardview_register.setOnClickListener(this);

        CardView cardview_deregister = view.findViewById(R.id.deregistration_card_view);
        cardview_deregister.setOnClickListener(this);

        CardView cardview_camera_preview = view.findViewById(R.id.preview_switch_card_view);
        cardview_camera_preview.setOnClickListener(this);

        return view;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    public void onResume() {
        super.onResume();
        EventBus.getDefault().register(this);
    }

    @Override
    public void onPause() {
        super.onPause();
        EventBus.getDefault().unregister(this);
    }

    public void onClickCommand(byte command_type) {

        Log.d(TAG, "+onClickRegistrationCMD");

        switch(command_type){
            case CHANGE_PASSWORD:
                ((SmartLockActivity)getActivity()).jumpToActivity(getActivity(),
                        ChangePasswordActivity.class,
                        ((SmartLockActivity)getActivity()).REQUEST_CODE_CHANGING_PASSWORD);
                break;
            case REGISTRATION:
                StatusPopUp.getStatusPopUpInstance().showProgress(
                        getActivity(), getView(), getString(R.string.state_sending_registration_command));
                BLEService.INSTANCE.sendRegistrationCmdReq();
                break;
            case DEREGISTRATION:
                StatusPopUp.getStatusPopUpInstance().showProgress(
                        getActivity(), getView(), getString(R.string.state_sending_deregistration_command));
                BLEService.INSTANCE.sendDeregistrationCmdReq();
                break;
            case PREVIEW_CAMERA_SWITCH:
                StatusPopUp.getStatusPopUpInstance().showProgress(
                        getActivity(), getView(), getString(R.string.state_switching_camera_preview));
                BLEService.INSTANCE.sendPreviewCameraSwitchReq();
                break;
        }
    }

    private void treatReturnValue(int CMDResult){
        switch (CMDResult){
            case CMD_RESULT_OK:
                StatusPopUp.getStatusPopUpInstance().showSuccessPopUp(
                        getActivity(), getView(), getString(R.string.success_command));
                break;
            case CMD_RESULT_INVALID:
                StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                        getActivity(), getView(), getString(R.string.error_command_sending));
                break;
            case INVALID_PACKET:
                StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                        getActivity(), getView(), getString(R.string.error_invalid_packet));
                break;
            default:
                StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                        getActivity(), getView(), getString(R.string.error_general));
        }
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.RegistrationCMDRes e) {
        Log.d(TAG, "+RegistrationCMDRes");

        if (e == null) return;

        treatReturnValue(e.mRegistrationCMDResult);

        if(e.mRegistrationCMDResult == CMD_RESULT_OK){
            ((SmartLockActivity)getActivity()).userChange = true;
        }

        Log.d(TAG, "-RegistrationCMDRes:" + e.mRegistrationCMDResult);
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.DeregistrationCMDRes e) {
        Log.d(TAG, "+DeregistrationCMDRes");

        if (e == null) return;

        treatReturnValue(e.mDeregistrationCMDResult);

        if(e.mDeregistrationCMDResult == CMD_RESULT_OK){
            ((SmartLockActivity)getActivity()).userChange = true;
        }

        Log.d(TAG, "-DeregistrationCMDRes:" + e.mDeregistrationCMDResult);
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.PrevCameraSwitchCMDRes e) {
        Log.d(TAG, "+PrevCameraSwitchCMDRes");

        if (e == null) return;

        treatReturnValue(e.mPrevCameraSwitchCMDResult);

        Log.d(TAG, "-PrevCameraSwitchCMDRes:" + e.mPrevCameraSwitchCMDResult);
    }

    @Override
    public void onClick(View v) {
        if (v.getId() == R.id.change_password_card_view)
            onClickCommand(CHANGE_PASSWORD);
        else if (v.getId() == R.id.registration_card_view)
            onClickCommand(REGISTRATION);
        else if (v.getId() == R.id.deregistration_card_view)
            onClickCommand(DEREGISTRATION);
        else if (v.getId() == R.id.preview_switch_card_view)
            onClickCommand(PREVIEW_CAMERA_SWITCH);
    }
}