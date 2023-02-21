package me.clarius.sdk.solum.example;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.MacAddress;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkRequest;
import android.net.wifi.WifiNetworkSpecifier;
import android.os.Build;
import android.util.Log;

import androidx.annotation.RequiresApi;

import java.util.Optional;

/**
 * Helper to auto-join a Wi-Fi network.
 */

public class WifiAutoJoin {
    private static final String TAG = "SolumAutoJoin";
    private MyCallback callback;

    /**
     * Join the specified Wi-Fi network.
     *
     * @param context    current context to get the network manager.
     * @param ssid       the network SSID, for example DIRECT-L738-B-1605-A0100.
     * @param passphrase the WPA2 passphrase.
     * @param macAddress is used to bypass user approval if the network was already joined before (optional).
     *                   https://developer.android.com/guide/topics/connectivity/wifi-bootstrap?#bypass-approval
     * @param result     get the result.
     * @throws BadApiLevelException if the API level is too low (auto-join is available starting API level 29).
     */
    public void join(Context context, String ssid, String passphrase, Optional<String> macAddress, Result result) throws BadApiLevelException {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            doJoin(context, ssid, passphrase, macAddress, result);
        } else {
            throw new BadApiLevelException(Build.VERSION_CODES.Q);
        }
    }

    @RequiresApi(api = Build.VERSION_CODES.Q)
    private void doJoin(Context context, String ssid, String passphrase, Optional<String> macAddress, Result result) {
        final ConnectivityManager connectivityManager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        assert null != connectivityManager;
        if (callback != null) {
            Log.i(TAG, "Removing existing connection request for " + callback.getSsid());
            connectivityManager.unregisterNetworkCallback(callback);
            callback = null;
        }
        callback = new MyCallback(ssid, result);
        WifiNetworkSpecifier.Builder specifierBuilder = new WifiNetworkSpecifier.Builder()
                .setSsid(ssid)
                .setWpa2Passphrase(passphrase);
        macAddress.ifPresent(s -> specifierBuilder.setBssid(MacAddress.fromString(s)));
        NetworkRequest request = new NetworkRequest.Builder()
                .addTransportType(NetworkCapabilities.TRANSPORT_WIFI)
                .addCapability(NetworkCapabilities.NET_CAPABILITY_NOT_RESTRICTED)
                .removeCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET)
                .setNetworkSpecifier(specifierBuilder.build())
                .build();
        connectivityManager.requestNetwork(request, callback);
    }

    @FunctionalInterface
    public interface Result {
        void accept(final boolean result, final String ssid, Optional<Long> networkID);
    }

    private static final class MyCallback extends ConnectivityManager.NetworkCallback {
        private final String ssid;
        private final Result result;

        MyCallback(final String ssid, final Result result) {
            Log.i(TAG, "Requesting connection to network: " + ssid);
            this.ssid = ssid;
            this.result = result;
        }

        @Override
        public void onAvailable(Network network) {
            Log.i(TAG, "Successfully connected to network: " + ssid + " with network ID: " + network.getNetworkHandle());
            result.accept(true, ssid, Optional.of(network.getNetworkHandle()));
        }

        @Override
        public void onCapabilitiesChanged(Network network, NetworkCapabilities networkCapabilities) {
            Log.d(TAG, "Network " + ssid + " capabilities changed to " + networkCapabilities.toString());
        }

        @Override
        public void onUnavailable() {
            Log.i(TAG, "Network unavailable: " + ssid);
            result.accept(false, ssid, Optional.empty());
        }

        public String getSsid() {
            return ssid;
        }
    }
}
