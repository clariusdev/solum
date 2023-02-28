package me.clarius.sdk.solum.example;

import android.Manifest;
import android.bluetooth.BluetoothDevice;
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
import androidx.annotation.RequiresPermission;
import androidx.databinding.ObservableBoolean;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.ViewModelProvider;
import androidx.navigation.fragment.NavHostFragment;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import java.util.List;
import java.util.Optional;
import java.util.function.Consumer;

import me.clarius.sdk.solum.example.bluetooth.GattService;
import me.clarius.sdk.solum.example.bluetooth.Probe;
import me.clarius.sdk.solum.example.bluetooth.ProbeStatus;
import me.clarius.sdk.solum.example.bluetooth.ProbeWifi;
import me.clarius.sdk.solum.example.bluetooth.ScanService;
import me.clarius.sdk.solum.example.databinding.FragmentBluetoothBinding;

public class BluetoothFragment extends Fragment {
    private static final String TAG = "BLE";
    private final ObservableBoolean scanning = new ObservableBoolean(false);
    private final ObservableBoolean probeConnected = new ObservableBoolean(false);
    private final ObservableBoolean probeReady = new ObservableBoolean(false);
    private final ObservableBoolean hasPermission = new ObservableBoolean(false);
    private final ObservableBoolean hasBle = new ObservableBoolean(false);

    private final Handler handler = new Handler(Looper.getMainLooper()); // post ui updates on ui thread
    private FragmentBluetoothBinding binding;
    private RecyclerView recyclerView;
    private PermissionHelper permissionHelper;
    private BluetoothViewModel bluetoothViewModel;
    private final ScanService.Listener scanServiceListener = new ScanService.Listener() {
        @Override
        public void newProbe(String deviceName, ScanResult result) {
            bluetoothViewModel.addOrUpdateProbe(deviceName, probe -> {
                probe.updateAddress(result.getDevice().getAddress());
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
        @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
        @Override
        public void connected(BluetoothDevice device) {
            handler.post(() -> probeConnected.set(true));
        }

        @Override
        public void disconnected() {
            handler.post(() -> {
                probeConnected.set(false);
                probeReady.set(false);
            });
        }

        @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
        @Override
        public void ready(BluetoothDevice device) {
            handler.post(() -> probeReady.set(true));
        }

        @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
        @Override
        public void powerChanged(BluetoothDevice device, boolean powered) {
            handler.post(() -> bluetoothViewModel.addOrUpdateProbe(device.getName(), probe -> probe.updatePowered(powered)));
        }

        @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
        @Override
        public void wifiChanged(BluetoothDevice device, ProbeWifi wifi) {
            if (null == wifi) return;
            handler.post(() -> {
                bluetoothViewModel.addOrUpdateProbe(device.getName(), probe -> probe.updateWifi(wifi));
                binding.wifiSsidValue.setText(wifi.ssid);
                binding.wifiPassphraseValue.setText(wifi.passphrase);
                binding.ipAddressValue.setText(wifi.ip);
                binding.tcpPortValue.setText(String.valueOf(wifi.port));
                binding.macAddressValue.setText(wifi.mac);
            });
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
            tryStartScan();
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
            hasPermission.set(checkPermissions());
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
        permissionHelper = new PermissionHelper(requireActivity().getActivityResultRegistry(), missingPermissions -> {
            if (missingPermissions.isEmpty()) {
                Log.d(TAG, "All permissions granted");
                hasPermission.set(true);
                scanService.startScan();
            } else {
                Log.w(TAG, "Cannot start scan because permissions are still missing: " + missingPermissions);
                hasPermission.set(false);
            }
        });
        getLifecycle().addObserver(permissionHelper);

        binding = FragmentBluetoothBinding.inflate(inflater, container, false);

        binding.setScanning(scanning);
        binding.setProbeConnected(probeConnected);
        binding.setProbeReady(probeReady);
        binding.setHasPermission(hasPermission);
        binding.setHasBle(hasBle);

        binding.permissionsButton.setOnClickListener(view -> {
            if (!PermissionHelper.missingPermissions(requireContext()).isEmpty()) {
                permissionHelper.requestPermissions();
            }
        });
        binding.rescanButton.setOnClickListener(v -> tryStartScan());
        binding.selectButton.setOnClickListener(v -> useSelectedProbe());
        binding.connectBluetoothButton.setOnClickListener(v -> connectToSelectedProbe());
        binding.disconnectBluetoothButton.setOnClickListener(v -> ifSelected(probe -> {
            gattService.disconnect();
        }));
        binding.sendAlertButton.setOnClickListener(v -> ifSelected(probe -> {
            gattService.sendAlert(false);
        }));
        binding.powerOnButton.setOnClickListener(v -> ifSelected(probe -> {
            gattService.powerOn(true);
        }));
        binding.powerOffButton.setOnClickListener(v -> ifSelected(probe -> {
            gattService.powerOn(false);
        }));

        recyclerView = binding.getRoot().findViewById(R.id.recycler_view);
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

    private void tryStartScan() {
        if (null != scanService && checkPermissions()) {
            scanService.startScan();
        }
    }

    private void useSelectedProbe() {
        ifSelected(probe -> {
            Log.d(TAG, "Selecting probe serial: " + probe.getSerial());
            Bundle result = probe.makeBundle();
            getParentFragmentManager().setFragmentResult("probe_info", result);
            NavHostFragment.findNavController(BluetoothFragment.this).navigate(R.id.action_BluetoothFragment_to_FirstFragment);
        });
    }

    private void connectToSelectedProbe() {
        ifSelected(probe -> {
            Optional<String> address = probe.getAddress();
            assert address.isPresent();
            gattService.connect(address.get());
        });
    }

    private void ifSelected(Consumer<Probe> consumer) {
        Probe probe = bluetoothViewAdapter.getSelectedProbe();
        if (null == probe) {
            Log.w(TAG, "No probe selected");
            return;
        }
        consumer.accept(probe);
    }

    private boolean checkPermissions() {
        List<String> missing = PermissionHelper.missingPermissions(requireContext());
        if (!missing.isEmpty()) {
            Log.w(TAG, "Missing permissions: " + missing);
            return false;
        } else {
            return true;
        }
    }
}
