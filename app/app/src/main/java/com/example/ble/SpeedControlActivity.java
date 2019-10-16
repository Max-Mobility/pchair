package com.example.ble;

import android.app.Activity;
import android.bluetooth.BluetoothGattCharacteristic;
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

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.speed_control);

        final Intent intent = getIntent();
        mDeviceName = intent.getStringExtra(EXTRAS_DEVICE_NAME);
        mDeviceAddress = intent.getStringExtra(EXTRAS_DEVICE_ADDRESS);
        Log.e(TAG, "get Intent." + mDeviceName);

        getActionBar().setTitle(mDeviceName);

        addListenerOnImageView();

    }

    private void addListenerOnImageView() {
        ImageView imageView_low = findViewById(R.id.speed_low);
        ImageView imageView_mid = findViewById(R.id.speed_mid);
        ImageView imageView_high = findViewById(R.id.speed_high);

        imageView_low.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Log.e(TAG, "Speed low.");
                setBackground(R.id.speed_low);
                sendChoiceToBluetooth("speed_low", DeviceControlActivity.mBluetoothGattCharacteristic);
            }
        });

        imageView_mid.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Log.e(TAG, "Speed mid.");
                setBackground(R.id.speed_mid);
                sendChoiceToBluetooth("speed_mid", DeviceControlActivity.mBluetoothGattCharacteristic);
            }
        });

        imageView_high.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Log.e(TAG, "Speed high.");
                setBackground(R.id.speed_high);
                sendChoiceToBluetooth("speed_high", DeviceControlActivity.mBluetoothGattCharacteristic);
            }
        });
    }

    private void sendChoiceToBluetooth(String string, BluetoothGattCharacteristic characteristic) {
        int button;
        switch (string) {
            case "speed_low":
                button = 11;
                break;
            case "speed_mid":
                button = 12;
                break;
            case "speed_high":
                button = 13;
                break;
            default:
                throw new IllegalStateException("Unexpected value: " + string);
        }
        DeviceControlActivity.mBluetoothLeService.writeCharacteristic(characteristic, button);
    }

    private void setBackground(int image) {
        findViewById(R.id.speed_low).setBackgroundColor(0x00000000);
        findViewById(R.id.speed_mid).setBackgroundColor(0x00000000);
        findViewById(R.id.speed_high).setBackgroundColor(0x00000000);
        findViewById(image).setBackgroundColor(Color.parseColor("#009688"));
    }


}
