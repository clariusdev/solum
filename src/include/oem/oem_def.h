#pragma once

#define CERT_INVALID        -1  ///< certificate is not valid

/// image formats
typedef enum _CusImageFormat
{
    Uncompressed,   ///< processed images are sent in a raw and uncompressed in 32 bit argb
    Jpeg,           ///< processed images are sent as a jpeg
    Png             ///< processed images are sent as a png

} CusImageFormat;

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
    ImagingNotReady,    ///< imaging is not ready, probe and application need to be loaded
    ImagingReady,       ///< imaging is ready
    CertExpired,        ///< cannot image due to expired certificate
    PoorWifi,           ///< stopped imaging due to poor wifi
    NoContact,          ///< stopped imaging due to no patient contact detected
    ChargingChanged     ///< probe started running or stopped due to change in charging status

} CusImagingState;

/// power down reason
typedef enum _CusPowerDown
{
    Idle,       ///< probe was idle from imaging for extended period
    TooHot,     ///< probe got too hot
    LowBattery, ///< low battery
    ButtonOff   ///< user held button to shut down probe

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

/// imaging parameters
typedef enum _CusParam
{
    ImageDepth,     ///< imaging depth in cm
    Gain,           ///< gain in percent
    AutoGain,       ///< auto gain enable
    DynamicRange,   ///< dynamic range in percent
    Chroma,         ///< chroma map enable
    Trapezoidal,    ///< trapezoidal imaging enable (available on linear probes)
    ColorGain,      ///< color/power gain in percent
    ColorPrf,       ///< color/power pulse repetition frequency in kHz
    ColorSteer,     ///< color/power steering angle in degrees
    ColorInvert,    ///< color map invert
    NeedleSide,     ///< needle enhance side
    RawBuffer,      ///< raw data buffering enable
    RfStreaming,    ///< rf streaming enable
    ImuStreaming,   ///< imu streaming enable
    SyncPulse,      ///< sync pulse enable
    EcoMode         ///< eco mode enable

} CusParam;

/// imaging modes
typedef enum _CusMode
{
    BMode,          ///< b/grayscale imaging mode
    Compounding,    ///< spatial compounding mode (available in most workflows)
    ColorMode,      ///< color flow imaging mode (available in most workflows)
    PowerMode,      ///< power doppler imaging mode (available in most workflows)
    NeedleEnhance,  ///< needle enhance mode (available on linear probes)
    RfMode,         ///< rf capture mode (interleaved with b)

} CusMode;

/// fan status
typedef enum _CusFanStatus
{
    NoFan,          ///< no fan detected
    FanDetected,    ///< fan detected
    FanRunning,     ///< fan running
    FanReversed     ///< fan placed in reversed orientation

} CusFanStatus;

/// charging status
typedef enum _CusChargingStatus
{
    NotCharging,    ///< probe is not charging
    Charging        ///< probe is charging

} CusChargingStatus;

/// physical buttons
typedef enum _CusButton
{
    ButtonUp,       ///< up button
    ButtonDown,     ///< down button
    ButtonHandle    ///< handle button (custom probes only)

} CusButton;

/// button settings
typedef enum _CusButtonSetting
{
    ButtonDisabled, ///< disable handling
    ButtonFreeze,   ///< freeze/unfreeze image
    ButtonUser      ///< send interrupt through api

} CusButtonSetting;

/// roi functions
typedef enum _CusRoi
{
    SizeRoi,    ///< roi resizing function (adjusts bottom/right)
    MoveRoi     ///< roi move function (adjusts top/left)

} CusRoi;

/// tgc structure
typedef struct _CusTgc
{
    double top;     ///< top tgc in dB
    double mid;     ///< mid tgc in dB
    double bottom;  ///< bottom tgc in dB

} CusTgc;

/// raw image information supplied with each frame
typedef struct _CusRawImageInfo
{
    int lines;              ///< number of ultrasound lines in the image
    int samples;            ///< number of samples per line in the image
    int bitsPerSample;      ///< bits per sample
    double axialSize;       ///< axial microns per sample
    double lateralSize;     ///< lateral microns per line
    long long int tm;       ///< timestamp of image
    int jpeg;               ///< size of the jpeg image, 0 if not a jpeg compressed image
    int rf;                 ///< flag specifying data is rf and not envelope

} CusRawImageInfo;

/// processed image information supplied with each frame
typedef struct _CusProcessedImageInfo
{
    int width;                  ///< width of the image in pixels
    int height;                 ///< height of the image in pixels
    int bitsPerPixel;           ///< bits per pixel
    int imageSize;              ///< total size of image
    double micronsPerPixel;     ///< microns per pixel (always 1:1 aspect ratio axially/laterally)
    double originX;             ///< image origin in microns in the horizontal axis
    double originY;             ///< image origin in microns in the vertical axis
    long long int tm;           ///< timestamp of images
    int overlay;                ///< flag that the image is an overlay without grayscale (ie. color doppler or strain)
    CusImageFormat format;      ///< flag specifying the format of the image

} CusProcessedImageInfo;

/// spectral image information supplied with each block
typedef struct _CusSpectralImageInfo
{
    int lines;                  ///< number of lines in the block
    int samples;                ///< number of samples per line
    int bitsPerSample;          ///< bits per sample
    double period;              ///< line acquisition period in seconds
    double micronsPerSample;    ///< microns per pixel/sample in an m spectrum
    double velocityPerSample;   ///< velocity in m/s per pixel/sample in a pw spectrum
    int pw;                     ///< flag specifying the data is pw and not m

} CusSpectralImageInfo;

/// status information
typedef struct _CusStatusInfo
{
    int battery;                ///< battery level in percent
    int temperature;            ///< temperature level in percent
    double frameRate;           ///< current imaging frame rate
    CusFanStatus fan;           ///< fan status
    CusChargingStatus charger;  ///< charger status

} CusStatusInfo;

/// probe information
typedef struct _CusProbeInfo
{
    int version;    ///< version (1 = clarius 1st Generation, 2 = clarius HD, 3 = clarius HD3)
    int elements;   ///< # of probe elements
    int pitch;      ///< element pitch
    int radius;     ///< radius in mm

} CusProbeInfo;

/// probe settings
typedef struct _CusProbeSettings
{
    int contactDetection;   ///< the number of seconds to enage a lower frame rate when no contact is detected. valid range is 0 - 30, where 0 turns the function off
    int autoFreeze;         ///< the number of seconds to enage freezing imaging after no contact mode has been engaged. valid range is 0 - 120, where 0 turns the function off
    int keepAwake;          ///< the number of minutes to power down the device once imaging has been frozen. valid range is 0 - 120, where 0 turns the function off
    int wifiOptimization;   ///< flag allowing the probe to automatically freeze when poor wifi connectivity is detected
    CusButtonSetting up;    ///< button up setting
    CusButtonSetting down;  ///< button down setting

} CusProbeSettings;

/// positional data information structure
typedef struct _CusPosInfo
{
    long long int tm;   ///< timestamp in nanoseconds
    double gx;          ///< gyroscope x; angular velocity is given in radians per second (rps)
    double gy;          ///< gyroscope y
    double gz;          ///< gyroscope z
    double ax;          ///< accelerometer x; acceleration is normalized to gravity [~9.81m/s^2] (m/s^2)/(m/s^2)
    double ay;          ///< accelerometer y
    double az;          ///< accelerometer z
    double mx;          ///< magnetometer x; magnetic flux density is normalized to the earth's field [~50 mT] (T/T)
    double my;          ///< magnetometer y
    double mz;          ///< magnetometer z
    double qw;          ///< w component (real) of the orientation quaternion
    double qx;          ///< x component (imaginary) of the orientation quaternion
    double qy;          ///< y component (imaginary) of the orientation quaternion
    double qz;          ///< z component (imaginary) of the orientation quaternion

} CusPosInfo;

/// string list callback function
/// @param[in] list the string list
/// @param[in] sz the size of the string buffer
typedef void (*CusListFn)(const char* list, int sz);
/// connection callback function
/// @param[in] res the connection result
/// @param[in] port udp port used for streaming
/// @param[in] status the status message
typedef void (*CusConnectFn)(CusConnection res, int port, const char* status);
/// certification callback function
/// @param[in] daysValid # of days valid for certificate
typedef void (*CusCertFn)(int daysValid);
/// powering down callback function
/// @param[in] res the power down reason
/// @param[in] tm time for when probe is powering down, 0 for immediately
typedef void (*CusPowerDownFn)(CusPowerDown res, int tm);
/// software update callback function
/// @param[in] res the software update result
typedef void (*CusSwUpdateFn)(CusSwUpdate res);
/// new data callback function
/// @param[in] img pointer to the new grayscale image information
/// @param[in] nfo image information associated with the image data
/// @param[in] npos number of positional information data tagged with the image
/// @param[in] pos the positional information data tagged with the image
typedef void (*CusNewRawImageFn)(const void* img, const CusRawImageInfo* nfo, int npos, const CusPosInfo* pos);
/// new image callback function
/// @param[in] img pointer to the new grayscale image information
/// @param[in] nfo image information associated with the image data
/// @param[in] npos number of positional information data tagged with the image
/// @param[in] pos the positional information data tagged with the image
typedef void (*CusNewProcessedImageFn)(const void* img, const CusProcessedImageInfo* nfo, int npos, const CusPosInfo* pos);
/// new spectral image callback function
/// @param[in] img pointer to the new grayscale image information
/// @param[in] nfo image information associated with the image data
typedef void (*CusNewSpectralImageFn)(const void* img, const CusSpectralImageInfo* nfo);
/// imaging callback function
/// @param[in] state the imaging ready state
/// @param[in] imaging 1 = running , 0 = stopped
typedef void (*CusImagingFn)(CusImagingState state, int imaging);
/// button callback function
/// @param[in] btn the button that was pressed
/// @param[in] clicks # of clicks performed
typedef void (*CusButtonFn)(CusButton btn, int clicks);
/// progress callback function
/// @param[in] progress the current progress in percent
typedef void (*CusProgressFn)(int progress);
/// raw data callback function
/// @param[in] res the raw data result, typically the size of the data package requested or actually downloaded
typedef void (*CusRawFn)(int res);
/// error callback function
/// @param[in] msg the error message with associated error that occurred
typedef void (*CusErrorFn)(const char* msg);
