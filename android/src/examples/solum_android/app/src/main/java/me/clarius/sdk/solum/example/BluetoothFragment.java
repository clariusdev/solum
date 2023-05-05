package me.clarius.sdk.solum.example;

import android.bluetooth.le.ScanResult;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.databinding.ObservableBoolean;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.ViewModelProvider;
import androidx.navigation.fragment.NavHostFragment;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.google.android.material.snackbar.Snackbar;

import java.util.List;
import java.util.Optional;

import me.clarius.sdk.solum.example.bluetooth.BluetoothException;
import me.clarius.sdk.solum.example.bluetooth.GattService;
import me.clarius.sdk.solum.example.bluetooth.PermissionHelper;
import me.clarius.sdk.solum.example.bluetooth.Probe;
import me.clarius.sdk.solum.example.bluetooth.ProbeStatus;
import me.clarius.sdk.solum.example.bluetooth.ProbeWifi;
import me.clarius.sdk.solum.example.bluetooth.ScanService;
import me.clarius.sdk.solum.example.databinding.FragmentBluetoothBinding;

public class BluetoothFragment extends Fragment implements PermissionHelper.Result {
    private static final String TAG = "BLE";
    private final ObservableBoolean scanning = new ObservableBoolean(false);
    private final ObservableBoolean probeConnected = new ObservableBoolean(false);
    private final ObservableBoolean probeReady = new ObservableBoolean(false);
    private final ObservableBoolean hasPermission = new ObservableBoolean(false);
    private final ObservableBoolean hasBle = new ObservableBoolean(false);

