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
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

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
    private SolumViewModel viewModel;
    private WorkflowViewModel workflowViewModel;
    private ImageConverter imageConverter;
    private final Solum.Listener solumListener = new Solum.Listener() {
        @Override
        public void error(String msg) {
            showError(msg);
        }

        @Override
        public void initializationResult(boolean result) {
            Log.d(TAG, "Initialization result: " + result);
            if (result) {
                solum.setCertificate(getCertificate());
                solum.getFirmwareVersion(Platform.HD, "asking FW version");
                workflowViewModel.refreshProbes(executorService, solum);
            }
        }

        @Override
        public void swUpdateResult(SwUpdate result) {
            Log.d(TAG, "SW update result: " + result);
        }

        @Override
        public void swUpdateProgress(int percent) {
            Log.d(TAG, "SW update progress: " + percent + "%");
        }

        @Override
        public void connectionResult(Connection result, int port, String status) {
            Log.d(TAG, "Connection result: " + result + ", port: " + port + ", status: " + status);
            if (result == Connection.ProbeConnected) {
                Log.d(TAG, "Connected");
            } else if (result == Connection.SwUpdateRequired) {
                Log.d(TAG, "Firmware update needed");
            } else {
                Log.d(TAG, "Disconnected");
            }
        }

        @Override
        public void certInfo(int daysValid) {
            Log.d(TAG, "Days valid for cert: " + daysValid);
        }

        @Override
        public void imaging(ImagingState state, boolean imaging) {
            Log.d(TAG, "Imaging state: " + state + " imaging? " + imaging);
            isRunning = imaging;
        }

        @Override
        public void newProcessedImage(ByteBuffer buffer, ProcessedImageInfo info, PosInfo[] pos) {
//            Log.d(TAG, "New processed image: " + Strings.fromProcessedImageInfo(info)
//                    + " capacity = " + buffer.capacity() + " bytes");
            imageConverter.convertImage(buffer, info);
        }

        @Override
        public void newRawImageFn(ByteBuffer buffer, RawImageInfo info, PosInfo[] pos) {
//            Log.d(TAG, "New raw image: " + Strings.fromRawImageInfo(info)
//                    + " capacity = " + buffer.capacity() + " bytes");
        }

        @Override
        public void newSpectralImageFn(ByteBuffer buffer, SpectralImageInfo info) {
            Log.d(TAG, "New raw image: " + Strings.fromSpectralImageInfo(info)
                    + " capacity = " + buffer.capacity() + " bytes");
        }

        @Override
        public void paramRetrieved(Optional<Double> maybeValue, Object user) {
            Param param = (Param) user;
            Log.d(TAG, "Retrieved param (" + param + "): " + maybeValue.orElse(-999.9));
        }

        @Override
        public void rangeRetrieved(Optional<Range> maybeRange, Object user) {
            Param param = (Param) user;
            String msg;
            msg = maybeRange.map(Strings::fromRange).orElse("<none>");
            Log.d(TAG, "Retrieved range (" + param + "): " + msg);
        }

        @Override
        public void tgcRetrieved(Optional<Tgc> maybeTgc, Object user) {
            String msg = (String) user;
            StringBuilder values = new StringBuilder();
            if (maybeTgc.isPresent()) {
                Tgc tgc = maybeTgc.get();
                values.append("top: ").append(tgc.top).append(", mid: ").append(tgc.mid).append(", bottom: ").append(tgc.bottom);
            }
            Log.d(TAG, "Retrieved TGC (" + msg + "): " + values);
        }

        @Override
        public void roiRetrieved(PointF[] points, Object user) {
            String msg = (String) user;
            StringBuilder values = new StringBuilder();
            if (points != null) {
                for (PointF p : points) {
                    values.append("[").append(p.x).append(",").append(p.y).append("]");
                }
            }
            Log.d(TAG, "Retrieved ROI (" + msg + "): " + values);
        }

        @Override
        public void modeRetrieved(Optional<Mode> mode, Object user) {
            String msg = (String) user;
            String text = mode.map(Mode::toString).orElse("???");
            Log.d(TAG, "Retrieved mode (" + msg + "): " + text);
        }

        @Override
        public void firmwareVersionRetrieved(Optional<String> maybeVersion, Object user) {
            String msg = (String) user;
            Log.d(TAG, "Retrieved FW version (" + msg + "): " + maybeVersion.orElse("???"));
        }

        @Override
        public void statusRetrieved(Optional<StatusInfo> maybeStatus, Object user) {
            String msg = (String) user;
            StringBuilder value = new StringBuilder();
            if (maybeStatus.isPresent()) {
                StatusInfo status = maybeStatus.get();
                value.append("batt: ").append(status.battery).append(" temp: ").append(status.temperature)
                        .append(" frameRate: ").append(status.frameRate)
                        .append(" fan: ").append(status.fan).append(" charing: ").append(status.charger);
            }
            Log.d(TAG, "Retrieved status (" + msg + "): " + value);
        }

        @Override
        public void probeInfoRetrieved(Optional<ProbeInfo> probeInfo, Object user) {
            String msg = (String) user;
            String text = probeInfo.map(Strings::fromProbeInfo).orElse("<none>");
            Log.d(TAG, "Probe info retrieved (" + msg + "): " + text);
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
        solum.initialize(getCertDir());
        solum.setProbeSettings(new ProbeSettings());

        workflowViewModel = new ViewModelProvider(requireActivity()).get(WorkflowViewModel.class);
        workflowViewModel.getSelectedProbe().observe(getViewLifecycleOwner(),
                currentProbe -> workflowViewModel.refreshApplications(executorService, solum, currentProbe));

        imageConverter = new ImageConverter(executorService, new ImageCallback(viewModel.getProcessedImage()));

        binding.buttonBluetooth.setOnClickListener(view1 -> NavHostFragment.findNavController(FirstFragment.this)
                .navigate(R.id.action_FirstFragment_to_BluetoothFragment));

        binding.buttonConnect.setOnClickListener(v -> doConnect());
        binding.buttonDisconnect.setOnClickListener(v -> doDisconnect());
        binding.buttonRun.setOnClickListener(v -> toggleRun());

        binding.buttonSwUpdate.setOnClickListener(v -> solum.updateSoftware());
        binding.buttonAskState.setOnClickListener(v -> doAskState());
        binding.buttonPowerDown.setOnClickListener(v -> solum.powerDown());
        binding.buttonLoadApplication.setOnClickListener(v -> doLoadApplication());
    }

    private void toggleRun() {
        boolean newState = !isRunning;
        Log.d(TAG, "Toggling run to: " + newState);
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
        Log.d("Jle", "Connecting to " + ipAddress + ":" + tcpPort);
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
        Log.d(TAG, "Loading application '" + currentApplication + "' for probe " + currentProbe);
        solum.loadApplication(currentProbe, currentApplication);
    }

    private void doAskState() {
        solum.getParam(Param.Gain, Param.Gain);
        solum.getParam(Param.ImageDepth, Param.ImageDepth);
        solum.getMode("asking mode");
        solum.getTgc("asking TGC values");
        solum.getRoi(6, "asking ROI");
        solum.getStatus("asking status");
        solum.getProbeInfo("asking probe info");
        solum.getRange(Param.DynamicRange, Param.DynamicRange);
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
