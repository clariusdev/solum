package me.clarius.sdk.solum.example;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.net.Uri;
import android.os.Build;
import android.provider.MediaStore;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.Arrays;

public class IOUtils {
    @RequiresApi(api = Build.VERSION_CODES.Q)
    private static Uri save(ByteBuffer buffer, String prefix, Context context) throws IOException {
        final String fileName = String.format("%s_%s.tar", prefix,
                LocalDateTime.now().format(DateTimeFormatter.ofPattern("yyyyMMdd_HHmmss")));
        final ContentValues contentValues = new ContentValues();
        contentValues.put(MediaStore.MediaColumns.DISPLAY_NAME, fileName);
        contentValues.put(MediaStore.MediaColumns.MIME_TYPE, "application/x-tar");
        contentValues.put(MediaStore.MediaColumns.RELATIVE_PATH, "Documents/Clarius");
        final ContentResolver contentResolver = context.getContentResolver();
        final Uri uri = MediaStore.Files.getContentUri(MediaStore.VOLUME_EXTERNAL_PRIMARY);
        final Uri itemUri = contentResolver.insert(uri, contentValues);
        if (itemUri == null) {
            throw new IOException("Failed to create the raw data file in the Documents folder");
        }
        try (OutputStream dest = contentResolver.openOutputStream(itemUri)) {
            dest.write(buffer.array());
        }
        return itemUri;
    }

    /**
     * Save the given byte buffer in the Documents folder.
     * <p>
     * NOTE: this method uses the MediaStore.Files.getContentUri() API which is only available on Android 10 and later.
     * Calling this method on older Android will raise an exception.
     *
     * @param buffer  the byte buffer to save.
     * @param context the context to retrieve the Documents folder.
     * @return the saved file location.
     * @throws IOException
     */
    public static Uri saveInDocuments(ByteBuffer buffer, String prefix, Context context) throws IOException {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            return save(buffer, prefix, context);
        } else {
            throw new IOException("Saving only supported on Android 10 and later (API Q)");
        }
    }

    /**
     * Convert a directly allocated buffer to a byte array.
     * The byte buffer must be backed by an accessible byte array.
     *
     * @param byteBuffer the byte buffer to convert
     * @return the converted byte[]
     */
    public static byte[] toByteArray(ByteBuffer byteBuffer) {
        assert byteBuffer.hasArray();
        int offSet = byteBuffer.arrayOffset();
        int length = offSet + byteBuffer.position();
        return Arrays.copyOfRange(byteBuffer.array(), offSet, length);
    }
}
