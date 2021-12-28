/**
 * Copyright 2016 Freescale Semiconductors, Inc.
 */

package com.smartlockmanager.view;

import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import androidx.core.content.ContextCompat;
import android.widget.TextView;

import com.afollestad.materialdialogs.MaterialDialog;
import com.smartlockmanager.R;

import com.smartlockmanager.event.BLEStateEvent;
import com.smartlockmanager.model.BLEAttributes;
import com.smartlockmanager.service.BLEService;
import com.smartlockmanager.utility.BLEConverter;

import butterknife.BindView;
import butterknife.ButterKnife;
import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

public class DeviceInfoDialog extends MaterialDialog {

    @BindView(R.id.dis_manufacture_name)
    TextView mManufactureName;

    @BindView(R.id.dis_model_number)
    TextView mModelName;

    @BindView(R.id.dis_serial_number)
    TextView mSerialNumber;

    @BindView(R.id.dis_hardware_rev)
    TextView mHardwareRev;

    @BindView(R.id.dis_firmware_rev)
    TextView mFirmwareRev;

    @BindView(R.id.dis_software_rev)
    TextView mSoftwareRev;

    @BindView(R.id.dis_system_id)
    TextView mSystemId;

    @BindView(R.id.dis_cert_data)
    TextView mCertData;

    public DeviceInfoDialog(Builder builder) {
        super(builder);
    }

    public static DeviceInfoDialog newInstance(Context context, String title) {
        MaterialDialog.Builder builder = new MaterialDialog.Builder(context)
                .title(title).titleColor(ContextCompat.getColor(context, R.color.red))
                .customView(R.layout.device_information, true)
                .positiveText("ok")
                .positiveColor(ContextCompat.getColor(context, R.color.red));
        DeviceInfoDialog dialog = new DeviceInfoDialog(builder);
        //dialog.getWindow().setBackgroundDrawable(ContextCompat.getDrawable(context, R.drawable.device_info_background));
        //Get original Dimension
        float scaleRatio = context.getResources().getDisplayMetrics().density;
        float dimenPix = context.getResources().getDimension(R.dimen.cycling_positive_button_text_size);
        float dimenOrginal = dimenPix/scaleRatio;
        dialog.positiveButton.setTextSize(dimenOrginal);

        dimenPix = context.getResources().getDimension(R.dimen.cycling_title_text_size);
        dimenOrginal = dimenPix/scaleRatio;
        dialog.title.setTextSize(dimenOrginal);
        dialog.setOnDismissListener(new OnDismissListener() {

            @Override
            public void onDismiss(DialogInterface dialog) {
                EventBus.getDefault().unregister(dialog);
            }
        });
        return dialog;
    }

    /**
     * If device is disconnected, then this dialog should dismiss as well.
     *
     * @param e
     */
    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.Disconnected e) {
        dismiss();
    }

    /**
     * Data available, need to check for assignedNumber and update corresponding field.
     *
     * @param e
     */
    @Subscribe(threadMode =ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.DataAvailable e) {
        int assignedNumber = BLEConverter.getAssignedNumber(e.characteristic.getUuid());
        switch (assignedNumber) {
            case BLEAttributes.MANUFACTURER_NAME:
                String name = e.characteristic.getStringValue(0);
                mManufactureName.setText(name);
                break;
            case BLEAttributes.MODEL_NUMBER_STRING:
                String model = e.characteristic.getStringValue(0);
                mModelName.setText(model);
                break;
            case BLEAttributes.SERIAL_NUMBER_STRING:
                String serial = e.characteristic.getStringValue(0);
                mSerialNumber.setText(serial);
                break;
            case BLEAttributes.HARDWARE_REV_STRING:
                String hardware = e.characteristic.getStringValue(0);
                mHardwareRev.setText(hardware);
                break;
            case BLEAttributes.FIRMWARE_REV_STRING:
                String firmware = e.characteristic.getStringValue(0);
                mFirmwareRev.setText(firmware);
                break;
            case BLEAttributes.SOFTWARE_REV_STRING:
                String software = e.characteristic.getStringValue(0);
                mSoftwareRev.setText(software);
                break;
            case BLEAttributes.SYSTEM_ID:
                final byte[] systemIdData = e.characteristic.getValue();
                if (systemIdData != null && systemIdData.length > 0) {
                    final StringBuilder stringBuilder = new StringBuilder(systemIdData.length);
                    for (byte byteChar : systemIdData) {
                        stringBuilder.append(String.format("%02X ", byteChar));
                    }
                    mSystemId.setText(stringBuilder.toString());
                }
                break;
            case BLEAttributes.REGULAR_CERT:
                final byte[] regularCertData = e.characteristic.getValue();
                if (regularCertData != null && regularCertData.length > 0) {
                    final StringBuilder stringBuilder = new StringBuilder(regularCertData.length);
                    for (byte byteChar : regularCertData) {
                        stringBuilder.append(String.format("%02X ", byteChar));
                    }
                    mCertData.setText(stringBuilder.toString());
                }
                break;
            default:
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        ButterKnife.bind(this);
        EventBus.getDefault().register(this);

        /**
         * Loop through all required information to request.
         */
        final int[] infos = new int[]{
                BLEAttributes.MANUFACTURER_NAME, BLEAttributes.MODEL_NUMBER_STRING,
                BLEAttributes.SERIAL_NUMBER_STRING, BLEAttributes.HARDWARE_REV_STRING,
                BLEAttributes.FIRMWARE_REV_STRING, BLEAttributes.SOFTWARE_REV_STRING,
                BLEAttributes.SYSTEM_ID, BLEAttributes.REGULAR_CERT};
        for (int id : infos) {
            BLEService.INSTANCE.request(BLEAttributes.DEVICE_INFORMATION_SERVICE, id, BLEService.Request.READ);
        }
    }
}