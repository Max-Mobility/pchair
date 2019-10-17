package com.example.ble;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.graphics.Color;
import android.graphics.drawable.Drawable;
import android.media.Image;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ExpandableListView;
import android.widget.ImageView;
import android.widget.SimpleExpandableListAdapter;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;

public class DeviceControlActivity extends Activity {

    private final static String TAG = DeviceControlActivity.class.getSimpleName();

    public static final String EXTRAS_DEVICE_NAME = "DEVICE_NAME";
    public static final String EXTRAS_DEVICE_ADDRESS = "DEVICE_ADDRESS";
    public static BluetoothGattCharacteristic mNotifyCharacteristic, mBluetoothGattCharacteristic;
    public static BluetoothLeService mBluetoothLeService;

    private TextView mConnectionState;
    private TextView mDataField;
    private String mDeviceName;
    private String mDeviceAddress;
    private String mSpeed;
    private ExpandableListView mGattServicesList;
    private ArrayList<ArrayList<BluetoothGattCharacteristic>> mGattCharacteristics = new ArrayList<>();
    private boolean mConnected = false;

    private ImageView imageView_0;
    private ImageView imageView_1;
    private ImageView imageView_2;
    private ImageView imageView_3;
    private ImageView imageView_4;
    private ImageView imageView_zoom;
    private ImageView imageView_plus;
    private ImageView imageView_minus;

    private ImageView imageView_run;
    private ImageView imageView_sleep;
    private ImageView imageView_low;
    private ImageView imageView_mid;
    private ImageView imageView_high;

    static private class system_modes {
        static byte DriveMode = 0;
        static byte Actuator_recline = 1;
        static byte Actuator_legrest = 2;
        static byte Actuator_tilt = 3;
        static byte Actuator_elevation = 4;
        static byte Actuator_stand = 5;
        static byte System_sleep = 6;
        static byte ErrorMode = 7;
        static byte system_mode_max = 8;
    }

    static private class Command {
        static byte CMD_SET_SPEED = 0;
        static byte CMD_CHANGE_SYSTEM_MODE = 1;
        static byte CMD_MOVE_ACTUATOR = 2;
    }

    static private class Actuator_moving_dir {
        static byte AMD_left = 0;
        static byte AMD_right = 1;
        static byte AMD_stop = 2;
    }

    static private class Speed_setting {
        static byte speed_low = 0;
        static byte speed_medium = 1;
        static byte speed_high = 2;
    }

    public static String mBluetooth = "BUSY";

