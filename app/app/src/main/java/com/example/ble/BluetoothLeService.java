package com.example.ble;

import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.util.Log;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.UUID;

public class BluetoothLeService extends Service {

    private final static String TAG = BluetoothLeService.class.getSimpleName();

    private BluetoothManager mBluetoothManager;
    private BluetoothAdapter mBluetoothAdapter;
    private String mBluetoothDeviceAddress;
    private BluetoothGatt mBluetoothGatt;
    private int mConnectionState = BluetoothAdapter.STATE_DISCONNECTED;
    private ArrayList<byte[]> mBluetoothArray = new ArrayList<byte[]>();

    private static final int STATE_DISCONNECTED = 0;
    private static final int STATE_CONNECTING = 1;
    private static final int STATE_CONNECTED = 2;

    public final static String ACTION_GATT_CONNECTED = "com.example.bluetooth.le.ACTION_GATT_CONNECTED";
    public final static String ACTION_GATT_DISCONNECTED = "com.example.bluetooth.le.ACTION_GATT_DISCONNECTED";
    public final static String ACTION_GATT_SERVICES_DISCOVERED = "com.example.bluetooth.le.ACTION_GATT_SERVICES_DISCOVERED";
    public final static String ACTION_DATA_AVAILABLE = "com.example.bluetooth.le.ACTION_DATA_AVAILABLE";
    public final static String EXTRA_DATA = "com.example.bluetooth.le.EXTRA_DATA";

    //public final static UUID UUID_MEASUREMENT = UUID.fromString(SampleGattAttributes.POSITION_1);

    private final IBinder mBinder = new LocalBinder();

    // Implements callback for GATT events
    private final BluetoothGattCallback mGattCallback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            //super.onConnectionStateChange(gatt, status, newState);
            String intentAction;

            if (newState == BluetoothProfile.STATE_CONNECTED) {

                intentAction = ACTION_GATT_CONNECTED;
                mConnectionState = STATE_CONNECTED;
                broadcastUpdate(intentAction);

                Log.e(TAG, "Connected to GATT server.");
                Log.e(TAG, "Attempting to start service discovery: " + mBluetoothGatt.discoverServices());

            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {

                intentAction = ACTION_GATT_DISCONNECTED;
                mConnectionState = STATE_DISCONNECTED;
                Log.e(TAG, "Disconnected from GATT server.");
                broadcastUpdate(intentAction);
            }
        }

