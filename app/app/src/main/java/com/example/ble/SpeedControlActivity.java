package com.example.ble;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

import androidx.annotation.Nullable;

public class SpeedControlActivity extends Activity {
    private final static String TAG = SpeedControlActivity.class.getSimpleName();

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.speed_control);

        final Intent intent = getIntent();
        Log.e(TAG, "get Intent.");

    }
}
