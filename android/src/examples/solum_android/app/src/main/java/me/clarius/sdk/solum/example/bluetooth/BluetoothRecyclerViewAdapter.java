package me.clarius.sdk.solum.example.bluetooth;

import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.constraintlayout.widget.ConstraintLayout;
import androidx.databinding.ObservableArrayList;
import androidx.databinding.ObservableList;
import androidx.lifecycle.LifecycleOwner;
import androidx.lifecycle.LiveData;
import androidx.recyclerview.widget.RecyclerView;

import java.util.function.Consumer;

import me.clarius.sdk.solum.example.R;
import me.clarius.sdk.solum.example.viewmodels.BluetoothViewModel;

public class BluetoothRecyclerViewAdapter extends RecyclerView.Adapter<BluetoothRecyclerViewAdapter.BluetoothViewHolder> {
    public static final String TAG = "BluetoothRecyclerViewAdapter";
    private final ObservableArrayList<LiveData<BluetoothProbeInfo>> devices;
    private final LifecycleOwner lifecycleOwner;
    private final Context fragmentContext;
    private final Consumer<Bundle> onBluetoothProbeInfoSentCallback;

    public BluetoothRecyclerViewAdapter
    (
        final BluetoothViewModel bluetoothViewModel,
        final LifecycleOwner lifecycleOwner,
        final Context context,
        final Consumer<Bundle> onBluetoothProbeInfoSentCallback
    )
    {
        this.devices = bluetoothViewModel.bluetoothProbes;
        this.lifecycleOwner = lifecycleOwner;
        this.fragmentContext = context;
        this.onBluetoothProbeInfoSentCallback = onBluetoothProbeInfoSentCallback;
        setListCallback();
        notifyItemRangeChanged(0 , this.devices.size());
    }

    @NonNull
    @Override
    public BluetoothViewHolder onCreateViewHolder(@NonNull final ViewGroup parent, final int viewType) {
        View view = LayoutInflater.from(parent.getContext())
                .inflate(R.layout.recyclerview_item, parent, false);
        return new BluetoothViewHolder(view);
    }

    @Override
    public void onBindViewHolder(@NonNull final BluetoothViewHolder holder, final int position) {
        LiveData<BluetoothProbeInfo> probeLiveData = devices.get(position);
        observeIfNotObserved(probeLiveData, position);

        BluetoothProbeInfo bluetoothProbeInfo = probeLiveData.getValue();
        if (bluetoothProbeInfo == null) return;

        holder.serialValue.setText(fragmentContext.getString(R.string.probe_serial, ": " + bluetoothProbeInfo.getSerial()));
        setWifiTextFields(bluetoothProbeInfo, holder);
        setPowerTextFields(bluetoothProbeInfo, holder);
        setDeviceTextFields(bluetoothProbeInfo, holder);
    }

    private void setWifiTextFields(final BluetoothProbeInfo probeInfo, final BluetoothViewHolder holder) {
        BluetoothProbeInfo.WifiInfo wifiInfo = probeInfo.getWifiInfo();
        String unknown = fragmentContext.getString(R.string.unknown);
        String text = ": " + (wifiInfo.getIp() == null ? unknown : wifiInfo.getIp());
        holder.ipAddress.setText(fragmentContext.getString(R.string.probe_ip_address, text));
        text = ": " + (wifiInfo.getSsid() == null ? unknown : wifiInfo.getSsid());
        holder.wifiSsid.setText(fragmentContext.getString(R.string.probe_wifi_ssid, text));
        text = ": " + (wifiInfo.getPassword() == null ? unknown : wifiInfo.getPassword());
        holder.wifiPassphrase.setText(fragmentContext.getString(R.string.probe_wifi_passphrase, text));
        text = ": " + (wifiInfo.getPort() == null ? unknown : wifiInfo.getPort());
        holder.tcpPort.setText(fragmentContext.getString(R.string.probe_tcp_port, text));
    }

    private void setPowerTextFields(final BluetoothProbeInfo probeInfo, final BluetoothViewHolder holder) {
        // TODO: retrieve the power info from BluetoothProbeInfo
        String unknown = fragmentContext.getString(R.string.unknown);
        holder.power.setText(fragmentContext.getString(R.string.probe_power, unknown));
    }