    private final ServiceConnection mServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName componentName, IBinder service) {
            mBluetoothLeService = ((BluetoothLeService.LocalBinder) service).getService();
            if (!mBluetoothLeService.initialize()) {
                Log.e(TAG, "Unable to initialize Bluetooth.");
                finish();
            }
            mBluetoothLeService.connect(mDeviceAddress);
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            mBluetoothLeService = null;
        }
    };

    private final ExpandableListView.OnChildClickListener servicesListClickListener = new ExpandableListView.OnChildClickListener() {
        @Override
        public boolean onChildClick(ExpandableListView expandableListView, View view, int groupPosition, int childPosition, long l) {

            if (mGattCharacteristics != null) {

                final BluetoothGattCharacteristic characteristic = mGattCharacteristics.get(groupPosition).get(childPosition);
                final int charaProp = characteristic.getProperties();
                Log.e(TAG, "charaProp: " + charaProp);

                if ((charaProp & BluetoothGattCharacteristic.PROPERTY_READ) > 0) {

                    Log.e(TAG, "PROPERTY_READ." + characteristic.getUuid());

                    // Clear Notification so it does not update user interface.
                    if (mNotifyCharacteristic != null) {
                        mBluetoothLeService.setCharacteristicNotification(mNotifyCharacteristic, false);
                        mNotifyCharacteristic = null;
                    }
                    mBluetoothLeService.readCharacteristic(characteristic);
                }

                if ((charaProp & BluetoothGattCharacteristic.PROPERTY_NOTIFY) > 0) {
                    Log.e(TAG, "PROPERTY_NOTIFY." + characteristic.getUuid());
                    mNotifyCharacteristic = characteristic;
                    mBluetoothLeService.setCharacteristicNotification(characteristic, true);
                }
                return true;
            }
            return false;
        }
    };

    private final BroadcastReceiver mGattUpdateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();
            if (BluetoothLeService.ACTION_GATT_CONNECTED.equals(action)) {
                mConnected = true;
                updateConnectionState(R.string.Connected);
                invalidateOptionsMenu();
            } else if (BluetoothLeService.ACTION_GATT_DISCONNECTED.equals(action)) {
                mConnected = false;
                updateConnectionState(R.string.disconnected);
                invalidateOptionsMenu();
                clearUI();
            } else if (BluetoothLeService.ACTION_GATT_SERVICES_DISCOVERED.equals(action)) {
                //displayGattServices(mBluetoothLeService.getSupportedGattServices());
                displayCharacteristic(mBluetoothLeService.getSupportedGattCharacteristic());

            } else if (BluetoothLeService.ACTION_DATA_AVAILABLE.equals(action)) {
                //displayData(intent.getStringExtra(BluetoothLeService.EXTRA_DATA));
                displayData(intent.getIntExtra(BluetoothLeService.EXTRA_DATA, 0));
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.gatt_services_characteristics);

        final Intent intent = getIntent();
        mDeviceName = intent.getStringExtra(EXTRAS_DEVICE_NAME);
        mDeviceAddress = intent.getStringExtra(EXTRAS_DEVICE_ADDRESS);
        //Log.e(TAG, "Device Address: " + mDeviceAddress);
        Log.e(TAG, "Device Name: " + mDeviceName);

        //Setup UI.
        ((TextView) findViewById(R.id.device_address)).setText(mDeviceAddress);
        //mGattServicesList = findViewById(R.id.gatt_services_list);
        //mGattServicesList.setOnChildClickListener(servicesListClickListener);
        mConnectionState = findViewById(R.id.connection_state);
        mDataField = findViewById(R.id.data_value);

        getActionBar().setTitle(mDeviceName);
        //getActionBar().setDisplayHomeAsUpEnabled(true);
        //getActionBar().setDisplayShowHomeEnabled(true);
        Intent gattServiceIntent = new Intent(this, BluetoothLeService.class);
        bindService(gattServiceIntent, mServiceConnection, BIND_AUTO_CREATE);

        mSpeed = "speed_low";

        imageView_0 = findViewById(R.id.image_0);
        imageView_1 = findViewById(R.id.image_1);
        imageView_2 = findViewById(R.id.image_2);
        imageView_3 = findViewById(R.id.image_3);
        imageView_4 = findViewById(R.id.image_4);
        imageView_zoom = findViewById(R.id.image_zoom);
        imageView_plus = findViewById(R.id.plusButton);
        imageView_minus = findViewById(R.id.minusButton);

        imageView_run = findViewById(R.id.run);
        imageView_sleep = findViewById(R.id.sleep);
        imageView_low = findViewById(R.id.speed_low);
        imageView_mid = findViewById(R.id.speed_mid);
        imageView_high = findViewById(R.id.speed_high);

        addListenerOnImageView();
        setShownButton(R.id.sleep);
    }

    @SuppressLint("ClickableViewAccessibility")
    public void addListenerOnImageView() {

        // update function and BLE activity

        imageView_plus.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View view, MotionEvent event) {
                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    Log.e(TAG, "plus: MOTION_DOWN. " + mBluetoothGattCharacteristic);
                    sendChoiceToBluetooth("plus down", mBluetoothGattCharacteristic);
                }
                if (event.getAction() == MotionEvent.ACTION_UP) {
                    Log.e(TAG, "plus: MOTION_UP. " + mBluetoothGattCharacteristic);
                    sendChoiceToBluetooth("plus up", mBluetoothGattCharacteristic);
                }
                return false;
            }
        });

        imageView_minus.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View view, MotionEvent event) {
                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    Log.e(TAG, "minus: MOTION_DOWN. " + mBluetoothGattCharacteristic);
                    sendChoiceToBluetooth("minus down", mBluetoothGattCharacteristic);
                }
                if (event.getAction() == MotionEvent.ACTION_UP) {
                    Log.e(TAG, "minus: MOTION_UP. " + mBluetoothGattCharacteristic);
                    sendChoiceToBluetooth("minus up", mBluetoothGattCharacteristic);
                }
                return false;
            }
        });

        imageView_run.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Log.e(TAG, "run");
                setShownButton(R.id.run);
                if(mSpeed.equals("speed_mid")){
                    setBackground(R.id.speed_mid);
                } else if (mSpeed.equals("speed_high")){
                    setBackground(R.id.speed_high);
                } else {
                    mSpeed="speed_low";
                    setBackground(R.id.speed_low);
                }

                sendChoiceToBluetooth("run", mBluetoothGattCharacteristic);


                // call SpeedControlActivity.java
