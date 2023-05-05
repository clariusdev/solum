package me.clarius.sdk.solum.example.bluetooth;

import android.Manifest;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothStatusCodes;
import android.os.Build;

import androidx.annotation.RequiresPermission;

public class Compat {
    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    static void writeCharacteristic(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, byte[] value) throws BluetoothException {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            int ret = gatt.writeCharacteristic(characteristic, value, BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
            if (BluetoothStatusCodes.SUCCESS != ret)
                throw new BluetoothException("Failed to write characteristic, return code: " + ret);
        } else {
            boolean set = characteristic.setValue(value);
            if (!set) throw new BluetoothException("Failed to write value in characteristic");
            characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
            boolean write = gatt.writeCharacteristic(characteristic);
            if (!write) throw new BluetoothException("Failed to write characteristic");
        }
    }

    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    static void writeDescriptor(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, byte[] value) throws BluetoothException {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            int ret = gatt.writeDescriptor(descriptor, value);
            if (BluetoothStatusCodes.SUCCESS != ret)
                throw new BluetoothException("Failed to write descriptor, return code: " + ret);
        } else {
            boolean set = descriptor.setValue(value);
            if (!set) throw new BluetoothException("Failed to write value in descriptor");
            boolean write = gatt.writeDescriptor(descriptor);
            if (!write) throw new BluetoothException("Failed to write descriptor");
        }
    }
}
