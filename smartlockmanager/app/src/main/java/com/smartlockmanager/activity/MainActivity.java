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

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.widget.Toolbar;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.lifecycle.ProcessLifecycleOwner;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import android.Manifest;
import android.bluetooth.BluetoothDevice;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Bundle;
import android.provider.Settings;
import android.text.InputType;
import android.text.TextUtils;
import android.util.Log;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.smartlockmanager.utility.ForegroundObserver;
import com.smartlockmanager.utility.SdkUtils;
import com.smartlockmanager.AppConfig;
import com.smartlockmanager.R;
import com.smartlockmanager.model.BLEDevice;
import com.smartlockmanager.view.AboutDialog;

import java.util.ArrayList;
import java.util.List;
import java.util.regex.Pattern;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;

import static com.smartlockmanager.utility.SdkUtils.fullScreen;

public class MainActivity extends BaseScanActivity {
    /**
     * Tag for the {@link Log}.
     */
    private static final String TAG = "FMD_MA";

    private BLEDeviceAdapter mAdapter;
    private List<BLEDevice> mDevices = new ArrayList<>();

    private boolean pendingDeviceConnection;
    private Intent pendingIntent;

    private String mAddressTobeConnect = null;

    @Nullable
    @BindView(R.id.status_view_info)
    View mInfoButton;

    public MainActivity(){
        super();
    }

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        fullScreen(getWindow());

        setContentView(R.layout.activity_device_scan);

        setSupportActionBar((Toolbar) findViewById(R.id.toolbar));
        getSupportActionBar().setDisplayShowTitleEnabled(false);
        ((TextView) findViewById(R.id.toolbar_title)).setText(getString(R.string.app_name));
        ((TextView) findViewById(R.id.toolbar_subtitle)).setText("");

        ButterKnife.bind(this);

        mRecyclerView = findViewById(R.id.device_list);
        mRecyclerView.setHasFixedSize(true);
        mRecyclerView.setLayoutManager(new LinearLayoutManager(this, RecyclerView.VERTICAL, false));

        mInfoButton.setVisibility(View.VISIBLE);

        ForegroundObserver.getForegroundObserver().updateActivity(this);
        ProcessLifecycleOwner.get().getLifecycle().addObserver(ForegroundObserver.getForegroundObserver());

