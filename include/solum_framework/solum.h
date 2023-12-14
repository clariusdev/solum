#pragma once

#import <Foundation/Foundation.h>

#include "solum_def.h"

/// initialization callback function
/// @param[in] result the initialization success
typedef void (^CusInitializeFn)(BOOL result);
//! generic boolean callback function
/// @param[in] result true on success, false on failure
typedef void (^CusBooleanFn)(BOOL result);
/// connection callback function
/// @param[in] res the connection result
/// @param[in] port udp port used for streaming
/// @param[in] status the status message
typedef void (^CusConnectFn)(CusConnection res, int port, NSString * _Nonnull status);
/// certification callback function
/// @param[in] daysValid # of days valid for certificate
typedef void (^CusCertFn)(int daysValid);
/// powering down callback function
/// @param[in] res the power down reason
/// @param[in] tm time for when probe is powering down, 0 for immediately
typedef void (^CusPowerDownFn)(CusPowerDown res, int tm);
/// software update callback function
/// @param[in] res the software update result
typedef void (^CusSwUpdateFn)(CusSwUpdate res);
/// new data callback function
/// @param[in] img pointer to the new grayscale image information
/// @param[in] nfo image information associated with the image data
/// @param[in] npos number of positional information data tagged with the image
/// @param[in] pos the positional information data tagged with the image
typedef void (^CusNewRawImageFn)(NSData * _Nonnull img, const CusRawImageInfo * _Nonnull nfo, int npos, const CusPosInfo * _Nonnull pos);
/// new image callback function
/// @param[in] img pointer to the new grayscale image information
/// @param[in] nfo image information associated with the image data
/// @param[in] npos number of positional information data tagged with the image
/// @param[in] pos the positional information data tagged with the image
typedef void (^CusNewProcessedImageFn)(NSData * _Nonnull img, const CusProcessedImageInfo * _Nonnull nfo, int npos, const CusPosInfo * _Nonnull pos);
/// new spectral image callback function
/// @param[in] img pointer to the new grayscale image information
/// @param[in] nfo image information associated with the image data
typedef void (^CusNewSpectralImageFn)(NSData * _Nonnull img, const CusSpectralImageInfo * _Nonnull nfo);
/// new imu data callback function
/// @param[in] pos the positional information data tagged with the image
typedef void (^CusNewImuDataFn)(const CusPosInfo * _Nonnull pos);
/// imaging callback function
/// @param[in] state the imaging ready state
/// @param[in] imaging 1 = running , 0 = stopped
typedef void (^CusImagingFn)(CusImagingState state, int imaging);
/// button callback function
/// @param[in] btn the button that was pressed
/// @param[in] clicks # of clicks performed
typedef void (^CusButtonFn)(CusButton btn, int clicks);
/// firmware callback function
/// @param[in] firmware the firmware version for the given platform, empty if an error occurred
typedef void (^CusFirmwareFn)(NSString * _Nullable firmware);
/// progress callback function
/// @param[in] progress the current progress in percent
typedef void (^CusProgressFn)(int progress);
/// raw data size callback function
/// @param[in] res the size of the data package requested or actually downloaded or -1 if an error occurred
typedef void (^CusRawFn)(int res);
/// raw data callback function
/// @param[in] res the raw data result, the size of the data package or -1 if an error occurred
/// @param[in] data the raw data
typedef void (^CusRawDataFn)(int res, NSData * _Nullable data);
/// error callback function
/// @param[in] msg the error message with associated error that occurred
typedef void (^CusErrorFn)(NSString * _Nonnull msg);
/// mode callback function
/// @param[in] mode the current imaging mode
typedef void (^CusModeFn)(CusMode mode);
/// ROI callback function
/// @param[in] points an array of CGPoint/NSPoint stored as NSValue
typedef void (^CusRoiFn)(NSArray* _Nullable points);
/// TGC callback function
/// @param[in] tgc the TGC
typedef void (^CusTgcFn)(const CusTgc* _Nullable tgc);
/// range callback function
/// @param[in] range the parameter range
typedef void (^CusRangeFn)(const CusRange* _Nullable range);
/// param callback function
/// @param[in] value the parameter value, or nil if the value cannot be retrieved
typedef void (^CusParamFn)(NSNumber* _Nullable value);
/// status callback function
/// @param[in] status the probe status
typedef void (^CusStatusFn)(const CusStatusInfo* _Nullable status);
/// probe info callback function
/// @param[in] info the probe info
typedef void (^CusProbeInfoFn)(const CusProbeInfo* _Nullable info);
/// low level param callback function
/// @param[in] value the parameter value, or nil if the value cannot be retrieved
typedef void (^CusLowLevelParamFn)(NSNumber* _Nullable value);
/// gate lines callback function
/// @param[in] lines the gate lines for the current mode, or nil if the gate lines cannot be retrieved
typedef void (^CusGateFn)(const CusGateLines* _Nullable lines);
/// tee connection callback function
/// @param[in] connected flag associated with the tee having a disposable probe connection
/// @param[in] serial if a probe is connected, the serial number of the probe
/// @param[in] timeRemaining if a probe is connected, the time remaining in percentage
/// @param[in] id patient id if burned in to the probe
/// @param[in] name patient name if burned in to the probe
/// @param[in] exam exam id if burned in to the probe
typedef void (^CusTeeConnectFn)(BOOL connected, NSString * _Nonnull serial, double timeRemaining, NSString * _Nonnull id, const NSString * _Nonnull name, const NSString * _Nonnull exam);

