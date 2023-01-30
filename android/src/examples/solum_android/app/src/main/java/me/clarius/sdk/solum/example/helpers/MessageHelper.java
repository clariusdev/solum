package me.clarius.sdk.solum.example.helpers;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.widget.Toast;

public class MessageHelper {
    private final Activity activity;
    private final String classTag;

    public MessageHelper(final Activity activity, final String classTag) {
        this.activity = activity;
        this.classTag = classTag;
    }

    public void showMessage(final CharSequence text)  {
        Log.d(classTag, (String) text);
        Handler mainHandler = new Handler(Looper.getMainLooper());
        mainHandler.post(() -> Toast.makeText(activity, text, Toast.LENGTH_SHORT).show());
    }

    public void showError(final CharSequence text) {
        Log.e(classTag, "Error: " + text);
        Handler mainHandler = new Handler(Looper.getMainLooper());
        mainHandler.post(() -> Toast.makeText(activity, text, Toast.LENGTH_SHORT).show());
    }

    public void showDialogError(final CharSequence message, final DialogInterface.OnClickListener positiveButtonEffect)
    {
        Log.e(classTag, "Error: " + message);
        getDialogError(
                activity,
                message,
                "Ok",
                positiveButtonEffect
        ).show();
    }

    public static AlertDialog getDialogError
    (
            final Activity activity,
            final CharSequence message,
            final CharSequence positiveButtonTitle,
            final DialogInterface.OnClickListener positiveButtonEffect
    )
    {
        return new AlertDialog.Builder(activity)
                .setTitle("Error")
                .setMessage(message)
                .setPositiveButton(positiveButtonTitle, positiveButtonEffect)
                .setNegativeButton("Dismiss", ((dialogInterface, i) -> dialogInterface.dismiss()))
                .create();
    }

    public static DialogInterface.OnClickListener getDismissAction() {
        return (((dialogInterface, i) -> dialogInterface.dismiss()));
    }
}
