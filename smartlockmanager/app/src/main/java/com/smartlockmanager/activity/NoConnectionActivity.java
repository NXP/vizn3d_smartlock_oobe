package com.smartlockmanager.activity;

import static com.smartlockmanager.utility.SdkUtils.fullScreen;

import androidx.appcompat.app.AppCompatActivity;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;

import com.smartlockmanager.R;
import com.smartlockmanager.utility.ForegroundObserver;

public class NoConnectionActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        fullScreen(getWindow());

        setContentView(R.layout.activity_no_connection);
    }

    @Override
    protected void onResume() {
        super.onResume();
        ForegroundObserver.getForegroundObserver().updateActivity(this);
    }

    static public void jumpToDisconnectActivity(Activity activity)
    {
        Intent pendingIntent = new Intent();
        pendingIntent.setClass(activity, NoConnectionActivity.class);
        activity.startActivity(pendingIntent);
    }

    private void jumpToMainActivity(){
        Intent pendingIntent = new Intent();
        pendingIntent.setClass(this, MainActivity.class);
        pendingIntent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK | Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(pendingIntent);
    }

    public void onClickGoToScan(View view){
        jumpToMainActivity();
    }

    @Override
    public void onBackPressed() {
        jumpToMainActivity();
    }
}