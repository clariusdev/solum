# Solum Parameter Reference

This document covers everything you can read and tune while imaging: standard imaging
**parameters**, imaging **modes**, **TGC**, **ROI/gate** controls, and the **low-level**
acoustic parameters intended for research use. For function signatures see the
[API reference](api-reference.md); for the call flow see [getting-started.md](getting-started.md).

## Imaging parameters (`CusParam`)

Set with `solumSetParam(param, value)`, read with `solumGetParam(param)`, and query the
valid range for the current probe/application/mode with `solumGetRange(param, &range)`.
Availability and defaults depend on the loaded application and mode.

| Parameter | Units | Default | Notes |
|-----------|-------|---------|-------|
| `ImageDepth` | cm | application-dependent | Imaging depth. |
| `Gain` | % | 50 | Overall brightness (post-processing gamma). |
| `AutoGain` | bool | on (most apps) | Automated gain over depth. |
| `Contrast` | % | 50 | Dynamic-range / contrast. |
| `Chroma` | bool | off | Colorized grayscale map. |
| `Smooth` | % | 50 | Smoothing filter strength. |
| `PenetrationMode` | bool | off | Penetration mode. |
| `AutoFocus` | bool | on | Automatic focus. |
| `FocusDepth` | cm | — | Focus depth (used when `AutoFocus` off). |
| `ExtendFov` | bool | off (most apps) | Trapezoidal/extended FOV (linear probes). |
| `ColorGain` | % | 50 | Color/power Doppler gain. |
| `ColorPrf` | kHz | application-dependent | Color/power PRF (velocity range). |
| `ColorSteer` | degrees | application-dependent | Color/power steering (linear probes). |
| `ColorInvert` | bool | off | Invert color map (red→probe / blue→probe). |
| `PwGain` | % | 50 | Pulsed-wave Doppler gain. |
| `PwPrf` | kHz | application-dependent | PW PRF (velocity range). |
| `PwSteer` | degrees | application-dependent | PW steering (linear probes). |
| `DopplerVelocity` | cm/s | read-only | Velocity range implied by the current PRF (`0` if not in a Doppler mode). |
| `NeedleSide` | 0/1 | 0 (LED side) | Needle-enhance side. |
| `StrainOpacity` | % | 50 | Strain overlay opacity. |
| `RawBuffer` | bool | off | Raw-data buffering (enable before capturing raw data). |
| `RfStreaming` | bool | on | RF streaming. |
| `ImuStreaming` | bool | off | IMU streaming. |
| `SyncPulse` | bool | off | Sync pulse output. |
| `EcoMode` | bool | off | Power-saving mode. |

Booleans are set with `1`/`0`. Because ranges and defaults vary by probe and application,
always call `solumGetRange()` before exposing a slider rather than hard-coding limits.

## Imaging modes (`CusMode`)

Set with `solumSetMode(mode)`, read with `solumGetMode()`. Availability varies by probe
and application.

| Mode | Description |
|------|-------------|
| `BMode` | B-mode grayscale (the default for most use cases). |
| `Compounding` | Spatial compounding (multi-frame grayscale). |
| `MMode` | M-mode (time–motion spectrum). |
| `ColorMode` | Color flow Doppler. |
| `PowerMode` | Power Doppler (no direction; excludes cardiac). |
| `PwMode` | Pulsed-wave Doppler spectrum. |
| `NeedleEnhance` | Needle enhancement (linear probes). |
| `Strain` | Strain elastography. |
| `RfMode` | RF capture interleaved with B-mode. |
| `ElemTest` | Element test raw signals. |

## TGC

Time-gain compensation uses three controls (top/mid/bottom), each in dB over the range
**−20 … 20**:

```c
CusTgc tgc = { .top = 0.0, .mid = 5.0, .bottom = 10.0 };
solumSetTgc(&tgc);
solumGetTgc(&tgc);
```

Per-frame TGC actually applied is reported in the `tgc[CUS_MAXTGC]` array of the image
info structs (`CusTgcInfo` = depth in mm + gain in dB). When auto gain is on, the curve
may differ slightly frame to frame.

## ROI and gate

For modes with a region of interest (color/power Doppler, strain, RF):

```c
double pts[64 * 2];
solumGetRoi(pts, 64);              // current ROI polygon (x,y pixels)
solumAdjustRoi(x, y, SizeRoi);     // resize via bottom-right
solumAdjustRoi(x, y, MoveRoi);     // move via top-left
solumMaximizeRoi();                // fill without computing coordinates
solumGetActiveRegion(pts, 64);     // active grayscale region
```

