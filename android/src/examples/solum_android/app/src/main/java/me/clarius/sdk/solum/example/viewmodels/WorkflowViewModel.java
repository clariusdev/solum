package me.clarius.sdk.solum.example.viewmodels;

import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;

import java.util.Arrays;
import java.util.List;

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

    public void refreshApplications(Solum solum, String probe) {
        solum.getApplications(probe, result -> applications.postValue(Arrays.asList(result)));
    }

    public void refreshProbes(Solum solum) {
        solum.getProbes(result -> probes.postValue(Arrays.asList(result)));
    }

    public void selectProbe(final String probe) {
        selectedProbe.postValue(probe);
        selectedApplication.postValue(null);
    }

    public void selectApplication(final String application) {
        selectedApplication.postValue(application);
    }
}
