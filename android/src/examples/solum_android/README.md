Solum Example
====

1. Copy the `aar` package into the `libs` subfolder
2. Re-sync Gradle
3. Build

Important Notes
----

- Add `android:extractNativeLibs="true"` in the manifest file, in the `application` tag, this ensures the native libraries are copied to the install folder and can be accessed at runtime.

        <manifest ...>
            <application
                android:extractNativeLibs="true" ...>

- Ensure the Bluetooth MTU size is big enough to read Wi-Fi info at once, otherwise the info will be truncated.

- When subscribing to notifications, first call `BluetoothGat.setCharacteristicNotification()`, then write `0x00` to the Client Characteristic Configuration Descriptor (`0x2902`).

- To keep the GATT connection alive, regularly write `0x00` to the alert level characteristic (`0x2a06`) every 5 seconds or so, this will prevent auto-disconnection.

- On Android < 33, allow some delay between successive writes to the Bluetooth GATT.

TODO
----

* Provide a multi-ABI package
