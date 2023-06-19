package me.clarius.sdk.solum.example.bluetooth;

import android.Manifest;
import android.app.Service;
import android.bluetooth.BluetoothManager;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.content.Context;
import android.content.Intent;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.util.Log;

import androidx.annotation.Nullable;
import androidx.annotation.RequiresPermission;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * Find clarius probes over bluetooth
 * <p>
 * The scan automatically stops after a preset time period to minimize energy consumption.
 * <p>
 * See: https://developer.android.com/guide/topics/connectivity/bluetooth/find-ble-devices
 */

public class ScanService extends Service {
    private static final long SCAN_PERIOD = 10000;
    private static final String TAG = "BLE(scan)";
    private final List<String> found = new ArrayList<>();
    private final Handler handler = new Handler(Looper.myLooper());
    private final IBinder binder = new ScanService.CustomBinder();
    private final AtomicBoolean scanning = new AtomicBoolean(false);
    private BluetoothLeScanner bluetoothLeScanner;
    private Listener listener;
    private final ScanCallback scanCallback = new ScanCallback() {
        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            super.onScanResult(callbackType, result);
            String deviceName = result.getScanRecord().getDeviceName();
            if (null != deviceName && deviceName.startsWith("CUS-")) {
                if (addIfMissing(deviceName)) {
                    Log.i(TAG, "Found clarius probe: " + deviceName + " with address: " + result.getDevice().getAddress());
                    listener.newProbe(deviceName, result);
                }
            }
        }

        @Override
        public void onScanFailed(int errorCode) {
            super.onScanFailed(errorCode);
            Log.e(TAG, "Scan failed with code: " + errorCode);
            listener.failed(errorCode);
        }
    };

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return binder;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d(TAG, "Creating bluetooth scan service");
        bluetoothLeScanner = ((BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE)).getAdapter().getBluetoothLeScanner();
        assert bluetoothLeScanner != null : "Unable to get the system BLE scanner, is bluetooth enabled?";
    }

    @RequiresPermission(Manifest.permission.BLUETOOTH_SCAN)
    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "Destroying bluetooth scan service");
        stopScan();
        bluetoothLeScanner = null;
    }

    public void initialize(Listener listener) {
        this.listener = listener;
    }

    @RequiresPermission(Manifest.permission.BLUETOOTH_SCAN)
    public void startScan() {
        if (scanning.compareAndSet(false, true)) {
            found.clear();
            bluetoothLeScanner.startScan(scanCallback);
            listener.started();
            handler.postDelayed(this::stopScan, SCAN_PERIOD);
            Log.d(TAG, "Scan started");
        }
    }

    @RequiresPermission(Manifest.permission.BLUETOOTH_SCAN)
    void stopScan() {
        if (scanning.compareAndSet(true, false)) {
            Log.d(TAG, "Scan finished, found " + found.size() + " probe(s)");
            bluetoothLeScanner.stopScan(scanCallback);
            listener.finished(found);
        }
    }

    synchronized private boolean addIfMissing(String deviceName) {
        if (null != deviceName && found.stream().noneMatch(it -> it.equals(deviceName))) {
            found.add(deviceName);
            return true;
        } else {
            return false;
        }
    }

    public interface Listener {
        void newProbe(String deviceName, ScanResult result);

        void failed(int errorCode);

        void started();

        void finished(List<String> found);
    }

    public class CustomBinder extends Binder {
        public ScanService getService() {
            return ScanService.this;
        }
    }
}
