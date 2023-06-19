package me.clarius.sdk.solum.example;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

import java.nio.ByteBuffer;
import java.util.concurrent.Executor;

import me.clarius.sdk.ImageFormat;
import me.clarius.sdk.ProcessedImageInfo;

/**
 * Convert image data in separate thread to avoid blocking Solum
 * <p>
 * Image conversion workflow:
 * <pre>
 * Solum -> ImageConverter -> thread work -> ImageCallback -> live data -> display
 * </pre>
 */

public class ImageConverter {
    private final Executor executor;
    private final Callback callback;

    ImageConverter(Executor executor, Callback callback) {
        this.executor = executor;
        this.callback = callback;
    }

    public void convertImage(ByteBuffer buffer, ProcessedImageInfo info) {
        executor.execute(() -> {
            try {
                Bitmap bitmap = doConvert(buffer, info);
                callback.onResult(bitmap);
            } catch (Exception e) {
                callback.onError(e);
            }
        });
    }

    private Bitmap doConvert(ByteBuffer buffer, ProcessedImageInfo info) {
        boolean isCompressed = info.format != ImageFormat.Uncompressed;
        Bitmap bitmap;
        if (isCompressed) {
            if (buffer.hasArray()) {
                byte[] bytes = buffer.array();
                int offset = buffer.arrayOffset();
                int length = info.imageSize;
                assert offset + length < bytes.length;
                bitmap = BitmapFactory.decodeByteArray(bytes, offset, length);
            } else {
                byte[] bytes = new byte[buffer.capacity()];
                buffer.get(bytes);
                bitmap = BitmapFactory.decodeByteArray(bytes, 0, bytes.length);
            }
        } else {
            bitmap = Bitmap.createBitmap(info.width, info.height, Bitmap.Config.ARGB_8888);
            bitmap.copyPixelsFromBuffer(buffer);
        }
        if (bitmap == null) throw new AssertionError("bad image data");
        return bitmap;
    }

    interface Callback {
        void onResult(Bitmap bitmap);

        void onError(Exception e);
    }
}
