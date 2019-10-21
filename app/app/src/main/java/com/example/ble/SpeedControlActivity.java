package com.example.ble;

import android.app.Activity;
import android.bluetooth.BluetoothGattCharacteristic;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;

import androidx.annotation.Nullable;

public class SpeedControlActivity extends Activity {

    private final static String TAG = SpeedControlActivity.class.getSimpleName();

    public static final String EXTRAS_DEVICE_NAME = "DEVICE_NAME";
    public static final String EXTRAS_DEVICE_ADDRESS = "DEVICE_ADDRESS";

    private String mDeviceName;
    private String mDeviceAddress;

    private ImageView imageView_back;
    private boolean mConnected = false;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.speed_control);

        final Intent intent = getIntent();
        mDeviceName = intent.getStringExtra(EXTRAS_DEVICE_NAME);
        mDeviceAddress = intent.getStringExtra(EXTRAS_DEVICE_ADDRESS);
        Log.e(TAG, "get Intent." + mDeviceName);

        getActionBar().setTitle(mDeviceName);

        imageView_back = findViewById(R.id.back_to_run);
        addListenerOnImageView();
    }

    private void addListenerOnImageView() {
        imageView_back.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.e(TAG, "back");
                byte[] button = new byte[2];
                button[0] = (byte) 1;
                button[1] = (byte) 0;
                DeviceControlActivity.mBluetoothLeService.writeCharacteristic(DeviceControlActivity.mBluetoothGattCharacteristic, button);

                final Intent intent = new Intent(SpeedControlActivity.this, DeviceControlActivity.class);
                intent.putExtra(DeviceControlActivity.EXTRAS_DEVICE_NAME, mDeviceName);
                intent.putExtra(DeviceControlActivity.EXTRAS_DEVICE_ADDRESS, mDeviceAddress);
                startActivity(intent);
            }
        });
    }

}
