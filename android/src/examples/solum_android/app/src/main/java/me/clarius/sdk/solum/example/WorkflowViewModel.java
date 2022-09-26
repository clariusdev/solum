package me.clarius.sdk.solum.example;

import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;

import java.util.List;
import java.util.concurrent.Executor;

import me.clarius.sdk.Solum;

public class WorkflowViewModel extends ViewModel {
    private final MutableLiveData<List<String>> applications = new MutableLiveData<>();
    private final MutableLiveData<List<String>> probes = new MutableLiveData<>();
    private final MutableLiveData<String> selectedApplication = new MutableLiveData<>();
    private final MutableLiveData<String> selectedProbe = new MutableLiveData<>();

    public MutableLiveData<List<String>> getApplications() {
        return applications;
    }

    public MutableLiveData<List<String>> getProbes() {
        return probes;
    }

    public MutableLiveData<String> getSelectedApplication() {
        return selectedApplication;
    }

    public MutableLiveData<String> getSelectedProbe() {
        return selectedProbe;
    }

    public void refreshApplications(Executor executor, Solum solum, String probe) {
        executor.execute(() -> {
            List<String> applications = solum.getApplications(probe);
            this.applications.postValue(applications);
        });
    }

    public void refreshProbes(Executor executor, Solum solum) {
        executor.execute(() -> {
            List<String> probes = solum.getProbes();
            this.probes.postValue(probes);
        });
    }

    public void selectProbe(final String probe) {
        selectedProbe.postValue(probe);
        selectedApplication.postValue(null);
    }

    public void selectApplication(final String application) {
        selectedApplication.postValue(application);
    }
}