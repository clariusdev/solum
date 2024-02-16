package me.clarius.sdk.solum.example;

import android.content.Context;
import android.graphics.Bitmap;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModelProvider;
import androidx.navigation.fragment.NavHostFragment;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.Optional;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import me.clarius.sdk.Button;
import me.clarius.sdk.Config;
import me.clarius.sdk.Connection;
import me.clarius.sdk.ImagingState;
import me.clarius.sdk.LogLevel;
import me.clarius.sdk.Mode;
import me.clarius.sdk.Param;
import me.clarius.sdk.Platform;
import me.clarius.sdk.PosInfo;
import me.clarius.sdk.PowerDown;
import me.clarius.sdk.ProbeReset;
import me.clarius.sdk.ProbeSettings;
import me.clarius.sdk.ProcessedImageInfo;
import me.clarius.sdk.RawImageInfo;
import me.clarius.sdk.Solum;
import me.clarius.sdk.SpectralImageInfo;
import me.clarius.sdk.WifiOpt;
import me.clarius.sdk.solum.example.databinding.FragmentFirstBinding;

public class FirstFragment extends Fragment {

    private static final String TAG = "Solum";
    private final ExecutorService executorService = Executors.newFixedThreadPool(1);
    private WifiAutoJoin wifiAutoJoin;
    private FragmentFirstBinding binding;
    private Solum solum;
    private boolean isRunning = false;
    private boolean isBuffering = false;
    private ViewModel viewModel;
    private ImageConverter imageConverter;
    private final Solum.Listener solumListener = new Solum.Listener() {
        @Override
        public void error(String msg) {
            showError(msg);
        }

        @Override
        public void connectionResult(Connection result, int port, String status) {
            Log.d(TAG, "Connection result: " + result + ", port: " + port + ", status: " + status);
            if (result == Connection.ProbeConnected) {
                showMessage("Connected");
            } else if (result == Connection.SwUpdateRequired) {
                showMessage("Firmware update needed");
            } else {
                showMessage("Disconnected");
            }
        }

        @Override
        public void certInfo(int daysValid) {
            showMessage("Days valid for cert: " + daysValid);
        }

        @Override
        public void imaging(ImagingState state, boolean imaging) {
            showMessage("Imaging state: " + state + " imaging? " + imaging);
            isRunning = imaging;
        }

        @Override
        public void newProcessedImage(ByteBuffer buffer, ProcessedImageInfo info, PosInfo[] pos) {
            imageConverter.convertImage(buffer, info);
        }

        @Override
        public void newRawImageFn(ByteBuffer buffer, RawImageInfo info, PosInfo[] pos) {
        }

        @Override
        public void newSpectralImageFn(ByteBuffer buffer, SpectralImageInfo info) {
        }

        @Override
        public void newImuDataFn(PosInfo[] pos) {
        }

        @Override
        public void poweringDown(PowerDown reason, int seconds) {
            Log.d(TAG, "Powering down in " + seconds + " seconds (reason: " + reason + ")");
        }

        @Override
        public void buttonPressed(Button button, int count) {
            Log.d(TAG, "Button '" + button + "' pressed, count: " + count);
        }
    };
    private CertificatesManager certificatesManager;
    private FirmwareManager firmwareManager;
    private String currentProbeSerial;
    private String oemKey;

