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

import me.clarius.sdk.solum.example.databinding.FragmentWorkflowBinding;

public class WorkflowFragment extends Fragment {

    private WorkflowViewModel viewModel;
    private FragmentWorkflowBinding binding;
    private ArrayAdapter<String> probeListAdapter;
    private ArrayAdapter<String> applicationListAdapter;

    public static WorkflowFragment newInstance() {
        return new WorkflowFragment();
    }

    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container,
                             @Nullable Bundle savedInstanceState) {
        binding = FragmentWorkflowBinding.inflate(inflater, container, false);
        return binding.getRoot();
    }

    @Override
    public void onViewCreated(@NonNull View view, Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        viewModel = new ViewModelProvider(requireActivity()).get(WorkflowViewModel.class);

        viewModel.getProbes().observe(getViewLifecycleOwner(), this::onNewProbeList);
        viewModel.getApplications().observe(getViewLifecycleOwner(), this::onNewApplicationList);

        probeListAdapter = new ArrayAdapter<>(requireContext(), android.R.layout.simple_spinner_item);
        probeListAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        binding.probeList.setAdapter(probeListAdapter);

        applicationListAdapter = new ArrayAdapter<>(requireContext(), android.R.layout.simple_spinner_item);
        applicationListAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        binding.applicationList.setAdapter(applicationListAdapter);

        binding.probeList.setOnItemSelectedListener(
                new AdapterView.OnItemSelectedListener() {
                    @Override
                    public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                        String probe = probeListAdapter.getItem(position);
                        viewModel.selectProbe(probe);
                        viewModel.selectApplication(null);
                    }

                    @Override
                    public void onNothingSelected(AdapterView<?> parent) {
                    }
                }
        );

        binding.applicationList.setOnItemSelectedListener(
                new AdapterView.OnItemSelectedListener() {
                    @Override
                    public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                        String application = probeListAdapter.getItem(position);
                        viewModel.selectApplication(application);
                    }

                    @Override
                    public void onNothingSelected(AdapterView<?> parent) {
                    }
                }
        );
    }

    private void onNewProbeList(List<String> probes) {
        probeListAdapter.clear();
        probeListAdapter.addAll(probes);
        if (!probes.isEmpty()) {
            viewModel.selectProbe(probes.get(0));
        }
    }

    private void onNewApplicationList(List<String> applications) {
        applicationListAdapter.clear();
        applicationListAdapter.addAll(applications);
        if (!applications.isEmpty()) {
            viewModel.selectApplication(applications.get(0));
        }
    }
}