package me.clarius.sdk.solum.example.bluetooth;

import java.util.Optional;

public class Probe {
    private final String deviceName;
    private String bleAddress;
    private Integer rssi;
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

    public void updatePowered(boolean powered) {
        this.powered = powered;
    }

    public void updateBleAddress(final String bleAddress) {
        this.bleAddress = bleAddress;
    }

    public String getDeviceName() {
        return deviceName;
    }

    public String getSerial() {
        return deviceName.substring(4);
    }

    public Optional<String> getBleAddress() {
        return Optional.ofNullable(bleAddress);
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
}
