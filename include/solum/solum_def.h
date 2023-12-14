#pragma once

// SDK: solum
// Version: 11.1.0

#define CUS_MAXTGC  10
#define CUS_SUCCESS 0
#define CUS_FAILURE (-1)
#define CERT_INVALID (-1) ///< certificate is not valid

/// The probe buttons
typedef enum _CusButton
{
    ButtonUp,           ///< Up button
    ButtonDown,         ///< Down button
    ButtonHandle,       ///< Handle button (custom probes only)

} CusButton;

/// The compression types available
typedef enum _CusImageFormat
{
    Uncompressed,       ///< Processed images are sent in a raw and uncompressed in 32 bits ARGB
    Uncompressed8Bit,   ///< Processed images are sent in a raw and uncompressed in 8 bit grayscale
    Jpeg,               ///< Processed images are sent as a jpeg (with header)
    Png,                ///< Processed images are sent as a png (with header)

} CusImageFormat;

/// Major Clarius platforms
typedef enum _CusPlatform
{
    V1,                 ///< First generation
    HD,                 ///< Second generation (HD)
    HD3,                ///< Third generation (HD3)

} CusPlatform;

/// Button hold functions
typedef enum _CusButtonHoldSetting
{
    ButtonHoldDisabled, ///< Disable handling
    ButtonHoldShutdown, ///< Shutdown probe
    ButtonHoldUser,     ///< Send interrupt through API

} CusButtonHoldSetting;

/// Button functions
typedef enum _CusButtonSetting
{
    ButtonDisabled,     ///< Disable handling
    ButtonFreeze,       ///< Freeze/unfreeze image
    ButtonUser,         ///< Send interrupt through API

} CusButtonSetting;

/// The reset commands
typedef enum _CusProbeReset
{
    ResetKeys,          ///< reset handshake keys
    ResetWifi,          ///< reset wifi info
    ResetFactory,       ///< reset everything
    ResetTime,          ///< reset encrypted time
    ResetCert,          ///< reset stored certificates
    ResetBattery,       ///< shutdown battery
    ResetSdCard,        ///< sd card repair
    ResetPerformReboot, ///< reboot system

} CusProbeReset;

/// Charging status
typedef enum _CusChargingStatus
{
    NotCharging,        ///< Probe is not charging
    Charging,           ///< Probe is charging

} CusChargingStatus;

/// TCP connection results
typedef enum _CusConnection
{
    ConnectionError = -1, ///< Connection error
    ProbeConnected,     ///< Connected to probe
    ProbeDisconnected,  ///< Disconnected from probe
    ConnectionFailed,   ///< Failed to connect to probe
    SwUpdateRequired,   ///< Software update required

} CusConnection;

/// Fan status
typedef enum _CusFanStatus
{
    NoFan,              ///< No fan detected
    FanDetected,        ///< Fan detected
    FanRunning,         ///< Fan running

} CusFanStatus;

/// Guide status
typedef enum _CusGuideStatus
{
    NoGuide,            ///< No guide detected
    GuideDetected,      ///< Guide detected
    GuideRunning,       ///< Guide detected and powered

} CusGuideStatus;

/// Imaging state
typedef enum _CusImagingState
{
    ImagingNotReady,    ///< Imaging is not ready, probe and application need to be loaded
    ImagingReady,       ///< Imaging is ready
    CertExpired,        ///< Cannot image due to expired certificate
    PoorWifi,           ///< Stopped imaging due to poor Wi-Fi
    NoContact,          ///< Stopped imaging due to no patient contact detected
    ChargingChanged,    ///< Probe started running or stopped due to change in charging status
    LowBandwidth,       ///< Low bandwidth was detected, imaging parameters were adjusted
    MotionSensor,       ///< Probe started running or stopped due to change in motion sensor
    NoTee,              ///< Cannot image due to tee being disconnected
    TeeExpired,         ///< Cannot image due to tee being expired

} CusImagingState;

/// Logging level
typedef enum _CusLogLevel
{
    None,               ///< No logging
    Info,               ///< Info level

} CusLogLevel;