//! @brief Solum interface
__attribute__((visibility("default"))) @interface Solum : NSObject

//! @brief initializes the class to enable connection.
//! @param[in] dir the directory to store security keys
/// @param[in] callback the callback function that reports success/failure
- (void)initialize:(NSString * _Nonnull)dir
        callback:(CusInitializeFn _Nonnull)callback;

//! @brief checks whether the class has been initialized.
//! @return YES if the class has been initialized.
- (BOOL)isInitialized;

/// connects to a probe that is on the same network as the caller
/// @param[in] address the ip address of the probe
/// @param[in] port the probe's tcp port to connect to
- (void)connect:(NSString * _Nonnull)address
        port:(unsigned int)port;

/// disconnects from an existing connection
- (void)disconnect;

/// retrieves the current connected state of the module
/// @return the connected state of the module
- (BOOL)isConnected;

/// retrieves the firmware version for a given platform
/// @param[in] platform the platform to retrieve the firmware version for
/// @param[in] callback callback to receive the firmware version
- (void) getFirmwareVersion:(CusPlatform) platform
        callback:(CusFirmwareFn _Nonnull)callback;

/// sets the certificate for the probe to be connected with
/// @param[in] cert the certificate provided by clarius
- (void)setCertificate:(NSString * _Nonnull)cert;

/// performs a software update once connected
///
/// Solum attempts to detect the connected probe's platform to select the appropriate firmware.
/// It is possible the auto-detection fails with error "undetermined hardware version", preventing the update.
/// In that case try to force the update by specifying the platform.
///
/// @param[in] filePath path to the firmware update file
/// @param[in] callback the callback function that reports the status
/// @param[in] progress software update progress callback
- (void)softwareUpdate:(NSString * _Nonnull)filePath
        callback:(CusSwUpdateFn _Nonnull)callback
        progress:(CusProgressFn _Nullable)progress;

/// force a software update for the given platform
///
/// Solum attempts to detect the connected probe's platform to select the appropriate firmware.
/// It is possible the auto-detection fails, preventing the update, in that case Solum will use the supplied platform parameter.
///
/// - If auto-detection succeeds, Solum uses the firmware for the detected platform and ignores the supplied platform parameter
/// - If auto-detection fails and a platform parameter was specified, Solum uses the firmware for the specified platform
///
/// WARNING: updating a probe with the incorrect firmware could brick it.
///
/// Recommended usage:
///
/// 1. Attempt an update without specifying a platform first (let Solum select the appropriate firmware)
/// 2. Only if the update fails, try specifying a platform
///
/// @param[in] filePath path to the firmware update file
/// @param[in] callback the callback function that reports the status
/// @param[in] progress software update progress callback
/// @param[in] platform upload the firmware for this platform
- (void)softwareUpdate:(NSString * _Nonnull)filePath
        callback:(CusSwUpdateFn _Nonnull)callback
        progress:(CusProgressFn _Nullable)progress
        platform:(CusPlatform)platform;

/// retrieves the available probe models the api supports
/// @return the list of probe models
- (NSArray<NSString*>* _Nonnull)probes;

