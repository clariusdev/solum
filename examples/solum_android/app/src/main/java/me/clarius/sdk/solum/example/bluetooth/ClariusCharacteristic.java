package me.clarius.sdk.solum.example.bluetooth;

import android.bluetooth.BluetoothGattCharacteristic;

import java.util.UUID;

public enum ClariusCharacteristic {
    Alert(
            "00001802-0000-1000-8000-00805f9b34fb",
            "00002a06-0000-1000-8000-00805f9b34fb"
    ),
    WiFiPublished(
            "F9EB3FAE-947A-4E5B-AB7C-C799E91ED780",
            "F9EB3FAE-947A-4E5B-AB7C-C799E91ED781"
    ),
    PowerPublished(
            "8C853B6A-2297-44C1-8277-73627C8D2ABC",
            "8C853B6A-2297-44C1-8277-73627C8D2ABD"
    ),
    PowerRequest(
            "8C853B6A-2297-44C1-8277-73627C8D2ABC",
            "8C853B6A-2297-44C1-8277-73627C8D2ABE"
    );

    public final UUID serviceUuid;
    public final UUID uuid;

    ClariusCharacteristic(final String serviceUuid, final String uuid) {
        this.serviceUuid = UUID.fromString(serviceUuid);
        this.uuid = UUID.fromString(uuid);
    }

    public boolean equals(final BluetoothGattCharacteristic characteristic) {
        return uuid.equals(characteristic.getUuid()) && serviceUuid.equals(characteristic.getService().getUuid());
    }
}
