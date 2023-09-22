package me.clarius.sdk.solum.example;

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;

import androidx.annotation.NonNull;

import org.chromium.net.CronetEngine;
import org.chromium.net.CronetException;
import org.chromium.net.UrlRequest;
import org.chromium.net.UrlResponseInfo;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.Executors;

public class CertificatesManager {
    private static final String TAG = "Cert Manager";
    private static final int BUFFER_SIZE = 51200;
    private final CronetEngine cronetEngine;
    private final String maybeCert;
    private final Activity activity;
    private final Listener listener;
    private Map<String, String> certificates;

    public CertificatesManager(@NonNull Activity activity, Listener listener) {
        this.activity = activity;
        cronetEngine = new CronetEngine.Builder(activity).build();
        maybeCert = Secrets.maybeCert().orElse(null);
        certificates = loadCachedCerts(activity);
        this.listener = listener;
    }

    private static HashMap<String, String> parseCertificates(String json) throws JSONException {
        JSONArray arrayOfCerts = new JSONObject(json).getJSONArray("results");
        int numberOfCertificates = arrayOfCerts.length();
        HashMap<String, String> result = new HashMap<>(numberOfCertificates);
        for (int i = 0; i < numberOfCertificates; i++) {
            JSONObject certificate = arrayOfCerts.getJSONObject(i);
            if (certificate.has("crt"))
                result.put(certificate.getJSONObject("device").getString("serial"), certificate.getString("crt"));
        }
        return result;
    }

    private static HashMap<String, String> loadCachedCerts(Activity activity) {
        String maybeCorrectCert = activity.getPreferences(Context.MODE_PRIVATE).getString("certs", null);
        if (maybeCorrectCert != null) {
            try {
                return parseCertificates(maybeCorrectCert);
            } catch (JSONException exception) {
                Log.e(TAG, "Invalid certificates; cannot be converted to JSON");
            }
        }

        return null;
    }

    public void downloadCertificates(@NonNull String oemKey) {
        cronetEngine.newUrlRequestBuilder("https://cloud.clarius.com/api/public/v0/devices/oem/", new CertificatesDownloadedCallback(),
                        Executors.newSingleThreadExecutor()).addHeader("Authorization", "Authorization: OEM-API-Key " + oemKey)
                .build()
                .start();
    }

    public String getCertificate(String serial) throws RuntimeException {
        if (maybeCert != null)
            return maybeCert;
        if (serial == null)
            throw new RuntimeException("No probe serial provided; the certificate cannot be found");
        if (certificates == null || certificates.isEmpty())
            throw new RuntimeException("No certificate provided");
        if (!certificates.containsKey(serial))
            throw new RuntimeException("No certificate found for the selected probe");
        return certificates.get(serial);
    }

    public interface Listener {
        void certsDownloaded();

        void error(String message);
    }

    private class CertificatesDownloadedCallback extends UrlRequest.Callback {
        private final StringBuilder certsBuilder = new StringBuilder();

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
            certsBuilder.append(new String(IOUtils.toByteArray(byteBuffer)));
            byteBuffer.clear();
            request.read(byteBuffer);
        }

        @Override
        public void onSucceeded(UrlRequest request, UrlResponseInfo info) {
            String certsString = certsBuilder.toString();
            try {
                certificates = parseCertificates(certsString);
            } catch (JSONException e) {
                listener.error(e.getMessage());
            }
            SharedPreferences preferences = activity.getPreferences(Context.MODE_PRIVATE);
            preferences.edit().putString("certs", certsString).apply();
            listener.certsDownloaded();
        }

        @Override
        public void onFailed(UrlRequest request, UrlResponseInfo info, CronetException error) {
            listener.error("The request failed: " + error);
        }
    }
}
