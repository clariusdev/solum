package me.clarius.sdk.solum.example;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Build;

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
 * From https://developer.android.com/training/basics/intents/result.
 */

public class PermissionHelper implements DefaultLifecycleObserver {
    public final static List<String> REQUIRED_PERMISSIONS = buildRequiredPermissions();
    private final ActivityResultRegistry registry;
    private final Result callback;
    private ActivityResultLauncher<String[]> requestPerms;

    public PermissionHelper(@NonNull ActivityResultRegistry registry, final Result callback) {
        this.registry = registry;
        this.callback = callback;
    }

    public static List<String> missingPermissions(Context context) {
        List<String> missing = new ArrayList<>();
        for (String permission : REQUIRED_PERMISSIONS) {
            if (ActivityCompat.checkSelfPermission(context, permission) != PackageManager.PERMISSION_GRANTED) {
                missing.add(permission);
            }
        }
        return missing;
    }

    private static List<String> buildRequiredPermissions() {
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

    @Override
    public void onCreate(@NonNull final LifecycleOwner owner) {
        DefaultLifecycleObserver.super.onCreate(owner);
        requestPerms = registry.register("perms", owner, new ActivityResultContracts.RequestMultiplePermissions(), mapOfPermissions -> {
            List<String> refused = mapOfPermissions.entrySet().stream().filter(entry -> !entry.getValue()).map(Map.Entry::getKey).collect(Collectors.toList());
            callback.accept(refused);
        });
    }

    public void requestPermissions() {
        requestPerms.launch(REQUIRED_PERMISSIONS.toArray(new String[0]));
    }

    @FunctionalInterface
    public interface Result {
        void accept(List<String> missingPermissions);
    }
}
