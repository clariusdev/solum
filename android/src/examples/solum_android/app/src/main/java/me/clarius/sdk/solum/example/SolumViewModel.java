package me.clarius.sdk.solum.example;

import android.graphics.Bitmap;

import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;

/**
 * View model for Solum live data
 * <p>
 * Read more about live data: https://developer.android.com/topic/libraries/architecture/livedata.
 */

public class SolumViewModel extends ViewModel {

    private final MutableLiveData<Bitmap> processedImage = new MutableLiveData<>();

    public MutableLiveData<Bitmap> getProcessedImage() {
        return processedImage;
    }
}
