package me.clarius.sdk.solum.example;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.databinding.ObservableList;
import androidx.recyclerview.widget.RecyclerView;

import java.util.List;

import me.clarius.sdk.solum.example.bluetooth.Probe;

public class BluetoothViewAdapter extends RecyclerView.Adapter<BluetoothViewAdapter.ViewHolder> {
    private final List<Probe> devices;
    private int selectedPosition = RecyclerView.NO_POSITION;

    public BluetoothViewAdapter(ObservableList<Probe> probes) {
        this.devices = probes;
        probes.addOnListChangedCallback(makeDeviceListCallback());
        notifyItemRangeChanged(0, this.devices.size());
    }

    public Probe getSelectedProbe() {
        if (selectedPosition != RecyclerView.NO_POSITION && selectedPosition < devices.size()) {
            return devices.get(selectedPosition);
        } else {
            return null;
        }
    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        View view = LayoutInflater.from(parent.getContext()).inflate(R.layout.probe_item, parent, false);
        return new ViewHolder(view);
    }

    @Override
    public void onBindViewHolder(@NonNull ViewHolder holder, int position) {
        holder.itemView.setSelected(selectedPosition == position);
        Probe probe = devices.get(position);
        holder.serialValue.setText(probe.getSerial());
        holder.battery.setText(probe.getBattery().map(String::valueOf).orElse("?"));
        holder.temperature.setText(probe.getTemperature().map(String::valueOf).orElse("?"));
        holder.rssi.setText(probe.getRssi().map(String::valueOf).orElse("?"));
        holder.powered.setText(probe.getPowered().map(String::valueOf).orElse("?"));
        holder.availability.setText(probe.getAvailability().map(Enum::name).orElse("?"));
        holder.chargingStatus.setText(probe.getChargingStatus().map(Enum::name).orElse("?"));
        holder.availability.setText(probe.getAvailability().map(Enum::name).orElse("?"));
        holder.listen.setText(probe.getListenPolicy().map(Enum::name).orElse("?"));
    }

    @Override
    public int getItemCount() {
        return devices == null ? 0 : devices.size();
    }

    private ObservableList.OnListChangedCallback<ObservableList<Probe>> makeDeviceListCallback() {
        return new ObservableList.OnListChangedCallback<>() {
            @Override
            public void onChanged(ObservableList sender) {
                notifyItemRangeChanged(0, devices.size());
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
        };
    }

    class ViewHolder extends RecyclerView.ViewHolder {
        final TextView serialValue;
        final TextView temperature;
        final TextView rssi;
        final TextView availability;
        final TextView battery;
        final TextView powered;
        final TextView listen;
        final TextView chargingStatus;

        ViewHolder(@NonNull final View itemView) {
            super(itemView);
            itemView.setOnClickListener(view -> {
                notifyItemChanged(selectedPosition);
                selectedPosition = getLayoutPosition();
                notifyItemChanged(selectedPosition);
            });
            serialValue = itemView.findViewById(R.id.serial_value);
            temperature = itemView.findViewById(R.id.temperature_value);
            rssi = itemView.findViewById(R.id.rssi_value);
            availability = itemView.findViewById(R.id.availability_value);
            battery = itemView.findViewById(R.id.battery_value);
            powered = itemView.findViewById(R.id.powered_value);
            listen = itemView.findViewById(R.id.listen_value);
            chargingStatus = itemView.findViewById(R.id.charging_status_value);
        }
    }
}
