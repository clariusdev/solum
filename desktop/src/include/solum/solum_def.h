#pragma once

#include "api_common.h"

#define CERT_INVALID    -1  ///< certificate is not valid

/// tcp connection results
typedef enum _CusConnection
{
    ConnectionError = -1,   ///< connection call error
    ProbeConnected = 0,     ///< connected to probe
    ProbeDisconnected,      ///< disconnected from probe
    ConnectionFailed,       ///< failed to connect to probe
    SwUpdateRequired,       ///< software update required

} CusConnection;

/// imaging state
typedef enum _CusImagingState
{
    ImagingNotReady,        ///< imaging is not ready, probe and application need to be loaded
    ImagingReady,           ///< imaging is ready
    CertExpired,            ///< cannot image due to expired certificate
    PoorWifi,               ///< stopped imaging due to poor wifi
    NoContact,              ///< stopped imaging due to no patient contact detected
    ChargingChanged         ///< probe started running or stopped due to change in charging status

} CusImagingState;

/// power down reason
typedef enum _CusPowerDown
{
    Idle,                   ///< probe was idle from imaging for extended period
    TooHot,                 ///< probe got too hot
    LowBattery,             ///< low battery
    ButtonOff               ///< user held button to shut down probe

} CusPowerDown;

/// software update results
typedef enum _CusSwUpdate
{
    SwUpdateError = -1,     ///< software update error
    SwUpdateSuccess = 0,    ///< successful update
    SwUpdateCurrent,        ///< software is current
    SwUpdateBattery,        ///< battery is too low to perform update
    SwUpdateCorrupt,        ///< probe file system may be corrupt

} CusSwUpdate;

/// imaging modes
typedef enum _CusMode
{
    BMode,                  ///< brightness (b) grayscale imaging mode
    Compounding,            ///< spatial compounding mode (available in most workflows)
    MMode,                  ///< motion (m) imaging mode
    ColorMode,              ///< color flow imaging mode (available in most workflows)
    PowerMode,              ///< power doppler imaging mode (available in most workflows, excluding cardiac)
    PwMode,                 ///< pulsed wave doppler imaging mode (available in most workflows)
    NeedleEnhance,          ///< needle enhance mode (available on linear probes)
    Strain,                 ///< strain elastography (available in most workflows)
    RfMode                  ///< rf capture mode (interleaved with b)

} CusMode;

/// imaging parameters
typedef enum _CusParam
{
    ImageDepth,             ///< imaging depth in [cm] (default dependent on application)
    Gain,                   ///< gain in [percent] (default 50%)
    AutoGain,               ///< auto gain enable (default on for most applications)
    DynamicRange,           ///< dynamic range in [percent] (default 50%)
    Chroma,                 ///< chroma map enable (default off)
    Smooth,                 ///< smooth enable (default on for most applications)
    Trapezoidal,            ///< trapezoidal imaging enable (available on linear probes, default off for most applications)
    ColorGain,              ///< color/power gain in [percent] (default 50%)
    ColorPrf,               ///< color/power pulse repetition frequency in [kHz] (default dependent on application)
    ColorSteer,             ///< color/power steering angle in [degrees] (available on linear probes, default dependent on application)
    ColorInvert,            ///< color map invert (default off - red towards probe, blue away from probe)
    PwGain,                 ///< pulsed wave doppler gain in [percent] (default 50%)
    PwPrf,                  ///< pulsed wave doppler pulse repetition frequency in [kHz] (default dependent on application)
    PwSteer,                ///< pulsed wave doppler steering angle in [degrees] (available on linear probes, default dependent on application)
    DopplerVelocity,        ///< read only velocity range in [cm/s] if the system is operating in a doppler mode based on the current prf programmed (returns 0 if not in a doppler mode)
    NeedleSide,             ///< needle enhance side (default 0 - led side)
    StrainOpacity,          ///< opacity of the strain image overlay (default 50%)
    RawBuffer,              ///< raw data buffering enable (default off)
    RfStreaming,            ///< rf streaming enable (default on)
    ImuStreaming,           ///< imu streaming enable (default off)
    SyncPulse,              ///< sync pulse enable (default off)
    EcoMode                 ///< eco mode enable (default off)

} CusParam;

