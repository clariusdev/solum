package me.clarius.sdk.solum.example.bluetooth;

import android.Manifest;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RequiresApi;
import androidx.annotation.RequiresPermission;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.ViewModelProvider;
import androidx.navigation.fragment.NavHostFragment;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import java.util.Objects;

import me.clarius.sdk.solum.example.R;
import me.clarius.sdk.solum.example.databinding.FragmentBluetoothBinding;
import me.clarius.sdk.solum.example.exceptions.BadApiLevelException;
import me.clarius.sdk.solum.example.helpers.MessageHelper;
import me.clarius.sdk.solum.example.helpers.PermissionHelper;
import me.clarius.sdk.solum.example.viewmodels.BluetoothViewModel;

public class BluetoothFragment extends Fragment {
    public static final String TAG = "BluetoothFragment";

    private MessageHelper messageHelper;
    private FragmentBluetoothBinding binding;
    private RecyclerView recyclerView;
    private PermissionHelper permissionHelper;
    private BluetoothViewModel bluetoothViewModel;

    public View onCreateView
    (
        @NonNull final LayoutInflater inflater,
        final ViewGroup container,
        final Bundle savedInstanceState
    )
    {
        binding = FragmentBluetoothBinding.inflate(inflater, container, false);
        bluetoothViewModel = new ViewModelProvider(requireActivity()).get(BluetoothViewModel.class);
        messageHelper = new MessageHelper(requireActivity(), TAG);
        permissionHelper = new PermissionHelper(
                requireActivity(),
                this::initScannerFinder
        );
        getLifecycle().addObserver(permissionHelper);

        return binding.getRoot();
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        if (requireActivity().getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE)) {
            if (checkPermissions()) {
                initScannerFinder();
            } else {
                permissionHelper.requestPermissions();
                addPermissionButton();
            }
        }
    }

    private void initScannerFinder() {
        bindBluetoothService();
        initRecyclerView();
        addRefreshButton();
    }

    private void bindBluetoothService() {
        Context context = requireContext();
        boolean wasBound = context.bindService(
                new Intent(context, BluetoothService.class),
                bluetoothViewModel.getBluetoothServiceConnection(),
                Context.BIND_AUTO_CREATE
        );
        if (wasBound) {
            Log.d(TAG, "BluetoothService was bound to this fragment");
        }
        else {
            Log.d(TAG, "BluetoothService was not bound");
        }
    }

    private boolean checkPermissions() {
        try {
            return permissionHelper.checkPermissions(requireContext());
        } catch (BadApiLevelException exception) {
            messageHelper.showError(exception.getMessage());
            exception.printStackTrace();
        }
        return false;
    }

    private void addRefreshButton() {
        binding.bluetoothFragmentButton.setText(R.string.refresh);
        binding.bluetoothFragmentButton.setOnClickListener(view ->
                ((BluetoothRecyclerViewAdapter) Objects.requireNonNull(recyclerView.getAdapter())).refresh());
    }

    private void addPermissionButton() {
        binding.bluetoothFragmentButton.setText(R.string.grant_permissions);
        binding.bluetoothFragmentButton.setOnClickListener(view -> {
            try {
                PermissionHelper.navigateToAppPermissionSettings(requireActivity());
            } catch (BadApiLevelException exception) {
                messageHelper.showError(exception.getMessage());
                exception.printStackTrace();
            }
            if (checkPermissions()) {
                initRecyclerView();
                addRefreshButton();
            }
        });
    }

    private void initRecyclerView() throws IllegalStateException {
        recyclerView = binding.getRoot().findViewById(R.id.recyclerView);
        assert recyclerView != null;
        recyclerView.setAdapter(new BluetoothRecyclerViewAdapter(
                bluetoothViewModel,
                getViewLifecycleOwner(),
                getContext(),
                (bundle) -> {
                    getParentFragmentManager().setFragmentResult("bluetoothProbeInfo", bundle);
                    NavHostFragment.findNavController(BluetoothFragment.this)
                            .navigate(R.id.action_BluetoothFragment_to_FirstFragment);
                }
        ));
        recyclerView.setLayoutManager(new LinearLayoutManager(getContext()));
    }

    @RequiresApi(api = Build.VERSION_CODES.S)
    @RequiresPermission(Manifest.permission.BLUETOOTH_SCAN)
    @Override
    public void onDestroyView() {
        super.onDestroyView();
        bluetoothViewModel.stopScanning();
        binding = null;
        recyclerView = null;
    }

}
