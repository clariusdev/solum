package me.clarius.sdk.solum.example.bluetooth;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Build;
import android.util.Log;

import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.ActivityResultRegistry;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;
import androidx.lifecycle.DefaultLifecycleObserver;
import androidx.lifecycle.LifecycleOwner;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * Get result from the permission request activity
 * <p>
 * From: https://developer.android.com/training/basics/intents/result.
 */

public class PermissionHelper implements DefaultLifecycleObserver {
    private final static String TAG = "BLE(perm)";
    private final ActivityResultRegistry registry;
    private final Result callback;
    private ActivityResultLauncher<String[]> requestPerms;

    public PermissionHelper(@NonNull ActivityResultRegistry registry, Context context, Result callback) {
        this.registry = registry;
        this.callback = callback;
        List<String> missing = getMissingPermissions(context);
        if (missing.isEmpty()) {
            Log.i(TAG, "All permissions granted");
        } else {
            Log.w(TAG, "Missing permissions: " + missing);
        }
    }

    private static List<String> requiredPermissions() {
        List<String> result = new ArrayList<>();
        if (Build.VERSION.SDK_INT >= 31) {
            result.add(Manifest.permission.BLUETOOTH_CONNECT);
            result.add(Manifest.permission.BLUETOOTH_SCAN);
        }
        if (Build.VERSION.SDK_INT <= 30) {
            result.add(Manifest.permission.ACCESS_FINE_LOCATION);
        }
        return result;
    }

    private static List<String> getMissingPermissions(Context context) {
        List<String> missing = new ArrayList<>();
        for (String permission : PermissionHelper.requiredPermissions()) {
            if (ActivityCompat.checkSelfPermission(context, permission) != PackageManager.PERMISSION_GRANTED)
                missing.add(permission);
        }
        return missing;
    }

    @Override
    public void onCreate(@NonNull final LifecycleOwner owner) {
        DefaultLifecycleObserver.super.onCreate(owner);
        requestPerms = registry.register("perms", owner, new ActivityResultContracts.RequestMultiplePermissions(), mapOfPermissions -> {
            List<String> refused = mapOfPermissions.entrySet().stream().filter(entry -> !entry.getValue()).map(Map.Entry::getKey).collect(Collectors.toList());
            if (refused.isEmpty()) {
                Log.i(TAG, "All permissions granted");
                callback.onRequestPermissionsResult(true);
            } else {
                Log.w(TAG, "Permissions refused: " + refused);
                callback.onRequestPermissionsResult(false);
            }
        });
    }

    // apparently the linter checks any call to a function spelled checkSelfPermission() to verify
    // if we check permissions before calling functions with the @RequiresPermission annotation
    public static boolean checkSelfPermission(Context context) {
        for (final String permission : PermissionHelper.requiredPermissions()) {
            if (ActivityCompat.checkSelfPermission(context, permission) != PackageManager.PERMISSION_GRANTED)
                return false;
        }
        return true;
    }

    public void requestPermissions(Context context) {
        List<String> missing = new ArrayList<>();
        for (String permission : requiredPermissions()) {
            if (ActivityCompat.checkSelfPermission(context, permission) != PackageManager.PERMISSION_GRANTED) {
                missing.add(permission);
            }
        }
        if (missing.isEmpty()) {
            Log.i(TAG, "All permissions already granted");
            callback.onRequestPermissionsResult(true);
        } else {
            Log.i(TAG, "Requesting missing permissions: " + missing);
            requestPerms.launch(missing.toArray(new String[0]));
        }
    }

    @FunctionalInterface
    public interface Result {
        void onRequestPermissionsResult(boolean granted);
    }
}
