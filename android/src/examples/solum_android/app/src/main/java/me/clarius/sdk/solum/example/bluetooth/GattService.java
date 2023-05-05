package me.clarius.sdk.solum.example.bluetooth;

import android.Manifest;
import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.content.Intent;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RequiresPermission;

import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.util.UUID;

/**
 * Connect to a clarius probe gatt service
 * <p>
 * See: https://developer.android.com/guide/topics/connectivity/bluetooth/connect-gatt-server
 */

public class GattService extends Service {
    private static final String TAG = "BLE(gatt)";
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
                listener.connected(gatt.getDevice().getName());
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
                handler.postDelayed(() -> subscribe(ClariusCharacteristic.PowerPublished), 200);
                handler.postDelayed(() -> subscribe(ClariusCharacteristic.WiFiPublished), 400);
                handler.postDelayed(() -> readWifi(), 600);
                listener.ready(gatt.getDevice().getName());
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
        private void onCharacteristicReadCompat(@NonNull BluetoothGatt gatt, @NonNull BluetoothGattCharacteristic characteristic, @NonNull byte[] ignoredValue, int status) {
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
                listener.powerChanged(gatt.getDevice().getName(), powered);
            } else if (ClariusCharacteristic.WiFiPublished.equals(characteristic)) {
                String yaml = new String(value, 0, value.length, StandardCharsets.UTF_8);
                ProbeWifi wifi = ProbeWifi.parse(yaml);
                Log.i(TAG, "Probe wifi changed: " + wifi);
                listener.wifiChanged(gatt.getDevice().getName(), wifi);
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
    private static void increaseMtu(BluetoothGatt gatt) {
        assert null != gatt;
        if (!gatt.requestMtu(MAX_MTU)) {
            Log.e(TAG, "Failed to change MTU (wifi info might be truncated)");
        }
    }

    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    private static void doPing(BluetoothGatt gatt) throws BluetoothException {
        assert null != gatt;
        byte[] value = {0x0};
        BluetoothGattCharacteristic characteristic = getCharacteristic(gatt, ClariusCharacteristic.Alert);
        Compat.writeCharacteristic(gatt, characteristic, value);
    }

    static BluetoothGattCharacteristic getCharacteristic(BluetoothGatt gatt, ClariusCharacteristic type) throws BluetoothException {
        assert null != gatt;
        BluetoothGattService service = gatt.getService(type.serviceUuid);
        if (null == service) throw new BluetoothException("Could not find service " + type.name());
        BluetoothGattCharacteristic characteristic = service.getCharacteristic(type.uuid);
        if (null == characteristic)
            throw new BluetoothException("Could not find characteristic for " + type.name());
        return characteristic;
    }

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return binder;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d(TAG, "Creating bluetooth gatt service");
        bluetoothAdapter = ((BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE)).getAdapter();
        assert bluetoothAdapter != null : "Unable to get the system BLE adapter, is bluetooth enabled?";
    }

    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "Destroying bluetooth gatt service");
        close();
        bluetoothAdapter = null;
    }

    public void initialize(Listener listener) {
        this.listener = listener;
    }

    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    public void connect(String address) throws BluetoothException {
        if (null == bluetoothAdapter)
            throw new BluetoothException("Missing ble adapter, did you connect?");
        if (null == address) throw new BluetoothException("Missing ble address");
        BluetoothDevice device = bluetoothAdapter.getRemoteDevice(address);
        if (null == device)
            throw new BluetoothException("Failed to find device with ble address " + address);
        close();
        Log.i(TAG, "Connecting to probe: " + device.getName());
        bluetoothGatt = device.connectGatt(this, false, bluetoothGattCallback);
    }

    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    public void sendAlert(boolean high) {
        execute(gatt -> {
            assert null != gatt;
            Log.d(TAG, "Sending alert to probe");
            byte[] value = {(byte) (high ? 0x1 : 0x2)};
            BluetoothGattCharacteristic characteristic = getCharacteristic(gatt, ClariusCharacteristic.Alert);
            Compat.writeCharacteristic(gatt, characteristic, value);
        });
    }

    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    public void subscribe(ClariusCharacteristic type) {
        execute(gatt -> {
            assert null != gatt;
            Log.d(GattService.TAG, "Subscribing to probe's service: " + type.name());
            BluetoothGattCharacteristic characteristic = getCharacteristic(gatt, type);
            boolean set = gatt.setCharacteristicNotification(characteristic, true);
            if (!set) throw new BluetoothException("Failed to set characteristic notification");
            BluetoothGattDescriptor descriptor = characteristic.getDescriptor(GattService.CLIENT_CHARACTERISTIC_CONFIGURATION_DESCRIPTOR_UUID);
            if (null == descriptor)
                throw new BluetoothException("Failed to get the client characteristic configuration descriptor");
            byte[] enable = {0x1, 0x0};
            Compat.writeDescriptor(gatt, descriptor, enable);
        });
    }

    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    public void powerOn(boolean on) {
        execute(gatt -> {
            assert null != gatt;
            Log.d(TAG, "Powering probe " + (on ? "on" : "off"));
            byte[] value = {(byte) (on ? 0x1 : 0x0)};
            BluetoothGattCharacteristic characteristic = getCharacteristic(gatt, ClariusCharacteristic.PowerRequest);
            Compat.writeCharacteristic(gatt, characteristic, value);
        });
    }

    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    void readWifi() {
        execute(gatt -> {
            assert null != gatt;
            Log.d(TAG, "Reading probe's Wi-Fi info");
            BluetoothGattCharacteristic characteristic = getCharacteristic(gatt, ClariusCharacteristic.WiFiPublished);
            gatt.readCharacteristic(characteristic);
        });
    }

    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    public void disconnect() {
        execute(gatt -> {
            bluetoothGatt.disconnect();
            connectedToProbe = false;
        });
    }

    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    private void close() {
        if (null != bluetoothGatt) {
            connectedToProbe = false;
            // close() releases all resources unlike disconnect() which allows subsequent re-connections
            bluetoothGatt.close();
            bluetoothGatt = null;
            listener.disconnected();
        }
    }

    // keep probe ble connection alive by writing data to the alert service
    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    private void startPingTimer() {
        handler.postDelayed(() -> {
            if (connectedToProbe) { // should be in critical section to avoid race conditions
                execute(GattService::doPing);
                startPingTimer();
            }
        }, 5000);
    }

    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    private void execute(RunnableThatCanFail runnable) {
        BluetoothGatt gatt = bluetoothGatt;
        if (null == gatt) return;
        try {
            runnable.run(gatt);
        } catch (BluetoothException e) {
            Log.e(TAG, e.toString());
            close();
            listener.error(e.toString());
        }
    }

    public interface Listener {
        void connected(String deviceName);

        void disconnected();

        void ready(String deviceName);

        void powerChanged(String deviceName, boolean powered);

        void wifiChanged(String deviceName, ProbeWifi wifi);

        void error(String error);
    }

    @FunctionalInterface
    interface RunnableThatCanFail {
        void run(BluetoothGatt gatt) throws BluetoothException;
    }

    public class CustomBinder extends Binder {
        public GattService getService() {
            return GattService.this;
        }
    }
}
