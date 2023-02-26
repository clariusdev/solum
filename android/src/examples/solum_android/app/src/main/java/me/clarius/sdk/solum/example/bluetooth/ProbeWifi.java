package me.clarius.sdk.solum.example.bluetooth;

import androidx.annotation.NonNull;

import org.yaml.snakeyaml.Yaml;

import java.util.Map;

/**
 * Wi-Fi information from the bluetooth wi-fi information service
 * <p>
 * Some fields may be null if they were not provided by the probe.
 */

public class ProbeWifi {
    public final String ssid;
    public final String passphrase;
    public final String ip;
    public final Integer port;
    public final String mac;

    protected ProbeWifi(final String ssid, final String passphrase, final String ip, final Integer port, final String mac) {
        this.ssid = ssid;
        this.passphrase = passphrase;
        this.ip = ip;
        this.port = port;
        this.mac = mac;
    }

    /**
     * Parse the YAML data returned from reading the bluetooth wi-fi information service
     *
     * @param yaml the YAML read over bluetooth
     * @return extracted wi-fi info
     */
    static ProbeWifi parse(final String yaml) {
        final Yaml parser = new Yaml();
        final Map map = parser.load(yaml);

        if ("disabled".equals(map.get("state"))) return null;

        String ssid = (String) map.get("ssid");
        String password = (String) map.get("pw");
        String ipv4 = (String) map.get("ip4");
        String ipv6 = (String) map.get("ip6");
        Integer port = (Integer) map.get("ctl");
        String mac = (String) map.get("mac");
        String ip = null != ipv4 ? ipv4 : ipv6;
        return new ProbeWifi(ssid, password, ip, port, mac);
    }

    @NonNull
    public String toString() {
        StringBuilder builder = new StringBuilder();
        builder.append("SSID: ").append(ssid).append("\n");
        builder.append("passphrase: ").append(passphrase).append("\n");
        builder.append("IP: ").append(ip).append("\n");
        builder.append("TCP port: ").append(port).append("\n");
        builder.append("Mac: ").append(mac);
        return builder.toString();
    }
}
