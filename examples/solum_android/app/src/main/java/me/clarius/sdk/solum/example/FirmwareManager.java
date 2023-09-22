package me.clarius.sdk.solum.example;

import android.app.Activity;
import android.content.Context;
import android.util.Log;

import androidx.annotation.NonNull;

import org.chromium.net.CronetEngine;
import org.chromium.net.CronetException;
import org.chromium.net.UrlRequest;
import org.chromium.net.UrlResponseInfo;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;
import java.nio.ByteBuffer;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class FirmwareManager {
    private static final String TAG = "FW Manager";
    private final CronetEngine cronetEngine;
    private final Context activity;
    private final Listener listener;
    private final ExecutorService executor = Executors.newSingleThreadExecutor();
    private String fileName;

    public FirmwareManager(@NonNull Activity activity, Listener listener) {
        this.activity = activity;
        cronetEngine = new CronetEngine.Builder(activity).build();
        this.listener = listener;
    }

    public void downloadFW(@NonNull String oemKey, @NonNull String firmwareVersion) throws RuntimeException {
        cronetEngine.newUrlRequestBuilder(String.format("https://cloud.clarius.com/api/public/v0/devices/oem/firmware/%s/", firmwareVersion), new DownloadLinkReceivedCallback(),
                        executor).addHeader("Authorization", "Authorization: OEM-API-Key " + oemKey)
                .build()
                .start();
        fileName = String.format("firmware_v%s.zip", firmwareVersion);
    }

    private void doStartDownload(@NonNull String url) {
        try {
            fileName = new File(new URL(url).getPath()).getName();
            cronetEngine.newUrlRequestBuilder(url, new FirmwareDownloadedCallback(), executor)
                    .build()
                    .start();
        } catch (MalformedURLException e) {
            listener.error(e.getMessage());
        }
    }

    public interface Listener {
        void firmwareDownloaded(String firmwarePath);

        void error(String message);
    }

    private class DownloadLinkReceivedCallback extends UrlRequest.Callback {
        private static final int BUFFER_SIZE = 1024;
        private final StringBuilder linkBuilder = new StringBuilder();

        @Override
        public void onRedirectReceived(UrlRequest request, UrlResponseInfo info, String newLocationUrl) {
            request.followRedirect();
        }

        @Override
        public void onResponseStarted(UrlRequest request, UrlResponseInfo info) {
            final int httpStatusCode = info.getHttpStatusCode();

            switch (httpStatusCode / 100) {
                case 2:
                    request.read(ByteBuffer.allocateDirect(BUFFER_SIZE));
                    break;
                case 4:
                    listener.error("Client Error; HTTP Status Code: " + httpStatusCode);
                    break;
                case 5:
                    listener.error("Server error; HTTP Status Code: " + httpStatusCode);
                    break;
                default:
                    Log.i(TAG, "HTTP Status Code: " + httpStatusCode);
            }
        }

        @Override
        public void onReadCompleted(UrlRequest request, UrlResponseInfo info, ByteBuffer byteBuffer) {
            linkBuilder.append(new String(IOUtils.toByteArray(byteBuffer)));
            byteBuffer.clear();
            request.read(byteBuffer);
        }

        @Override
        public void onSucceeded(UrlRequest request, UrlResponseInfo info) {
            String link = linkBuilder.toString();
            try {
                doStartDownload(parseDownloadLink(link));
            } catch (JSONException e) {
                listener.error(e.getMessage());
            }
        }

        @Override
        public void onFailed(UrlRequest request, UrlResponseInfo info, CronetException error) {
            listener.error("The request failed: " + error);
        }

        private String parseDownloadLink(String maybeValidJson) throws JSONException {
            return new JSONObject(maybeValidJson).getString("file");
        }
    }

    private class FirmwareDownloadedCallback extends UrlRequest.Callback {
        private FileOutputStream fos;
        private static final int BUFFER_SIZE = 1024 * 1024;
        private String firmwarePath;

        @Override
        public void onRedirectReceived(UrlRequest request, UrlResponseInfo info, String newLocationUrl) {
            request.followRedirect();
        }

        @Override
        public void onResponseStarted(UrlRequest request, UrlResponseInfo info) {
            final int httpStatusCode = info.getHttpStatusCode();

            switch (httpStatusCode / 100) {
                case 2:
                    try {
                        fos = activity.openFileOutput(fileName, Context.MODE_PRIVATE);
                        firmwarePath = new File(activity.getFilesDir(), fileName).getAbsolutePath();
                    } catch (IOException e) {
                        listener.error(e.getMessage());
                    }
                    request.read(ByteBuffer.allocateDirect(BUFFER_SIZE));
                    break;
                case 4:
                    listener.error("Client Error; HTTP Status Code: " + httpStatusCode);
                    break;
                case 5:
                    listener.error("Server error; HTTP Status Code: " + httpStatusCode);
                    break;
                default:
                    Log.i(TAG, "HTTP Status Code: " + httpStatusCode);
            }
        }

        @Override
        public void onReadCompleted(UrlRequest request, UrlResponseInfo info, ByteBuffer byteBuffer) {
            try {
                fos.write(IOUtils.toByteArray(byteBuffer));
            } catch (IOException e) {
                listener.error(e.getMessage());
            }
            byteBuffer.clear();
            request.read(byteBuffer);
        }

        @Override
        public void onSucceeded(UrlRequest request, UrlResponseInfo info) {
            listener.firmwareDownloaded(firmwarePath);
        }

        @Override
        public void onFailed(UrlRequest request, UrlResponseInfo info, CronetException error) {
            listener.error("The request failed: " + error);
        }
    }
}
