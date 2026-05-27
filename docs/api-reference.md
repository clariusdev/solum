# Solum API Reference

Complete reference for the **v12** Solum C API. Declarations live in
[`include/solum`](../include/solum):

- [`solum.h`](../include/solum/solum.h) — functions and the init/connection structs
- [`solum_def.h`](../include/solum/solum_def.h) — enumerations and data structs
- [`solum_cb.h`](../include/solum/solum_cb.h) — callback function signatures

All functions are `extern "C"` and exported with `SOLUM_EXPORT`. Unless stated
otherwise, functions return `0` (`CUS_SUCCESS`) on success and `-1` (`CUS_FAILURE`) on
failure. For a task-oriented walkthrough see [getting-started.md](getting-started.md);
for imaging parameters and modes see [parameters.md](parameters.md).

## Conventions

| Constant | Value | Meaning |
|----------|-------|---------|
| `CUS_SUCCESS` | `0` | call succeeded |
| `CUS_FAILURE` | `-1` | call failed |
| `CERT_INVALID` | `-1` | certificate invalid / not found |
| `CUS_MAXTGC` | `10` | size of TGC arrays in image info structs |

Many results are delivered **asynchronously** via callbacks rather than the function's
return value. For example, `solumConnect()` returns `0` to mean "the attempt was started";
the actual outcome arrives on `connectFn`.

## Lifecycle & connection

