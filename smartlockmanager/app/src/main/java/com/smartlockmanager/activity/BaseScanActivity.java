/**
 * Copyright 2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 *
 */

/**
 * Copyright 2016 Freescale Semiconductors, Inc.
 */

package com.smartlockmanager.activity;

import android.Manifest;
import android.annotation.TargetApi;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.ScanSettings;
import android.content.Context;
import android.content.Intent;
import android.content.IntentSender;
import android.content.pm.PackageManager;
import android.graphics.Canvas;
import android.graphics.Rect;
import android.graphics.drawable.Drawable;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.provider.Settings;
import android.view.MenuItem;
import android.view.View;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.core.content.ContextCompat;
import androidx.recyclerview.widget.RecyclerView;

import com.google.android.gms.common.api.ApiException;
import com.google.android.gms.common.api.ResolvableApiException;
import com.google.android.gms.location.LocationRequest;
import com.google.android.gms.location.LocationServices;
import com.google.android.gms.location.LocationSettingsRequest;
import com.google.android.gms.location.LocationSettingsResponse;
import com.google.android.gms.location.LocationSettingsStatusCodes;
import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.Task;
import com.smartlockmanager.AppConfig;
import com.smartlockmanager.R;
import com.smartlockmanager.model.BLEAttributes;
import com.smartlockmanager.event.BLEStateEvent;
import com.smartlockmanager.utility.BLEConverter;
import com.smartlockmanager.utility.SdkUtils;

import java.util.Collections;
import java.util.List;
import java.util.UUID;

import butterknife.BindView;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import static java.lang.Thread.sleep;

public abstract class BaseScanActivity extends BaseActivity {

    @BindView(R.id.device_list)
    RecyclerView mRecyclerView;

    protected int mPhy = BluetoothDevice.PHY_LE_1M_MASK;

    private final Handler mHandler = new Handler();
    private final Runnable mListRefresher = new Runnable() {

        @Override
        public void run() {
            refreshList();
            mHandler.postDelayed(this, 1000);
        }
    };

    private final Runnable mContinuouslyScan = new Runnable() {

        @Override
        public void run() {
            internalToggleScanState(false);
            internalToggleScanState(true);
            mHandler.postDelayed(mContinuouslyScan, AppConfig.DEVICE_CONTINUOUSLY_SCAN);
        }
    };

    private final Runnable mScanStopScheduler = new Runnable() {

        @Override
        public void run() {
            toggleScanState(false);
        }
    };

    // legacy BLE stack
    protected BluetoothAdapter mBluetoothAdapter;
    protected final BluetoothAdapter.LeScanCallback mLeScanCallback =
            new BluetoothAdapter.LeScanCallback() {

                @Override
                public void onLeScan(BluetoothDevice device, int rssi, byte[] scanRecord) {
                    if (device != null && isRequestedUuidExist(scanRecord)) {//&& isRequestedUuidExist(scanRecord)
                        appendDevice(device, rssi, scanRecord);
                    }
                }
            };

    // those callbacks will be used in Lollipop and above
    protected BluetoothLeScanner mScanner;
    protected ScanCallback mCallback;

    protected boolean isScanning;
    protected boolean requestingBluetoothEnable;
    protected boolean requestingLocationEnable;

    protected UUID targetUuid;

    public BaseScanActivity() {
        super();
    }
    public BaseScanActivity(int layout) {
        super(layout);
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.BluetoothStateChanged e) {
        if (e.newState == BluetoothAdapter.STATE_OFF) {
            toggleScanState(false);
        } else if (e.newState == BluetoothAdapter.STATE_ON) {
            toggleScanState(true);
        }
    }

    public void onPressFABCardView(View view) {
        toggleScanState(!isScanning);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            finish();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == REQUEST_ENABLE_BT) {
            if (resultCode == RESULT_OK) {
                toggleScanState(true);
            }
        }