/// Imaging modes
typedef enum _CusMode
{
    BMode,              ///< Brightness (B) grayscale imaging mode
    Compounding,        ///< Spatial compounding mode (available in most workflows)
    MMode,              ///< Motion (M) imaging mode
    ColorMode,          ///< Color flow imaging mode (available in most workflows)
    PowerMode,          ///< Power Doppler imaging mode (available in most workflows, excluding cardiac)
    PwMode,             ///< Pulsed wave Doppler imaging mode (available in most workflows)
    NeedleEnhance,      ///< Needle enhance mode (available on linear probes)
    Strain,             ///< Strain elastography (available in most workflows)
    RfMode,             ///< RF capture mode (interleaved with B)

} CusMode;

/// Imaging parameters
typedef enum _CusParam
{
    ImageDepth,         ///< Imaging depth in cm (default dependent on application)
    Gain,               ///< Gain in percent (default 50%)
    AutoGain,           ///< Auto gain enable (default on for most applications)
    DynamicRange,       ///< Dynamic range in percent (default 50%)
    Chroma,             ///< Chroma map enable (default off)
    Smooth,             ///< Smooth range in percent (default 50%)
    PenetrationMode,    ///< Penetration mode enable (default off)
    AutoFocus,          ///< Auto focus enable (default on all applications)
    FocusDepth,         ///< Focus depth in cm (applied when auto focus turned off)
    Trapezoidal,        ///< Trapezoidal imaging enable (available on linear probes, default off for most applications)
    ColorGain,          ///< Color/power gain in percent (default 50%)
    ColorPrf,           ///< Color/power pulse repetition frequency in kHz (default dependent on application)
    ColorSteer,         ///< Color/power steering angle in degrees (available on linear probes, default dependent on application)
    ColorInvert,        ///< Color map invert (default off - red towards probe, blue away from probe)
    PwGain,             ///< Pulsed wave Doppler gain in [percent] (default 50%)
    PwPrf,              ///< Pulsed wave Doppler pulse repetition frequency in [kHz] (default dependent on application)
    PwSteer,            ///< Pulsed wave Doppler steering angle in [degrees] (available on linear probes, default dependent on application)
    DopplerVelocity,    ///< Read only velocity range in [cm/s] if the system is operating in a Doppler mode based on the current PRF programmed (returns 0 if not in a doppler mode)
    NeedleSide,         ///< Needle enhance side (default 0 - LED side)
    StrainOpacity,      ///< Opacity of the strain image overlay (default 50%)
    RawBuffer,          ///< Raw data buffering enable (default off)
    RfStreaming,        ///< RF streaming enable (default on)
    ImuStreaming,       ///< IMU streaming enable (default off)
    SyncPulse,          ///< Sync pulse enable (default off)
    EcoMode,            ///< Eco mode enable (default off)

} CusParam;

/// Power down reason
typedef enum _CusPowerDown
{
    Idle,               ///< Probe was idle from imaging for extended period
    TooHot,             ///< Probe got too hot
    LowBattery,         ///< Low battery
    ButtonOff,          ///< User held button to shut down probe

} CusPowerDown;

/// ROI functions
typedef enum _CusRoiFunction
{
    SizeRoi,            ///< ROI resizing function (adjusts bottom-right)
    MoveRoi,            ///< ROI move function (adjusts top-left)

} CusRoiFunction;

/// Software update results
typedef enum _CusSwUpdate
{
    SwUpdateError = -1, ///< Software update error
    SwUpdateSuccess,    ///< Successful update
    SwUpdateCurrent,    ///< Software is current
    SwUpdateBattery,    ///< Battery is too low to perform update
    SwUpdateUnsupported, ///< Firmware being sent is no longer supported
    SwUpdateCorrupt,    ///< Probe file system may be corrupt

} CusSwUpdate;

/// Wireless optimization methods
///
/// Available channels differ based on locale.
/// The standard 5Ghz 20MHz bandwidth channels include: 36, 40, 44, 48, 149, 153, 157, 161, 165.
typedef enum _CusWifiOpt
{
    WifiOptDefault,     ///< Default to the production channel of 165
    WifiOptNext,        ///< Jump immediately to the next channel
    WifiOptSearch,      ///< Scan the surrounding networks to select the best channel (takes up to 10 seconds to search and optimize)

} CusWifiOpt;