/// retrieves the available applications for a specific probe model
/// @param[in] probe the probe model to retrieve applications for
/// @return the list of applications for that probe
- (NSArray<NSString*>* _Nonnull)applications:(NSString * _Nonnull)probe;

/// loads an application with the given probe model
/// @param[in] application the application to load
/// @param[in] probe the probe model to load
- (void)loadApplication: (NSString * _Nonnull)application
        probe:(NSString * _Nonnull)probe;

/// sets the dimensions of the output display for scan conversion
/// @param[in] width the number of horizontal pixels in the output
/// @param[in] height the number of vertical pixels in the output
/// @note the output will always result in a 1:1 pixel ratio, depending on geometry of scanning array, and parameters
///       the frame will have various sizes of black borders around the image
- (void)setOutputWidth:(int)width
        withHeight:(int)height;

/// sets a flag to separate overlays into separate images, for example if color/power Doppler or strain
/// imaging is enabled, two callbacks will be generated, one with the grayscale frame, and the other with the overlay
/// @param[in] enable the enable flag for separating overlays
- (void)separateOverlays:(BOOL)enable;

/// runs or stops imaging
/// @param[in] run the run state to set, NO to stop imaging, YES to start imaging
- (void)run:(BOOL)run;

/// tries to power down the probe if actively connected
- (void)powerDown;

/// retrieves the probe status
/// @param[in] callback callback to receive the status
- (void)getStatus:(CusStatusFn _Nonnull)callback;

/// retrieves the probe info
/// @param[in] callback callback to receive the probe info
- (void)getProbeInfo:(CusProbeInfoFn _Nonnull)callback;

/// sets the internal probe settings to be applied upon a connection or when an existing connection exists
/// @param[in] settings the structure containing the probe settings
- (void)setProbeSettings:(const CusProbeSettings * _Nonnull)settings;

/// sets an imaging parameter
/// @param[in] param the parameter to set
/// @param[in] value the value to set the parameter to
- (void)setParam:(CusParam)param
        value:(double)value;

/// retrieves an imaging parameter value
/// @param[in] callback callback to receive the value (nil if the parameter value retrieval could not be made)
/// @param[in] param the parameter to retrieve the value for
- (void)getParam:(CusParamFn _Nonnull)callback
        param:(CusParam)param;

/// retrieves the range for a specific parameter
/// @param[in] callback callback to receive the range
/// @param[in] param the parameter to retrieve the range for
- (void)getRange:(CusRangeFn _Nonnull)callback
        param:(CusParam)param;

/// sets the TGC
/// @param[in] tgc the values to use for the TGC
- (void)setTgc:(const CusTgc* _Nonnull)tgc;

/// retrieves the TGC values
/// @param[in] callback callback to receive the TGC
- (void)getTgc:(CusTgcFn _Nonnull)callback;

/// retrieves the ROI for the current mode if valid
/// @param[in] callback callback to receive the array of points
/// @param[in] count the number of points to generate (points buffer must be count x 2 or larger)
- (void)getRoi:(CusRoiFn _Nonnull)callback
        count:(int)count;

/// moves the top/left of the ROI to a specific point
/// @param[in] fn ROI function
/// @param[in] x the pixel x position
/// @param[in] y the pixel y position
- (void)adjustRoi:(CusRoiFunction)fn
        x:(double)x
        y:(double)y;

/// maximizes the ROI
- (void)maximizeRoi;

/// sets an imaging mode
/// @param[in] mode the imaging mode to set
- (void)setMode:(CusMode)mode;

/// retrieves the current imaging mode
/// @param callback function to call with the current imaging mode
- (void)getMode:(CusModeFn _Nonnull)callback;

/// enables the 5v output on or off
/// @param[in] en the enable state, set to 1 to turn 5v on, or 0 to turn off
- (void)enable5v:(BOOL)en;

/// sets the format for processed images, by default the format will be uncompressed argb
/// @param[in] format the format of the image
- (void)setFormat:(CusImageFormat)format;

/// makes a request for raw data from the probe
/// @param[in] start the first frame to request, as determined by timestamp in nanoseconds, set to 0 along with end to requests all data in buffer
/// @param[in] end the last frame to request, as determined by timestamp in nanoseconds, set to 0 along with start to requests all data in buffer
/// @param[in] res result callback function, will return size of buffer required upon success, 0 if no raw data was buffered, or -1 if request could not be made,
/// @note the probe must be frozen and in a raw data buffering mode in order for the call to succeed
- (void)requestRawData:(CusRawFn _Nonnull)res
        start:(long long int)start
        end:(long long int)end;