        if (requestCode == REQUEST_ENABLE_LOCATION) {
            if (resultCode == RESULT_OK) {
                toggleScanState(true);
            }
        }
    }

    @TargetApi(Build.VERSION_CODES.LOLLIPOP)
    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        requestPermission();

        int targetService = BLEAttributes.WUART;//getIntent().getIntExtra(INTENT_KEY_SERVICE, 0);
        if (targetService != 0) {
            targetUuid = BLEConverter.uuidFromAssignedNumber(targetService);
        }
        final BluetoothManager bluetoothManager =
                (BluetoothManager) getApplicationContext().getSystemService(Context.BLUETOOTH_SERVICE);
        mBluetoothAdapter = bluetoothManager.getAdapter();
        if (SdkUtils.hasLollipop()) {
            mCallback = new ScanCallback() {

                @Override
                public void onScanResult(int callbackType, ScanResult result) {
                    if (result != null && result.getDevice() != null && result.getScanRecord() != null) {
                        if (isRequestedUuidExist(result.getScanRecord().getBytes())) {
                            appendDevice(result.getDevice(), result.getRssi(), result.getScanRecord().getBytes());
                        }
                    }
                }

                @Override
                public void onBatchScanResults(List<ScanResult> results) {
                    if (results != null) {
                        for (ScanResult result : results) {
                            if (result != null && result.getDevice() != null && result.getScanRecord() != null) {
                                if (isRequestedUuidExist(result.getScanRecord().getBytes())) {
                                    appendDevice(result.getDevice(), result.getRssi(), result.getScanRecord().getBytes());
                                }
                            }
                        }
                    }
                }

                @Override
                public void onScanFailed(int errorCode) {
                }
            };
        }
    }

    @TargetApi(Build.VERSION_CODES.M)
    @Override
    protected void onResume() {
        super.onResume();
        EventBus.getDefault().register(this);

        if (SdkUtils.hasMarshmallow()) {
            if (PackageManager.PERMISSION_GRANTED != checkSelfPermission(Manifest.permission.ACCESS_COARSE_LOCATION)) {
                Toast.makeText(this, R.string.grant_permission, Toast.LENGTH_SHORT).show();
                finish();
                return;
            }
        }

        if (requestingBluetoothEnable) {
            requestingBluetoothEnable = false;
        } else if (requestingLocationEnable) {
            requestingLocationEnable = false;
        } else {
            toggleScanState(true);
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        EventBus.getDefault().unregister(this);
        toggleScanState(false);
    }

    protected abstract boolean isBeaconScanning();

    protected abstract void clearData();

    protected abstract void appendDevice(BluetoothDevice device, int rssi, byte[] scanRecord);

    protected abstract void initList();

    protected abstract void refreshList();

    protected abstract void requestPermission();

    protected void toggleScanState(boolean isScanning) {
        if (isScanning && !mBluetoothAdapter.isEnabled()) {
            requestingBluetoothEnable = true;
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
            return;
        }
        if (isScanning && !isLocationEnabled()) {
            requestingLocationEnable = true;
            LocationRequest locationRequest = LocationRequest.create();
            locationRequest.setPriority(LocationRequest.PRIORITY_LOW_POWER);

            LocationSettingsRequest.Builder builder = new LocationSettingsRequest.Builder().addLocationRequest(locationRequest);
            builder.setAlwaysShow(true);

            Task<LocationSettingsResponse> result = LocationServices.getSettingsClient(this)
                    .checkLocationSettings(builder.build());

            result.addOnCompleteListener(new OnCompleteListener<LocationSettingsResponse>() {
                @Override
                public void onComplete(@NonNull Task<LocationSettingsResponse> task) {
                    try {
                        LocationSettingsResponse response =
                                task.getResult(ApiException.class);
                    } catch (ApiException ex) {
                        switch (ex.getStatusCode()) {
                            case LocationSettingsStatusCodes.RESOLUTION_REQUIRED:
                                try {
                                    ResolvableApiException resolvableApiException =
                                            (ResolvableApiException) ex;
                                    resolvableApiException
                                            .startResolutionForResult(BaseScanActivity.this,
                                                    REQUEST_ENABLE_LOCATION);
                                } catch (IntentSender.SendIntentException e) {
                                }
                                break;
                            case LocationSettingsStatusCodes.SETTINGS_CHANGE_UNAVAILABLE:
                                break;
                        }
                    }
                }
            });
            return;
        }
        this.isScanning = isScanning;
        SdkUtils.changeToolbarFABButtonState(this, R.string.menu_scan_stop, R.string.device_scan, isScanning);
//        supportInvalidateOptionsMenu();
        if (isScanning) {
            mHandler.post(mListRefresher);
            if (isBeaconScanning()) {
                clearData();
                SdkUtils.changeToolbarFABButtonState(this, R.string.menu_scan_stop, R.string.device_scan, isScanning);
            } else {
                clearData();
                SdkUtils.changeToolbarFABButtonState(this, R.string.menu_scan_stop, R.string.device_scan, isScanning);
                mHandler.postDelayed(mScanStopScheduler, AppConfig.DEVICE_SCAN_PERIOD);
            }

            internalToggleScanState(true);
            mHandler.postDelayed(mContinuouslyScan, AppConfig.DEVICE_CONTINUOUSLY_SCAN);
        } else {
            mHandler.removeCallbacks(mScanStopScheduler);
            mHandler.removeCallbacks(mListRefresher);
            mHandler.removeCallbacks(mContinuouslyScan);
            internalToggleScanState(false);
        }
    }

    protected boolean isLocationEnabled(){
        int locationMode = 0;

        try {
            locationMode = Settings.Secure.getInt(this.getContentResolver(), Settings.Secure.LOCATION_MODE);
        } catch (Settings.SettingNotFoundException e) {
            e.printStackTrace();
        }

        return locationMode != Settings.Secure.LOCATION_MODE_OFF;
    }

    protected boolean isRequestedUuidExist(byte[] scanRecord) {
        if (isBeaconScanning()) {
            // beacon scanning has no UUIDs
            return true;
        }
        return BLEConverter.uuidsFromScanRecord(scanRecord).contains(targetUuid);
    }

    @TargetApi(Build.VERSION_CODES.LOLLIPOP)
    private void internalToggleScanState(boolean isScanning) {
        if (isScanning) {
            if (SdkUtils.hasLollipop()) {
                if (mScanner == null) {
                    mScanner = mBluetoothAdapter.getBluetoothLeScanner();
                }
                // Scanner instance can be null here, if BLE has been disabled right after we perform check in toggleScanState()
                // this can be happened on devices which Bluetooth turning on/off takes a few seconds
                if (mScanner == null) {
                    toggleScanState(false);
                    return;
                }
                ScanSettings.Builder settingsBuilder  =  new ScanSettings.Builder()
                        .setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY);

//                && mBluetoothAdapter.isLeCodedPhySupported()
                if(SdkUtils.hasO()){
                    settingsBuilder.setLegacy(false);
                    settingsBuilder.setPhy(ScanSettings.PHY_LE_ALL_SUPPORTED);
                }
                mScanner.startScan(Collections.<ScanFilter>emptyList(),settingsBuilder.build(), mCallback);
            } else {
                mBluetoothAdapter.startLeScan(mLeScanCallback);
            }
        } else {
            if (SdkUtils.hasLollipop()) {
                if (mScanner != null && mBluetoothAdapter.isEnabled()) {
                    mScanner.stopScan(mCallback);
                }
                mScanner = null;
            } else {
                mBluetoothAdapter.stopLeScan(mLeScanCallback);
            }
        }
    }

    static class CustomHeightItemDecoration extends RecyclerView.ItemDecoration {

        private final int mHeight;
        private final Drawable mDivider;

        public CustomHeightItemDecoration(@NonNull Context context, int mVerticalSpaceHeight) {
            this.mHeight = mVerticalSpaceHeight;
            this.mDivider = ContextCompat.getDrawable(context, R.drawable.list_divider);
        }

        @Override
        public void getItemOffsets(Rect outRect, View view, RecyclerView parent, RecyclerView.State state) {
            if (parent.getChildAdapterPosition(view) != parent.getAdapter().getItemCount() - 1) {
                outRect.bottom = mHeight;
            }
        }

        public void onDrawOver(Canvas c, RecyclerView parent, RecyclerView.State state) {
            int left = parent.getPaddingLeft();
            int right = parent.getWidth() - parent.getPaddingRight();

            int childCount = parent.getChildCount();
            for (int i = 0; i < childCount; i++) {
                View child = parent.getChildAt(i);

                RecyclerView.LayoutParams params = (RecyclerView.LayoutParams) child.getLayoutParams();

                int top = child.getBottom() + params.bottomMargin;
                int bottom = top + mDivider.getIntrinsicHeight();

                mDivider.setBounds(left, top, right, bottom);
                mDivider.draw(c);
            }
        }
    }
}
