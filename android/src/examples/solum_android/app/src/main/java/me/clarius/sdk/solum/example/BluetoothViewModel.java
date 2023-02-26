package me.clarius.sdk.solum.example;

import androidx.databinding.ObservableArrayList;
import androidx.databinding.ObservableList;
import androidx.lifecycle.ViewModel;

import java.util.List;
import java.util.function.Consumer;
import java.util.stream.IntStream;

import me.clarius.sdk.solum.example.bluetooth.Probe;

public class BluetoothViewModel extends ViewModel {
    private final ObservableList<Probe> probes = new ObservableArrayList<>();

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
}
