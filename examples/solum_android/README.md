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
To manually download the certificates:

1. Download the certificate from cloud:

        curl -H "Authorization: OEM-API-Key <your key>" "https://cloud.clarius.com/api/public/v0/devices/oem/" | jq

2. Create file `secret.properties`

3. Add a property `clariusProbeCertificate="<your cert>"` (escape newline characters in the string: `"\n"` -> `"\\n"`)

4. This property will be added to the Java class `BuildConfig` by plugin secrets-gradle-plugin

To automatically download the certificates:

1. Create file `secret.properties`

2. Add a property `clarius_oem_api_key="<your OEM-API-Key>"`

3. This property will be added to the Java class `BuildConfig` by plugin secrets-gradle-plugin

4. The certificates will be automatically downloaded when the app is launched and saved to [SharedPreferences](https://developer.android.com/training/data-storage/shared-preferences)