    private final Handler handler = new Handler(Looper.getMainLooper()); // post ui updates on ui thread
    private PermissionHelper permissionHelper;
    private BluetoothViewModel bluetoothViewModel;
    private final ScanService.Listener scanServiceListener = new ScanService.Listener() {
        @Override
        public void newProbe(String deviceName, ScanResult result) {
            bluetoothViewModel.addOrUpdateProbe(deviceName, probe -> {
                probe.updateBleAddress(result.getDevice().getAddress());
                probe.updateRssi(result.getRssi());
                ProbeStatus probeStatus = ProbeStatus.fromScanRecord(result.getScanRecord());
                if (null != probeStatus) {
                    Log.d(TAG, "Manufacturer data: " + probeStatus);
                    probe.updateStatus(probeStatus);
                }
            });
        }

        @Override
        public void failed(int errorCode) {
            showError("Scan failed with code " + errorCode);
        }

        @Override
        public void started() {
            scanning.set(true);
        }

        @Override
        public void finished(List<String> found) {
            scanning.set(false);
        }
    };
    private final GattService.Listener gattServiceListener = new GattService.Listener() {
        @Override
        public void connected(String deviceNameIgnored) {
            handler.post(() -> probeConnected.set(true));
        }

        @Override
        public void disconnected() {
            handler.post(() -> {
                probeConnected.set(false);
                probeReady.set(false);
            });
        }

        @Override
        public void ready(String deviceNameIgnored) {
            handler.post(() -> probeReady.set(true));
        }

        @Override
        public void powerChanged(String deviceName, boolean powered) {
            handler.post(() -> bluetoothViewModel.addOrUpdateProbe(deviceName, probe -> probe.updatePowered(powered)));
        }

        @Override
        public void wifiChanged(String deviceNameIgnored, ProbeWifi wifi) {
            if (null == wifi) return;
            handler.post(() -> bluetoothViewModel.updateProbeWifi(wifi));
        }

        @Override
        public void error(String error) {
            showError(error);
        }
    };
    private BluetoothViewAdapter bluetoothViewAdapter;
    private GattService gattService;
    private final ServiceConnection gattServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            gattService = ((GattService.CustomBinder) service).getService();
            assert gattService != null;
            gattService.initialize(gattServiceListener);
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            gattService = null;
        }
    };
    private ScanService scanService;
    private final ServiceConnection scanServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            scanService = ((ScanService.CustomBinder) service).getService();
            assert scanService != null;
            scanService.initialize(scanServiceListener);
            startBleScan();
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            gattService = null;
        }
    };

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (requireActivity().getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE)) {
            requireContext().bindService(new Intent(requireContext(), ScanService.class), scanServiceConnection, Context.BIND_AUTO_CREATE);
            requireContext().bindService(new Intent(requireContext(), GattService.class), gattServiceConnection, Context.BIND_AUTO_CREATE);
            hasBle.set(true);
        } else {
            Log.e(TAG, "No BLE");
            hasBle.set(false);
        }
    }

    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        bluetoothViewModel = new ViewModelProvider(requireActivity()).get(BluetoothViewModel.class);
        bluetoothViewAdapter = new BluetoothViewAdapter(bluetoothViewModel.getProbes());
        permissionHelper = new PermissionHelper(requireActivity().getActivityResultRegistry(), requireContext(), this);
        getLifecycle().addObserver(permissionHelper);

        hasPermission.set(checkSelfPermission());

        FragmentBluetoothBinding binding = FragmentBluetoothBinding.inflate(inflater, container, false);

        binding.setScanning(scanning);
        binding.setProbeConnected(probeConnected);
        binding.setProbeReady(probeReady);
        binding.setHasPermission(hasPermission);
        binding.setHasBle(hasBle);

        binding.permissionsButton.setOnClickListener(view -> permissionHelper.requestPermissions(requireContext()));
        binding.rescanButton.setOnClickListener(v -> startBleScan());
        binding.selectButton.setOnClickListener(v -> useSelectedProbe());
        binding.connectBluetoothButton.setOnClickListener(v -> connectToSelectedProbe());
        binding.disconnectBluetoothButton.setOnClickListener(v -> disconnectProbe());
        binding.sendAlertButton.setOnClickListener(v -> sendAlert());
        binding.powerOnButton.setOnClickListener(v -> powerOn(true));
        binding.powerOffButton.setOnClickListener(v -> powerOn(false));

        bluetoothViewModel.getProbeWifi().observe(getViewLifecycleOwner(),
                wifi -> {
                    binding.wifiSsidValue.setText(wifi.ssid);
                    binding.wifiPassphraseValue.setText(wifi.passphrase);
                    binding.ipAddressValue.setText(wifi.ip);
                    binding.tcpPortValue.setText(String.valueOf(wifi.port));
                    binding.macAddressValue.setText(wifi.mac);
                });

        RecyclerView recyclerView = binding.getRoot().findViewById(R.id.recycler_view);
        recyclerView.setAdapter(bluetoothViewAdapter);
        recyclerView.setLayoutManager(new LinearLayoutManager(getContext()));

        return binding.getRoot();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        requireContext().unbindService(scanServiceConnection);
        requireContext().unbindService(gattServiceConnection);
    }

    private void startBleScan() {
        if (null != scanService && checkSelfPermission()) {
            scanService.startScan();
        }
    }

    private void useSelectedProbe() {
        Bundle result = bluetoothViewModel.makeProbeInfoBundle();
        getParentFragmentManager().setFragmentResult("probe_info", result);
        NavHostFragment.findNavController(BluetoothFragment.this).navigate(R.id.action_BluetoothFragment_to_FirstFragment);
    }

    private void connectToSelectedProbe() {
        try {
            if (null == gattService) throw new BluetoothException("No gatt service");
            Probe probe = bluetoothViewAdapter.getSelectedProbe();
            if (null == probe) throw new BluetoothException("No probe selected");
            Optional<String> address = probe.getBleAddress();
            if (!address.isPresent()) throw new BluetoothException("Missing ble address");
            if (!checkSelfPermission()) throw new BluetoothException("Missing permission");
            gattService.connect(address.get());
        } catch (BluetoothException e) {
            Log.e(TAG, e.toString());
            showError(e.toString());
        }
    }

    private void sendAlert() {
        try {
            if (!probeConnected.get()) throw new BluetoothException("No probe connected");
            if (!checkSelfPermission()) throw new BluetoothException("Missing permission");
            gattService.sendAlert(false);
        } catch (BluetoothException e) {
            Log.e(TAG, e.toString());
            showError(e.toString());
        }
    }

    private void disconnectProbe() {
        try {
            if (!probeConnected.get()) throw new BluetoothException("No probe connected");
            if (!checkSelfPermission()) throw new BluetoothException("Missing permission");
            gattService.disconnect();
        } catch (BluetoothException e) {
            Log.e(TAG, e.toString());
            showError(e.toString());
        }
    }

    private void powerOn(boolean on) {
        try {
            if (!probeConnected.get()) throw new BluetoothException("No probe connected");
            if (!checkSelfPermission()) throw new BluetoothException("Missing permission");
            gattService.powerOn(on);
        } catch (BluetoothException e) {
            Log.e(TAG, e.toString());
            showError(e.toString());
        }
    }

    private void showError(String error) {
        Snackbar.make(requireView(), error, Snackbar.LENGTH_LONG).show();
    }

    private boolean checkSelfPermission() {
        return PermissionHelper.checkSelfPermission(requireContext());
    }

    @Override
    public void onRequestPermissionsResult(boolean granted) {
        hasPermission.set(granted);
        if (granted) startBleScan();
    }
}