| Function | Description |
|----------|-------------|
| `int solumInit(const CusInitParams* params)` | Initialize the module. Must be called first. |
| `CusInitParams solumDefaultInitParams(void)` | Zero-initialized init params to fill in. |
| `int solumDestroy(void)` | Free all resources; call before exit. |
| `int solumConnect(const CusConnectionParams* params)` | Start a TCP connection to a probe on the same network. |
| `CusConnectionParams solumDefaultConnectionParams(void)` | Zero-initialized connection params. |
| `int solumDisconnect(void)` | Drop the current connection. |
| `int solumIsConnected(void)` | `1` connected, `0` not connected, `-1` not initialized. |
| `int solumSetCert(const char* cert)` | Set the probe certificate (see [getting started](getting-started.md#1-obtain-a-probe-certificate)). |
| `int solumPowerDown(void)` | Power the probe off over TCP. |
| `int solumSetTeeFn(CusTeeConnectFn tee)` | Register the TEE (transesophageal) connectivity callback. |
| `int solumSetTeeExamInfo(const char* id, const char* name, const char* exam)` | Burn patient/exam info into a connected TEE probe. |

### `CusInitParams`

| Field | Type | Notes |
|-------|------|-------|
| `args.argc`, `args.argv` | `int`, `char**` | Forwarded to the library (Qt graphics buffer init). |
| `storeDir` | `const char*` | Writable directory for security keys. |
| `connectFn` | `CusConnectFn` | Connection status. |
| `certFn` | `CusCertFn` | Certificate validity. |
| `powerDownFn` | `CusPowerDownFn` | Probe power-down notifications. |
| `imagingFn` | `CusImagingFn` | Imaging-ready / state changes. |
| `buttonFn` | `CusButtonFn` | Physical button presses. |
| `errorFn` | `CusErrorFn` | Error messages. |
| `elemTestFn` | `CusElementTestFn` | Element test results. |
| `newProcessedImageFn` | `CusNewProcessedImageFn` | Scan-converted frames. |
| `newRawImageFn` | `CusNewRawImageFn` | Pre-scan-converted / RF frames. |
| `newSpectralImageFn` | `CusNewSpectralImageFn` | M / PW spectral blocks. |
| `newImuPortFn` | `CusNewImuPortFn` | IMU UDP port assignment. |
| `newImuDataFn` | `CusNewImuDataFn` | IMU samples (when streaming). |
| `width`, `height` | `int` | Output buffer dimensions in pixels. |

### `CusConnectionParams`

| Field | Type | Notes |
|-------|------|-------|
| `ipAddress` | `const char*` | Probe IP (from the BLE Wi-Fi Published characteristic). |
| `port` | `unsigned int` | Probe TCP control port. |
| `networkId` | `long long int` | Wi-Fi network id from Android auto-join; `0` by default. |

## Firmware & updates

| Function | Description |
|----------|-------------|
| `int solumFwVersion(CusPlatform platform, char* version, int sz)` | Firmware version the **SDK** expects for a platform (buffer ≥ 128 bytes). Use it to download the matching firmware. |
| `int solumSoftwareUpdate(const char* path, CusSwUpdateFn fn, CusProgressFn progress, int hwVer)` | Push firmware to the connected probe. `hwVer` = `0` (auto) unless forcing `1`/`2`/`3`. |

## Workflow (probes & applications)

| Function | Description |
|----------|-------------|
| `int solumProbes(CusListFn fn)` | List supported probe models (CSV via callback). |
| `int solumApplications(const char* probe, CusListFn fn)` | List applications/workflows for a probe model. |
| `int solumLoadApplication(const char* probe, const char* workflow)` | Load an application. *(v13 adds an `int array` argument.)* |

## Imaging control

| Function | Description |
|----------|-------------|
| `int solumRun(int run)` | `1` start imaging, `0` stop/freeze. |
| `int solumIsImaging(void)` | `1` imaging, `0` not, `-1` not initialized. |
| `int solumSetMode(CusMode mode)` | Set the imaging mode. |
| `CusMode solumGetMode(void)` | Current imaging mode. |
| `int solumSetOutputSize(int w, int h)` | Set scan-conversion output dimensions (1:1 pixel aspect, black borders fill the rest). |
| `int solumSeparateOverlays(int en)` | Deliver grayscale and color/strain overlay as separate image callbacks. |
| `int solumSetFormat(CusImageFormat format)` | Output format for processed images (default ARGB). |
| `int solumEnable5v(int en)` | Toggle the 5 V output. |
| `int solumOptimizeWifi(CusWifiOpt opt)` | Optimize the probe's own Wi-Fi channel (fails if on an external WLAN). |
| `int solumResetProbe(CusProbeReset reset)` | Run a probe reset action (see `CusProbeReset`). |

## Parameters, TGC, ROI & gate

| Function | Description |
|----------|-------------|
| `int solumSetParam(CusParam param, double val)` | Set an imaging parameter. |
| `double solumGetParam(CusParam param)` | Get a parameter value (`-1` on failure). |
| `int solumGetRange(CusParam param, CusRange* range)` | Valid min/max for a parameter in the current state. |
| `int solumSetTgc(const CusTgc* tgc)` | Set top/mid/bottom TGC (each −20…20 dB). |
| `int solumGetTgc(CusTgc* tgc)` | Get current TGC. |
| `int solumGetActiveRegion(double* points, int count)` | Active grayscale region as `count` x/y points. |
| `int solumGetRoi(double* points, int count)` | ROI polygon for the current mode. |
| `int solumAdjustRoi(int x, int y, CusRoiFunction fn)` | Move/resize the ROI by pixel position. |
| `int solumMaximizeRoi(void)` | Maximize the ROI without computing coordinates. |
| `int solumGetGate(CusGateLines* lines)` | M/PW gate lines for drawing. |
| `int solumAdjustGate(int x, int y)` | Adjust the gate by pixel position. |
| `int solumGetAcousticIndices(CusAcoustic* indices)` | MI / TIB / TIC / TIS for the current setup. |

## Low-level (research) control

> ⚠️ These functions reach below the standard parameter set and can drive the transducer
> outside the safety envelope Clarius validates. See [parameters.md](parameters.md#low-level-parameters).

| Function | Description |
|----------|-------------|
| `int solumSetLowLevelParam(const char* prm, double val)` | Set a low-level numeric parameter. |
| `double solumGetLowLevelParam(const char* prm)` | Read a low-level parameter (booleans return `0`/`1`). |
| `int solumEnableLowLevelParam(const char* prm, int en)` | Enable/disable a low-level boolean parameter. |
| `int solumSetLowLevelPulse(const char* prm, const char* shape)` | Set a transmit pulse shape string. |

## Status & probe info

| Function | Description |
|----------|-------------|
| `int solumStatusInfo(CusStatusInfo* info)` | Battery, temperature, frame rate, fan/guide/charger status. |
| `int solumProbeInfo(CusProbeInfo* info)` | Hardware version, element count, pitch, radius, frequency. |
| `int solumSetProbeSettings(const CusProbeSettings* settings)` | Program persistent probe settings (timeouts, flags, button mapping). |

## Diagnostics & calibration

| Function | Description |
|----------|-------------|
| `int solumCalibrateImu(int stationary, CusImuCalibrationFn fn, CusProgressFn progress)` | Calibrate the IMU (probe must be frozen). `stationary=1` calibrates stationary detection. |
| `int solumBatteryHealth(CusBatteryHealthFn fn)` | Battery health test (frozen; up to ~10 s). |

## Raw data

See [getting started, step 10](getting-started.md#10-capture-raw-data) for the flow. The
probe must be **frozen** with `RawBuffer` enabled.

| Function | Description |
|----------|-------------|
| `int solumRawDataAvailability(CusRawAvailabilityFn fn)` | Report buffered B and IQ/RF frame timestamps. |
| `int solumRequestRawData(long long start, long long end, int lzo, CusRawRequestFn fn)` | Request frames by ns timestamp (`0,0` = all). `lzo=1` → compressed tarball. Callback returns the byte size to allocate. |
| `int solumReadRawData(void** data, CusRawFn fn, CusProgressFn progress)` | Download the requested data into a pre-allocated buffer. |

## Callbacks

Signatures from [`solum_cb.h`](../include/solum/solum_cb.h):

| Callback | Signature | Fires when |
|----------|-----------|------------|
| `CusListFn` | `(const char* list, int sz)` | A CSV list is returned (probes, applications). |
| `CusConnectFn` | `(CusConnection res, int port, const char* status)` | Connection state changes; `port` is the UDP stream port. |
| `CusCertFn` | `(int daysValid)` | Certificate set; `CERT_INVALID` invalid, `0` expired. |
| `CusPowerDownFn` | `(CusPowerDown res, int tm)` | Probe is powering down in `tm` seconds. |
| `CusSwUpdateFn` | `(CusSwUpdate res)` | Software update result. |
| `CusNewRawImageFn` | `(const void* img, const CusRawImageInfo* nfo, int npos, const CusPosInfo* pos)` | Pre-scan / RF frame ready. |
| `CusNewProcessedImageFn` | `(const void* img, const CusProcessedImageInfo* nfo, int npos, const CusPosInfo* pos)` | Scan-converted frame ready. |
| `CusNewSpectralImageFn` | `(const void* img, const CusSpectralImageInfo* nfo)` | M/PW spectral block ready. |
| `CusImagingFn` | `(CusImagingState state, int imaging)` | Imaging ready / error / freeze / re-optimization. |
| `CusButtonFn` | `(CusButton btn, int clicks)` | Physical button pressed. |
| `CusProgressFn` | `(int progress)` | Percent progress (updates, downloads, calibration). |
| `CusRawAvailabilityFn` | `(int res, int n_b, const long long* b, int n_iqrf, const long long* iqrf)` | Buffered raw-data timestamps. |
| `CusRawRequestFn` | `(int res, const char* extension)` | Raw request sized; `extension` is the package type. |
| `CusRawFn` | `(int res)` | Raw download complete (`res` = bytes). |
| `CusErrorFn` | `(CusErrorCode code, const char* msg)` | An error occurred. |
| `CusTeeConnectFn` | `(bool connected, const char* serial, double timeRemaining, const char* id, const char* name, const char* exam)` | TEE disposable connection state. |
| `CusNewImuPortFn` | `(int port)` | IMU UDP streaming port (`0` = off). |
| `CusNewImuDataFn` | `(const CusPosInfo* pos)` | Streamed IMU sample. |
| `CusImuCalibrationFn` | `(CusImuCalibration res)` | IMU calibration result. |
| `CusBatteryHealthFn` | `(CusBatteryHealth res, double val)` | Battery health result/value. |
| `CusElementTestFn` | `(CusElementTest res, double val)` | Element test result/value. |

> Callbacks run on the SDK's threads. Keep them short and hand data off to your own
> queues; don't block or call back into long-running SDK operations from within them.

## Enumerations

From [`solum_def.h`](../include/solum/solum_def.h). Modes (`CusMode`) and parameters
(`CusParam`) have their own detailed tables in [parameters.md](parameters.md).

**`CusPlatform`** — `V1` (first generation), `HD`, `HD3`

**`CusImageFormat`** — `Uncompressed` (32-bit ARGB), `Uncompressed8Bit` (8-bit gray),
`Jpeg`, `Png`

**`CusButton`** — `ButtonUp`, `ButtonDown`, `ButtonHandle`

**`CusConnection`** — `ConnectionError` (−1), `ProbeConnected`, `ProbeDisconnected`,
`ConnectionFailed`, `SwUpdateRequired`, `OSUpdateRequired`

**`CusImagingState`** — `ImagingNotReady`, `ImagingReady`, `CertExpired`, `PoorWifi`,
`NoContact`, `ChargingChanged`, `LowBandwidth`, `MotionSensor`, `NoTee`, `TeeExpired`

**`CusPowerDown`** — `Idle`, `TooHot`, `LowBattery`, `ButtonOff`, `ChargingInDock`,
`SoftwareShutdown`

**`CusSwUpdate`** — `SwUpdateError` (−1), `SwUpdateSuccess`, `SwUpdateCurrent`,
`SwUpdateBattery`, `SwUpdateUnsupported`, `SwUpdateCorrupt`

**`CusErrorCode`** — `ErrorGeneric`, `ErrorSetup`, `ErrorProbe`, `ErrorApplication`,
`ErrorSwUpdate`, `ErrorGl`, `ErrorRawData`

**`CusProbeReset`** — `ResetKeys`, `ResetWifi`, `ResetFactory`, `ResetTime`, `ResetCert`,
`ResetBattery`, `ResetSdCard`, `ResetPerformReboot`

**`CusWifiOpt`** — `WifiOptDefault` (channel 165), `WifiOptNext`, `WifiOptSearch`
(scans, up to ~10 s)

**`CusRoiFunction`** — `SizeRoi` (resize bottom-right), `MoveRoi` (move top-left)

**`CusFanStatus`** — `NoFan`, `FanDetected`, `FanRunning`

**`CusGuideStatus`** — `NoGuide`, `GuideDetected`, `GuideRunning`

**`CusChargingStatus`** — `NotCharging`, `Charging`

**`CusButtonSetting`** — `ButtonDisabled`, `ButtonFreeze`, `ButtonUser`

**`CusButtonHoldSetting`** — `ButtonHoldDisabled`, `ButtonHoldShutdown`, `ButtonHoldUser`

**`CusImuCalibration`** — `ImuCalibrationInProgress`, `ImuCalibrationFailed`,
`ImuCalibrationSuccess`

**`CusBatteryHealth`** — `BatteryHealthError`, `BatteryHealthSuccess`,
`BatteryHealthFailed`

**`CusElementTest`** — `ElementTestError`, `ElementTestSuccess`, `ElementTestFailed`

**`CusLogLevel`** — `None`, `Info`

## Data structures

From [`solum_def.h`](../include/solum/solum_def.h).

### `CusProcessedImageInfo` (scan-converted frames)

| Field | Type | Notes |
|-------|------|-------|
| `width`, `height` | `int` | Pixels. |
| `bitsPerPixel` | `int` | Bits per pixel. |
| `imageSize` | `int` | Total bytes. |
| `micronsPerPixel` | `double` | 1:1 axial/lateral. |
| `originX`, `originY` | `double` | Image origin in microns. |
| `tm` | `long long` | Timestamp (ns). |
| `angle` | `double` | Acquisition angle (volumetric). |
| `fps` | `double` | Frame rate (Hz). |
| `overlay` | `int` | `1` if a color/strain overlay frame. |
| `format` | `CusImageFormat` | Pixel format. |
| `tgc[CUS_MAXTGC]` | `CusTgcInfo` | TGC points. |

### `CusRawImageInfo` (pre-scan / RF frames)

| Field | Type | Notes |
|-------|------|-------|
| `lines`, `samples` | `int` | Ultrasound lines / samples per line. |
| `bitsPerSample` | `int` | Bits per sample. |
| `axialSize`, `lateralSize` | `double` | Microns per sample / per line. |
| `tm` | `long long` | Timestamp (ns). |
| `jpeg` | `int` | JPEG byte size, `0` if uncompressed. |
| `rf` | `int` | `1` = RF data, `0` = envelope. |
| `angle`, `fps` | `double` | Acquisition angle / frame rate. |
| `tgc[CUS_MAXTGC]` | `CusTgcInfo` | TGC points. |

### `CusSpectralImageInfo` (M / PW)

| Field | Type | Notes |
|-------|------|-------|
| `lines`, `samples` | `int` | Lines / samples per line. |
| `bitsPerSample` | `int` | Bits per sample. |
| `period` | `double` | Line acquisition period (s). |
| `micronsPerSample` | `double` | For M spectrum. |
| `velocityPerSample` | `double` | m/s per sample for PW spectrum. |
| `pw` | `int` | `1` = PW, `0` = M. |

### `CusPosInfo` (IMU / positional)

`tm` (ns), gyro `gx/gy/gz` (rad/s), accel `ax/ay/az` (normalized to g), magnetometer
`mx/my/mz` (normalized to Earth's field), orientation quaternion `qw/qx/qy/qz`.

### `CusProbeInfo`

`version` (1 = gen 1, 2 = HD, 3 = HD3), `elements`, `pitch`, `radius` (mm),
`frequency` (Hz).

### `CusStatusInfo`

`battery` (%), `temperature` (%), `frameRate` (Hz), `teeTimeRemaining`, `fan`
(`CusFanStatus`), `guide` (`CusGuideStatus`), `charger` (`CusChargingStatus`).

### `CusAcoustic`

`mi` (mechanical index), `tib`, `tic`, `tis` (thermal indices).

### `CusTgc` / `CusTgcInfo`

`CusTgc`: `top`, `mid`, `bottom` in dB (−20…20). `CusTgcInfo`: `depth` (mm), `gain` (dB).

### `CusRange`

`min`, `max` (doubles).

### `CusGateLines`

`active`, `top`, `normalTop`, `normalBottom`, `bottom` — each a `CusLineF` (two
`CusPointF`) in pixel coordinates for drawing an M/PW gate.

### `CusProbeSettings`

Persistent settings programmed via `solumSetProbeSettings`. Timeouts (seconds unless
noted): `contactDetection` (0–30), `autoFreeze` (0–120), `keepAwake` (minutes, 0–120),
`deepSleep` (hours, 0–96), `stationary` (0–120). Flags (0/1): `powerFan`, `autoBoot`,
`wifiOptimization`, `htWifi`, `keepAwakeCharging`, `powerOn`, `sounds`, `wakeOnShake`,
`bandwidthOptimization`, `forceLogSend`, `imageOnUndock`, `alarmOnUndock`. Buttons:
`up`/`down`/`handle` (`CusButtonSetting`), `upHold`/`downHold` (`CusButtonHoldSetting`).

## Deprecated aliases

`CusRoi` → `CusRoiFunction`, `CusLine` → `CusLineF`, `CusPoint` → `CusPointF`.

## v13 preview (not yet released)

The next SDK adds (subject to change): `ignoreFirmwareMismatch` in `CusConnectionParams`;
a `newImageReadyFn` callback + `solumGetLatestImage()` pull model; an `array` argument on
`solumLoadApplication`; and `CusRawCompression` replacing the integer `lzo` flag on
`solumRequestRawData`. Build against v12 until v13 is published.
