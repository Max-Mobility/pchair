package com.example.ble;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.recyclerview.widget.RecyclerView;

import android.Manifest;
import android.app.Activity;
import android.app.ListActivity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattServer;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.ScanSettings;
import android.content.ClipData;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.ParcelUuid;
import android.provider.Settings;
import android.renderscript.Element;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.google.android.material.snackbar.Snackbar;

import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

public class MainActivity extends ListActivity {

    public static final String TAG = "BLE";

    private BluetoothAdapter mBluetoothAdapter;
    private BluetoothLeScanner mLEScanner;
    private ScanSettings settings;
    private List<ScanFilter> filters;
    private BluetoothGatt mGatt;
    private boolean mScanning;
    private Handler mHandler;
    private LeDeviceListAdapter mLeDeviceListAdapter;

    private static final int PERMISSION_REQUEST_COARSE_LOCATION = 456;
    private static final int REQUEST_ENABLE_BT = 123;

    private static final long SCAN_PERIOD = 5000; //ms

    private static final UUID TEST_SERVICE = UUID.fromString("000012ff-0000-1000-8000-00805f9b34fb");

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        findViewById(R.id.loadingPanel).setVisibility(View.GONE);


        mHandler = new Handler();


        int phonePermissionCheck = ActivityCompat.checkSelfPermission(this, Manifest.permission.ACCESS_COARSE_LOCATION);