/// TGC information
typedef struct _CusTgcInfo
{
    double depth;       ///< Depth in millimeters
    double gain;        ///< Gain in decibels

} CusTgcInfo;

/// Positional data information structure
typedef struct _CusPosInfo
{
    long long int tm;   ///< Timestamp in nanoseconds
    double gx;          ///< Gyroscope x; angular velocity is given in radians per second (RPS)
    double gy;          ///< Gyroscope y; angular velocity is given in radians per second (RPS)
    double gz;          ///< Gyroscope z; angular velocity is given in radians per second (RPS)
    double ax;          ///< Accelerometer x; acceleration is normalized to gravity [~9.81m/s^2] (m/s^2)/(m/s^2)
    double ay;          ///< Accelerometer y; acceleration is normalized to gravity [~9.81m/s^2] (m/s^2)/(m/s^2)
    double az;          ///< Accelerometer z; acceleration is normalized to gravity [~9.81m/s^2] (m/s^2)/(m/s^2)
    double mx;          ///< Magnetometer x; magnetic flux density is normalized to the earth's field [~50 mT] (T/T)
    double my;          ///< Magnetometer y; magnetic flux density is normalized to the earth's field [~50 mT] (T/T)
    double mz;          ///< Magnetometer z; magnetic flux density is normalized to the earth's field [~50 mT] (T/T)
    double qw;          ///< W component (real) of the orientation quaternion
    double qx;          ///< X component (imaginary) of the orientation quaternion
    double qy;          ///< Y component (imaginary) of the orientation quaternion
    double qz;          ///< Z component (imaginary) of the orientation quaternion

} CusPosInfo;

/// Probe information
typedef struct _CusProbeInfo
{
    int version;        ///< Version (1 = clarius 1st generation, 2 = clarius HD, 3 = clarius HD3)
    int elements;       ///< Number of probe elements
    int pitch;          ///< Element pitch
    int radius;         ///< Radius in millimeters

} CusProbeInfo;

/// Processed image information supplied with each frame
typedef struct _CusProcessedImageInfo
{
    int width;          ///< width of the image in pixels
    int height;         ///< height of the image in pixels
    int bitsPerPixel;   ///< bits per pixel
    int imageSize;      ///< total size of image in bytes
    double micronsPerPixel; ///< microns per pixel (always 1:1 aspect ratio axially/laterally)
    double originX;     ///< image origin in microns in the horizontal axis
    double originY;     ///< image origin in microns in the vertical axis
    long long int tm;   ///< timestamp of images
    double angle;       ///< acquisition angle for volumetric data
    double fps;         ///< frame rate in hz
    int overlay;        ///< flag that the image is an overlay without grayscale (ie. color doppler or strain)
    CusImageFormat format; ///< flag specifying the format of the image (see format definitions above)
    CusTgcInfo tgc [CUS_MAXTGC]; ///< tgc points

} CusProcessedImageInfo;

/// Raw image information supplied with each frame
typedef struct _CusRawImageInfo
{
    int lines;          ///< number of ultrasound lines in the image
    int samples;        ///< number of samples per line in the image
    int bitsPerSample;  ///< bits per sample
    double axialSize;   ///< axial microns per sample
    double lateralSize; ///< lateral microns per line
    long long int tm;   ///< timestamp of image
    int jpeg;           ///< size of the jpeg image, 0 if not a jpeg compressed image
    int rf;             ///< flag specifying data is rf and not envelope
    double angle;       ///< acquisition angle for volumetric data
    double fps;         ///< frame rate in hz
    CusTgcInfo tgc [CUS_MAXTGC]; ///< tgc points

} CusRawImageInfo;

/// Spectral image information supplied with each block
typedef struct _CusSpectralImageInfo
{
    int lines;          ///< Number of lines in the block
    int samples;        ///< Number of samples per line
    int bitsPerSample;  ///< Bits per sample
    double period;      ///< Line acquisition period in seconds
    double micronsPerSample; ///< Microns per pixel/sample in an M spectrum
    double velocityPerSample; ///< Velocity in m/s per pixel/sample in a PW spectrum
    int pw;             ///< Flag specifying the data is PW and not M

} CusSpectralImageInfo;

