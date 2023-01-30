package me.clarius.sdk.solum.example.bluetooth;

import android.Manifest;
import android.app.Service;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.content.Context;
import android.content.Intent;
import android.os.Binder;
import android.os.Build;
import android.os.IBinder;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RequiresApi;
import androidx.annotation.RequiresPermission;

import java.util.UUID;

import me.clarius.sdk.solum.example.bluetooth.enums.CharacteristicType;
import me.clarius.sdk.solum.example.bluetooth.enums.ServiceType;

/**
 * Service used for BLE communications
 *
 * @see Service
 * @see BluetoothLeScanner
 * @see BluetoothGatt
 */
public class BluetoothService extends Service {
    public static final String TAG = "BluetoothService";

    private final IBinder binder = new BluetoothBinder();
    private BluetoothLeScanner scanner;

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return binder;
    }

    /**
     * Initializes BluetoothLeScanner to scan BLE devices on Create
     */
    @Override
    public void onCreate() {
        super.onCreate();
        scanner = ((BluetoothManager) getApplicationContext()
                .getSystemService(Context.BLUETOOTH_SERVICE))
                .getAdapter()
                .getBluetoothLeScanner();
        Log.v(TAG, "service was created");
    }

    /**
     * Scans the BLE devices asynchronously while this service is bound.
     *
     * @param leScanCallback called when a BLE device is discovered
     * @see me.clarius.sdk.solum.example.viewmodels.BluetoothViewModel for callback implementation
     */
    @RequiresApi(api = Build.VERSION_CODES.S)
    @RequiresPermission(value = Manifest.permission.BLUETOOTH_SCAN)
    public void scanDevices(final ScanCallback leScanCallback) {
        scanner.startScan(leScanCallback);
    }

    /**
     * Stops scanning BLE devices.
     * Must be called as this service get destroyed.
     *
     * @param leScanCallback called when scanning is stopped.
     */
    @RequiresApi(api = Build.VERSION_CODES.S)
    @RequiresPermission(value = Manifest.permission.BLUETOOTH_SCAN)
    public void stopScanning(final ScanCallback leScanCallback) {
        scanner.stopScan(leScanCallback);
    }

    /**
     * Connect to the given device asynchronously.
     * {@link BluetoothGattCallback#onConnectionStateChange(BluetoothGatt, int, int)} is called to report the result.
     *
     * @param device a Clarius scanner
     * @param gattCallback called to report the connection status
     * @see me.clarius.sdk.solum.example.viewmodels.BluetoothViewModel BluetoothGattCallback implementation
     */
    @RequiresApi(api = Build.VERSION_CODES.S)
    @RequiresPermission(value = Manifest.permission.BLUETOOTH_CONNECT)
    public void connect(BluetoothDevice device, BluetoothGattCallback gattCallback) {
        device.connectGatt(getApplicationContext(), false, gattCallback);
    }

    /**
     * Retrieves scanner's Wi-Fi information asynchronously if read property is enabled.
     * {@link BluetoothGattCallback#onCharacteristicRead(BluetoothGatt, BluetoothGattCharacteristic, byte[], int)}
     * or {@link BluetoothGattCallback#onCharacteristicRead(BluetoothGatt, BluetoothGattCharacteristic, int)} is called when Wi-Fi information is received.
     *
     * @param connection BluetoothGatt connection received in {@link BluetoothGattCallback#onConnectionStateChange(BluetoothGatt, int, int)}
     * @implNote services must be first discovered with {@link BluetoothService#discoverServices(BluetoothGatt)}
     * @see me.clarius.sdk.solum.example.viewmodels.BluetoothViewModel for BluetoothGattCallback implementation
     */
    @RequiresApi(api = Build.VERSION_CODES.S)
    @RequiresPermission(value = Manifest.permission.BLUETOOTH_CONNECT)
    public void retrieveWifiInfo(final BluetoothGatt connection) {
        BluetoothGattService service = getService(connection, ServiceType.WifiInformationService);
        if (service == null) {
            Log.e(TAG, "Cannot get service information from the scanner");
        }
        BluetoothGattCharacteristic characteristic = getCharacteristic(service, ServiceType.WifiInformationService, CharacteristicType.Info);
        readCharacteristic(connection, characteristic);
        subscribeToCharacteristic(connection, characteristic);
    }

    /*
     * Reads the given characteristic if read property is enabled.
     */
    @RequiresApi(api = Build.VERSION_CODES.S)
    @RequiresPermission(value = Manifest.permission.BLUETOOTH_CONNECT)
    private void readCharacteristic
    (
        @NonNull final BluetoothGatt connection,
        @NonNull final BluetoothGattCharacteristic characteristic
    )
    {
        assert hasProperty(characteristic, BluetoothGattCharacteristic.PROPERTY_READ);
        if (connection.readCharacteristic(characteristic))
        {
            Log.d(TAG, "Start reading the characteristic");
            Log.d(TAG, String.valueOf(Build.VERSION.SDK_INT));
        } else {
            Log.e(TAG, "Unable to read the characteristic");
        }
    }

    /*
     * Gets the selected service if it's available.
     */
    private BluetoothGattService getService(final BluetoothGatt connection, final ServiceType type) {
        BluetoothGattService service = connection.getService(type.serviceUuid);
        if (service == null) {
            Log.v(TAG, type.name() + " is unavailable.");
        }
        return service;
    }

    /*
     * Gets the selected characteristic if it's available.
     */
    private BluetoothGattCharacteristic getCharacteristic(BluetoothGattService service, ServiceType serviceType, CharacteristicType characteristicType) {
        BluetoothGattCharacteristic characteristic = characteristicType == CharacteristicType.Info
                ? getCharacteristic(service, serviceType.infoCharacteristicUuid)
                : getCharacteristic(service, serviceType.requestCharacteristicUuid);
        if (characteristic == null) {
            Log.v(TAG, characteristicType.name() + " of " + serviceType.name() + "is unavailable");
        }
        return characteristic;
    }

    private BluetoothGattCharacteristic getCharacteristic(BluetoothGattService service, UUID characteristicUuid) {
        if (service == null) {
            Log.e(TAG, "No longer connected to the service");
            return null;
        }
        return service.getCharacteristic(characteristicUuid);
    }

    /**
     * Subscribes to the given characteristic's notifications if write property is enabled.
     *
     * @param connection BluetoothGatt
     * @param characteristic BluetoothGattCharacteristic characteristic offered by one of the Clarius scanner BLE services
     * @implNote notifications are received in {@link BluetoothGattCallback#onServiceChanged(BluetoothGatt)}
     */
    @RequiresApi(api = Build.VERSION_CODES.S)
    @RequiresPermission(value = Manifest.permission.BLUETOOTH_CONNECT)
    public void subscribeToCharacteristic(final BluetoothGatt connection, final BluetoothGattCharacteristic characteristic) {
        if (connection == null || characteristic == null) return;
        if (hasProperty(characteristic, BluetoothGattCharacteristic.PROPERTY_WRITE)) {
            connection.setCharacteristicNotification(characteristic, true);
        }
    }

    /**
     * Checks if the given characteristic has the given property.
     *
     * @param characteristic characteristic offered by one of the Clarius scanner BLE services
     * @param property integer representing property flag
     * @return true if the characteristic has the property.
     */
    public boolean hasProperty(final BluetoothGattCharacteristic characteristic, final int property) {
        return characteristic != null && (characteristic.getProperties() & property) != 0;
    }

    /**
     * Discover the services offered by Clarius scanners asynchronously.
     * Calls {@link BluetoothGattCallback#onServicesDiscovered(BluetoothGatt, int)} on successful service discovery.
     *
     * @param bluetoothGatt BluetoothGatt connection received in {@link BluetoothGattCallback#onConnectionStateChange(BluetoothGatt, int, int)}
     * @return true if started discovering services, else false
     */
    @RequiresApi(api = Build.VERSION_CODES.S)
    @RequiresPermission(value = Manifest.permission.BLUETOOTH_CONNECT)
    public boolean discoverServices(final BluetoothGatt bluetoothGatt) {
        if (bluetoothGatt == null) return false;
        return bluetoothGatt.discoverServices();
    }

    public class BluetoothBinder extends Binder {
        public BluetoothService getService() {
            return BluetoothService.this;
        }
    }
}
