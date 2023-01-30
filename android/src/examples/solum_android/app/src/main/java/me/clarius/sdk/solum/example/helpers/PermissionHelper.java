package me.clarius.sdk.solum.example.helpers;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;
import android.provider.Settings;
import android.util.Log;

import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.ActivityResultRegistry;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;
import androidx.core.app.ActivityCompat;
import androidx.fragment.app.FragmentActivity;
import androidx.lifecycle.DefaultLifecycleObserver;
import androidx.lifecycle.LifecycleOwner;

import java.util.ArrayList;
import java.util.Map;

import me.clarius.sdk.solum.example.exceptions.BadApiLevelException;

public class PermissionHelper implements DefaultLifecycleObserver {
    public final static String TAG = "PermissionHelper";
    public final static int NAVIGATE_TO_SETTINGS_CODE = 1000;
    public final static ArrayList<String> REQUIRED_PERMISSIONS = buildRequiredPermissions();

    private ActivityResultLauncher<String[]> activityResultLauncher;

    private final ActivityResultRegistry resultRegistry;
    private final Runnable onPermissionsReceivedCallback;
    private final MessageHelper messageHelper;

    public PermissionHelper(@NonNull final FragmentActivity activity, final Runnable onPermissionsReceivedCallback) {
        this.resultRegistry  = activity.getActivityResultRegistry();
        this.onPermissionsReceivedCallback = onPermissionsReceivedCallback;
        this.messageHelper = new MessageHelper(activity, TAG);

    }

    @Override
    public void onCreate(@NonNull final LifecycleOwner owner) {
        DefaultLifecycleObserver.super.onCreate(owner);

        activityResultLauncher = resultRegistry.register("PermissionHelper", owner, new ActivityResultContracts.RequestMultiplePermissions(), mapOfPermissions -> {
            for (Map.Entry<String, Boolean> permission : mapOfPermissions.entrySet()) {
                if (!permission.getValue()) {
                    messageHelper.showDialogError(permission.getKey() + " is required to demonstrate this feature", MessageHelper.getDismissAction());
                    return;
                }
            }
            onPermissionsReceivedCallback.run();
        });
    }

    public void requestPermissions() {
        activityResultLauncher.launch(REQUIRED_PERMISSIONS.toArray(new String[0])); //https://stackoverflow.com/questions/53284214/toarray-with-pre-sized-array
    }

    public boolean checkPermissions(Context context) throws BadApiLevelException {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.S) {
            throw new BadApiLevelException(Build.VERSION.SDK_INT);
        }
        return doCheckPermissions(context);
    }

    @RequiresApi(api = Build.VERSION_CODES.S)
    private boolean doCheckPermissions(Context context) {
        for (String permission : REQUIRED_PERMISSIONS) {
            if (ActivityCompat.checkSelfPermission(context, permission) != PackageManager.PERMISSION_GRANTED) {
                Log.e(TAG, permission + " is not granted");
                return false;
            } else {
                Log.d(TAG, permission + " is granted");
            }
        }
        return true;
    }

    private static ArrayList<String> buildRequiredPermissions() {
        ArrayList<String> result = new ArrayList<>();
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            result.add(Manifest.permission.BLUETOOTH_CONNECT);
            result.add(Manifest.permission.BLUETOOTH_SCAN);
        }
        result.add(Manifest.permission.ACCESS_FINE_LOCATION);
        result.add(Manifest.permission.ACCESS_COARSE_LOCATION);
        return result;
    }

    public static void navigateToAppPermissionSettings(Activity activity) throws BadApiLevelException {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.S) {
            throw new BadApiLevelException(Build.VERSION.SDK_INT);
        }
        doNavigateToAppPermissionSettings(activity);
    }

    @RequiresApi(api = Build.VERSION_CODES.S)
    private static void doNavigateToAppPermissionSettings(Activity activity) {
        MessageHelper.getDialogError(
                activity,
                "You need to grant permission in Settings manually",
                "Go to Settings",
                (dialogInterface, i) -> {
                    Intent intent = new Intent(Settings.ACTION_APPLICATION_DETAILS_SETTINGS);
                    Uri uri = Uri.fromParts("package", activity.getPackageName(), null);
                    intent.setData(uri);
                    activity.startActivityForResult(intent, NAVIGATE_TO_SETTINGS_CODE);
                }
        ).show();
    }
}
