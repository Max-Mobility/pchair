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
    private ExpandableListView mGattServicesList;
    private ArrayList<ArrayList<BluetoothGattCharacteristic>> mGattCharacteristics = new ArrayList<>();
    private boolean mConnected = false;



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

        addListenerOnImageView();
    }

    @SuppressLint("ClickableViewAccessibility")
    public void addListenerOnImageView() {
        ImageView imageView_plus = findViewById(R.id.plusButton);
        ImageView imageView_minus = findViewById(R.id.minusButton);
        ImageView imageView_run = findViewById(R.id.run);
        ImageView imageView_sleep = findViewById(R.id.sleep);

        ImageView imageView_0 = findViewById(R.id.image_0);
        ImageView imageView_1 = findViewById(R.id.image_1);
        ImageView imageView_2 = findViewById(R.id.image_2);
        ImageView imageView_3 = findViewById(R.id.image_3);
        ImageView imageView_4 = findViewById(R.id.image_4);

        // update function and BLE activity

        imageView_plus.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View view, MotionEvent event) {
                if(event.getAction() == MotionEvent.ACTION_DOWN){
                    Log.e(TAG, "plus: MOTION_DOWN. " + mBluetoothGattCharacteristic);
                    sendChoiceToBluetooth("plus down", mBluetoothGattCharacteristic);
                }
                if(event.getAction() == MotionEvent.ACTION_UP){
                    Log.e(TAG, "plus: MOTION_UP. " + mBluetoothGattCharacteristic);
                    sendChoiceToBluetooth("plus up", mBluetoothGattCharacteristic);
                }
                return false;
            }
        });

        imageView_minus.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View view, MotionEvent event) {
                if(event.getAction() == MotionEvent.ACTION_DOWN){
                    Log.e(TAG, "minus: MOTION_DOWN. " + mBluetoothGattCharacteristic);
                    sendChoiceToBluetooth("minus down", mBluetoothGattCharacteristic);
                }
                if(event.getAction() == MotionEvent.ACTION_UP){
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
                sendChoiceToBluetooth("run", mBluetoothGattCharacteristic);
                // call SpeedControlActivity.java
                final Intent intent = new Intent(DeviceControlActivity.this, SpeedControlActivity.class);
                intent.putExtra(SpeedControlActivity.EXTRAS_DEVICE_NAME, mDeviceName);
                intent.putExtra(SpeedControlActivity.EXTRAS_DEVICE_ADDRESS, mDeviceAddress);
                startActivity(intent);
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
                setZoomImage(R.id.image_0);
                sendChoiceToBluetooth("image_0", mBluetoothGattCharacteristic);
            }
        });

        imageView_1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Log.e(TAG, "Image_1");
                setBackground(R.id.image_1);
                setZoomImage(R.id.image_1);
                sendChoiceToBluetooth("image_1", mBluetoothGattCharacteristic);
            }
        });

        imageView_2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Log.e(TAG, "Image_2");
                setBackground(R.id.image_2);
                setZoomImage(R.id.image_2);
                sendChoiceToBluetooth("image_2", mBluetoothGattCharacteristic);
            }
        });

        imageView_3.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Log.e(TAG, "Image_3");
                setBackground(R.id.image_3);
                setZoomImage(R.id.image_3);
                sendChoiceToBluetooth("image_3", mBluetoothGattCharacteristic);
            }
        });

        imageView_4.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Log.e(TAG, "Image_4");
                setBackground(R.id.image_4);
                setZoomImage(R.id.image_4);
                sendChoiceToBluetooth("image_4", mBluetoothGattCharacteristic);
            }
        });
    }

    private void setZoomImage(int image) {
        ImageView imageView_zoom = findViewById(R.id.image_zoom);
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
        int button;
        switch (string) {
            case "image_0":
                button = 0;
                break;
            case "image_1":
                button = 1;
                break;
            case "image_2":
                button = 2;
                break;
            case "image_3":
                button = 3;
                break;
            case "image_4":
                button = 4;
                break;
            case "plus down":
                button = 5;
                break;
            case "plus up":
                button = 6;
                break;
            case "minus down":
                button = 7;
                break;
            case "minus up":
                button = 8;
                break;
            case "run":
                button = 9;
                break;
            case "sleep":
                button = 10;
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
        findViewById(R.id.image_0).setBackgroundColor(0x00000000);
        findViewById(R.id.image_1).setBackgroundColor(0x00000000);
        findViewById(R.id.image_2).setBackgroundColor(0x00000000);
        findViewById(R.id.image_3).setBackgroundColor(0x00000000);
        findViewById(R.id.image_4).setBackgroundColor(0x00000000);
        findViewById(image).setBackgroundColor(Color.parseColor("#009688"));
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
