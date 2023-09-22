package me.clarius.sdk.solum.example;

import android.util.Log;

import java.lang.reflect.Field;
import java.util.Optional;

/**
 * Get constants defined in class BuildConfig generated from file secret.properties.
 * <p>
 * The constants are defined in file secret.properties and added in the generated class BuildConfig by plugin secrets-gradle-plugin.
 * The class and fields are loaded dynamically to avoid compilation errors if the class or fields are missing.
 * This is useful during development to import network values instead of typing them in the GUI.
 */

public class Secrets {
    private static final String TAG = "Secrets";

    public static Optional<String> maybeOemApiKey() {
        return Optional.ofNullable((String) getFieldValue("clarius_oem_api_key"));
    }

    public static Optional<String> maybeCert() {
        return Optional.ofNullable((String) getFieldValue("clarius_cert"));
    }

    public static Optional<String> maybeSSID() {
        return Optional.ofNullable((String) getFieldValue("clarius_ssid"));
    }

    public static Optional<String> maybePassphrase() {
        return Optional.ofNullable((String) getFieldValue("clarius_passphrase"));
    }

    public static Optional<String> maybeMacAddress() {
        return Optional.ofNullable((String) getFieldValue("clarius_mac_address"));
    }

    private static Object getFieldValue(String name) {
        try {
            final Class<?> buildConfigClass = Class.forName("me.clarius.sdk.solum.example.BuildConfig");
            Field field = buildConfigClass.getField(name);
            return field.get(null);
        } catch (NoSuchFieldException | IllegalAccessException e) {
            Log.w(TAG, "Failed to load property '" + name + "' because it is missing from file 'secrets.properties'");
        } catch (ClassNotFoundException e) {
            Log.w(TAG, "Failed to load property '" + name + "' because the 'BuildConfig' class was not generated");
        }
        return null;
    }
}
