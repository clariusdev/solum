package me.clarius.sdk.solum.example.bluetooth;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import android.os.Parcel;
import android.os.Parcelable;
import android.util.Log;

import androidx.annotation.NonNull;

import org.yaml.snakeyaml.Yaml;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Map;

/**
 * Represents the parcelable scanner information received over Bluetooth.
 *
 * @see android.os.Parcelable
 */
public class BluetoothProbeInfo implements Parcelable {
    public static final String TAG = "BluetoothProbeInfo";
    public static final String DEFAULT_IP_ADDRESS = "192.168.1.1";
    public static final String DEFAULT_PORT = "5000";

    /**
     * Parses BluetoothProbeInfo from Parcel.
     *
     * Required for {@link android.os.Bundle#getParcelable(String, Class)} or {@link android.os.Bundle#getParcelable(String)}
     */
    public static final Creator<BluetoothProbeInfo> CREATOR = new Creator<BluetoothProbeInfo>() {
        @Override
        public BluetoothProbeInfo createFromParcel(Parcel source) {
            return new BluetoothProbeInfo(source);
        }

        @Override
        public BluetoothProbeInfo[] newArray(int size) {
            return new BluetoothProbeInfo[size];
        }
    };

    private WifiInfo wifiInfo;
    private final String serial;

    /**
     * Constructs an object without Wi-Fi information.
     *
     * @param serial serial number of the scanner
     */
    public BluetoothProbeInfo(final String serial) {
        this.serial = serial;
        wifiInfo = new WifiInfo();
    }

    private BluetoothProbeInfo(final Parcel data) {
        ArrayList<String> stringData = data.createStringArrayList();
        assert stringData.size() == 5;
        serial = stringData.get(0);
        wifiInfo = new WifiInfo(stringData.get(1), stringData.get(2), stringData.get(3), stringData.get(4));
    }

    /**
     * Sets the Wi-Fi info obtained from reading the {@link me.clarius.sdk.solum.example.bluetooth.enums.ServiceType#WifiInformationService
     * Clarius Wi-Fi Information Service}
     *
     * @param wifiData a String retrieved from {@link android.bluetooth.BluetoothGattCallback#onCharacteristicRead(BluetoothGatt, BluetoothGattCharacteristic, byte[], int)
     * reading a characteristic}
     */
    public void setWifiInfo(final String wifiData) {
        wifiInfo = parseWifiData(wifiData);
    }


    private WifiInfo parseWifiData(final String wifiData) {
        final Yaml parser = new Yaml();
        final Map map = (Map) parser.load(wifiData);

        if ("disabled".equals((String) map.get("status"))) return new WifiInfo();

        String ssid = (String) map.get("ssid");
        String password = (String) map.get("pw");
        String ipv4 = (String) map.get("ip4");
        String ipv6 = (String) map.get("ip6");
        Integer port = (Integer) map.get("ctl");
        String stringPort = port == null ? DEFAULT_PORT : port.toString();
        String ip = ipv4 == null ? (ipv6 == null ? DEFAULT_IP_ADDRESS : ipv6) : ipv4;
        return new WifiInfo(ssid, password, ip, stringPort);
    }

    public WifiInfo getWifiInfo() {
        return wifiInfo;
    }

    public String getSerial() {
        return serial;
    }

    /**
     * Checks if device name is a scanner serial.
     *
     * @param deviceName A string representing {@link BluetoothDevice#getName() the name of a bluetooth device}
     * @return serial number as a String or null
     */
    public static String serialFromStringOrNull(final String deviceName) {
        if (deviceName == null) return null;
        Log.v(TAG, deviceName);
        if (!deviceName.startsWith("CUS-")) return null;
        return deviceName.substring(4);
    }

    @Override
    public boolean equals(final Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        BluetoothProbeInfo that = (BluetoothProbeInfo) o;
        return equals(that.getSerial());
    }

    public boolean equals(final String other) {
        return this.serial.equals(other);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    /**
     * Flatten this object in to a Parcel.
     *
     * @param dest  The Parcel in which the object should be written.
     * @param flags unused
     */
    @Override
    public void writeToParcel(@NonNull Parcel dest, int flags) {
        dest.writeStringList(Arrays.asList(
                serial, wifiInfo.getSsid(), wifiInfo.getPassword(), wifiInfo.getIp(),
                wifiInfo.getPort()
        ));

    }

    /**
     * Represents the scanner Wi-Fi information received over Bluetooth
     */
    public static class WifiInfo {
        private final String ssid;
        private final String password;
        private final String ip;
        private final String port;

        public WifiInfo() {
            this.ssid = null;
            this.password = null;
            this.ip = null;
            this.port = null;
        }

        public WifiInfo(final String ssid, final String password, final String ip, final String port) {
            this.ssid = ssid == null  ? "" : ssid;
            this.password = password == null ? "" : password;
            this.ip = ip;
            this.port = port;
        }

        public String getSsid() {
            return ssid;
        }

        public String getPassword() {
            return password;
        }

        public String getIp() {
            return ip;
        }

        public String getPort() {
            return port;
        }
    }
}
