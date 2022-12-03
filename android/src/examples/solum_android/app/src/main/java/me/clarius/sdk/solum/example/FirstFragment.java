package me.clarius.sdk.solum.example;

import android.content.Context;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModelProvider;
import androidx.navigation.fragment.NavHostFragment;

import java.nio.ByteBuffer;
import java.util.Optional;
import java.util.StringJoiner;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.function.Consumer;

import me.clarius.sdk.Button;
import me.clarius.sdk.Connection;
import me.clarius.sdk.ImagingState;
import me.clarius.sdk.Mode;
import me.clarius.sdk.Param;
import me.clarius.sdk.Platform;
import me.clarius.sdk.PointF;
import me.clarius.sdk.PosInfo;
import me.clarius.sdk.PowerDown;
import me.clarius.sdk.ProbeInfo;
import me.clarius.sdk.ProbeSettings;
import me.clarius.sdk.ProcessedImageInfo;
import me.clarius.sdk.Range;
import me.clarius.sdk.RawImageInfo;
import me.clarius.sdk.Solum;
import me.clarius.sdk.SpectralImageInfo;
import me.clarius.sdk.StatusInfo;
import me.clarius.sdk.SwUpdate;
import me.clarius.sdk.Tgc;
import me.clarius.sdk.solum.example.databinding.FragmentFirstBinding;

public class FirstFragment extends Fragment {

    private static final String TAG = "Solum";
    private final ExecutorService executorService = Executors.newFixedThreadPool(1);
    private FragmentFirstBinding binding;
    private Solum solum;
    private boolean isRunning = false;
    private boolean isBuffering = false;
    private SolumViewModel viewModel;
    private WorkflowViewModel workflowViewModel;
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
        public void poweringDown(PowerDown reason, int seconds) {
            Log.d(TAG, "Powering down in " + seconds + " seconds (reason: " + reason + ")");
        }

        @Override
        public void buttonPressed(Button button, int count) {
            Log.d(TAG, "Button '" + button + "' pressed, count: " + count);
        }
    };

    @Override
    public View onCreateView(
            @NonNull LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState
    ) {
        binding = FragmentFirstBinding.inflate(inflater, container, false);
        return binding.getRoot();
    }

    private String getCertDir() {
        return requireContext().getDir("cert", Context.MODE_PRIVATE).toString();
    }

    private String getCertificate() {
        return "certificate"; // get certificate from https://cloud.clarius.com/api/public/v0/devices/oem/
    }

    public void onViewCreated(@NonNull View view, Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        viewModel = new ViewModelProvider(this).get(SolumViewModel.class);
        viewModel.getProcessedImage().observe(getViewLifecycleOwner(), binding.imageView::setImageBitmap);

        solum = new Solum(requireContext(), solumListener);
        solum.initialize(getCertDir(), new Solum.InitializationResult() {
            @Override
            public void accept(boolean connected) {
                Log.d(TAG, "Initialization result: " + connected);
                if (connected) {
                    solum.setCertificate(getCertificate());
                    solum.getFirmwareVersion(Platform.HD,
                            maybeVersion -> showMessage("Retrieved FW version: " + maybeVersion.orElse("???")));
                    workflowViewModel.refreshProbes(solum);
                }
            }
        });
        solum.setProbeSettings(new ProbeSettings());

        workflowViewModel = new ViewModelProvider(requireActivity()).get(WorkflowViewModel.class);
        workflowViewModel.getSelectedProbe().observe(getViewLifecycleOwner(),
                currentProbe -> workflowViewModel.refreshApplications(solum, currentProbe));

        imageConverter = new ImageConverter(executorService, new ImageCallback(viewModel.getProcessedImage()));

        binding.buttonBluetooth.setOnClickListener(view1 -> NavHostFragment.findNavController(FirstFragment.this)
                .navigate(R.id.action_FirstFragment_to_BluetoothFragment));

        binding.buttonConnect.setOnClickListener(v -> doConnect());
        binding.buttonDisconnect.setOnClickListener(v -> doDisconnect());
        binding.buttonRun.setOnClickListener(v -> toggleRun());

        binding.buttonSwUpdate.setOnClickListener(v -> doSwUpdate());
        binding.buttonAskState.setOnClickListener(v -> doAskState());
        binding.buttonPowerDown.setOnClickListener(v -> solum.powerDown());
        binding.buttonLoadApplication.setOnClickListener(v -> doLoadApplication());
        binding.buttonToggleRawDataBuffering.setOnClickListener(v -> toggleBuffering());
        binding.buttonGetRawData.setOnClickListener(v -> doRequestRawData());
    }

    private void doSwUpdate() {
        solum.updateSoftware(
                result -> showMessage("SW update result: " + result),
                (progress, total) -> {
                    binding.progressBar.setMax(total);
                    binding.progressBar.setProgress(progress);
                });
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
            Log.d(TAG, "Raw data read: " + result);
            if (result < 0) {
                showError("Failed to read raw data (ensure buffering is enabled and probe is frozen)");
            } else if (result == 0) {
                showError("No raw data available");
            } else {
                showMessage("Raw data size: " + result + " bytes");
            }
        }

        void progress(int progress, int total) {
            binding.progressBar.setMax(total);
            binding.progressBar.setProgress(progress);
        }
    }

    private void doRequestRawData() {
        final int start = 0;
        final int end = 0;
        RawDataCallback callback = new RawDataCallback();
        solum.requestRawData(start, end, callback::requestResult);
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
        doDisconnect();
        solum.release();
        solum = null;
        binding = null;
    }

    private void doConnect() {
        if (solum == null) {
            showError("Solum not initialized");
            return;
        }
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
        showMessage("Connecting to " + ipAddress + ":" + tcpPort);
        solum.connect(ipAddress, tcpPort);
    }

    private void doDisconnect() {
        if (solum == null) {
            return;
        }
        solum.disconnect();
    }

    private void doLoadApplication() {
        String currentProbe = workflowViewModel.getSelectedProbe().getValue();
        if (currentProbe == null || currentProbe.isEmpty()) {
            showError("No probe selected");
            return;
        }
        String currentApplication = workflowViewModel.getSelectedApplication().getValue();
        if (currentApplication == null || currentApplication.isEmpty()) {
            showError("No application selected");
            return;
        }
        showMessage("Loading application '" + currentApplication + "' for probe " + currentProbe);
        solum.loadApplication(currentProbe, currentApplication);
    }

    private void doAskState() {
        if (solum == null) {
            return;
        }
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
