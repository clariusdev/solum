Solum Example
====

1. Copy the `aar` package into the `libs` subfolder
2. Re-sync Gradle
3. Build

Important Notes
----

- Set the DSL option `useLegacyPackaging = true` in the application's `build.gradle`, this ensures the native libraries are extracted and can be loaded at runtime because the current implementation does not support loading uncompressed libraries from the APK:

- Ensure the Bluetooth MTU size is big enough to read Wi-Fi info at once, otherwise the info will be truncated.

- When subscribing to notifications, first call `BluetoothGat.setCharacteristicNotification()`, then write `0x00` to the Client Characteristic Configuration Descriptor (`0x2902`).

- To keep the GATT connection alive, regularly write `0x00` to the alert level characteristic (`0x2a06`) every 5 seconds or so, this will prevent auto-disconnection.

- On Android < 33, allow some delay between successive writes to the Bluetooth GATT.

TODO
----

* Provide a multi-ABI package
* Download certificates

Certificates
----

The certificates are not downloaded automatically (TBD), for now, the certificates are imported from a property in file `secrets.properties`:

1. Download the certificate from cloud:

        curl -H "Authorization: OEM-API-Key <your key>" "https://cloud.clarius.com/api/public/v0/devices/oem/" | jq

2. Create file `secret.properties`

3. Add a property `clariusProbeCertificate="<your cert>"` (escape newline characters in the string: `"\n"` -> `"\\n"`)

4. This property will be added to the Java class `BuildConfig` by plugin secrets-gradle-plugin
