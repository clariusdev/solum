package me.clarius.sdk.solum.example;

import android.os.Bundle;

import androidx.databinding.ObservableArrayList;
import androidx.databinding.ObservableList;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;

import java.util.List;
import java.util.function.Consumer;
import java.util.stream.IntStream;

import me.clarius.sdk.solum.example.bluetooth.Probe;
import me.clarius.sdk.solum.example.bluetooth.ProbeWifi;

public class BluetoothViewModel extends ViewModel {
    private final ObservableList<Probe> probes = new ObservableArrayList<>();
    private final MutableLiveData<ProbeWifi> probeWifi = new MutableLiveData<>();

    private static int findIndex(List<Probe> devices, String deviceName) {
        return IntStream.range(0, devices.size()).filter(i -> devices.get(i).getDeviceName().equals(deviceName)).findFirst().orElse(-1);
    }

    public ObservableList<Probe> getProbes() {
        return probes;
    }

    public void addOrUpdateProbe(String deviceName, Consumer<Probe> updater) {
        int index = findIndex(probes, deviceName);
        if (index >= 0) {
            Probe device = probes.get(index);
            updater.accept(device);
            probes.set(index, device);
        } else {
            Probe device = new Probe(deviceName);
            updater.accept(device);
            probes.add(device);
        }
    }

    public void updateProbeWifi(ProbeWifi probeWifi) {
        this.probeWifi.postValue(probeWifi);
    }

    public MutableLiveData<ProbeWifi> getProbeWifi() {
        return probeWifi;
    }

    public Bundle makeProbeInfoBundle() {
        ProbeWifi wifi = probeWifi.getValue();
        if (null == wifi) return new Bundle();
        Bundle fields = new Bundle();
        fields.putString("wifi_ssid", wifi.ssid);
        fields.putString("wifi_passphrase", wifi.passphrase);
        fields.putString("ip_address", wifi.ip);
        fields.putInt("tcp_port", wifi.port);
        fields.putString("mac_address", wifi.mac);
        return fields;
    }
}
