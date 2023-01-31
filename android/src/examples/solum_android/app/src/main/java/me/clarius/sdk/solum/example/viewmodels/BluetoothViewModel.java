package me.clarius.sdk.solum.example.viewmodels;

import android.Manifest;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.content.ComponentName;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.IBinder;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;
import androidx.annotation.RequiresPermission;
import androidx.core.app.ActivityCompat;
import androidx.databinding.ObservableArrayList;
import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;

import java.lang.ref.WeakReference;
import java.nio.charset.StandardCharsets;

import me.clarius.sdk.solum.example.bluetooth.BluetoothProbeInfo;
import me.clarius.sdk.solum.example.bluetooth.BluetoothService;
import me.clarius.sdk.solum.example.bluetooth.enums.ServiceType;

/**
 * ViewModel to work with BluetoothService.
 *
 * @see BluetoothService
 */
public class BluetoothViewModel extends ViewModel {
    public static final String TAG = "BluetoothViewModel";
    public final ObservableArrayList<LiveData<BluetoothProbeInfo>> bluetoothProbes = new ObservableArrayList<>();

    private WeakReference<BluetoothService> bluetoothServiceReference;
    private BluetoothGatt bluetoothGatt;

    /*
     * Callback for asynchronous discovery of BLE devices.
     */
    private final ScanCallback leScanCallback = new ScanCallback() {
        @RequiresApi(api = Build.VERSION_CODES.S)
        @RequiresPermission(value = Manifest.permission.BLUETOOTH_CONNECT)
        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            super.onScanResult(callbackType, result);
            String maybeSerial = BluetoothProbeInfo.serialFromStringOrNull(result.getScanRecord().getDeviceName());
            if (maybeSerial != null) {
                Log.v(TAG, "the BLE device is a valid probe");
                connect(result.getDevice());
                addProbe(maybeSerial);
            }
        }
    };

    /*
     * Callback for asynchronous BLE connection/service/characteristic operations.
     */
    private final BluetoothGattCallback gattCallback = new BluetoothGattCallback() {

        /**
         * Callback indicating when GATT client has connected/disconnected to/from a remote GATT server.
         * Starts service discovery if successfully connected to the scanner, else close the connection.
         *
         * @param gatt BluetoothGatt connection
         * @param status integer flag representing the status of operation.
         * @param newState integer flag representing the connection state.
         */
        @RequiresApi(api = Build.VERSION_CODES.S)
        @RequiresPermission(value = Manifest.permission.BLUETOOTH_CONNECT)
        @Override
        public void onConnectionStateChange(final BluetoothGatt gatt, final int status, final int newState) {
            super.onConnectionStateChange(gatt, status, newState);
            if (status == BluetoothGatt.GATT_SUCCESS) {
                if (newState == BluetoothProfile.STATE_CONNECTED) {
                    Log.d(TAG, "Connected to the scanner.");
                    bluetoothGatt = gatt;
                    discoverServices();
                } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                    Log.d(TAG, "Disconnected from the scanner.");
                    gatt.close();
                }
            } else {
                Log.e(TAG, "Unable to connect to the scanner with the address of " + gatt.getDevice().getAddress());
                gatt.close();
            }
        }

        /**
         * Callback indicating service changed event is received.
         *
         * Receiving this event means that the GATT database is out of sync with the remote device.
         * {@link BluetoothService#discoverServices(BluetoothGatt)} is called to re-discover the services.
         * @param gatt BluetoothGatt connection; cannot be null
         */
        @RequiresApi(api = Build.VERSION_CODES.S)
        @Override
        @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
        public void onServiceChanged(@NonNull final BluetoothGatt gatt) {
            super.onServiceChanged(gatt);
            gatt.discoverServices();
            Log.e(TAG, "Services are updated; re-discovering services");
        }

        /**
         * Callback invoked when the list of remote services, characteristics and descriptors for the remote device have been updated.
         *
         * @param gatt BluetoothGatt connection
         * @param status integer representing operation status
         */
        @RequiresApi(api = Build.VERSION_CODES.S)
        @RequiresPermission(value = Manifest.permission.BLUETOOTH_CONNECT)
        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            super.onServicesDiscovered(gatt, status);
            if (status == BluetoothGatt.GATT_SUCCESS) {
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
                    Log.d(TAG, "Services discovered; start retrieving information");
                    bluetoothGatt = gatt;
                    retrieveScannerInfo();
                }
            } else {
                Log.e(TAG, "Unable to discover services of " + gatt.getDevice().getName());
            }
        }

        /**
         * Callback reporting the result of a characteristic read operation.
         * Parse Wi-Fi information if operation was successful.
         * Subscribe to characteristic updates.
         *
         * @param gatt BluetoothGatt connection; cannot be null
         * @param characteristic BluetoothGattCharacteristic; cannot be null
         * @param value byte[] value of the characteristic; cannot be null
         * @param status integer representing the status of operation
         */
        @RequiresPermission(value = Manifest.permission.BLUETOOTH_CONNECT)
        @RequiresApi(Build.VERSION_CODES.TIRAMISU)
        @Override
        public void onCharacteristicRead
        (
                @NonNull final BluetoothGatt gatt,
                @NonNull final BluetoothGattCharacteristic characteristic,
                @NonNull final byte[] value,
                final int status
        ) {
            super.onCharacteristicRead(gatt, characteristic, value, status);
            if (bluetoothServiceReference.get() != null)
                bluetoothServiceReference.get().subscribeToCharacteristic(gatt, characteristic);
            switch (status) {
                case BluetoothGatt.GATT_SUCCESS:
                    Log.d(TAG, "Retrieved the scanner's info");
                    if (characteristic.getUuid().equals(ServiceType.WifiInformationService.infoCharacteristicUuid)) {
                        processWifiCharacteristicRead(value, gatt);
                    }
                case BluetoothGatt.GATT_READ_NOT_PERMITTED:
                    Log.e(TAG, "Failed to read the characteristic; read is not permitted");
                default:
                    Log.e(TAG, "Failed to read the characteristic");
            }
        }

        /**
         * Callback reporting the result of a characteristic read operation.
         * Parse Wi-Fi information if operation was successful.
         * Subscribe to characteristic updates.
         *
         * @param gatt BluetoothGatt connection
         * @param characteristic BluetoothGattCharacteristic
         * @param status integer representing the status of operation
         */
        @RequiresApi(api = Build.VERSION_CODES.S)
        @RequiresPermission(value = Manifest.permission.BLUETOOTH_CONNECT)
        @Override
        public void onCharacteristicRead(final BluetoothGatt gatt, final BluetoothGattCharacteristic characteristic, final int status) {
            super.onCharacteristicRead(gatt, characteristic, status);
            if (bluetoothServiceReference.get() != null)
                bluetoothServiceReference.get().subscribeToCharacteristic(gatt, characteristic);
            switch (status) {
                case BluetoothGatt.GATT_SUCCESS:
                    Log.d(TAG, "Retrieved the scanner's info");
                    if (characteristic.getUuid().equals(ServiceType.WifiInformationService.infoCharacteristicUuid)) {
                        processWifiCharacteristicRead(characteristic.getValue(), gatt);
                    }
                case BluetoothGatt.GATT_READ_NOT_PERMITTED:
                    Log.e(TAG, "Failed to read the characteristic; read is not permitted");
                default:
                    Log.e(TAG, "Failed to read the characteristic");
            }
        }

        /**
         * Callback triggered as a result of a remote characteristic notification.
         * Parse the updated Wi-Fi information.
         *
         * @param gatt BluetoothGatt connection; cannot be null
         * @param characteristic BluetoothGattCharacteristic; cannot be null
         * @param value byte[] value of the characteristic; cannot be null
         * @apiNote the value within the characteristic object may have changed since receiving the remote characteristic notification
         */
        @RequiresApi(Build.VERSION_CODES.TIRAMISU)
        @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
        @Override
        public void onCharacteristicChanged
        (
                @NonNull final BluetoothGatt gatt,
                @NonNull final BluetoothGattCharacteristic characteristic,
                @NonNull final byte[] value
        ) {
            super.onCharacteristicChanged(gatt, characteristic, value);
            Log.d(TAG, "Characteristic update received: " + new String(value, StandardCharsets.UTF_8));
            if (characteristic.getUuid().equals(ServiceType.WifiInformationService.infoCharacteristicUuid)) {
                processWifiCharacteristicRead(value, gatt);
            }
        }

        /**
         * Callback triggered as a result of a remote characteristic notification.
         * Parse the updated Wi-Fi information.
         *
         * @param gatt BluetoothGatt connection
         * @param characteristic BluetoothGattCharacteristic
         */
        @Override
        public void onCharacteristicChanged(final BluetoothGatt gatt, final BluetoothGattCharacteristic characteristic) {
            super.onCharacteristicChanged(gatt, characteristic);
        }
    };

    /*
     * Callback for asynchronous binding of BluetoothService.
     * Starts/stops scanning BLE devices as service gets bound/unbound
     */
    private final ServiceConnection bluetoothServiceConnection = new ServiceConnection() {
        @RequiresApi(api = Build.VERSION_CODES.S)
        @RequiresPermission(value = Manifest.permission.BLUETOOTH_SCAN)
        @Override
        public void onServiceConnected(final ComponentName componentName, final IBinder iBinder) {
            Log.v(TAG, "BluetoothService was connected; setting the binder");
            bluetoothServiceReference = new WeakReference<>(((BluetoothService.BluetoothBinder) iBinder).getService());
            Log.v(TAG, "scanning for BLE devices");
            bluetoothServiceReference.get().scanDevices(leScanCallback);
        }

        @RequiresApi(api = Build.VERSION_CODES.S)
        @RequiresPermission(value = Manifest.permission.BLUETOOTH_SCAN)
        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            Log.v(TAG, "BluetoothService was disconnected; destroying the binder");
            if (bluetoothServiceReference != null) {
                bluetoothServiceReference.get().stopScanning(leScanCallback);
            }
            bluetoothServiceReference = null;
        }
    };

    @RequiresApi(api = Build.VERSION_CODES.S)
    @RequiresPermission(value = Manifest.permission.BLUETOOTH_CONNECT)
    private void addProbe(final String maybeSerial) {
        MutableLiveData<BluetoothProbeInfo> bluetoothProbeLivaData = new MutableLiveData<>(new BluetoothProbeInfo(maybeSerial));
        if (!contains(bluetoothProbes, bluetoothProbeLivaData)) {
            bluetoothProbes.add(bluetoothProbeLivaData);
        }
    }

    /*
     * The bluetoothProbes array contains the LiveData item if there's a ProbeInfo with matching scanner.
     */
    private boolean contains(final ObservableArrayList<LiveData<BluetoothProbeInfo>> list, final LiveData<BluetoothProbeInfo> item) {
        for (LiveData<BluetoothProbeInfo> liveData : list) {
            if (liveData.getValue() != null && liveData.getValue().equals(item.getValue()))
                return true;
        }
        return false;
    }

    @RequiresApi(api = Build.VERSION_CODES.S)
    @RequiresPermission(value = Manifest.permission.BLUETOOTH_CONNECT)
    private void retrieveScannerInfo() {
        if (bluetoothServiceReference.get() != null) {
            bluetoothServiceReference.get().retrieveWifiInfo(bluetoothGatt);
        }
    }

    @RequiresApi(api = Build.VERSION_CODES.S)
    @RequiresPermission(value = Manifest.permission.BLUETOOTH_CONNECT)
    private void discoverServices() {
        if (bluetoothServiceReference.get() != null) {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
                bluetoothServiceReference.get().discoverServices(bluetoothGatt);
            }
        }
    }

    @RequiresApi(api = Build.VERSION_CODES.S)
    @RequiresPermission(value = Manifest.permission.BLUETOOTH_CONNECT)
    private void connect(final BluetoothDevice device) {
        if (bluetoothServiceReference.get() != null) {
            bluetoothServiceReference.get().connect(device, gattCallback);
        }
    }

    @RequiresApi(api = Build.VERSION_CODES.S)
    @RequiresPermission(value = Manifest.permission.BLUETOOTH_SCAN)
    public void stopScanning() {
        if (bluetoothServiceReference.get() == null) return;
        bluetoothServiceReference.get().stopScanning(leScanCallback);
    }

    /*
     * Updates the Wi-Fi information of a BluetoothProbeInfo object with matching serial.
     */
    @RequiresApi(api = Build.VERSION_CODES.S)
    @RequiresPermission(value = Manifest.permission.BLUETOOTH_CONNECT)
    private void processWifiCharacteristicRead(final byte[] data, final BluetoothGatt gatt) {
        String deviceName = gatt.getDevice().getName();
        Log.d(TAG, "Characteristic data is :" + new String(data, StandardCharsets.UTF_8));
        bluetoothProbes.iterator().forEachRemaining(probeLiveData -> {
            BluetoothProbeInfo probeInfo = probeLiveData.getValue();
            if (probeInfo != null && probeInfo.equals(deviceName)) {
                String yamlData = new String(data, StandardCharsets.UTF_8);
                probeInfo.setWifiInfo(yamlData);
                ((MutableLiveData<BluetoothProbeInfo>) probeLiveData).postValue(probeInfo);
            }
        });
    }

    public ServiceConnection getBluetoothServiceConnection() {
        return bluetoothServiceConnection;
    }

}
