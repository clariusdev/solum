package me.clarius.sdk.solum.example.bluetooth;

import android.os.Bundle;

import java.util.Optional;

public class Probe {
    private final String deviceName;
    private String address;
    private Integer rssi;
    private ProbeWifi wifi;
    private Integer battery;
    private Integer temperature;
    private ProbeStatus.Availability availability;
    private ProbeStatus.ListenPolicy listenPolicy;
    private ProbeStatus.ChargingStatus chargingStatus;
    private Boolean powered;

    public Probe(final String deviceName) {
        this.deviceName = deviceName;
    }

    public void updateRssi(final int rssi) {
        this.rssi = rssi;
    }

    public void updateStatus(final ProbeStatus status) {
        this.battery = status.battery;
        this.temperature = status.temperature;
        this.availability = status.availability;
        this.listenPolicy = status.listenPolicy;
        this.chargingStatus = status.chargingStatus;
        this.powered = status.powered;
    }

    public void updateWifi(final ProbeWifi wifi) {
        this.wifi = wifi;
    }

    public void updatePowered(boolean powered) {
        this.powered = powered;
    }

    public void updateAddress(final String address) {
        this.address = address;
    }

    public String getDeviceName() {
        return deviceName;
    }

    public String getSerial() {
        return deviceName.substring(4);
    }

    public Optional<String> getAddress() {
        return Optional.ofNullable(address);
    }

    public Optional<Integer> getRssi() {
        return Optional.ofNullable(rssi);
    }

    public Optional<Integer> getBattery() {
        return Optional.of(battery);
    }

    public Optional<Integer> getTemperature() {
        return Optional.of(temperature);
    }

    public Optional<ProbeStatus.Availability> getAvailability() {
        return Optional.of(availability);
    }

    public Optional<ProbeStatus.ListenPolicy> getListenPolicy() {
        return Optional.of(listenPolicy);
    }

    public Optional<ProbeStatus.ChargingStatus> getChargingStatus() {
        return Optional.of(chargingStatus);
    }

    public Optional<Boolean> getPowered() {
        return Optional.of(powered);
    }

    public Optional<String> getWifiSsid() {
        return null != wifi ? Optional.ofNullable(wifi.ssid) : Optional.empty();
    }

    public Optional<String> getWifiPassphrase() {
        return null != wifi ? Optional.ofNullable(wifi.passphrase) : Optional.empty();
    }

    public Optional<String> getIpAddress() {
        return null != wifi ? Optional.ofNullable(wifi.ip) : Optional.empty();
    }

    public Optional<Integer> getTcpPort() {
        return null != wifi ? Optional.ofNullable(wifi.port) : Optional.empty();
    }

    public Bundle makeBundle() {
        Bundle fields = new Bundle();
        fields.putString("device_name", getDeviceName());
        fields.putString("serial", getSerial());
        if (null != battery) fields.putInt("battery", battery);
        if (null != temperature) fields.putInt("temperature", temperature);
        if (null != availability) fields.putSerializable("availability", availability);
        if (null != listenPolicy) fields.putSerializable("listenPolicy", listenPolicy);
        if (null != chargingStatus) fields.putSerializable("chargingStatus", chargingStatus);
        if (null != powered) fields.putBoolean("powered", powered);
        if (null != wifi) {
            if (null != wifi.ssid) fields.putString("wifi_ssid", wifi.ssid);
            if (null != wifi.passphrase) fields.putString("wifi_password", wifi.passphrase);
            if (null != wifi.ip) fields.putString("ip_address", wifi.ip);
            if (null != wifi.port) fields.putInt("tcp_port", wifi.port);
            if (null != wifi.mac) fields.putString("mac_address", wifi.mac);
        }
        return fields;
    }
}
