package com.smartlockmanager.utility;

import android.app.Activity;

import androidx.lifecycle.Lifecycle;
import androidx.lifecycle.LifecycleObserver;
import androidx.lifecycle.OnLifecycleEvent;

import com.smartlockmanager.activity.BaseServiceActivity;
import com.smartlockmanager.activity.MainActivity;
import com.smartlockmanager.activity.NoConnectionActivity;

import org.greenrobot.eventbus.EventBus;

public class ForegroundObserver extends BaseServiceActivity implements LifecycleObserver {

    Activity callingActivity = null;
    private int foregroundActivities = 0;

    private static volatile ForegroundObserver OBSERVER_INSTANCE = null;

    public static ForegroundObserver getForegroundObserver() {
        if (OBSERVER_INSTANCE == null) {
            synchronized (UserInteractionTimer.class) {
                if (OBSERVER_INSTANCE == null) {
                    OBSERVER_INSTANCE = new ForegroundObserver();
                }
            }
        }
        return OBSERVER_INSTANCE;
    }

    public void updateActivity(Activity activity){
        callingActivity = activity;
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_START)
    public void onEnterForeground() {
        EventBus.getDefault().register(OBSERVER_INSTANCE);

        foregroundActivities++;
        if (foregroundActivities == 1) {
            String callingActivityName = callingActivity.getClass().getSimpleName();
            if (!callingActivityName.equals(MainActivity.class.getSimpleName()) &&
                    !callingActivityName.equals(NoConnectionActivity.class.getSimpleName())) {
                releaseConnection();
            }
        }
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_STOP)
    public void onEnterBackground() {
        foregroundActivities--;
        if (foregroundActivities == 0) {
            EventBus.getDefault().unregister(OBSERVER_INSTANCE);
        }
    }

}