/// fan status
typedef enum _CusFanStatus
{
    NoFan,                  ///< no fan detected
    FanDetected,            ///< fan detected
    FanRunning,             ///< fan running
    FanReversed             ///< fan placed in reversed orientation

} CusFanStatus;

/// charging status
typedef enum _CusChargingStatus
{
    NotCharging,            ///< probe is not charging
    Charging                ///< probe is charging

} CusChargingStatus;

/// button settings
typedef enum _CusButtonSetting
{
    ButtonDisabled,         ///< disable handling
    ButtonFreeze,           ///< freeze/unfreeze image
    ButtonUser              ///< send interrupt through api

} CusButtonSetting;

/// button hold settings
typedef enum _CusButtonHoldSetting
{
    ButtonHoldDisabled,     ///< disable handling
    ButtonHoldShutdown,     ///< shutdown probe
    ButtonHoldUser          ///< send interrupt through api

} CusButtonHoldSetting;

/// roi functions
typedef enum _CusRoi
{
    SizeRoi,                ///< roi resizing function (adjusts bottom/right)
    MoveRoi                 ///< roi move function (adjusts top/left)

} CusRoi;

/// tgc structure
typedef struct _CusTgc
{
    double top;             ///< top tgc in dB. valid range is -20 to 20
    double mid;             ///< mid tgc in dB. valid range is -20 to 20
    double bottom;          ///< bottom tgc in dB. valid range is -20 to 20

} CusTgc;

/// probe settings
typedef struct _CusProbeSettings
{
    int contactDetection;   ///< the number of seconds to enage a lower frame rate when no contact is detected. valid range is 0 - 30, where 0 turns the function off (default 3s)
    int autoFreeze;         ///< the number of seconds to enage freezing imaging after no contact mode has been engaged. valid range is 0 - 120, where 0 turns the function off (default 30s)
    int keepAwake;          ///< the number of minutes to power down the device once imaging has been frozen. valid range is 0 - 120, where 0 turns the function off (default 15 min)
    int deepSleep;          ///< the number of hours for and hd3 probe to go into deep sleep. valid range is 0 - 96, where 0 disables deep sleep (default 3 hours)
    int stationary;         ///< the number of seconds to enage freezing imaging after being stationary for a specific timeframe. valid range is 0 - 120, where 0 turns the function off (default 0s)
    int wifiOptimization;   ///< flag allowing the probe to automatically freeze when poor wifi connectivity is detected (default on)
    int wifiSearch;         ///< flag to force the probe to scan the networks and choose the appropriate channel before bringing up it's wi-fi (default off)
    int htWifi;             ///< flag to enable 40mhz bands for the probe's wi-fi network
    int keepAwakeCharging;  ///< flag to force the probe to stay powered while being charged (default off)
    int powerOn;            ///< flag allowing the probe's buttons to power the device on (default on)
    int sounds;             ///< flag allowing the probe to make beeping sounds (default on)
    int wakeOnShake;        ///< flag allowing the probe start imaging when it is picked up while frozen (default off)
    CusButtonSetting up;            ///< button up setting
    CusButtonSetting down;          ///< button down setting
    CusButtonHoldSetting upHold;    ///< button up hold setting
    CusButtonHoldSetting downHold;  ///< button down hold setting

} CusProbeSettings;


/// status information
typedef struct _CusStatusInfo
{
    int battery;                ///< battery level in percent
    int temperature;            ///< temperature level in percent
    double frameRate;           ///< current imaging frame rate
    CusFanStatus fan;           ///< fan status
    CusChargingStatus charger;  ///< charger status

} CusStatusInfo;

/// parameter range
typedef struct _CusRange
{
    double min;                 ///< minimum value
    double max;                 ///< maximum value

} CusRange;

/// holds the lines in pixel co-ordinates that can be used to draw an m or pw gate
typedef struct _CusGateLines
{
    CusLine active;             ///< active region for signal
    CusLine top;                ///< line above the active region
    CusLine normalTop;          ///< normal line above the active region
    CusLine normalBottom;       ///< normal line below the active region
    CusLine bottom;             ///< line below the active region

} CusGateLines;
