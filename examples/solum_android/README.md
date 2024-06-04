Solum Example
====

1. Copy the `aar` package (found in the repository's release section) into the libs subfolder
2. Add the OEM API key in the `secrets.properties` file
3. Re-sync Gradle
4. Build

Important Notes
----

- Set the DSL option `useLegacyPackaging = true` in the application's `build.gradle`, this ensures the native libraries are extracted and can be loaded at runtime because the current implementation does not support loading uncompressed libraries from the APK:

- Ensure the Bluetooth MTU size is big enough to read Wi-Fi info at once, otherwise the info will be truncated.

- When subscribing to notifications, first call `BluetoothGat.setCharacteristicNotification()`, then write `0x00` to the Client Characteristic Configuration Descriptor (`0x2902`).

- To keep the GATT connection alive, regularly write `0x00` to the alert level characteristic (`0x2a06`) every 5 seconds or so, this will prevent auto-disconnection.

- On Android < 33, allow some delay between successive writes to the Bluetooth GATT.

OEM API Key
----

The OEM API key is used to authenticate against the Clarius Cloud REST API for example when downloading probe certificates and firmware.

Obtain the OEM API key from the Clarius Cloud settings and paste it in the `secrets.properties` file:

    clarius_oem_api_key=<your key>

This property will be added to the Java class `BuildConfig` by plugin secrets-gradle-plugin and used in the code.
