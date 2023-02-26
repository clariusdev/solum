package me.clarius.sdk.solum.example;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.MacAddress;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkRequest;
import android.net.wifi.WifiNetworkSpecifier;
import android.util.Log;

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
     */
    public void join(Context context, String ssid, String passphrase, String macAddress, Result result) {
        final ConnectivityManager connectivityManager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        assert null != connectivityManager;
        if (callback != null) {
            Log.i(TAG, "Removing existing connection request for " + callback.getSsid());
            connectivityManager.unregisterNetworkCallback(callback);
            callback = null;
        }
        callback = new MyCallback(ssid, result);
        WifiNetworkSpecifier.Builder specifierBuilder = new WifiNetworkSpecifier.Builder().setSsid(ssid).setWpa2Passphrase(passphrase);
        if (null != macAddress && !macAddress.isEmpty()) {
            specifierBuilder.setBssid(MacAddress.fromString(macAddress));
        }
        NetworkRequest request = new NetworkRequest.Builder().addTransportType(NetworkCapabilities.TRANSPORT_WIFI).addCapability(NetworkCapabilities.NET_CAPABILITY_NOT_RESTRICTED).removeCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET).setNetworkSpecifier(specifierBuilder.build()).build();
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