    private void setDeviceTextFields(final BluetoothProbeInfo probeInfo, final BluetoothViewHolder holder) {
        // TODO: retrieve other device info from BluetoothProbeInfo
        String unknown = fragmentContext.getString(R.string.unknown);
        holder.temperature.setText(fragmentContext.getString(R.string.probe_temperature, unknown));
        holder.rssi.setText(fragmentContext.getString(R.string.probe_rssi, unknown));
        holder.availability.setText(fragmentContext.getString(R.string.probe_availability, unknown));
        holder.battery.setText(fragmentContext.getString(R.string.probe_battery, unknown));
        holder.certificate.setText(fragmentContext.getString(R.string.probe_certificate, unknown));
        holder.listen.setText(fragmentContext.getString(R.string.probe_listen, unknown));
    }

    private void observeIfNotObserved(final LiveData<BluetoothProbeInfo> bluetoothProbeLiveData, int position) {
        if (!bluetoothProbeLiveData.hasObservers()) {
            bluetoothProbeLiveData.observe(lifecycleOwner, probeInfo -> {
                if (hasStableIds()) notifyItemChanged(position);
            });
        }
    }

    @Override
    public int getItemCount() {
        return devices == null ? 0 : devices.size();
    }

    public void refresh() {
        if (devices != null) {
            int size = devices.size();
            devices.clear();
            notifyItemRangeChanged(0, size);
        }
    }

    private void setListCallback() {
        devices.addOnListChangedCallback(new ObservableList.OnListChangedCallback<ObservableList<BluetoothDevice>>() {
            @Override
            public void onChanged(ObservableList sender) {
                notifyDataSetChanged();
            }

            @Override
            public void onItemRangeChanged(ObservableList sender, int positionStart, int itemCount) {
                notifyItemRangeChanged(positionStart, itemCount);
            }

            @Override
            public void onItemRangeInserted(ObservableList sender, int positionStart, int itemCount) {
                notifyItemRangeInserted(positionStart, itemCount);
            }

            @Override
            public void onItemRangeMoved(ObservableList sender, int fromPosition, int toPosition, int itemCount) {
                notifyItemMoved(fromPosition, toPosition);
            }

            @Override
            public void onItemRangeRemoved(ObservableList sender, int positionStart, int itemCount) {
                notifyItemRangeRemoved(positionStart, itemCount);
            }
        });
    }

    public class BluetoothViewHolder extends RecyclerView.ViewHolder {
        ConstraintLayout recyclerViewItem;
        TextView serialValue;
        TextView ipAddress;
        TextView wifiSsid;
        TextView wifiPassphrase;
        TextView tcpPort;
        TextView temperature;
        TextView rssi;
        TextView availability;
        TextView certificate;
        TextView battery;
        TextView power;
        TextView listen;
        Button button;

        public BluetoothViewHolder(@NonNull final View itemView) {
            super(itemView);
            serialValue = itemView.findViewById(R.id.serial_value);
            button = itemView.findViewById(R.id.select_button);
            recyclerViewItem = itemView.findViewById(R.id.recycler_view_item);
            ipAddress = itemView.findViewById(R.id.probe_ip_address);
            wifiSsid = itemView.findViewById(R.id.ssid);
            wifiPassphrase = itemView.findViewById(R.id.password);
            tcpPort = itemView.findViewById(R.id.probe_tcp_port);
            temperature = itemView.findViewById(R.id.temperature);
            rssi = itemView.findViewById(R.id.rssi);
            availability = itemView.findViewById(R.id.availability);
            certificate = itemView.findViewById(R.id.certificate);
            battery = itemView.findViewById(R.id.battery);
            power = itemView.findViewById(R.id.powered);
            listen = itemView.findViewById(R.id.listen);
            button.setOnClickListener(v -> {
                Bundle result = new Bundle();
                result.putParcelable("bluetoothProbeInfo", devices.get(getAdapterPosition()).getValue());
                Log.d(TAG, "Sending Bluetooth probe info to the first fragment");
                onBluetoothProbeInfoSentCallback.accept(result);
            });
        }
    }
}
