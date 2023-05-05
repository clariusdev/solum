package me.clarius.sdk.solum.example.bluetooth;

import android.bluetooth.le.ScanRecord;

import androidx.annotation.NonNull;

import java.nio.ByteBuffer;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;

/**
 * Probe status
 * <p>
 * Can be extracted from the advertised bluetooth manufacturer data with fromScanRecord().
 */

public class ProbeStatus {
    static final byte[] CLARIUS_ID = {(byte) 0xfb, (byte) 0x02}; // clarius id = 0x02fb
    final int battery;
    final int temperature;
    final Availability availability;
    final ListenPolicy listenPolicy;
    final ChargingStatus chargingStatus;
    final boolean powered;

    protected ProbeStatus(int battery, int temperature, Availability availability, ListenPolicy listenPolicy, ChargingStatus chargingStatus, boolean powered) {
        this.battery = battery;
        this.temperature = temperature;
        this.availability = availability;
        this.listenPolicy = listenPolicy;
        this.chargingStatus = chargingStatus;
        this.powered = powered;
    }

    private static <T extends Enum<T>> T enumFromOrdinal(final Class<T> type, final int ordinal) {
        int index = Math.min(ordinal, Objects.requireNonNull(type.getEnumConstants()).length);
        return type.getEnumConstants()[index];
    }

    /**
     * Get and parse the manufacturer data from the bluetooth advertising data
     *
     * @param record scan record from ScanResult.getScanRecord()
     * @return parsed advertising data or null if parsing failed
     */
    public static ProbeStatus fromScanRecord(final ScanRecord record) {
        Map<Integer, byte[]> adRecords = AdvertisingRecords.getAdvertisingDataMap(record);
        Optional<Map.Entry<Integer, byte[]>> maybeEntry = adRecords.entrySet().stream().filter(entry -> entry.getKey().equals(AdvertisingRecords.FIELD_MANUFACTURER_SPECIFIC_DATA)).findAny();
        return maybeEntry.map(entry -> parse(entry.getValue())).orElse(null);
    }

    /**
     * Parse manufacturer data extracted from the bluetooth advertising data
     *
     * @param data manufacturer data record extracted from ScanRecord.getBytes()
     * @return parsed advertising data or null if parsing failed
     */
    static ProbeStatus parse(final byte[] data) {
        if (data.length < 6) {
            return null;
        }
        if (!ByteBuffer.wrap(data, 0, 2).equals(ByteBuffer.wrap(CLARIUS_ID))) {
            return null;
        }
        int battery = Math.min(data[2], 100);
        int temperature = Math.min(data[3], 100);
        int packedBits = data[4];
        Availability availability = enumFromOrdinal(Availability.class, packedBits & 0x07);
        ListenPolicy listenPolicy = enumFromOrdinal(ListenPolicy.class, (packedBits >> 3) & 0x03);
        ChargingStatus chargingStatus = enumFromOrdinal(ChargingStatus.class, (packedBits >> 6) & 0x03);
        boolean powered = ((data[5]) & 0x01) != 0;
        return new ProbeStatus(battery, temperature, availability, listenPolicy, chargingStatus, powered);
    }

    @NonNull
    public String toString() {
        return "batt: " + battery + "% " +
                "temp: " + temperature + " " +
                "avail: " + availability.name() + " " +
                "listen: " + listenPolicy.name() + " " +
                "charging: " + chargingStatus.name() + " " +
                "on: " + powered;
    }

    public enum Availability {
        AVAILABLE, LISTEN_ONLY, NOT_AVAILABLE
    }

    public enum ListenPolicy {
        DISABLED, INSTITUTION, GLOBAL, RESEARCH
    }

    public enum ChargingStatus {
        NONE, PRE, FAST, DONE
    }
}
