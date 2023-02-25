package me.clarius.sdk.solum.example.bluetooth;

import android.Manifest;
import android.app.Service;
import android.bluetooth.BluetoothManager;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;

import androidx.annotation.Nullable;
import androidx.core.app.ActivityCompat;

import java.util.ArrayList;
import java.util.List;

/**
 * Find clarius probes over bluetooth
 * <p>
 * The scan automatically stops after a preset time period to minimize energy consumption.
 * <p>
 * See: https://developer.android.com/guide/topics/connectivity/bluetooth/find-ble-devices
 */

public class ScanService extends Service {
    private static final long SCAN_PERIOD = 10000;
    private final List<String> found = new ArrayList<>();
    private final Handler handler = new Handler(Looper.myLooper());
    private final IBinder binder = new ScanService.CustomBinder();
    private BluetoothLeScanner bluetoothLeScanner;
    private Listener listener;
    private final ScanCallback scanCallback = new ScanCallback() {
        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            super.onScanResult(callbackType, result);
            String deviceName = result.getScanRecord().getDeviceName();
            if (null != deviceName && deviceName.startsWith("CUS-")) {
                if (addIfMissing(deviceName)) {
                    listener.newProbe(deviceName, result);
                }
            }
        }

        @Override
        public void onScanFailed(int errorCode) {
            super.onScanFailed(errorCode);
            listener.failed(errorCode);
        }
    };
    private boolean scanning = false;

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return binder;
    }

    @Override
    public boolean onUnbind(Intent intent) {
        stopScan();
        return super.onUnbind(intent);
    }

    public boolean initialize(Listener listener) {
        bluetoothLeScanner = ((BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE)).getAdapter().getBluetoothLeScanner();
        if (bluetoothLeScanner == null) {
            return false;
        }
        this.listener = listener;
        return true;
    }

    public void startScan() {
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_SCAN) != PackageManager.PERMISSION_GRANTED) {
        }
        if (setScanningFlag(true)) {
            found.clear();
            bluetoothLeScanner.startScan(scanCallback);
            listener.started();
            handler.postDelayed(this::stopScan, SCAN_PERIOD);
        }
    }

    void stopScan() {
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_SCAN) != PackageManager.PERMISSION_GRANTED) {
        }
        if (setScanningFlag(false)) {
            bluetoothLeScanner.stopScan(scanCallback);
            listener.finished(found);
        }
    }

    private synchronized boolean setScanningFlag(boolean scanning) {
        if (scanning != this.scanning) {
            this.scanning = scanning;
            return true;
        } else {
            return false;
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
