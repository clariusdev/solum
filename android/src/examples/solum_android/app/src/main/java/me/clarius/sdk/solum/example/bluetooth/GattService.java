package me.clarius.sdk.solum.example.bluetooth;

import android.Manifest;
import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.BluetoothStatusCodes;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Binder;
import android.os.Build;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RequiresPermission;
import androidx.core.app.ActivityCompat;

import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.util.UUID;

/**
 * Connect to a clarius probe gatt service
 * <p>
 * See: https://developer.android.com/guide/topics/connectivity/bluetooth/connect-gatt-server
 * <p>
 * NOTE: this class avoids error management by using asserts to focus on BLE commands.
 */

public class GattService extends Service {
    private static final String TAG = "BLE(GATT)";
    private static final UUID CLIENT_CHARACTERISTIC_CONFIGURATION_DESCRIPTOR_UUID = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");
    private static final int MAX_MTU = 256;

    private final Handler handler = new Handler(Looper.myLooper());
    private final IBinder binder = new CustomBinder();
    private boolean connectedToProbe = false;
    private BluetoothAdapter bluetoothAdapter;
    private BluetoothGatt bluetoothGatt;
    private Listener listener;
    private final BluetoothGattCallback bluetoothGattCallback = new BluetoothGattCallback() {

        @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            super.onConnectionStateChange(gatt, status, newState);
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                Log.d(TAG, "Connected to probe " + gatt.getDevice().getName());
                gatt.discoverServices();
                connectedToProbe = true;
                listener.connected(gatt.getDevice());
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                Log.d(TAG, "Probe disconnected with status: " + status);
                connectedToProbe = false;
                listener.disconnected();
            }
        }

        @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            super.onServicesDiscovered(gatt, status);
            if (status == BluetoothGatt.GATT_SUCCESS) {
                Log.d(TAG, "Probe services discovered");
                increaseMtu(gatt);
                // some delay is needed between accesses to the gatt service on some devices
                handler.postDelayed(() -> subscribe(gatt, ClariusCharacteristic.PowerPublished), 200);
                handler.postDelayed(() -> subscribe(gatt, ClariusCharacteristic.WiFiPublished), 400);
                handler.postDelayed(() -> readWifi(), 600);
                listener.ready(gatt.getDevice());
                startPingTimer();
            } else {
                Log.e(TAG, "Failed to discover services, status: " + status);
            }
        }

        @Override
        public void onMtuChanged(BluetoothGatt gatt, int mtu, int status) {
            super.onMtuChanged(gatt, mtu, status);
            if (status != BluetoothGatt.GATT_SUCCESS) {
                Log.e(TAG, "Failed to change MTU (wifi info might be truncated), status: " + status);
            }
        }

        @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
        @Override
        public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            super.onCharacteristicRead(gatt, characteristic, status);
            onCharacteristicReadCompat(gatt, characteristic, characteristic.getValue(), status);
        }

        @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
        @Override
        public void onCharacteristicRead(@NonNull BluetoothGatt gatt, @NonNull BluetoothGattCharacteristic characteristic, @NonNull byte[] value, int status) {
            super.onCharacteristicRead(gatt, characteristic, value, status);
            onCharacteristicReadCompat(gatt, characteristic, value, status);
        }

        @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
        private void onCharacteristicReadCompat(@NonNull BluetoothGatt gatt, @NonNull BluetoothGattCharacteristic characteristic, @NonNull byte[] value, int status) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                parseCharacteristic(gatt, characteristic, characteristic.getValue());
            } else {
                Log.e(TAG, "Failed to read characteristic, status: " + status);
            }
        }

        @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
        private void parseCharacteristic(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, byte[] value) {
            final android.bluetooth.BluetoothGattService service = characteristic.getService();
            if (ClariusCharacteristic.PowerPublished.equals(characteristic)) {
                boolean powered = (value[0] & 0xFF) == 0x01;
                Log.i(TAG, "Probe power changed: " + powered);
                listener.powerChanged(gatt.getDevice(), powered);
            } else if (ClariusCharacteristic.WiFiPublished.equals(characteristic)) {
                String yaml = new String(value, 0, value.length, StandardCharsets.UTF_8);
                ProbeWifi wifi = ProbeWifi.parse(yaml);
                Log.i(TAG, "Probe wifi changed: " + wifi);
                listener.wifiChanged(gatt.getDevice(), wifi);
            } else {
                Log.d(TAG, "Retrieved unknown characteristic value: " + ByteBuffer.wrap(value) + " service: " + service.getUuid() + " characteristic: " + characteristic.getUuid());
            }
        }

        @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
        @Override
        public void onCharacteristicChanged(@NonNull BluetoothGatt gatt, @NonNull BluetoothGattCharacteristic characteristic, @NonNull byte[] value) {
            super.onCharacteristicChanged(gatt, characteristic, value);
            onCharacteristicChangedCompat(gatt, characteristic, value);
        }

        @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
            super.onCharacteristicChanged(gatt, characteristic);
            onCharacteristicChangedCompat(gatt, characteristic, characteristic.getValue());
        }

        @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
        public void onCharacteristicChangedCompat(@NonNull BluetoothGatt gatt, @NonNull BluetoothGattCharacteristic characteristic, @NonNull byte[] value) {
            parseCharacteristic(gatt, characteristic, value);
        }
    };

    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    private static void writeCharacteristicCompat(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, byte[] value, int writeType) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            int ret = gatt.writeCharacteristic(characteristic, value, writeType);
            assert BluetoothStatusCodes.SUCCESS == ret : "Failed to write characteristic, return code: " + ret;
        } else {
            boolean set = characteristic.setValue(value);
            assert set : "Failed to write value in characteristic";
            characteristic.setWriteType(writeType);
            boolean write = gatt.writeCharacteristic(characteristic);
            assert write : "Failed to write characteristic";
        }
    }

    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    private static void writeDescriptorCompat(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, byte[] value) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            int ret = gatt.writeDescriptor(descriptor, value);
            assert BluetoothStatusCodes.SUCCESS == ret : "Failed to write descriptor, return code: " + ret;
        } else {
            boolean set = descriptor.setValue(value);
            assert set : "Failed to write value in descriptor";
            boolean write = gatt.writeDescriptor(descriptor);
            assert write : "Failed to write descriptor";
        }
    }

    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    private static void subscribe(BluetoothGatt gatt, ClariusCharacteristic type) {
        BluetoothGattCharacteristic characteristic = getCharacteristic(gatt, type);
        boolean set = gatt.setCharacteristicNotification(characteristic, true);
        assert set : "Failed to set characteristic notification";
        BluetoothGattDescriptor descriptor = characteristic.getDescriptor(CLIENT_CHARACTERISTIC_CONFIGURATION_DESCRIPTOR_UUID);
        assert null != descriptor : "Failed to get the client characteristic configuration descriptor";
        byte[] enable = {0x1, 0x0};
        writeDescriptorCompat(gatt, descriptor, enable);
    }

    private static BluetoothGattCharacteristic getCharacteristic(BluetoothGatt gatt, ClariusCharacteristic type) {
        android.bluetooth.BluetoothGattService service = gatt.getService(type.serviceUuid);
        assert null != service : "Could not find service " + type.name();
        BluetoothGattCharacteristic characteristic = service.getCharacteristic(type.uuid);
        assert null != characteristic : "Could not find characteristic for " + type.name();
        return characteristic;
    }

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return binder;
    }

    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    @Override
    public boolean onUnbind(Intent intent) {
        close();
        return super.onUnbind(intent);
    }

    public boolean initialize(Listener listener) {
        bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (bluetoothAdapter == null) {
            Log.e(TAG, "Unable to obtain a BluetoothAdapter.");
            return false;
        }
        this.listener = listener;
        return true;
    }

    public void connect(String address) {
        assert null != bluetoothAdapter && null != address;
        BluetoothDevice device = bluetoothAdapter.getRemoteDevice(address);
        assert null != device : "Failed to find device with address " + address;
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
        }
        Log.i(TAG, "Connecting to probe: " + device.getName());
        close();
        bluetoothGatt = device.connectGatt(this, false, bluetoothGattCallback);
    }

    public void sendAlert(boolean high) {
        assert null != bluetoothGatt;
        byte[] value = {(byte) (high ? 0x1 : 0x2)};
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
        }
        Log.d(TAG, "Sending alert to probe: " + bluetoothGatt.getDevice().getName());
        BluetoothGattCharacteristic characteristic = getCharacteristic(bluetoothGatt, ClariusCharacteristic.Alert);
        writeCharacteristicCompat(bluetoothGatt, characteristic, value, BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
    }

    public void powerOn(boolean on) {
        assert null != bluetoothGatt;
        byte[] value = {(byte) (on ? 0x1 : 0x0)};
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
        }
        Log.d(TAG, "Powering " + (on ? "on" : "off") + " probe: " + bluetoothGatt.getDevice().getName());
        BluetoothGattCharacteristic characteristic = getCharacteristic(bluetoothGatt, ClariusCharacteristic.PowerRequest);
        writeCharacteristicCompat(bluetoothGatt, characteristic, value, BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
    }

    void readWifi() {
        assert null != bluetoothGatt;
        BluetoothGattCharacteristic characteristic = getCharacteristic(bluetoothGatt, ClariusCharacteristic.WiFiPublished);
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
        }
        bluetoothGatt.readCharacteristic(characteristic);
    }

    public void disconnect() {
        if (null != bluetoothGatt) {
            if (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
            }
            connectedToProbe = false;
            bluetoothGatt.disconnect();
        }
    }

    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    private void close() {
        if (null != bluetoothGatt) {
            connectedToProbe = false;
            // close() releases all resources unlike disconnect() which allows subsequent re-connections
            bluetoothGatt.close();
            bluetoothGatt = null;
        }
    }

    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    private void increaseMtu(BluetoothGatt gatt) {
        if (!gatt.requestMtu(MAX_MTU)) {
            Log.e(TAG, "Failed to change MTU (wifi info might be truncated)");
        }
    }

    // keep probe ble connection alive by writing data to the alert service
    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    private void startPingTimer() {
        handler.postDelayed(() -> {
            if (connectedToProbe) { // should be in critical section to avoid race conditions
                ping();
                startPingTimer();
            }
        }, 5000);
    }

    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    private void ping() {
        assert null != bluetoothGatt;
        byte[] value = {0x0};
        BluetoothGattCharacteristic characteristic = getCharacteristic(bluetoothGatt, ClariusCharacteristic.Alert);
        writeCharacteristicCompat(bluetoothGatt, characteristic, value, BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
    }

    public interface Listener {
        void connected(BluetoothDevice device);

        void disconnected();

        void ready(BluetoothDevice device);

        void powerChanged(BluetoothDevice device, boolean powered);

        void wifiChanged(BluetoothDevice device, ProbeWifi wifi);
    }

    public class CustomBinder extends Binder {
        public GattService getService() {
            return GattService.this;
        }
    }
}