        // callback of discoverServices().
        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                Log.e(TAG, "onServicesDiscovered");
                DeviceControlActivity.mBluetooth = "IDLE";
                broadcastUpdate(ACTION_GATT_SERVICES_DISCOVERED);
            } else {
                Log.e(TAG, "onServiceDiscovered received: " + status);
            }
        }


        // callback of readCharacteristic(..)
        @Override
        public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            //super.onCharacteristicRead(gatt, characteristic, status);
            Log.e(TAG, "onCharacteristicRead:" + status + " " + BluetoothGatt.GATT_SUCCESS);


            if (status == BluetoothGatt.GATT_SUCCESS) {
                broadcastUpdate(ACTION_DATA_AVAILABLE, characteristic);
                //DeviceControlActivity.mBluetooth = "IDLE";
                Log.e(TAG, "call: setCharacteristicNotification");
                setCharacteristicNotification(characteristic, true);
            }
        }

        // callback of setCharacteristicNotification(..).
        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
            //super.onCharacteristicChanged(gatt, characteristic);
            Log.e(TAG, "onCharacteristicChanged");
            //DeviceControlActivity.mBluetooth = "IDLE";
            broadcastUpdate(ACTION_DATA_AVAILABLE, characteristic);

        }

        // callback of writeCharacteristic(..)
        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            //super.onCharacteristicWrite(gatt, characteristic, status);
            Log.e(TAG, "onCharacteristicWrite" + status);
            DeviceControlActivity.mBluetooth = "IDLE";
            mBluetoothArray.remove(0);
            writeBLE(characteristic);
        }

        @Override
        public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
            //super.onDescriptorWrite(gatt, descriptor, status);
            Log.e(TAG, "onDescriptorWrite" + status);
            DeviceControlActivity.mBluetooth = "IDLE";
        }
    };

    private void broadcastUpdate(final String action) {
        final Intent intent = new Intent(action);
        sendBroadcast(intent);
    }

    private void broadcastUpdate(String action, BluetoothGattCharacteristic characteristic) {

        final Intent intent = new Intent(action);

        //https://www.bluetooth.com/specifications/gatt/characteristics/
//        if (UUID_MEASUREMENT.equals(characteristic.getUuid())) {
//            // Heart rate:
//            int flag = characteristic.getProperties();
//            int format = -1;
//
//            if ((flag & 0x01) != 0) {
//                format = BluetoothGattCharacteristic.FORMAT_UINT16;
//                Log.e(TAG, "Measurement format UINT16.");
//            } else {
//                format = BluetoothGattCharacteristic.FORMAT_UINT8;
//                Log.d(TAG, "Measurement format UINT8.");
//            }
//            final int measurement = characteristic.getIntValue(format, 1);
//            Log.e(TAG, "Receiving measurement: %d" + measurement);
//            intent.putExtra(EXTRA_DATA, String.valueOf(measurement));
//        } else {
//            // For all other profiles, write data formatted in HEX.
//            final byte[] data = characteristic.getValue();
//
//            if (data != null && data.length > 0) {
//
//                final StringBuilder stringBuilder = new StringBuilder(data.length);
//                for (byte byteChar : data)
//                    stringBuilder.append(String.format("%02X ", byteChar));
//                intent.putExtra(EXTRA_DATA, new String(data) + "\n" + stringBuilder.toString());
//            }
//        }
        final byte[] data = characteristic.getValue();

        if (data != null && data.length > 0) {

            //final StringBuilder stringBuilder = new StringBuilder(data.length);
            //for (byte byteChar : data)
            //    stringBuilder.append(String.format("%02X ", byteChar));
            //intent.putExtra(EXTRA_DATA, new String(data) + "\n" + stringBuilder.toString());
            //final int value = data[0] & 0xFF;
            intent.putExtra(EXTRA_DATA, data);
        } else if (data == null) {
            Log.e(TAG, "Extra data is null.");
        }
        sendBroadcast(intent);
    }


    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    public class LocalBinder extends Binder {
        BluetoothLeService getService() {
            return BluetoothLeService.this;
        }
    }

    @Override
    public boolean onUnbind(Intent intent) {
        close();
        return super.onUnbind(intent);
    }

    public void close() {
        if (mBluetoothGatt == null) {
            return;
        }
        mBluetoothGatt.close();
        mBluetoothGatt = null;
    }

    public boolean initialize() {
        if (mBluetoothManager == null) {
            mBluetoothManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
            if (mBluetoothManager == null) {
                Log.e(TAG, "Unable to initialize BluetoothManager.");
                return false;
            }
        }

        mBluetoothAdapter = mBluetoothManager.getAdapter();
        if (mBluetoothAdapter == null) {
            Log.e(TAG, "Unable to obtain a BluetoothAdapter.");
            return false;
        }
        return true;
    }

    public boolean connect(final String address) {

        if (mBluetoothAdapter == null || address == null) {
            Log.e(TAG, "BluetoothAdapter not initialized or unspecified address.");
            return false;
        }

        //Previously connected, try to reconnected.
        if (mBluetoothDeviceAddress != null && address.equals(mBluetoothDeviceAddress) && mBluetoothGatt != null) {
            Log.e(TAG, "Trying to use an existing mBluetoothGatt for connection.");
            if (mBluetoothGatt.connect()) {
                mConnectionState = STATE_CONNECTING;
                //readCharacteristic(mBluetoothGatt.getService(UUID.fromString(SampleGattAttributes.MEASURE)).getCharacteristic(UUID.fromString(SampleGattAttributes.POSITION_1)));
                return true;
            } else {
                return false;
            }
        }

        final BluetoothDevice device = mBluetoothAdapter.getRemoteDevice(address);

        if (device == null) {
            Log.e(TAG, "Device not found.  Unable to connect.");
            return false;
        }

        //set autoConnect
        mBluetoothGatt = device.connectGatt(this, false, mGattCallback);
        Log.e(TAG, "Trying to create a new connection.");
        mBluetoothDeviceAddress = address;
        mConnectionState = STATE_CONNECTING;
        return true;
    }

    public void setCharacteristicNotification(BluetoothGattCharacteristic characteristic, boolean enabled) {

        if (mBluetoothAdapter == null || mBluetoothGatt == null) {
            Log.e(TAG, "BluetoothAdapter not initialized");
            return;
        }
        mBluetoothGatt.setCharacteristicNotification(characteristic, enabled);

        Log.e(TAG, "setCharacteristicNotification run");

        // For Characteristics with Descriptor, enable Notification
        if (UUID.fromString(SampleGattAttributes.POSITION_1).equals(characteristic.getUuid())) {
            BluetoothGattDescriptor descriptor = characteristic.getDescriptor(UUID.fromString(SampleGattAttributes.CLIENT_CHARACTERISTIC_CONFIG));
            descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
            boolean notifyStatus = mBluetoothGatt.writeDescriptor(descriptor);
            Log.e(TAG, "WriteDescriptor: " + notifyStatus);
        }
    }

    public void readCharacteristic(BluetoothGattCharacteristic characteristic) {

        if (mBluetoothAdapter == null || mBluetoothGatt == null) {
            Log.e(TAG, "BluetoothAdapter not initialized");
            return;
        }
        mBluetoothGatt.readCharacteristic(characteristic);
    }

    public void writeCharacteristic(BluetoothGattCharacteristic characteristic, byte[] value) {
        if (mBluetoothAdapter == null || mBluetoothGatt == null) {
            Log.e(TAG, "BluetoothAdapter not initialized");
            return;
        }
        if (value != null) {
            mBluetoothArray.add(value);
        }

        if (!mBluetoothArray.isEmpty() && DeviceControlActivity.mBluetooth.equals("IDLE")) {
            DeviceControlActivity.mBluetooth = "BUSY";
            writeBLE(characteristic);
        }
    }

    private void writeBLE(BluetoothGattCharacteristic characteristic) {
        if (!mBluetoothArray.isEmpty()) {
            byte[] value = mBluetoothArray.get(0);
            characteristic.setValue(value);
            mBluetoothGatt.writeCharacteristic(characteristic);
            //boolean wStatus = mBluetoothGatt.writeCharacteristic(characteristic);
            //Log.e(TAG, "writeCharacteristic run." + Arrays.toString(characteristic.getValue()) + " " + wStatus);
        } else {
            DeviceControlActivity.mBluetooth = "IDLE";
        }
    }

    // Connect to Device.Service.Characteristic -> POSITION_1
    public BluetoothGattCharacteristic getSupportedGattCharacteristic() {
        if (mBluetoothGatt == null)
            return null;

        return mBluetoothGatt.getService(UUID.fromString(SampleGattAttributes.MEASURE)).getCharacteristic(UUID.fromString(SampleGattAttributes.POSITION_1));
    }

    public void disconnect() {
        if (mBluetoothAdapter == null || mBluetoothGatt == null) {
            Log.e(TAG, "BluetoothAdapter not initialized.");
            return;
        }
        mBluetoothGatt.disconnect();
    }
}