//                final Intent intent = new Intent(DeviceControlActivity.this, SpeedControlActivity.class);
//                intent.putExtra(SpeedControlActivity.EXTRAS_DEVICE_NAME, mDeviceName);
//                intent.putExtra(SpeedControlActivity.EXTRAS_DEVICE_ADDRESS, mDeviceAddress);
//                startActivity(intent);
            }
        });

        imageView_sleep.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Log.e(TAG, "sleep");
                sendChoiceToBluetooth("sleep", mBluetoothGattCharacteristic);

            }
        });

        imageView_0.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Log.e(TAG, "Image_0");
                setBackground(R.id.image_0);
                setShownButton(R.id.image_0);
                setZoomImage(R.id.image_0);
                sendChoiceToBluetooth("image_recline", mBluetoothGattCharacteristic);
            }
        });

        imageView_1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Log.e(TAG, "Image_1");
                setBackground(R.id.image_1);
                setShownButton(R.id.image_1);
                setZoomImage(R.id.image_1);
                sendChoiceToBluetooth("image_leg", mBluetoothGattCharacteristic);
            }
        });

        imageView_2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Log.e(TAG, "Image_2");
                setBackground(R.id.image_2);
                setShownButton(R.id.image_2);
                setZoomImage(R.id.image_2);
                sendChoiceToBluetooth("image_tilt", mBluetoothGattCharacteristic);
            }
        });

        imageView_3.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Log.e(TAG, "Image_3");
                setBackground(R.id.image_3);
                setShownButton(R.id.image_3);
                setZoomImage(R.id.image_3);
                sendChoiceToBluetooth("image_elevation", mBluetoothGattCharacteristic);
            }
        });

        imageView_4.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Log.e(TAG, "Image_4");
                setBackground(R.id.image_4);
                setShownButton(R.id.image_4);
                setZoomImage(R.id.image_4);
                sendChoiceToBluetooth("image_stand", mBluetoothGattCharacteristic);
            }
        });

        imageView_low.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Log.e(TAG, "Speed low.");
                mSpeed = "speed_low";
                setBackground(R.id.speed_low);
                sendChoiceToBluetooth("speed_low", mBluetoothGattCharacteristic);
            }
        });

        imageView_mid.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Log.e(TAG, "Speed mid.");
                mSpeed = "speed_mid";
                setBackground(R.id.speed_mid);
                sendChoiceToBluetooth("speed_mid", mBluetoothGattCharacteristic);
            }
        });

        imageView_high.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Log.e(TAG, "Speed high.");
                mSpeed = "speed_high";
                setBackground(R.id.speed_high);
                sendChoiceToBluetooth("speed_high", mBluetoothGattCharacteristic);
            }
        });
    }

    private void setShownButton(int image) {

        switch (image) {
            case R.id.run:
                imageView_low.setVisibility(View.VISIBLE);
                imageView_mid.setVisibility(View.VISIBLE);
                imageView_high.setVisibility(View.VISIBLE);

                imageView_plus.setVisibility(View.INVISIBLE);
                imageView_minus.setVisibility(View.INVISIBLE);
                imageView_zoom.setVisibility(View.INVISIBLE);
                break;
            default:
                imageView_low.setVisibility(View.INVISIBLE);
                imageView_mid.setVisibility(View.INVISIBLE);
                imageView_high.setVisibility(View.INVISIBLE);

                imageView_plus.setVisibility(View.VISIBLE);
                imageView_minus.setVisibility(View.VISIBLE);
                imageView_zoom.setVisibility(View.VISIBLE);
        }
    }

    private void setZoomImage(int image) {

        switch (image) {
            case R.id.image_0:
                imageView_zoom.setImageResource(R.drawable.btn_star_big_off);
                break;
            case R.id.image_1:
                imageView_zoom.setImageResource(R.drawable.btn_star_big_on);
                break;
            case R.id.image_2:
                imageView_zoom.setImageResource(R.drawable.btn_star_big_on_disable);
                break;
            case R.id.image_3:
                imageView_zoom.setImageResource(R.drawable.btn_star_big_on_disable_focused);
                break;
            case R.id.image_4:
                imageView_zoom.setImageResource(R.drawable.btn_star_big_on_pressed);
                break;
            default:
                imageView_zoom.setImageResource(R.drawable.sleep);
        }
    }

    private void sendChoiceToBluetooth(String string, BluetoothGattCharacteristic characteristic) {
        // Send click information to bluetooth
        byte[] button = new byte[2];

        switch (string) {
            case "image_recline":
                button[0] = Command.CMD_CHANGE_SYSTEM_MODE;
                button[1] = system_modes.Actuator_recline;
                break;
            case "image_leg":
                button[0] = Command.CMD_CHANGE_SYSTEM_MODE;
                button[1] = system_modes.Actuator_legrest;
                break;
            case "image_tilt":
                button[0] = Command.CMD_CHANGE_SYSTEM_MODE;
                button[1] = system_modes.Actuator_tilt;
                break;
            case "image_elevation":
                button[0] = Command.CMD_CHANGE_SYSTEM_MODE;
                button[1] = system_modes.Actuator_elevation;
                break;
            case "image_stand":
                button[0] = Command.CMD_CHANGE_SYSTEM_MODE;
                button[1] = system_modes.Actuator_stand;
                break;
            case "plus down":
                button[0] = Command.CMD_MOVE_ACTUATOR;
                button[1] = Actuator_moving_dir.AMD_right;
                break;
            case "plus up":
            case "minus up":
                button[0] = Command.CMD_MOVE_ACTUATOR;
                button[1] = Actuator_moving_dir.AMD_stop;
                break;
            case "minus down":
                button[0] = Command.CMD_MOVE_ACTUATOR;
                button[1] = Actuator_moving_dir.AMD_left;
                break;
            case "run":
                button[0] = Command.CMD_CHANGE_SYSTEM_MODE;
                button[1] = system_modes.DriveMode;
                break;
            case "sleep":
                button[0] = Command.CMD_CHANGE_SYSTEM_MODE;
                button[1] = system_modes.System_sleep;
                break;
            case "speed_low":
                button[0] = Command.CMD_SET_SPEED;
                button[1] = Speed_setting.speed_low;
                break;
            case "speed_mid":
                button[0] = Command.CMD_SET_SPEED;
                button[1] = Speed_setting.speed_medium;
                break;
            case "speed_high":
                button[0] = Command.CMD_SET_SPEED;
                button[1] = Speed_setting.speed_high;
                break;
            default:
                throw new IllegalStateException("Unexpected value: " + string);
        }
        mBluetoothLeService.writeCharacteristic(characteristic, button);
    }

    @Override
    protected void onResume() {
        super.onResume();

        registerReceiver(mGattUpdateReceiver, makeGattUpdateIntentFilter());

        if (mBluetoothLeService != null) {
            final boolean result = mBluetoothLeService.connect(mDeviceAddress);
            Log.e(TAG, "Connect request result= " + result);
        }
    }

    private void updateConnectionState(final int resourceId) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mConnectionState.setText(resourceId);
            }
        });
    }

    private void clearUI() {
        //mGattServicesList.setAdapter((SimpleExpandableListAdapter) null);
        mDataField.setText(R.string.no_data);
    }

    private void displayCharacteristic(BluetoothGattCharacteristic characteristic) {

        final int charaProp = characteristic.getProperties();

        if ((charaProp & BluetoothGattCharacteristic.PROPERTY_READ) > 0 && (charaProp & BluetoothGattCharacteristic.PROPERTY_NOTIFY) > 0 && (charaProp & BluetoothGattCharacteristic.PROPERTY_WRITE) > 0 && mBluetooth == "IDLE") {

            mBluetooth = "BUSY";

            Log.e(TAG, "PROPERTY_READ & NOTIFY." + characteristic.getUuid());

            // Clear Notification so it does not update user interface.
            if (mNotifyCharacteristic != null) {
                mBluetoothLeService.setCharacteristicNotification(mNotifyCharacteristic, false);
                mNotifyCharacteristic = null;
            }


            mBluetoothLeService.readCharacteristic(characteristic);
            mBluetoothGattCharacteristic = characteristic;
            sendChoiceToBluetooth(mSpeed,mBluetoothGattCharacteristic);
        }

//        if ((charaProp & BluetoothGattCharacteristic.PROPERTY_NOTIFY) > 0 && mBluetooth == "IDLE") {
//            mBluetooth = "BUSY";
//
//            Log.e(TAG, "PROPERTY_NOTIFY." + characteristic.getUuid());
//            mNotifyCharacteristic = characteristic;
//            mBluetoothLeService.setCharacteristicNotification(characteristic, true);
//        }

//        if ((charaProp & BluetoothGattCharacteristic.PROPERTY_WRITE) > 0 && mBluetooth == "IDLE") {
//            mBluetooth="BUSY";
//
//            Log.e(TAG, "PROPERTY_WRITE." + characteristic.getUuid());
//            BluetoothGattCharacteristic wCharacteristic = characteristic;
//            byte[] writev = new byte[1];
//            Arrays.fill(writev, (byte) '0');
//            wCharacteristic.setValue(writev);
//            wCharacteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
//            Log.e(TAG, "PROPERTY_WRITE." + writev);
//            mBluetoothLeService.writeCharacteristic(wCharacteristic);
//        }
    }

    private void displayGattServices(List<BluetoothGattService> gattServices) {

        if (gattServices == null) return;

        String uuid = null;
        String unknownServiceString = getResources().getString(R.string.unknown_service);
        String unknownCharaString = getResources().getString(R.string.unknown_characteristic);

        ArrayList<HashMap<String, String>> gattServiceData = new ArrayList<>();
        ArrayList<ArrayList<HashMap<String, String>>> gattCharacteristicData = new ArrayList<>();
        mGattCharacteristics = new ArrayList<>();

        //Loops through available GATT Services.
        String LIST_UUID = "UUID";
        String LIST_NAME = "NAME";
        for (BluetoothGattService gattService : gattServices) {

            HashMap<String, String> currentServiceData = new HashMap<>();
            uuid = gattService.getUuid().toString();
            currentServiceData.put(LIST_NAME, SampleGattAttributes.lookup(uuid, unknownServiceString));
            currentServiceData.put(LIST_UUID, uuid);
            gattServiceData.add(currentServiceData);

            ArrayList<HashMap<String, String>> gattCharacteristicGroupData = new ArrayList<>();
            List<BluetoothGattCharacteristic> gattCharacteristics = gattService.getCharacteristics();
            ArrayList<BluetoothGattCharacteristic> charas = new ArrayList<>();

            for (BluetoothGattCharacteristic gattCharacteristic : gattCharacteristics) {

                charas.add(gattCharacteristic);
                HashMap<String, String> currentCharaData = new HashMap<>();
                uuid = gattCharacteristic.getUuid().toString();

                if (uuid.equals(SampleGattAttributes.POSITION_1)) {
                    currentCharaData.put(LIST_NAME, SampleGattAttributes.lookup(uuid, unknownCharaString));
                    currentCharaData.put(LIST_UUID, uuid);
                    gattCharacteristicGroupData.add(currentCharaData);
                }


            }
            mGattCharacteristics.add(charas);
            gattCharacteristicData.add(gattCharacteristicGroupData);
        }

        SimpleExpandableListAdapter gattServiceAdapter = new SimpleExpandableListAdapter(
                this,
                gattServiceData,
                android.R.layout.simple_expandable_list_item_2,
                new String[]{LIST_NAME, LIST_UUID},
                new int[]{android.R.id.text1, android.R.id.text2},
                gattCharacteristicData,
                android.R.layout.simple_expandable_list_item_2,
                new String[]{LIST_NAME, LIST_UUID},
                new int[]{android.R.id.text1, android.R.id.text2}
        );
        mGattServicesList.setAdapter(gattServiceAdapter);
    }

    private void displayData(int data) {
        //if (data != null) {
        mDataField.setText(Integer.toString(data));
        //Log.e(TAG,"data: "+data);

        if (data % 5 == 0) {
            setBackground(R.id.image_0);
            setZoomImage(R.id.image_0);
        }
        if (data % 5 == 1) {
            setBackground(R.id.image_1);
            setZoomImage(R.id.image_1);
        }

        if (data % 5 == 2) {
            setBackground(R.id.image_2);
            setZoomImage(R.id.image_2);
        }
        if (data % 5 == 3) {
            setBackground(R.id.image_3);
            setZoomImage(R.id.image_3);
        }
        if (data % 5 == 4) {
            setBackground(R.id.image_4);
            setZoomImage(R.id.image_4);
        }

    }

    private void setBackground(int image) {
        imageView_run.setBackgroundColor(Color.parseColor("#2196F3"));
        imageView_low.setBackgroundColor(0x00000000);
        imageView_mid.setBackgroundColor(0x00000000);
        imageView_high.setBackgroundColor(0x00000000);
        imageView_0.setBackgroundColor(0x00000000);
        imageView_1.setBackgroundColor(0x00000000);
        imageView_2.setBackgroundColor(0x00000000);
        imageView_3.setBackgroundColor(0x00000000);
        imageView_4.setBackgroundColor(0x00000000);
        findViewById(image).setBackgroundColor(Color.parseColor("#009688"));
        if (image == R.id.speed_low || image == R.id.speed_mid || image == R.id.speed_high) {
            imageView_run.setBackgroundColor(Color.parseColor("#00574B"));
        }
    }

    private static IntentFilter makeGattUpdateIntentFilter() {
        final IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(BluetoothLeService.ACTION_GATT_CONNECTED);
        intentFilter.addAction(BluetoothLeService.ACTION_GATT_DISCONNECTED);
        intentFilter.addAction(BluetoothLeService.ACTION_GATT_SERVICES_DISCOVERED);
        intentFilter.addAction(BluetoothLeService.ACTION_DATA_AVAILABLE);
        return intentFilter;
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        unregisterReceiver(mGattUpdateReceiver);
        unbindService(mServiceConnection);
        mBluetoothLeService = null;
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.menu_main, menu);
        if (mDeviceAddress != null) {
            menu.findItem(R.id.scan).setVisible(false);
        } else {
            menu.findItem(R.id.scan).setVisible(true);
        }
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                finish();
                return true;
            case R.id.scan:
                //scan function
                Log.e(TAG, "scan");
                return (true);
            case R.id.connect:
                Log.e(TAG, "connect");
                Log.e(TAG, "DeviceAddress: " + mDeviceAddress);
                mBluetoothLeService.connect(mDeviceAddress);
                return (true);
            case R.id.disconnect:
                //disconnect function
                Log.e(TAG, "disconnect");
                mBluetoothLeService.disconnect();
                return (true);
        }
        return (super.onOptionsItemSelected(item));
    }
}
