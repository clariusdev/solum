package me.clarius.sdk.solum.example;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.MacAddress;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkRequest;
import android.net.wifi.WifiNetworkSpecifier;
import android.util.Log;

/**
 * Helper to auto-join a Wi-Fi network.
 */

public class WifiAutoJoin {
    private static final String TAG = "SolumAutoJoin";
    private final Listener listener;
    private NetworkCallback networkCallback;

    public WifiAutoJoin(Listener listener) {
        this.listener = listener;
    }

    /**
     * Join the specified Wi-Fi network.
     *
     * @param context    current context to get the network manager.
     * @param ssid       the network SSID, for example DIRECT-L738-B-1605-A0100.
     * @param passphrase the WPA2 passphrase.
     * @param macAddress is used to bypass user approval if the network was already joined before (optional).
     *                   https://developer.android.com/guide/topics/connectivity/wifi-bootstrap?#bypass-approval
     */
    public void join(Context context, String ssid, String passphrase, String macAddress) {
        final ConnectivityManager connectivityManager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        assert null != connectivityManager;
        if (networkCallback != null) {
            Log.i(TAG, "Removing existing connection request for " + networkCallback.getSsid());
            connectivityManager.unregisterNetworkCallback(networkCallback);
            networkCallback = null;
        }
        networkCallback = new NetworkCallback(ssid);
        WifiNetworkSpecifier.Builder specifierBuilder = new WifiNetworkSpecifier.Builder().setSsid(ssid).setWpa2Passphrase(passphrase);
        if (null != macAddress && !macAddress.isEmpty()) {
            specifierBuilder.setBssid(MacAddress.fromString(macAddress));
        }
        NetworkRequest request = new NetworkRequest.Builder().addTransportType(NetworkCapabilities.TRANSPORT_WIFI).addCapability(NetworkCapabilities.NET_CAPABILITY_NOT_RESTRICTED).removeCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET).setNetworkSpecifier(specifierBuilder.build()).build();
        connectivityManager.requestNetwork(request, networkCallback);
    }

    public interface Listener {
        void onSuccess(final String ssid, final Long networkID);
        void onFailure(final String ssid);
    }

    private final class NetworkCallback extends ConnectivityManager.NetworkCallback {
        private final String ssid;

        NetworkCallback(final String ssid) {
            Log.i(TAG, "Requesting connection to network: " + ssid);
            this.ssid = ssid;
        }

        @Override
        public void onAvailable(Network network) {
            Log.i(TAG, "Successfully connected to network: " + ssid + " with network ID: " + network.getNetworkHandle());
            listener.onSuccess(ssid, network.getNetworkHandle());
        }

        @Override
        public void onCapabilitiesChanged(Network network, NetworkCapabilities networkCapabilities) {
            Log.d(TAG, "Network " + ssid + " capabilities changed to " + networkCapabilities.toString());
        }

        @Override
        public void onUnavailable() {
            Log.i(TAG, "Network unavailable: " + ssid);
            listener.onFailure(ssid);
        }

        public String getSsid() {
            return ssid;
        }
    }
}