/// retrieves raw data from a previous request
/// @param[in] res result callback function, will return size of buffer required upon success, 0 if no raw data was buffered, or -1 if request could not be made,
/// @param[in] progress download progress callback function that outputs the progress in percent
/// @note the probe must be frozen and a successful call to requestRawData must have taken place in order for the call to succeed
- (void)readRawData:(CusRawDataFn _Nonnull)res
        progress:(CusProgressFn _Nullable)progress;

/// will try and optimize the wireless channel when the probe is running its own network
/// the function will return a failure if the probe is on an external wlan as nothing can be optimized, except for switching over to the probe's own network
/// to switch to the probe's network, see the bluetooth documentation for the wireless management service
/// @param[in] wifiOpt the optimization type to run
/// @param[in] callback callback to receive the call result
- (void)optimizeWifi:(CusWifiOpt)wifiOpt
        callback:(CusBooleanFn _Nonnull)callback;

/// performs a reset of a function on the probe
/// @param[in] probeReset the reset type to perform
/// @param[in] callback callback to receive the call result
- (void)resetProbe:(CusProbeReset)probeReset
        callback:(CusBooleanFn _Nonnull)callback;

- (void)getGate:(CusGateFn _Nonnull)callback;

/// adjusts the gate based on the input provided
/// @param[in] x the horizontal pixel position
/// @param[in] y the vertical pixel position
- (void)adjustGate:(double)x
        y:(double)y;

/// sets a low level parameter to a specific value to gain access to lower level device control
/// @param[in] prm the parameter to change
/// @param[in] val the value to set the parameter to
/// @note see external documentation for supported parameters
- (void)setLowLevelParam:(NSString * _Nonnull)prm
        val:(double)val;

/// enables or disables a low level parameter to gain access to lower device control
/// @param[in] prm the parameter to change
/// @param[in] en the enable flag, 0 to disable, 1 to enable
/// @note see external documentation for supported parameters
- (void)enableLowLevelParam:(NSString * _Nonnull)prm
        val:(BOOL)en;

/// sets a pulse shape parameter to a specific value to gain access to lower level device control
/// @param[in] prm the parameter to change
/// @param[in] shape the shape to set the pulse as
/// @note see external documentation for supported parameters
- (void)setLowLevelPulse:(NSString * _Nonnull)prm
        shape:(NSString * _Nonnull)shape;

/// retrieves a low level parameter value
/// @param[in] prm the parameter to retrieve the value for
/// @param[in] callback callback to receive the call result
- (void)getLowLevelParam:(NSString * _Nonnull)prm
        callback:(CusLowLevelParamFn _Nonnull)callback;

/// sets a callback for the tee connectivity function
/// @param[in] tee the callback function
- (void)setTeeFn:(CusTeeConnectFn _Nonnull)tee;

/// set the tee exam info for a connected probe
/// @param[in] idStr the patient id
/// @param[in] name the patient name
/// @param[in] exam the exam id
- (void)setTeeExamInfo:(NSString * _Nonnull)idStr
        name:(NSString * _Nonnull)name
        exam:(NSString * _Nonnull)exam;

- (void)setConnectCallback:(CusConnectFn _Nullable)connectCallback;
- (void)setCertCallback:(CusCertFn _Nullable)certCallback;
- (void)setPowerDownCallback:(CusPowerDownFn _Nullable)powerDownCallback;
- (void)setNewRawImageCallback:(CusNewRawImageFn _Nullable)newRawImageCallback;
- (void)setNewProcessedImageCallback:(CusNewProcessedImageFn _Nullable)newProcessedImageCallback;
- (void)setNewSpectralImageCallback:(CusNewSpectralImageFn _Nullable)newSpectralImageCallback;
- (void)setNewImuDataCallback:(CusNewImuDataFn _Nullable)newImuDataCallback;
- (void)setImagingCallback:(CusImagingFn _Nullable)imagingCallback;
- (void)setButtonCallback:(CusButtonFn _Nullable)buttonCallback;
- (void)setErrorCallback:(CusErrorFn _Nullable)errorCallback;

@end
