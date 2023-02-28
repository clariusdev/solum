package me.clarius.sdk.solum.example;

import android.util.Log;

import java.lang.reflect.Field;
import java.util.Optional;

/**
 * Get constants defined in class BuildConfig generated from file local.properties.
 * <p>
 * The constants are defined in file local.properties and added in the generated class BuildConfig by plugin secrets-gradle-plugin.
 * If a constant is missing from the file, it will not be added in class BuildConfig causing a compilation error if trying to access it.
 * This helper class uses reflection to read the fields only if they exist.
 * This is useful during development to import network values instead of typing them in the GUI.
 * <p>
 * Known properties:
 * - clariusProbeCert: the scanner cert here, obtained from https://cloud.clarius.com/api/public/v0/devices/oem/
 * - clariusProbeSSID: the scanner Wi-Fi Direct SSID, obtained from Bluetooth scanning
 * - clariusProbePassphrase: the scanner Wi-Fi WPA2 passphrase, obtained from Bluetooth scanning
 * - clariusProbeMacAddress: the probe's MAC address, obtained from Bluetooth scanning
 */

public class ClariusConfig {
    private static final String TAG = "ClariusConfig";

    public static Optional<String> maybeCert() {
        return Optional.ofNullable((String) getFieldValue(BuildConfig.class, "clariusProbeCert"));
    }

    public static Optional<String> maybeSSID() {
        return Optional.ofNullable((String) getFieldValue(BuildConfig.class, "clariusProbeSSID"));
    }

    public static Optional<String> maybePassphrase() {
        return Optional.ofNullable((String) getFieldValue(BuildConfig.class, "clariusProbePassphrase"));
    }

    public static Optional<String> maybeMacAddress() {
        return Optional.ofNullable((String) getFieldValue(BuildConfig.class, "clariusProbeMacAddress"));
    }

    private static Object getFieldValue(Class<?> fromClass, String name) {
        try {
            Field field = fromClass.getField(name);
            return field.get(null);
        } catch (NoSuchFieldException | IllegalAccessException e) {
            Log.w(TAG, "Trying to access property '" + name + "' which is missing from file 'local.properties'.");
        }
        return null;
    }
}
