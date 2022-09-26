package me.clarius.sdk.solum.example;

import me.clarius.sdk.PosInfo;
import me.clarius.sdk.ProbeInfo;
import me.clarius.sdk.ProcessedImageInfo;
import me.clarius.sdk.Range;
import me.clarius.sdk.RawImageInfo;
import me.clarius.sdk.SpectralImageInfo;
import me.clarius.sdk.TgcInfo;

public class Strings {
    public static String fromProcessedImageInfo(final ProcessedImageInfo info) {
        return info.width + "x" + info.height +
                ", " + info.bitsPerPixel + " bpp" +
                " = " + info.imageSize + " bytes" +
                " @ " + info.tm;
    }

    public static String fromRawImageInfo(final RawImageInfo info) {
        return info.lines + "x" + info.samples +
                ", " + info.bitsPerSample + " bps" +
                " @ " + info.tm;
    }

    public static String fromSpectralImageInfo(final SpectralImageInfo info) {
        return info.lines + "x" + info.samples +
                ", " + info.bitsPerSample + " bps" +
                ", period: " + info.period;
    }

    public static String fromTgc(final TgcInfo[] tgcInfo) {
        StringBuilder b = new StringBuilder();
        if (tgcInfo != null) {
            for (TgcInfo tgc : tgcInfo) {
                b.append("[").append(tgc.gain).append("@").append(tgc.depth).append("]");
            }
        }
        return b.toString();
    }

    public static String fromPosInfo(final PosInfo[] posInfo) {
        StringBuilder b = new StringBuilder();
        if (posInfo != null) {
            for (PosInfo pos : posInfo) {
                b.append("[").append(pos.qw).append(",").append(pos.qx).append(",")
                        .append(pos.qy).append(",").append(pos.qz).append("]");
            }
        }
        return b.toString();
    }

    public static String fromProbeInfo(final ProbeInfo probeInfo) {
        return "version: " + probeInfo.version + ", element count: " + probeInfo.elements +
                ", pitch: " + probeInfo.pitch + ", radius: " + probeInfo.radius + " mm";
    }

    public static String fromRange(final Range range) {
        return range.min + "-" + range.max;
    }
}