        if (phonePermissionCheck != PackageManager.PERMISSION_GRANTED) {
            requestRuntimePermissions();
        } else {
            // check BLE permission
            if (!getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE)) {
                Toast.makeText(this, "BLE is not supported.", Toast.LENGTH_SHORT).show();
                finish();
            } else {

                Log.e(TAG, "BLE is supported.");
                //START BLE
                final BluetoothManager bluetoothManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
                mBluetoothAdapter = bluetoothManager.getAdapter();
                Log.e(TAG, "Got BLE adapter.");
            }
        }
    }


    @Override
    protected void onResume() {
        super.onResume();

        if (mBluetoothAdapter == null || !mBluetoothAdapter.isEnabled()) {
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
        } else {
            mLEScanner = mBluetoothAdapter.getBluetoothLeScanner();
            settings = new ScanSettings.Builder()
                    .setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY)
                    .build();
            filters = new ArrayList<>();
            filters.add(new ScanFilter.Builder().setServiceUuid(new ParcelUuid(TEST_SERVICE)).build());
        }
        mLeDeviceListAdapter = new LeDeviceListAdapter();
        setListAdapter(mLeDeviceListAdapter);

        scanLeDevice(true);

    }

    @Override
    protected void onPause() {
        super.onPause();
        if (mBluetoothAdapter != null && mBluetoothAdapter.isEnabled()) {
            scanLeDevice(false);
        }
    }

    @Override
    protected void onDestroy() {
        if (mGatt == null) {
            return;
        }
//        mGatt.close();
//        mGatt=null;
        mBluetoothAdapter = null;
        super.onDestroy();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == REQUEST_ENABLE_BT) {
            if (resultCode == Activity.RESULT_CANCELED) {
                finish();
                return;
            }
        }
        super.onActivityResult(requestCode, resultCode, data);
    }


    // scan callback
    private ScanCallback mScanCallback = new ScanCallback() {
        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            //super.onScanResult(callbackType, result);
            //Log.e(TAG,"callbackType: "+callbackType);
            //Log.e(TAG,"result: "+result.getDevice().toString());

            mLeDeviceListAdapter.addDevice(result.getDevice());
            mLeDeviceListAdapter.notifyDataSetChanged();
            //Log.e(TAG,"Result: "+result.getDevice().getUuids());

            //BluetoothDevice btDevice = result.getDevice();
            //connectToDevice(btDevice);

            final BluetoothDevice device = mLeDeviceListAdapter.getDevice(0);

            if (device == null)
                return;

            //connectToDevice(device);

            // call DeviceControlActivity.java
            final Intent intent = new Intent(MainActivity.this, DeviceControlActivity.class);
            intent.putExtra(DeviceControlActivity.EXTRAS_DEVICE_NAME, device.getName());
            intent.putExtra(DeviceControlActivity.EXTRAS_DEVICE_ADDRESS, device.getAddress());
            //Log.e(TAG,"UUID: "+device.getUuids());

            if (mScanning) {
                mLEScanner.stopScan(mScanCallback);
                mScanning = false;
            }
            startActivity(intent);
        }


        @Override
        public void onScanFailed(int errorCode) {
            Log.e(TAG, "Scan Failed. Error Code: " + errorCode);
        }
    };

    private void scanLeDevice(final boolean enable) {

        if (enable) {
            mHandler.postDelayed(new Runnable() {
                @Override
                public void run() {
                    mScanning = false;
                    mLEScanner.stopScan(mScanCallback);
                    findViewById(R.id.loadingPanel).setVisibility(View.GONE);
                }
            }, SCAN_PERIOD);

            mScanning = true;
            findViewById(R.id.loadingPanel).setVisibility(View.VISIBLE);
            mLEScanner.startScan(filters, settings, mScanCallback);

        } else {
            mScanning = false;
            findViewById(R.id.loadingPanel).setVisibility(View.GONE);
            mLEScanner.stopScan(mScanCallback);
        }
    }

    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        final BluetoothDevice device = mLeDeviceListAdapter.getDevice(position);
        Log.e(TAG, "Position: " + position);
        if (device == null)
            return;

        //connectToDevice(device);

        // call DeviceControlActivity.java
        final Intent intent = new Intent(this, DeviceControlActivity.class);
        intent.putExtra(DeviceControlActivity.EXTRAS_DEVICE_NAME, device.getName());
        intent.putExtra(DeviceControlActivity.EXTRAS_DEVICE_ADDRESS, device.getAddress());
        //Log.e(TAG,"UUID: "+device.getUuids());

        if (mScanning) {
            mLEScanner.stopScan(mScanCallback);
            mScanning = false;
        }
        startActivity(intent);
    }


    //simple test, Not used any more, combined to DeviceControlActivity.java
    public void connectToDevice(BluetoothDevice device) {
        if (mGatt == null) {
            mGatt = device.connectGatt(this, false, gattCallback);
            scanLeDevice(false);
        }
    }

    // connectGatt callback, simple test, Not used any more, combined to DeviceControlActivity.java
    private final BluetoothGattCallback gattCallback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            //super.onConnectionStateChange(gatt, status, newState);
            Log.e(TAG, "onConnectionStateChange Status: " + status);
            switch (newState) {
                case BluetoothProfile.STATE_CONNECTED:
                    Log.e(TAG, "gattCallback STATE_CONNECTED");
                    gatt.discoverServices();
                    break;
                case BluetoothProfile.STATE_DISCONNECTED:
                    Log.e(TAG, "gattCallback STATE_CONNECTED");
                    break;
                default:
                    Log.e(TAG, "gattCallback STATE_OTHER");
            }
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            //super.onServicesDiscovered(gatt, status);
            List<BluetoothGattService> services = gatt.getServices();
            Log.e(TAG, "onServicesDiscovered: " + services.toString());
            gatt.readCharacteristic(services.get(1).getCharacteristics().get(0));
        }

        @Override
        public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            //super.onCharacteristicRead(gatt, characteristic, status);
            Log.e(TAG, "onCharacteristicRead: " + characteristic.toString());
            //gatt.disconnect();
        }
    };

    private void requestRuntimePermissions() {

        boolean shouldProvideRetionale =
                ActivityCompat.shouldShowRequestPermissionRationale(this, Manifest.permission.ACCESS_COARSE_LOCATION);

        if (shouldProvideRetionale) {

            Log.e(TAG, "Displaying permission rationale");

            Snackbar.make(
                    findViewById(R.id.main_activity_view),
                    "Location data is used as part of the BLE App",
                    Snackbar.LENGTH_INDEFINITE)
                    .setAction(
                            "OK",
                            new View.OnClickListener() {
                                @Override
                                public void onClick(View view) {
                                    ActivityCompat.requestPermissions(
                                            MainActivity.this,
                                            new String[]{Manifest.permission.ACCESS_COARSE_LOCATION},
                                            PERMISSION_REQUEST_COARSE_LOCATION);
                                }
                            })
                    .show();
        } else {
            ActivityCompat.requestPermissions(
                    MainActivity.this,
                    new String[]{Manifest.permission.ACCESS_COARSE_LOCATION},
                    PERMISSION_REQUEST_COARSE_LOCATION);
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions,
                                           @NonNull int[] grantResults) {

        Log.e(TAG, "Request COARSE_LOCATION Permission Result:");

        if (requestCode == PERMISSION_REQUEST_COARSE_LOCATION) {

            // User interaction is interrupted or cancelled -> empty array
            if (grantResults.length <= 0) {

                Log.e(TAG, "User interaction was cancelled.");

            } else if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {

                Log.e(TAG, "Granted.");

                //CHECK BLE PERMISSION
                if (!getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE)) {
                    Toast.makeText(this, "BLE is not supported.", Toast.LENGTH_SHORT).show();
                    finish();
                } else {
                    Log.e(TAG, "BLE is supported.");
                    //start ble
                    final BluetoothManager bluetoothManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
                    mBluetoothAdapter = bluetoothManager.getAdapter();
                    Log.e(TAG, "Got BLE adapter.");
                }

            } else {
                Snackbar.make(
                        findViewById(R.id.main_activity_view),
                        "Location data is used as part of the BLE App",
                        Snackbar.LENGTH_INDEFINITE)
                        .setAction(
                                "OK",
                                new View.OnClickListener() {
                                    @Override
                                    public void onClick(View view) {
                                        ActivityCompat.requestPermissions(
                                                MainActivity.this,
                                                new String[]{Manifest.permission.ACCESS_COARSE_LOCATION},
                                                PERMISSION_REQUEST_COARSE_LOCATION);
                                    }
                                })
                        .show();
            }
        }
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.scan:
                //scan function
                Log.e(TAG, "scan");
                scanLeDevice(true);
                return (true);
            case R.id.disconnect:
                //disconnect function
                Log.e(TAG, "disconnect");
//            if(mGatt != null) {
//                mGatt.close();
//                mGatt=null;
//            }
                return (true);
        }
        return (super.onOptionsItemSelected(item));
    }

    private class LeDeviceListAdapter extends BaseAdapter {
        private ArrayList<BluetoothDevice> mLeDevices;
        private LayoutInflater mInflater;

        public LeDeviceListAdapter() {
            super();
            mLeDevices = new ArrayList<>();
            mInflater = MainActivity.this.getLayoutInflater();
        }

        public void addDevice(BluetoothDevice device) {
            if (!mLeDevices.contains(device)) {
                mLeDevices.add(device);
            }
        }

        public BluetoothDevice getDevice(int position) {
            return mLeDevices.get(position);
        }

        public void clear() {
            mLeDevices.clear();
        }

        @Override
        public int getCount() {
            return mLeDevices.size();
        }

        @Override
        public Object getItem(int i) {
            return mLeDevices.get(i);
        }

        @Override
        public long getItemId(int i) {
            return i;
        }

        @Override
        public View getView(int i, View view, ViewGroup viewGroup) {
            ViewHolder viewHolder;

            if (view == null) {
                view = mInflater.inflate(R.layout.listitem_device, null);
                viewHolder = new ViewHolder();
                viewHolder.deviceAddress = view.findViewById(R.id.device_address);
                viewHolder.deviceName = view.findViewById(R.id.device_name);
                view.setTag(viewHolder);
            } else {
                viewHolder = (ViewHolder) view.getTag();
            }

            BluetoothDevice device = mLeDevices.get(i);
            final String deviceName = device.getName();
            if (deviceName != null && deviceName.length() > 0)
                viewHolder.deviceName.setText(deviceName);
            else
                viewHolder.deviceName.setText("Unknown device");
            viewHolder.deviceAddress.setText(device.getAddress());

            return view;
        }
    }

    static class ViewHolder {
        TextView deviceName;
        TextView deviceAddress;
    }


}