/// 2D point with double precision
typedef struct _CusPointF
{
    double x;           ///< X coordinate
    double y;           ///< Y coordinate

} CusPointF;

/// 2D line with double precision
typedef struct _CusLineF
{
    CusPointF p1;       ///< First point in the line
    CusPointF p2;       ///< Second point in the line

} CusLineF;

/// Parameter range
typedef struct _CusRange
{
    double min;         ///< Minimum value
    double max;         ///< Maximum value

} CusRange;

/// TGC values to send to the API
typedef struct _CusTgc
{
    double top;         ///< Top tgc in dB. valid range is -20 to 20
    double mid;         ///< Mid tgc in dB. valid range is -20 to 20
    double bottom;      ///< Bottom tgc in dB. valid range is -20 to 20

} CusTgc;

/// Status information
typedef struct _CusStatusInfo
{
    int battery;        ///< Battery level in percent
    int temperature;    ///< Temperature level in percent
    double frameRate;   ///< Current imaging frame rate
    double teeTimeRemaining; ///< Current time remaining for tee imaging
    CusFanStatus fan;   ///< Fan status
    CusGuideStatus guide; ///< Guide status
    CusChargingStatus charger; ///< Charger status

} CusStatusInfo;

/// Probe settings
typedef struct _CusProbeSettings
{
    int contactDetection; ///< The number of seconds to engage a lower frame rate when no contact is detected, valid range is 0 - 30, where 0 turns the function off
    int autoFreeze;     ///< The number of seconds to engage freezing imaging after no contact mode has been engaged, valid range is 0 - 120, where 0 turns the function off
    int keepAwake;      ///< The number of minutes to power down the device once imaging has been frozen, valid range is 0 - 120, where 0 turns the function off
    int deepSleep;      ///< The number of hours for probe to go into deep sleep, valid range is 0 - 96, where 0 disables deep sleep
    int stationary;     ///< The number of seconds to engage freezing imaging after being stationary for a specific time frame
    int wifiOptimization; ///< Flag allowing the probe to automatically freeze when poor wifi connectivity is detected
    int wifiSearch;     ///< Flag to force the probe to scan the networks and choose the appropriate channel before bringing up its Wi-Fi
    int htWifi;         ///< Flag to enable 40 MHz bands for the probe's Wi-Fi network
    int keepAwakeCharging; ///< Flag to force the probe to stay powered while being charged
    int powerOn;        ///< Flag allowing the probe's buttons to power the device on
    int sounds;         ///< Flag allowing the probe to make beeping sounds
    int wakeOnShake;    ///< Flag allowing the probe to start imaging when it is picked up while frozen
    int bandwidthOptimization; ///< Flag allowing the system to adjust bandwidth parameters automatically when lag or dropped frames are determined
    CusButtonSetting up; ///< Button up setting
    CusButtonSetting down; ///< Button down setting
    CusButtonSetting handle; ///< Button handle setting
    CusButtonHoldSetting upHold; ///< Button up hold setting
    CusButtonHoldSetting downHold; ///< Button down hold setting

} CusProbeSettings;

/// Gate coordinates
///
/// Hold the lines in pixel coordinates that can be used to draw an M or PW gate.
typedef struct _CusGateLines
{
    CusLineF active;    ///< Active region for signal
    CusLineF top;       ///< Line above the active region
    CusLineF normalTop; ///< Normal line above the active region
    CusLineF normalBottom; ///< Normal line below the active region
    CusLineF bottom;    ///< Line below the active region

} CusGateLines;

/// SDK configuration
typedef struct _CusConfig
{
    CusLogLevel logLevel; ///< Logging level

} CusConfig;

#ifndef SOLUM_DEPRECATED
#  ifdef _MSC_VER
#    define SOLUM_DEPRECATED __declspec(deprecated)
#  else
#    define SOLUM_DEPRECATED __attribute__ ((__deprecated__))
#  endif
#endif

SOLUM_DEPRECATED typedef enum _CusRoiFunction CusRoi;
SOLUM_DEPRECATED typedef struct _CusLineF CusLine;
SOLUM_DEPRECATED typedef struct _CusPointF CusPoint;
