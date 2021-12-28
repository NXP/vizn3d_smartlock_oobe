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
import android.app.AlertDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.text.TextUtils;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.afollestad.materialdialogs.MaterialDialog;

import com.smartlockmanager.AppConfig;
import com.smartlockmanager.R;
import com.smartlockmanager.event.BLEStateEvent;
import com.smartlockmanager.model.BLEAttributes;
import com.smartlockmanager.service.BLEService;
import com.smartlockmanager.utility.BLEConverter;
import com.smartlockmanager.utility.SdkUtils;
import com.smartlockmanager.utility.StatusPopUp;
import com.smartlockmanager.utility.UserInteractionTimer;
import com.smartlockmanager.view.DeviceInfoDialog;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import butterknife.BindView;
import butterknife.OnClick;

public class BaseServiceActivity extends BaseActivity {

    /**
     * Tag for the {@link Log}.
     */
    private static final String TAG = "SLM_BSA";

    /**
     * Requested application should pass MAC Address of BLE device here.
     */
    public static final String INTENT_KEY_ADDRESS = "intent.key.address";
    public static final String INTENT_KEY_NAME = "intent.key.name";
    public static final String INTENT_KEY_PHY = "intent.key.phy";
    public static final String INTENT_KEY_FEATURE = "intent.key.feature";
    public static final String INTENT_KEY_IMAG = "intent.key.img";
    public boolean isShowMenuOption;

    @Nullable
    @BindView(R.id.status_connection)
    TextView mStatusConnection;

    @Nullable
    @BindView(R.id.status_battery)
    TextView mStatusBattery;

    @Nullable
    @BindView(R.id.status_view_info)
    View mInfoButton;

    protected String mDeviceAddress;
    protected String mDeviceName;
    
    protected final Handler mHandler = new Handler();

    protected int mPhy = BluetoothDevice.PHY_LE_1M_MASK;