        initList();
    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();
        finishAffinity();
    }

    @Override
    protected boolean isBeaconScanning() {
        return false;
    }

    @Override
    protected void clearData() {
        mDevices.clear();
        if (mAdapter != null) {
            mAdapter.notifyDataSetChanged();
        }
    }

    /**
     * This alert shows when user deny the permission.
     */
    private void showAlertForSettingScreen(int requestCode) {
        Log.d(TAG, "+showAlertForSettingScreen");
        android.app.AlertDialog.Builder builder = new android.app.AlertDialog.Builder(MainActivity.this);

        builder.setTitle(R.string.title_camera_permission);
        builder.setMessage(R.string.msg_camera_permission);

        builder.setPositiveButton(R.string.ok, (dialogInterface, i) -> {
            Intent intent = new Intent(Settings.ACTION_APPLICATION_DETAILS_SETTINGS);
            Uri uri = Uri.fromParts("package", getPackageName(), null);
            intent.setData(uri);
            startActivityForResult(intent, 100);
        });
        builder.setNegativeButton(R.string.cancel, (dialogInterface, i) -> finish());
        builder.setCancelable(false);
        android.app.AlertDialog dialog = builder.create();
        dialog.show();
        // dialog.getButton(AlertDialog.BUTTON_POSITIVE).setTextColor(getResources().getColor(R.color.colorPrimaryDark));
        // dialog.getButton(AlertDialog.BUTTON_NEGATIVE).setTextColor(getResources().getColor(R.color.colorPrimaryDark));
        Log.d(TAG, "-showAlertForSettingScreen");
    }

    @Override
    protected void requestPermission() {
        Log.d(TAG, "+requestPermission");
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED
            || ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED
            || ContextCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this,
                    new String[] { Manifest.permission.CAMERA, Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.ACCESS_FINE_LOCATION}, 101);
        }
        Log.d(TAG, "-requestPermission");
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions,
                                           @NonNull int[] grantResults) {
        Log.d(TAG, "+onRequestPermissionsResult:[" + requestCode + "]");
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        switch (requestCode) {
            case 101:
                if (grantResults.length <= 0 || grantResults[0] != PackageManager.PERMISSION_GRANTED) {
                    // permission was not granted
                    if (ActivityCompat.shouldShowRequestPermissionRationale(this, Manifest.permission.ACCESS_FINE_LOCATION)) {
                        requestPermission();
                    } else {
                        showAlertForSettingScreen(requestCode);
                    }
                }
                break;
        }
        Log.d(TAG, "-onRequestPermissionsResult");
    }

    @OnClick(R.id.status_view_info)
    public void viewAboutInfo() {
        AboutDialog.newInstance(this).show();
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == R.id.menu_address) {
            final EditText inputServer = new EditText(this);
            inputServer.setInputType(InputType.TYPE_NUMBER_FLAG_SIGNED);
            android.app.AlertDialog.Builder builder = new android.app.AlertDialog.Builder(this);
            builder.setTitle("Address").setIcon(android.R.drawable.ic_dialog_info).setView(inputServer)
                    .setNegativeButton("CANCEL", null);
            builder.setPositiveButton(R.string.menu_connect, new DialogInterface.OnClickListener() {

                public void onClick(DialogInterface dialog, int which) {
                    String deviceAddress = inputServer.getText().toString().toUpperCase();
                    String patternMac= "([A-Fa-f0-9]{2}:){5}[A-Fa-f0-9]{2}";
                    if(!Pattern.compile(patternMac).matcher(deviceAddress).find()){
                        Toast.makeText(MainActivity.this,"invalid mac address",Toast.LENGTH_LONG).show();
                        return;
                    }
                    mAddressTobeConnect = deviceAddress;
                    toggleScanState(true);
                }
            });
            builder.show();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    public void jumpToActivity( Intent intent){
        startActivity(intent);
    }

    @Override
    protected void appendDevice(BluetoothDevice device, int rssi, byte[] scanRecord) {
        BLEDevice bleDevice = new BLEDevice(device);
        if (mDevices.contains(bleDevice)) {
            bleDevice = mDevices.get(mDevices.indexOf(bleDevice));
        } else {
            mDevices.add(bleDevice);
        }
        bleDevice.setRssi(rssi);
        bleDevice.setLastScannedTime(System.nanoTime());
        if(mAddressTobeConnect != null){
            if(bleDevice.getInternalDevice().getAddress().equalsIgnoreCase(mAddressTobeConnect)){
                mAddressTobeConnect = null;
                startDevice(mDevices.size()-1);
                return;
            }
        }
    }

    @Override
    protected void initList() {
        mAdapter = new BLEDeviceAdapter();
        if (mRecyclerView != null) {
            mRecyclerView.setAdapter(mAdapter);
        }
    }

    @Override
    protected void refreshList() {
        final long now = System.nanoTime();
        RecyclerView deviceList = findViewById(R.id.device_list);
        LinearLayout placeholder = findViewById(R.id.no_boards_placeholder);

        if (mDevices.size() == 0){
            deviceList.setVisibility(View.GONE);
            placeholder.setVisibility(View.VISIBLE);
        } else {
            deviceList.setVisibility(View.VISIBLE);
            placeholder.setVisibility(View.GONE);
        }

        for (int i = 0; i < mDevices.size(); ++i) {
            if ((now - mDevices.get(i).getLastScannedTime()) / 1000 / 1000 > AppConfig.BEACON_OUT_OF_RANGE_PERIOD) {
                BLEDevice device = mDevices.remove(i);
                if (pendingIntent != null && pendingDeviceConnection) {
                    String pendingAddress = pendingIntent.getStringExtra(BaseServiceActivity.INTENT_KEY_ADDRESS);
                    if (device.getInternalDevice().getAddress().equals(pendingAddress)) {
                        pendingDeviceConnection = false;
                        Toast.makeText(this, "This device is no longer available.", Toast.LENGTH_SHORT).show();
                        pendingIntent = null;
                    }
                }
            }
        }
        mAdapter.notifyDataSetChanged();
    }

    @Override
    protected void toggleScanState(boolean isScanning) {
        super.toggleScanState(isScanning);
        if (!isScanning && pendingIntent != null && pendingDeviceConnection) {
            pendingIntent = null;
            pendingDeviceConnection = false;
        }
    }

    public void startDevice(int itemPosition){
        if (mDevices == null) {
            Toast.makeText(MainActivity.this, "device list is null.", Toast.LENGTH_SHORT).show();
            return;
        }

        if (itemPosition < 0 || itemPosition >= mDevices.size()) {
            return;
        }

        if (!isScanning) {
            Toast.makeText(MainActivity.this, getString(R.string.scanning_not_active), Toast.LENGTH_SHORT).show();
            return;
        }

        pendingIntent = new Intent();
        pendingIntent.setClass(MainActivity.this, AuthenticationActivity.class);

        BLEDevice bleDevice = mDevices.get(itemPosition);
        pendingDeviceConnection = true;
        pendingIntent.putExtra(BaseServiceActivity.INTENT_KEY_PHY,mPhy);
        pendingIntent.putExtra(BaseServiceActivity.INTENT_KEY_ADDRESS, bleDevice.getInternalDevice().getAddress());
        pendingIntent.putExtra(BaseServiceActivity.INTENT_KEY_NAME, bleDevice.getInternalDevice().getName());

        jumpToActivity(pendingIntent);
    }

    private final View.OnClickListener mDeviceClickHandler = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            int itemPosition = mRecyclerView.getChildAdapterPosition(v);
            startDevice(itemPosition);
        }
    };

    static class BLEDeviceHolder extends RecyclerView.ViewHolder {

        private final int LVL1_RSSI = -55;
        private final int LVL2_RSSI = LVL1_RSSI - 10;
        private final int LVL3_RSSI = LVL2_RSSI - 10;

        @BindView(R.id.device_name)
        TextView name;

        @BindView(R.id.device_mac_address)
        TextView mac;

        @BindView(R.id.rssi_icon)
        ImageView rssi;

        public BLEDeviceHolder(View itemView) {
            super(itemView);
            ButterKnife.bind(this, itemView);
        }
    }

    private void setInitialPhy(){
        if(!SdkUtils.hasO())return;
        mPhy = BluetoothDevice.PHY_LE_1M_MASK;
        if(choice[0]){
            mPhy = BluetoothDevice.PHY_LE_1M_MASK;
            choice[0] = true;
        }
        if(choice[1]){
            if(mBluetoothAdapter.isLe2MPhySupported()){
                mPhy = BluetoothDevice.PHY_LE_2M_MASK;
                choice[1]= false;
            }
        }
        if(choice[2]){
            if(mBluetoothAdapter.isLeCodedPhySupported()){
                mPhy = BluetoothDevice.PHY_LE_CODED_MASK;
                choice[2]=false;
            }
        }

    }

    private boolean[] choice ={true,false,false};
    public void showPhySetting(final int position){
        final String[] item = { "LE 1M(Legacy)", "LE 2M(Double speed)", "LE Coded(Long range)" };
        final AlertDialog.Builder singleDialog = new AlertDialog.Builder(MainActivity.this);
        singleDialog.setTitle("Set initial preferred PHY").setMultiChoiceItems(item,new boolean[]{true,false,false}, new DialogInterface.OnMultiChoiceClickListener() {
            @Override
            public void onClick(DialogInterface dialogInterface, int i, boolean b) {
                choice[i]=b;
                boolean checked = false;
                for(int j = 0;j<choice.length;j++){
                    checked |=choice[j];
                }
                Button positive = ((AlertDialog)dialogInterface).getButton(AlertDialog.BUTTON_POSITIVE);
                if(!checked){
                    positive.setEnabled(false);
                }else {
                    positive.setEnabled(true);
                }
            }
        }).setPositiveButton("Connect", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialogInterface, int i) {
                MainActivity.this.setInitialPhy();
                startDevice(position);
            }
        }).setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialogInterface, int i) {

            }
        }) .setCancelable(false).show();
    }

    class BLEDeviceAdapter extends RecyclerView.Adapter<BLEDeviceHolder> {

        @Override
        public int getItemCount() {
            return mDevices == null ? 0 : mDevices.size();
        }

        @Override
        public BLEDeviceHolder onCreateViewHolder(ViewGroup viewGroup, int i) {
            View view = getLayoutInflater().inflate(R.layout.ble_device_item, viewGroup, false);
            view.setOnClickListener(mDeviceClickHandler);
            return new BLEDeviceHolder(view);
        }

        @Override
        public void onBindViewHolder(final BLEDeviceHolder holder, final int position) {
            BLEDevice bleDevice = mDevices.get(position);
            BluetoothDevice device = bleDevice.getInternalDevice();
            holder.name.setText(TextUtils.isEmpty(device.getName()) ? "Unknown" : device.getName() +
                    " (" + (device.getBondState() == BluetoothDevice.BOND_BONDED ? "Bonded" : "Unbonded") + ")");
            holder.mac.setText(device.getAddress());

            if (bleDevice.getRssi() >= holder.LVL1_RSSI) {
                holder.rssi.setImageDrawable(getDrawable(R.drawable.ic_signal_high));
            } else if (bleDevice.getRssi() >= holder.LVL2_RSSI && bleDevice.getRssi() < holder.LVL1_RSSI) {
                holder.rssi.setImageDrawable(getDrawable(R.drawable.ic_signal_three));
            } else if (bleDevice.getRssi() >= holder.LVL3_RSSI && bleDevice.getRssi() < holder.LVL2_RSSI) {
                holder.rssi.setImageDrawable(getDrawable(R.drawable.ic_signal_two));
            } else if (bleDevice.getRssi() < holder.LVL3_RSSI) {
                holder.rssi.setImageDrawable(getDrawable(R.drawable.ic_signal_low));
            }
        }
    }
}