    private static Optional<Long> maybeLong(CharSequence from) throws RuntimeException {
        if (null == from || 0 == from.length()) return Optional.empty();
        return Optional.of(Long.parseLong(String.valueOf(from)));
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getParentFragmentManager().setFragmentResultListener("probe_info", this, (requestKey, result) -> {
            Log.d(TAG, "Received bluetooth info");
            parseInfoFromBluetooth(result);

        });
    }

    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        binding = FragmentFirstBinding.inflate(inflater, container, false);
        return binding.getRoot();
    }

    private String getCertDir() {
        return requireContext().getDir("cert", Context.MODE_PRIVATE).toString();
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        wifiAutoJoin = new WifiAutoJoin(new WifiAutoJoin.Listener() {
            @Override
            public void onSuccess(final String ssid, final Long networkID) {
                showMessage("Joined Wi-Fi " + ssid);
                binding.networkId.setText(String.valueOf(networkID));
            }

            public void onFailure(final String ssid) {
                showError("Failed to join Wi-Fi " + ssid);
            }
        });
        certificatesManager = new CertificatesManager(requireActivity(), new CertificatesManager.Listener() {
            @Override
            public void certsDownloaded() {
                showMessage("Certificates downloaded successfully");
            }

            @Override
            public void error(String message) {
                showError(message);
            }
        });

        firmwareManager = new FirmwareManager(requireActivity(), new FirmwareManager.Listener() {
            @Override
            public void firmwareDownloaded(String firmwarePath) {
                showMessage("Firmware downloaded successfully");
                viewModel.setFirmwarePath(firmwarePath);
            }

            @Override
            public void error(String message) {
                showError(message);
            }
        });

        SolumViewModel solumViewModel = new ViewModelProvider(this).get(SolumViewModel.class);
        solumViewModel.getProcessedImage().observe(getViewLifecycleOwner(), binding.imageView::setImageBitmap);

        Config config = new Config(LogLevel.Info);

        solum = new Solum(requireContext().getApplicationInfo().nativeLibraryDir, config, solumListener);
        solum.initialize(getCertDir(), connected -> {
            Log.d(TAG, "Initialization result: " + connected);
            if (connected) {
                try {
                    solum.setCertificate(certificatesManager.getCertificate(currentProbeSerial));
                } catch (RuntimeException e) {
                    showError(e.getMessage());
                }
                solum.getFirmwareVersion(Platform.HD, maybeVersion -> showMessage("Retrieved FW version: " + maybeVersion.orElse("???")));
                viewModel.refreshProbes(solum);
            }
        });
        solum.setProbeSettings(new ProbeSettings());

        viewModel = new ViewModelProvider(requireActivity()).get(ViewModel.class);
        viewModel.getSelectedProbe().observe(getViewLifecycleOwner(), currentProbe -> viewModel.refreshApplications(solum, currentProbe));

        imageConverter = new ImageConverter(executorService, new ImageCallback(solumViewModel.getProcessedImage()));

        binding.buttonBluetooth.setOnClickListener(view1 -> NavHostFragment.findNavController(FirstFragment.this).navigate(R.id.action_FirstFragment_to_BluetoothFragment));

        binding.buttonConnect.setOnClickListener(v -> doConnect());
        binding.buttonDisconnect.setOnClickListener(v -> solum.disconnect());
        binding.buttonRun.setOnClickListener(v -> toggleRun());

        binding.buttonSwUpdate.setOnClickListener(v -> doSwUpdate());
        binding.buttonAskState.setOnClickListener(v -> doAskState());
        binding.buttonFactoryReset.setOnClickListener(v -> doFactoryReset());
        binding.buttonOptimizeWifi.setOnClickListener(v -> doOptimizeWifi());
        binding.buttonPowerDown.setOnClickListener(v -> solum.powerDown());
        binding.buttonLoadApplication.setOnClickListener(v -> doLoadApplication());
        binding.buttonToggleRawDataBuffering.setOnClickListener(v -> toggleBuffering());
        binding.buttonGetRawData.setOnClickListener(v -> doRequestRawData());

        binding.buttonWifiAutoJoin.setOnClickListener(v -> doWifiAutoJoin());


        Secrets.maybeSSID().ifPresent(s -> binding.wifiSsid.setText(s));
        Secrets.maybePassphrase().ifPresent(s -> binding.wifiPassphrase.setText(s));
        Secrets.maybeMacAddress().ifPresent(s -> binding.macAddress.setText(s));
        Optional<String> maybeOemKey = Secrets.maybeOemApiKey();
        if (maybeOemKey.isPresent()) {
            oemKey = maybeOemKey.get();
            binding.buttonDownloadFirmware.setOnClickListener(v -> downloadFirmware());
            binding.buttonDownloadCertificates.setOnClickListener(view1 -> certificatesManager.downloadCertificates(oemKey));
        } else {
            View.OnClickListener listener = v -> showError("No OEM-API-Key provided; cannot download the certificates or firmware");
            binding.buttonDownloadFirmware.setOnClickListener(listener);
            binding.buttonDownloadCertificates.setOnClickListener(listener);
        }
    }

    private void doSwUpdate() {
        Optional<Platform> maybePlatform = Optional.ofNullable(viewModel.getSelectedPlatformVersion().getValue());
        if (!maybePlatform.isPresent()) {
            showError("No platform provided; cannot download the firmware");
            return;
        }
        String firmwarePath = viewModel.getFirmwarePath().getValue();
        solum.updateSoftware(firmwarePath, result -> showMessage("SW update result: " + result), (progress, total) -> {
            binding.swUpdateProgressBar.setMax(total);
            binding.swUpdateProgressBar.setProgress(progress);
        }, maybePlatform);
    }

    private void downloadFirmware() {
        Platform maybePlatform = viewModel.getSelectedPlatformVersion().getValue();
        if (maybePlatform == null) {
            showError("No platform provided; cannot download the firmware");
            return;
        }
        solum.getFirmwareVersion(maybePlatform, result -> {
            if (!result.isPresent()) {
                showError("No firmware version found; cannot download the firmware");
                return;
            }
            firmwareManager.downloadFW(oemKey, result.get());
        });

    }

    private void doRequestRawData() {
        final int start = 0;
        final int end = 0;
        final boolean lzo = true;
        RawDataCallback callback = new RawDataCallback();
        solum.requestRawData(start, end, lzo, callback::requestResult);
    }

    private void toggleBuffering() {
        boolean newState = !isBuffering;
        showMessage("Toggling buffering to: " + newState);
        isBuffering = newState;
        solum.setParam(Param.RawBuffer, newState ? 1 : 0);
    }

    private void toggleRun() {
        boolean newState = !isRunning;
        showMessage("Toggling run to: " + newState);
        isRunning = newState;
        solum.run(newState);
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        solum.disconnect();
        solum.release();
        solum = null;
        binding = null;
    }

    private void doConnect() {
        binding.ipAddressLayout.setError(null);
        binding.tcpPortLayout.setError(null);
        String ipAddress = String.valueOf(binding.ipAddress.getText());
        if (ipAddress.isEmpty()) {
            binding.ipAddressLayout.setError("Cannot be empty");
            return;
        }
        int tcpPort;
        try {
            tcpPort = Integer.parseInt(String.valueOf(binding.tcpPort.getText()));
        } catch (RuntimeException e) {
            binding.tcpPortLayout.setError("Invalid number");
            return;
        }
        Optional<Long> maybeNetworkId;
        try {
            maybeNetworkId = maybeLong(binding.networkId.getText());
        } catch (RuntimeException e) {
            binding.networkIdLayout.setError("Invalid number");
            return;
        }

        showMessage("Connecting to " + ipAddress + ":" + tcpPort);
        solum.connect(ipAddress, tcpPort, maybeNetworkId);
    }

    private void doLoadApplication() {
        String currentProbe = viewModel.getSelectedProbe().getValue();
        if (currentProbe == null || currentProbe.isEmpty()) {
            showError("No probe selected");
            return;
        }
        String currentApplication = viewModel.getSelectedApplication().getValue();
        if (currentApplication == null || currentApplication.isEmpty()) {
            showError("No application selected");
            return;
        }
        showMessage("Loading application '" + currentApplication + "' for probe " + currentProbe);
        solum.loadApplication(currentProbe, currentApplication);
    }

    private void doAskState() {
        showMessage("Printing state in logcat");
        solum.getParam(Param.Gain, result -> Log.d(TAG, "Gain: " + result.map(Object::toString).orElse("<none>")));
        solum.getParam(Param.ImageDepth, result -> Log.d(TAG, "Depth: " + result.map(Object::toString).orElse("<none>")));
        solum.getMode(result -> Log.d(TAG, "Mode: " + result.map(Mode::toString).orElse("<none>")));
        solum.getTgc(result -> Log.d(TAG, "TGC: " + result.map(Strings::fromTgc).orElse("<none>")));
        solum.getRoi(6, result -> Log.d(TAG, "ROI: " + Strings.fromPoints(result)));
        solum.getStatus(result -> Log.d(TAG, "Status: " + result.map(Strings::fromStatusInfo).orElse("<none>")));
        solum.getProbeInfo(result -> Log.d(TAG, "Probe Info: " + result.map(Strings::fromProbeInfo).orElse("<none>")));
        solum.getRange(Param.DynamicRange, result -> Log.d(TAG, "Dynamic Range: " + result.map(Strings::fromRange).orElse("<none>")));
    }

    private void doFactoryReset() {
        solum.resetProbe(ProbeReset.ResetFactory, success -> {
            if (success) showMessage("Factory reset successful");
            else showError("Factory reset failed");
        });
    }

    private void doOptimizeWifi() {
        solum.optimizeWifi(WifiOpt.WifiOptSearch, success -> {
            if (success) showMessage("Wifi optimization successful");
            else showError("Wifi optimization failed");
        });
    }

    private void doWifiAutoJoin() {
        binding.wifiSsidLayout.setError(null);
        binding.wifiPassphraseLayout.setError(null);
        binding.networkId.setText(null);
        final String ssid = String.valueOf(binding.wifiSsid.getText());
        if (ssid.isEmpty()) {
            binding.wifiSsid.setError("Cannot be empty");
            return;
        }
        final String passphrase = String.valueOf(binding.wifiPassphrase.getText());
        if (passphrase.isEmpty()) {
            binding.wifiPassphrase.setError("Cannot be empty");
            return;
        }
        final String macAddress = String.valueOf(binding.macAddress.getText());
        showMessage("Auto-joining Wi-Fi " + ssid);
        wifiAutoJoin.join(requireContext(), ssid, passphrase, macAddress);
    }

    private void showMessage(CharSequence text) {
        Log.d(TAG, (String) text);
        Handler mainHandler = new Handler(Looper.getMainLooper());
        mainHandler.post(() -> Toast.makeText(requireContext(), text, Toast.LENGTH_SHORT).show());
    }

    private void showError(CharSequence text) {
        Log.e(TAG, "Error: " + text);
        Handler mainHandler = new Handler(Looper.getMainLooper());
        mainHandler.post(() -> Toast.makeText(requireContext(), text, Toast.LENGTH_SHORT).show());
    }

    private void parseInfoFromBluetooth(Bundle fields) {
        if (fields.containsKey("wifi_ssid"))
            binding.wifiSsid.setText(fields.getString("wifi_ssid"));
        if (fields.containsKey("wifi_passphrase"))
            binding.wifiPassphrase.setText(fields.getString("wifi_passphrase"));
        if (fields.containsKey("ip_address"))
            binding.ipAddress.setText(fields.getString("ip_address"));
        if (fields.containsKey("tcp_port"))
            binding.tcpPort.setText(String.valueOf(fields.getInt("tcp_port")));
        if (fields.containsKey("mac_address"))
            binding.macAddress.setText(String.valueOf(fields.getString("mac_address")));
        if (fields.containsKey("serial")) {
            currentProbeSerial = String.valueOf(fields.getString("serial"));
            viewModel.setSelectedPlatformFromSerial(currentProbeSerial);
        }
    }

    private class RawDataCallback {
        void requestResult(int result) {
            Log.d(TAG, "Raw data request: " + result);
            if (result < 0) {
                showError("Failed to request raw data (ensure buffering is enabled and probe is frozen)");
            } else if (result == 0) {
                showError("No raw data available");
            } else {
                solum.readRawData(this::retrieved, this::progress);
            }
        }

        void retrieved(int result, ByteBuffer data) {
            if (result > 0) {
                try {
                    showMessage("Saving raw data");
                    Uri uri = IOUtils.saveInDocuments(data, "solum_raw_data", requireContext());
                    showMessage("Saved raw data in file " + uri);
                } catch (IOException e) {
                    showError(e.toString());
                }
            } else {
                showError("Could not retrieve raw data");
            }
        }

        void progress(int progress, int total) {
            binding.rawDataProgressBar.setMax(total);
            binding.rawDataProgressBar.setProgress(progress);
        }
    }

    /**
     * "Glue class" between the conversion thread and display thread
     */
    private class ImageCallback implements ImageConverter.Callback {
        private final MutableLiveData<Bitmap> dest;

        ImageCallback(MutableLiveData<Bitmap> dest) {
            this.dest = dest;
        }

        @Override
        public void onResult(Bitmap bitmap) {
            dest.postValue(bitmap);
        }

        @Override
        public void onError(Exception e) {
            showError("Error while converting image: " + e);
        }
    }
}
