package me.clarius.sdk.solum.example;

import java.util.StringJoiner;

import me.clarius.sdk.PointF;
import me.clarius.sdk.ProbeInfo;
import me.clarius.sdk.Range;
import me.clarius.sdk.StatusInfo;
import me.clarius.sdk.Tgc;

public class Strings {
    public static String fromProbeInfo(final ProbeInfo probeInfo) {
        return new StringJoiner(", ").add("version: " + probeInfo.version).add("element count: " + probeInfo.elements).add("pitch: " + probeInfo.pitch).add("radius: " + probeInfo.radius + " mm").toString();
    }

    public static String fromRange(final Range range) {
        return range.min + "-" + range.max;
    }

    public static String fromTgc(final Tgc tgc) {
        return new StringJoiner(", ").add("top: " + tgc.top).add("mid: " + tgc.mid).add("bottom: " + tgc.bottom).toString();
    }

    public static String fromStatusInfo(final StatusInfo status) {
        return new StringJoiner(", ").add("batt: " + status.battery).add("temp: " + status.temperature).add("frame rate: " + status.frameRate).add("fan: " + status.fan).add("charging: " + status.charger).toString();
    }

    public static String fromPoints(final PointF[] points) {
        StringJoiner strings = new StringJoiner("; ", "[", "]");
        for (PointF p : points) {
            strings.add(p.x + "," + p.y);
        }
        return strings.toString();
    }
}
