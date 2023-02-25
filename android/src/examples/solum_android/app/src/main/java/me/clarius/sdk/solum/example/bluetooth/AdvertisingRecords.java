package me.clarius.sdk.solum.example.bluetooth;

import android.bluetooth.le.ScanRecord;
import android.os.Build;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

/**
 * Parse bluetooth advertising data
 * <p>
 * This provides an implementation of ScanRecord.getAdvertisingDataMap() for android < 33.
 * <p>
 * Field values taken from the bluetooth document "Assigned Numbers", section 2.3 "Common Data Types".
 */

public class AdvertisingRecords {
    static final int FIELD_FLAGS = 0x01;
    static final int FIELD_COMPLETE_LOCAL_NAME = 0x09;
    static final int FIELD_MANUFACTURER_SPECIFIC_DATA = 0xff;

    static Map<Integer, byte[]> getAdvertisingDataMap(ScanRecord record) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            return record.getAdvertisingDataMap();
        } else {
            return parseScanRecord(record.getBytes());
        }
    }

    private static Map<Integer, byte[]> parseScanRecord(byte[] scanRecord) {
        HashMap<Integer, byte[]> records = new HashMap<>();
        int index = 0;
        while (index < scanRecord.length) {
            int length = scanRecord[index++];
            // Done once we run out of records
            if (length == 0) break;
            byte type = scanRecord[index];
            // Done if our record isn't a valid type
            if (type == 0) break;
            byte[] data = Arrays.copyOfRange(scanRecord, index + 1, index + length);
            records.put(type & 0xFF, data);
            // Advance
            index += length;
        }
        return records;
    }
}