For M-mode and PW gates:

```c
CusGateLines lines;
solumGetGate(&lines);   // active/top/bottom lines in pixel coordinates for drawing
solumAdjustGate(x, y);  // reposition by pixel
```

## Acoustic indices

`solumGetAcousticIndices(&a)` fills a `CusAcoustic` with the mechanical index (`mi`) and
thermal indices (`tib`, `tic`, `tis`) for the current application, mode, and parameter
settings. Re-query after changing parameters that affect acoustic output.

## Low-level parameters

> ⚠️ **Safety.** Low-level parameters bypass the optimized presets and can change the
> acoustic output of the transducer, pushing imaging **outside the safety limits Clarius
> validates**, degrading image quality, or destabilizing the system. They are intended
> for research and bring-up. Parameters marked **\*\*** directly affect acoustic output.

These map onto the persistent imaging "scripts" Clarius uses internally per probe,
application, and mode. Access them through:

```c
solumSetLowLevelParam("txFreq", 5.0);     // numeric value
solumEnableLowLevelParam("sa", 1);        // boolean enable/disable
solumSetLowLevelPulse("txPulseGen", "+-"); // transmit pulse shape string
double v = solumGetLowLevelParam("txFreq");
```

The same parameter set is exposed through the Cast API for researchers; see the
[research repository](https://github.com/clariusdev/research) for the authoritative,
mode-by-mode list and the meaning of each value.

### Numeric parameters

**Generic**

| Parameter | Meaning |
|-----------|---------|
| `maxFrameRate` | Frame-rate limit in Hz (0–100, `0` = no limit). |

**Greyscale**

| Parameter | Meaning |
|-----------|---------|
| `txFreq` ** | Transmit frequency (MHz). |
| `txFreqInv` ** | Transmit frequency for the inversion pulse (MHz). |
| `txFn` ** | Transmit f-number. |
| `txApt` ** | Max transmit aperture in elements (2–64). |
| `txFocus` ** | Focus depth (cm). |
| `vpp` ** | Positive amplitude (V, 10–37). |
| `vnn` ** | Negative amplitude (V, 10–37). |
| `ceVpp` ** | Positive amplitude for CEUS (V, 10–37). |
| `ceVnn` ** | Negative amplitude for CEUS (V, 10–37). |
| `steer` ** | Image steering (degrees). |
| `rxFn` | Receive f-number. |
| `rxFreqShallow` | Start demodulation frequency (MHz). |
| `rxFreqDeep` | End demodulation frequency (MHz). |
| `speedOfSound` | Speed-of-sound correction (m/s). |
| `rfDecimation` | Decimation factor for IQ data. |
| `envDecimation` | Decimation factor for envelope/grayscale data. |
| `rfDecim` | Decimation factor of RF acquisition. |
| `dyn` | Dynamic range (dB). |
| `nf` | Noise floor (dB). |

**Color / Power Doppler**

| Parameter | Meaning |
|-----------|---------|
| `cfiPrf` ** | PRF (kHz; range varies per probe/preset). |
| `cfiTxFreq` ** | Transmit frequency (MHz). |
| `color steer` | Steering angle (degrees). |
| `cfiEnsemble` ** | Ensemble / transmits per line (6–16). |

**Pulsed-Wave Doppler**

| Parameter | Meaning |
|-----------|---------|
| `pwPrf` ** | PRF (kHz; range varies per probe/preset). |
| `pwTxFreq` ** | Transmit frequency (MHz). |
| `pw gate size` | Gate size (mm). |
| `pw steer` | Steering angle (degrees). |
| `pw angle correct` | Correction angle (degrees). |

### Boolean parameters (greyscale)

| Parameter | Meaning |
|-----------|---------|
| `sa` | Synthetic aperture. |
| `pih` | Pulse-inversion harmonics. |
| `trapezoidal` | Extended FOV for linear probes. |

### Pulse-shape (string) parameters

| Parameter | Meaning |
|-----------|---------|
| `txPulseGen` ** | Transmit pulse. |
| `txPulsePen` ** | Transmit pulse in penetration mode. |
| `txPulseInv` ** | Transmit pulse for the inversion pulse. |
| `cfiPulse` ** | Color/power transmit pulse. |
| `pwPulse` ** | PW transmit pulse. |

** Changing this parameter may alter the acoustic output of the transducer, making
imaging operate outside the safety parameters Clarius has programmed into the device.
