package me.clarius.sdk.solum.example;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.ViewModelProvider;

import java.util.List;
import java.util.Optional;
import java.util.stream.Collectors;
import java.util.stream.Stream;

import me.clarius.sdk.Platform;
import me.clarius.sdk.solum.example.databinding.FragmentFirmwareUpdateBinding;

public class FirmwareUpdateFragment extends Fragment {

    private final List<PlatformItem> platformList;
    private ViewModel viewModel;
    private FragmentFirmwareUpdateBinding binding;
    private ArrayAdapter<PlatformItem> platformListAdapter;

    public FirmwareUpdateFragment() {
        platformList = Stream.of(Platform.values())
                .map(PlatformItem::fromPlatform)
                .collect(Collectors.toList());
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        binding = FragmentFirmwareUpdateBinding.inflate(inflater, container, false);
        return binding.getRoot();
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        viewModel = new ViewModelProvider(requireActivity()).get(ViewModel.class);
        platformListAdapter = new ArrayAdapter<>(requireContext(), android.R.layout.simple_spinner_item);
        platformListAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        platformListAdapter.addAll(platformList);
        binding.platformList.setAdapter(platformListAdapter);
        binding.platformList.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                PlatformItem platformItem = platformListAdapter.getItem(position);
                viewModel.setSelectedPlatformVersion(platformItem.platform);
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
            }
        });
        viewModel.getSelectedPlatformVersion().observe(getViewLifecycleOwner(), platform -> {
            if (!(((PlatformItem) binding.platformList.getSelectedItem()).platform.equals(platform))) { // if the given platform is not equal to the currently selected platform
                binding.platformList.setSelection(platformListAdapter.getPosition(PlatformItem.fromPlatform(platform)));
            }
        });
    }

    private static class PlatformItem {
        final Platform platform;
        final String description;

        PlatformItem(Platform platform, String description) {
            this.platform = platform;
            this.description = description;
        }

        static PlatformItem fromPlatform(Platform platform) {
            return new PlatformItem(platform, String.format("%s firmware", platform.name()));
        }

        public String toString() {
            return description;
        }

        @Override
        public boolean equals(Object o) {
            if (this == o) return true;
            if (o == null || getClass() != o.getClass()) return false;

            PlatformItem that = (PlatformItem) o;

            return this.platform.equals(that.platform);
        }
    }
}