    /**
     * Preodically check for battery value.
     */
    private final Runnable mBatteryRunner = new Runnable() {

        @Override
        public void run() {
//            invokeBatteryCheck();
        }
    };

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.BluetoothStateChanged e) {
        if (e.newState == BluetoothAdapter.STATE_OFF) {
            if (BLEService.INSTANCE.getConnectionState() != BLEService.State.STATE_DISCONNECTED) {
                BLEService.INSTANCE.disconnect();
                new MaterialDialog.Builder(this).title(R.string.error_title)
                        .titleColor(Color.RED)
                        .content(R.string.error_lost_connection)
                        .positiveText(android.R.string.ok).show();
            }
        }
    }

    /**
     * Update connection state to Connected in main thread. Subclasses can override this method to update any other UI part.
     *
     * @param e
     */
    @Subscribe(threadMode =ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.Connected e) {
        if (mStatusConnection != null) {
            mStatusConnection.setText("Status: " + getString(R.string.state_connected));
        }
//        supportInvalidateOptionsMenu();
    }

    /**
     * Update connection state to Connecting in main thread. Subclasses can override this method to update any other UI part.
     *
     * @param e
     */
    @Subscribe(threadMode =ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.Connecting e) {
        if (mStatusConnection != null) {
            mStatusConnection.setText("Status: " + getString(R.string.state_connecting));
        }
//        supportInvalidateOptionsMenu();
    }

    /**
     * Update connection state to Disconnected in main thread. Subclasses can override this method to update any other UI part.
     * Subclasses MUST override this method if they want to clear current values when BLE device is disconnected.
     *
     * @param e
     */
    @Subscribe(threadMode =ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.Disconnected e) {
        if (mStatusConnection != null) {
            mStatusConnection.setText("Status: " + getString(R.string.state_disconnected));
        }
        if (mStatusBattery != null) {
            mStatusBattery.setVisibility(View.GONE);
        }
        if (mInfoButton != null) {
            mInfoButton.setVisibility(View.INVISIBLE);
        }
//        supportInvalidateOptionsMenu();
        mHandler.removeCallbacks(mBatteryRunner);
    }

    /**
     * Whenever all services inside BLE device are discovered, this event will be fire.
     * Subclasses MUST override this method and can call any required requests to get data from here.
     * Note that this method is called in BLE thread. Invoking UI actions need to be done in main thread.
     *
     * @param e
     */
    @Subscribe(threadMode =ThreadMode.MAIN)
    public void onEvent(BLEStateEvent.ServiceDiscovered e) {
        invokeBatteryCheck();
        if (BLEService.INSTANCE.getService(BLEAttributes.DEVICE_INFORMATION_SERVICE) != null) {
            if (mInfoButton != null) {
                runOnUiThread(new Runnable() {

                    @Override
                    public void run() {
                        mInfoButton.setVisibility(View.VISIBLE);
                    }
                });
            }
        }
    }
    @Subscribe(threadMode =ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.PHYUpdated e){
        if(e.status== BluetoothGatt.GATT_SUCCESS){
            Toast.makeText(this,"PHY update success", Toast.LENGTH_LONG).show();
        }else{
            Toast.makeText(this,"PHY update fail", Toast.LENGTH_LONG).show();
        }
    }
    @Subscribe(threadMode =ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.PHYReaded e){
        View layout = getLayoutInflater().inflate(R.layout.dialog_set_phy, (ViewGroup) findViewById(R.id.dialog_set_phy));
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setView(layout).setTitle("PHY Value").setPositiveButton("OK",null).setCancelable(false).show();
        layout.setEnabled(false);
        // tx phy
        CheckBox cb_1m_tx = (CheckBox) layout.findViewById(R.id.r_tx_1m);


        if(e.txPhy== BluetoothDevice.PHY_LE_1M_MASK){
            cb_1m_tx.setChecked(true);
            cb_1m_tx.setClickable(false);
        }else{
            cb_1m_tx.setVisibility(View.GONE);
        }

        CheckBox cb_2m_tx = (CheckBox) layout.findViewById(R.id.r_tx_2m);

        if(e.txPhy== BluetoothDevice.PHY_LE_2M_MASK){
            cb_2m_tx.setChecked(true);
            cb_2m_tx.setClickable(false);

        }else{
            cb_2m_tx.setVisibility(View.GONE);
        }

        CheckBox cb_c_tx = (CheckBox) layout.findViewById(R.id.r_tx_c);
        if(e.txPhy== BluetoothDevice.PHY_LE_CODED){
            cb_c_tx.setChecked(true);
            cb_c_tx.setClickable(false);

        }else{
            cb_c_tx.setVisibility(View.GONE);
        }
        //rx phy

        CheckBox cb_1m_rx = (CheckBox) layout.findViewById(R.id.r_rx_1m);
        if(e.rxPhy== BluetoothDevice.PHY_LE_1M_MASK){
            cb_1m_rx.setChecked(true);
            cb_1m_rx.setClickable(false);

        }else{
            cb_1m_rx.setVisibility(View.GONE);
        }


        CheckBox cb_2m_rx = (CheckBox) layout.findViewById(R.id.r_rx_2m);
        if(e.rxPhy== BluetoothDevice.PHY_LE_2M_MASK){
            cb_2m_rx.setChecked(true);
            cb_2m_rx.setClickable(false);

        }else{
            cb_2m_rx.setVisibility(View.GONE);
        }

        CheckBox cb_c_rx = (CheckBox) layout.findViewById(R.id.r_rx_c);
        if(e.rxPhy == BluetoothDevice.PHY_LE_CODED){
            cb_c_rx.setChecked(true);
            cb_c_rx.setClickable(false);

        }else{
            cb_c_rx.setVisibility(View.GONE);
        }
    }

    /**
     * BLE device has sent us a data package. Parse it and extract assigned number to do action if needed.
     * Subclasses MUST override this method to handle corresponding data.
     *
     * @param e
     */
    @Subscribe(threadMode =ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.DataAvailable e) {

        updateBatteryInfo(e.characteristic);
    }

    /**
     * Subclasses SHOULD have a view with id of {@link R.id#status_view_info} so it can receive the click handler automatically to view device info.
     */
    @OnClick(R.id.status_view_info)
    public void viewDeviceInfo() {
        CharSequence title = getSupportActionBar() != null ? getSupportActionBar().getSubtitle() : getTitle();
        if (TextUtils.isEmpty(title)) {
            title = getString(R.string.app_name);
        }
        DeviceInfoDialog.newInstance(this, title.toString()).show();
    }

    @Subscribe(threadMode =ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.BluetoothClientStateChanged e) {

    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        return true;
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        if(isShowMenuOption){
            if(!SdkUtils.hasO()){
//                menu.findItem(R.id.menu_read_phy).setVisible(false);
//                menu.findItem(R.id.menu_s_phy).setVisible(false);
                //menu.findItem(R.id.menu_read_phy).setEnabled(false);
                //menu.findItem(R.id.menu_s_phy).setEnabled(false);
            }
            if (menu.findItem(R.id.menu_connect) != null) {
                if (BLEService.INSTANCE.getConnectionState() == BLEService.State.STATE_DISCONNECTED) {
                    menu.findItem(R.id.menu_connect).setTitle(R.string.menu_connect);
                } else {
                    menu.findItem(R.id.menu_connect).setTitle(R.string.menu_disconnect);
                }
            }
            return true;
        }else {
            return false;
        }
    }

    public void releaseConnection(){
        if(EventBus.getDefault().isRegistered(this))
        {
            BLEService.INSTANCE.disconnect();
            EventBus.getDefault().unregister(this);
        }

    }
    
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            releaseConnection();
            finish();
            return true;
        }

        if (item.getItemId() == R.id.menu_connect) {
            if (BLEService.INSTANCE.getConnectionState() == BLEService.State.STATE_DISCONNECTED) {
                toggleState(true);
            } else {
                toggleState(false);
                onEventMainThread(new BLEStateEvent.Disconnected());
            }
        }
        if(item.getItemId() == R.id.menu_s_phy){
            showPhySettings();
        }
        if(item.getItemId() == R.id.menu_read_phy){
            showPhy();
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == REQUEST_ENABLE_BT) {
            if (resultCode == RESULT_OK) {
                toggleState(true);
            }
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        isShowMenuOption = true;
        mPhy = getIntent().getIntExtra(INTENT_KEY_PHY, BluetoothDevice.PHY_LE_1M_MASK);
        mDeviceAddress = getIntent().getStringExtra(INTENT_KEY_ADDRESS);
        mDeviceName = getIntent().getStringExtra(INTENT_KEY_NAME);

        EventBus.getDefault().register(this);
    }

    @TargetApi(Build.VERSION_CODES.M)
    @Override
    protected void onResume() {
        super.onResume();

        if (SdkUtils.hasMarshmallow()) {
            if (PackageManager.PERMISSION_GRANTED != checkSelfPermission(Manifest.permission.ACCESS_COARSE_LOCATION)) {
                Toast.makeText(this, R.string.grant_permission, Toast.LENGTH_SHORT).show();
                finish();
                return;
            }
        }
    }

    /**
     * Automatically clear and unregister event handler.
     */
    @Override
    protected void onDestroy() {
        super.onDestroy();
        EventBus.getDefault().unregister(this);

        if(BLEService.INSTANCE.getConnectionState() != BLEService.State.STATE_DISCONNECTED) {
            releaseConnection();
        }
    }

    /**
     * Connect, or disconnect from BLE device.
     *
     * @param connected
     */
    protected void toggleState(boolean connected) {
        Log.d(TAG, "toggleState:" + connected);
        if (connected) {
            if (BLEService.INSTANCE.isBluetoothAvailable()) {
                Log.d(TAG, "connect address: " + mDeviceAddress);
                BLEService.INSTANCE.connect(mDeviceAddress, needUartSupport(),mPhy);
            } else {
                Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
            }
        } else {
            BLEService.INSTANCE.disconnect();
        }
    }

    /**
     * Update battery info with a piece of data. Will need to check for assigned number first.
     *
     * @param characteristic
     */
    protected void updateBatteryInfo(@NonNull BluetoothGattCharacteristic characteristic) {
        int assignedNumber = BLEConverter.getAssignedNumber(characteristic.getUuid());
        if (BLEAttributes.BATTERY_LEVEL == assignedNumber) {
            if (mStatusBattery != null) {
                int batteryLevel = characteristic.getIntValue(BluetoothGattCharacteristic.FORMAT_UINT8, 0);
                int batteryRes = batteryLevel <= 25 ? R.drawable.ic_empty_battery :
                        (batteryLevel >= 75 ? R.drawable.ic_full_battery : R.drawable.ic_half_battery);
                mStatusBattery.setVisibility(View.VISIBLE);
                mStatusBattery.setText(batteryLevel + "%");
                mStatusBattery.setCompoundDrawablesWithIntrinsicBounds(0, 0, batteryRes, 0);
            }
        }
    }

    /**
     * Preodically invoke battery check by {@link AppConfig#BATTERY_UPDATE_INTERVAL}.
     */
    protected void invokeBatteryCheck() {
        boolean batteryServiceFound = BLEService.INSTANCE.request(BLEAttributes.BATTERY_SERVICE,
                BLEAttributes.BATTERY_LEVEL, BLEService.Request.READ);
        if (mStatusBattery != null && !batteryServiceFound) {
            mHandler.post(new Runnable() {

                @Override
                public void run() {
                    mStatusBattery.setVisibility(View.GONE);
                }
            });
        }
        mHandler.removeCallbacks(mBatteryRunner);
        mHandler.postDelayed(mBatteryRunner, AppConfig.BATTERY_UPDATE_INTERVAL);
    }

    protected boolean needUartSupport() {
        return true;
    }


    protected void removeBatteryCheck(){
        mHandler.removeCallbacks(mBatteryRunner);
    }


    public void showPhy(){
        BLEService.INSTANCE.readPhy();
    }

    int tx_phy = BluetoothDevice.PHY_LE_1M_MASK;
    int rx_phy = BluetoothDevice.PHY_LE_1M_MASK;
    int option_phy = BluetoothDevice.PHY_OPTION_S2;
    boolean[] tx_bools;// = new boolean[]{true,false,false};
    boolean[] rx_bools;// = new boolean[]{true,false,false};
    private void resetBools(){
        if(tx_bools == null){
            tx_bools = new boolean[]{true,false,false};
        }else {
            tx_bools[0] = true;
            tx_bools[1] = false;
            tx_bools[2] = false;
        }

        if(rx_bools == null){
            rx_bools = new boolean[]{true,false,false};
        }else {
            rx_bools[0] = true;
            rx_bools[1] = false;
            rx_bools[2] = false;
        }

    }


    private boolean checkOK(){
        boolean tx_result = false;
        boolean rx_result = false;
        for (int i = 0;i<tx_bools.length;i++){
            tx_result = tx_result||tx_bools[i];
            rx_result = rx_result||rx_bools[i];
        }
        return (tx_result&&rx_result);
    }

    protected void showPhySettings(){
        if(!SdkUtils.hasO())return;
        resetBools();
        View layout = getLayoutInflater().inflate(R.layout.dialog_set_phy, (ViewGroup) findViewById(R.id.dialog_set_phy));


        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        AlertDialog dialog =  builder.setView(layout).setPositiveButton("OK", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialogInterface, int i) {
                if(tx_bools[0]){
                    tx_phy = BluetoothDevice.PHY_LE_1M_MASK;
                }
                if(tx_bools[1]){
                    tx_phy = BluetoothDevice.PHY_LE_2M_MASK;
                }
                if(tx_bools[2]){
                    tx_phy = BluetoothDevice.PHY_LE_CODED_MASK;
                    option_phy = BluetoothDevice.PHY_OPTION_S2;
                }

                if(rx_bools[0]){
                    rx_phy = BluetoothDevice.PHY_LE_1M_MASK;
                }
                if(rx_bools[1]){
                    rx_phy = BluetoothDevice.PHY_LE_2M_MASK;
                }
                if(rx_bools[2]){
                    rx_phy = BluetoothDevice.PHY_LE_CODED_MASK;
                }

                BLEService.INSTANCE.updatePreferredPhy(tx_phy,rx_phy,option_phy);
            }
        }).setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialogInterface, int i) {

            }
        }).setTitle("Set preferred PHY").setCancelable(false).show();

        final Button ok = dialog.getButton(AlertDialog.BUTTON_POSITIVE);

        CheckBox cb_1m_tx = (CheckBox) layout.findViewById(R.id.r_tx_1m);
        cb_1m_tx.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                tx_bools[0]=b;
                ok.setEnabled(checkOK());
            }
        });
        CheckBox cb_2m_tx = (CheckBox) layout.findViewById(R.id.r_tx_2m);
        cb_2m_tx.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                tx_bools[1]=b;
                ok.setEnabled(checkOK());
            }
        });
        CheckBox cb_c_tx = (CheckBox) layout.findViewById(R.id.r_tx_c);
        final RadioGroup rg_tx = (RadioGroup)layout.findViewById(R.id.rg_coded);
        rg_tx.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(RadioGroup radioGroup, int i) {
                switch (i){
                    case R.id.r_s2:
                        option_phy = BluetoothDevice.PHY_OPTION_S2;
                        break;
                    case R.id.r_s8:
                        option_phy = BluetoothDevice.PHY_OPTION_S8;
                        break;
                    case R.id.r_nm:
                        option_phy = BluetoothDevice.PHY_OPTION_NO_PREFERRED;
                        break;
                }
            }
        });
        cb_c_tx.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                tx_bools[2]=b;
                if(b){
                    rg_tx.setVisibility(View.VISIBLE);
                }else {
                    rg_tx.setVisibility(View.GONE);
                }
                ok.setEnabled(checkOK());
            }
        });


        CheckBox cb_1m_rx = (CheckBox) layout.findViewById(R.id.r_rx_1m);
        cb_1m_rx.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                rx_bools[0]=b;
                ok.setEnabled(checkOK());
            }
        });
        CheckBox cb_2m_rx = (CheckBox) layout.findViewById(R.id.r_rx_2m);
        cb_2m_rx.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                rx_bools[1]=b;
                ok.setEnabled(checkOK());
            }
        });
        CheckBox cb_c_rx = (CheckBox) layout.findViewById(R.id.r_rx_c);
        cb_c_rx.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                rx_bools[2]=b;
                ok.setEnabled(checkOK());
            }
        });

    }
}
