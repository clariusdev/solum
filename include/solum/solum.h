#pragma once

#include "solum_export.h"
#include "solum_cb.h"

/// probe connection parameters for solumConnect
typedef struct _CusConnectionParams
{
    const char* ipAddress;      ///< the ip address of the probe
    unsigned int port;          ///< the probe's tcp port to connect to
    long long int networkId;    ///< the wifi network id obtained when auto-joining wifi on android. optional, 0 by default

} CusConnectionParams;

/// initialization parameters for solumInit
typedef struct _CusInitParams
{
    struct Args
    {
        int argc;       ///< the argument count for input parameters to pass to the library
        char** argv;    ///< the arguments to pass to the library, possibly required for qt graphics buffer initialization
    }
    args;
    const char* storeDir;                       ///< the directory to store security keys
    CusConnectFn connectFn;                     ///< connection status callback
    CusCertFn certFn;                           ///< certificate status callback
    CusPowerDownFn powerDownFn;                 ///< probe power down callback
    CusImagingFn imagingFn;                     ///< imaging state callback
    CusButtonFn buttonFn;                       ///< button press callback
    CusErrorFn errorFn;                         ///< error message callback
    CusNewProcessedImageFn newProcessedImageFn; ///< new processed image callback (scan-converted image)
    CusNewRawImageFn newRawImageFn;             ///< new raw image callback (pre scan-converted image or rf data)
    CusNewSpectralImageFn newSpectralImageFn;   ///< new processed spectral image callback
    CusNewImuPortFn newImuPortFn;               ///< new imu udp port callback
    CusNewImuDataFn newImuDataFn;               ///< new imu data callback
    int width;                                  ///< the width of the output buffer
    int height;                                 ///< the height of the output buffer

} CusInitParams;

#ifdef __cplusplus
extern "C" {
#endif

    /// initializes the solum module
    /// @param[in] params the sdk configuration parameters
    /// @return success of the call
    /// @retval 0 the initialization was successful
    /// @retval -1 the initialization was not successful
    /// @note must be called before any other functions will succeed
    SOLUM_EXPORT int solumInit(const CusInitParams* params);

    /// get init params with default values
    /// @return a zero initialized struct
    SOLUM_EXPORT CusInitParams solumDefaultInitParams(void);

    /// cleans up memory allocated by the solum module
    /// @retval 0 the destroy attempt was successful
    /// @retval -1 the destroy attempt was not successful
    /// @note should be called prior to exiting the application
    SOLUM_EXPORT int solumDestroy(void);

    /// sets a callback for the tee connectivity function
    /// @param[in] tee the callback function
    /// @return success of the call
    /// @retval 0 the function was successful
    /// @retval -1 the function was not successful
    SOLUM_EXPORT int solumSetTeeFn(CusTeeConnectFn tee);

    /// retrieves the firmware version for a given platform
    /// @note this is the version supported by the sdk, not the version of any connected probe,
    ///       use this string to download the firmware binary from clarius cloud and update the probe
    /// @param[in] platform the platform to retrieve the firmware version for
    /// @param[out] version holds the firmware version for the given platform
    /// @param[in] sz size of the version string buffer, with at least 128 bytes allocated
    /// @return success of the call
    /// @retval 0 the information was retrieved
    /// @retval -1 the information could not be retrieved
    SOLUM_EXPORT int solumFwVersion(CusPlatform platform, char* version, int sz);

    /// get connection params with default values
    /// @return a zero initialized struct
    SOLUM_EXPORT CusConnectionParams solumDefaultConnectionParams(void);

    /// connects to a probe that is on the same network as the caller
    /// @param[in] params the connection parameters
    /// @return success of the call
    /// @retval 0 the connection attempt was successful
    /// @retval -1 the connection attempt was not successful
    SOLUM_EXPORT int solumConnect(const CusConnectionParams* params);

    /// disconnects from an existing connection
    /// @return success of the call
    /// @retval 0 disconnection was successful
    /// @retval -1 the disconnection was unsuccessful
    SOLUM_EXPORT int solumDisconnect(void);

    /// retrieves the current connected state of the module
    /// @return the connected state of the module
    /// @retval 0 there is currently no connection
    /// @retval 1 there is currently a connection
    /// @retval -1 the module is not initialized
    SOLUM_EXPORT int solumIsConnected(void);

    /// sets the certificate for the probe to be connected with
    /// @param[in] cert the certificate provided by clarius
    /// @return success of the call
    /// @retval 0 the certificate update attempt was successful
    /// @retval -1 the certificate update attempt was not successful
    SOLUM_EXPORT int solumSetCert(const char* cert);

    /// performs a software update once connected
    /// @param[in] path path to the firmware update file
    /// @param[in] fn the callback function that reports the status
    /// @param[in] progress software update progress callback
    /// @param[in] hwVer optional hardware version to set if the library cannot determine version on initial tcp connection
    ///                  set to 0, unless there is a reason to force the version (1, 2, or 3)
    /// @return success of the call
    /// @retval 0 the software is being sent
    /// @retval -1 the software could not be sent
    SOLUM_EXPORT int solumSoftwareUpdate(const char* path, CusSwUpdateFn fn, CusProgressFn progress, int hwVer);

    /// retrieves the available probe models the api supports
    /// @param[in] fn the callback function that reports the list
    /// @return success of the call
    /// @retval 0 the information was retrieved
    /// @retval -1 the information could not be retrieved
    SOLUM_EXPORT int solumProbes(CusListFn fn);

    /// retrieves the available applications for a specific probe model
    /// @param[in] probe the probe model to retrieve applications for
    /// @param[in] fn the callback function that reports the list
    /// @return success of the call
    /// @retval 0 the information was retrieved
    /// @retval -1 the information could not be retrieved
    SOLUM_EXPORT int solumApplications(const char* probe, CusListFn fn);

    /// loads an application
    /// @param[in] probe the probe model to load
    /// @param[in] workflow the workflow to load
    /// @return success of the call
    /// @retval 0 the application was loaded
    /// @retval -1 the application could not be loaded
    SOLUM_EXPORT int solumLoadApplication(const char* probe, const char* workflow);

    /// retrieves the current probe status if there's a connection
    /// @param[out] info the status information
    /// @return success of the call
    /// @retval 0 the information was retrieved
    /// @retval -1 the information could not be retrieved
    SOLUM_EXPORT int solumStatusInfo(CusStatusInfo* info);

    /// retrieves the current probe information
    /// @param[out] info the probe information
    /// @return success of the call
    /// @retval 0 the information was retrieved
    /// @retval -1 the information could not be retrieved
    SOLUM_EXPORT int solumProbeInfo(CusProbeInfo* info);

    /// sets the dimensions of the output display for scan conversion
    /// @param[in] w the number of pixels in the horizontal direction
    /// @param[in] h the number of pixels in the vertical direction
    /// @return success of the call
    /// @retval 0 the output size was successfully programmed
    /// @retval -1 the output size could not be set
    /// @note the output will always result in a 1:1 pixel ratio, depending on geometry of scanning array, and parameters
    ///       the frame will have various sizes of black borders around the image
    SOLUM_EXPORT int solumSetOutputSize(int w, int h);

    /// sets a flag to separate overlays into separate images, for example if color/power Doppler or strain
    /// imaging is enabled, two callbacks will be generated, one with the grayscale frame, and the other with the overlay
    /// @param[in] en the enable flag for separating overlays
    /// @return success of the call
    /// @retval 0 the flag was successfully programmed
    /// @retval -1 the flag could not be set
    SOLUM_EXPORT int solumSeparateOverlays(int en);

    /// runs or stops imaging
    /// @param[in] run the run state to set, 0 to stop imaging, 1 to start imaging
    /// @return success of the call
    /// @retval 0 run request was successfully made
    /// @retval -1 the run request could not be made
    SOLUM_EXPORT int solumRun(int run);

    /// shuts down the scanner
    /// @return success of the call
    /// @retval 0 shutdown was successful
    /// @retval -1 the shutdown request could not be made
    /// @note it is typically desirable to disconnect from bluetooth once the tcp connection has been established
    ///       instead of relying on the power service, this function can be used to power down the probe over tcp
    SOLUM_EXPORT int solumPowerDown(void);

    /// sets the internal probe settings to be applied upon a connection or when an existing connection exists
    /// @param[in] settings the structure containing the probe settings
    /// @return success of the call
    /// @retval 0 the settings were successfully programmed
    /// @retval -1 the settings could not be programmed
    SOLUM_EXPORT int solumSetProbeSettings(const CusProbeSettings* settings);

    /// sets an imaging parameter
    /// @param[in] param the parameter to set
    /// @param[in] val the value to set the parameter to
    /// @return success of the call
    /// @retval 0 parameter set request successfully made
    /// @retval -1 parameter set request could not be made
    SOLUM_EXPORT int solumSetParam(CusParam param, double val);

    /// retrieves an imaging parameter value
    /// @param[in] param the parameter to retrieve the value for
    /// @return the parameter value
    /// @retval -1 if the parameter value retrieval could not be made
    SOLUM_EXPORT double solumGetParam(CusParam param);

    /// retrieves the range for a specific parameter
    /// @param[in] param the parameter to retrieve the range for
    /// @param[out] range holds the range values for the parameter queried
    /// @return success of the call
    /// @retval 0 if the range retrieval was made
    /// @retval -1 if the range retrieval could not be made
    SOLUM_EXPORT int solumGetRange(CusParam param, CusRange* range);

    /// sets the tgc
    /// @param[in] tgc the value to set the tgc to
    /// @return success of the call
    /// @retval 0 tgc set request successfully made
    /// @retval -1 tgc set request could not be made
    SOLUM_EXPORT int solumSetTgc(const CusTgc* tgc);

    /// retrieves the tgc values
    /// @param[out] tgc holds the tgc values
    /// @return success of the call
    /// @retval 0 tgc set request successfully made
    /// @retval -1 tgc set request could not be made
    SOLUM_EXPORT int solumGetTgc(CusTgc* tgc);

    /// retrieves the active region for the grayscale image
    /// @param[out] points holds a vector of points in x/y format
    /// @param[in] count the number of points to generate (points buffer must be count x 2 or larger)
    /// @return success of the call
    /// @retval 0 roi was retrieved
    /// @retval -1 roi could not be retrieved
    SOLUM_EXPORT int solumGetActiveRegion(double* points, int count);

    /// retrieves the roi for the current mode if valid
    /// @param[out] points holds a vector of points in x/y format
    /// @param[in] count the number of points to generate (points buffer must be count x 2 or larger)
    /// @return success of the call
    /// @retval 0 roi was retrieved
    /// @retval -1 roi could not be retrieved
    SOLUM_EXPORT int solumGetRoi(double* points, int count);

    /// adjusts the roi based on the input provided
    /// @param[in] x the horizontal pixel position
    /// @param[in] y the vertical pixel position
    /// @param[in] fn roi function
    /// @return success of the call
    /// @retval 0 roi could be adjusted
    /// @retval -1 roi could not be adjusted
    SOLUM_EXPORT int solumAdjustRoi(int x, int y, CusRoiFunction fn);

    /// maximizes the roi without having to manipulate or calculate pixel co-ordinates
    /// @return success of the call
    /// @retval 0 roi could be adjusted
    /// @retval -1 roi could not be adjusted
    SOLUM_EXPORT int solumMaximizeRoi(void);

    /// retrieves the gate for the current mode if valid
    /// @param[out] lines holds the lines that can be drawn to portray the gate on the image
    /// @return success of the call
    /// @retval 0 gate was retrieved
    /// @retval -1 gate could not be retrieved
    SOLUM_EXPORT int solumGetGate(CusGateLines* lines);

    /// adjusts the gate based on the input provided
    /// @param[in] x the horizontal pixel position
    /// @param[in] y the vertical pixel position
    /// @return success of the call
    /// @retval 0 gate could be adjusted
    /// @retval -1 gate could not be adjusted
    SOLUM_EXPORT int solumAdjustGate(int x, int y);

    /// sets an imaging mode
    /// @param[in] mode the imaging mode to set
    /// @return success of the call
    /// @retval 0 mode set request successfully made
    /// @retval -1 mode set request could not be made
    SOLUM_EXPORT int solumSetMode(CusMode mode);

    /// retrieves the current imaging mode
    /// @return the current imaging mode
    SOLUM_EXPORT CusMode solumGetMode(void);

    /// enables the 5v output on or off
    /// @param[in] en the enable state, set to 1 to turn 5v on, or 0 to turn off
    /// @return success of the call
    /// @retval 0 enable request successfully made
    /// @retval -1 enable request could not be made
    SOLUM_EXPORT int solumEnable5v(int en);

    /// sets the format for processed images, by default the format will be uncompressed argb
    /// @param[in] format the format of the image
    /// @return success of the call
    /// @retval 0 the format was successfully set
    /// @retval -1 the format could not be set
    SOLUM_EXPORT int solumSetFormat(CusImageFormat format);

    /// will try and optimize the wireless channel when the probe is running its own network
    /// the function will return a failure if the probe is on an external wlan as nothing can be optimized, except for switching over to the probe's own network
    /// to switch to the probe's network, see the bluetooth documentation for the wireless management service
    /// @param[in] opt the optimization type to run
    /// @return success of the call
    /// @retval 0 the optimization is being performed
    /// @retval -1 the optimization could not be performed
    /// @note on some platforms it may be necessary to run the operation of re-connecting to the network through the operating system.
    ///       it will not be necessary to re-parse the connection data through the bluetooth service, as ip address and port will not change after optimization
    SOLUM_EXPORT int solumOptimizeWifi(CusWifiOpt opt);

    /// performs a reset of a function on the probe
    /// @param[in] reset the reset type to perform
    /// @return success of the call
    /// @retval 0 the reset is being performed
    /// @retval -1 the reset could not be performed
    SOLUM_EXPORT int solumResetProbe(CusProbeReset reset);

    /// makes a request to return the availability of all the raw data currently buffered on the probe
    /// @param[in] fn result callback function that will return all the timestamps of the data blocks that are buffered
    /// @return success of the call
    /// @retval 0 the request was successfully made
    /// @retval -1 the request could not be made
    /// @note the probe must be frozen with raw data buffering enabled prior to calling the function
    SOLUM_EXPORT int solumRawDataAvailability(CusRawAvailabilityFn fn);

    /// makes a request for raw data from the probe
    /// @param[in] start the first frame to request, as determined by timestamp in nanoseconds, set to 0 along with end to requests all data in buffer
    /// @param[in] end the last frame to request, as determined by timestamp in nanoseconds, set to 0 along with start to requests all data in buffer
    /// @param[in] lzo flag to specify a tarball with lzo compressed raw data inside (default) vs no compression of raw data
    /// @param[in] fn result callback function, will return size of buffer required upon success, 0 if no raw data was buffered, or -1 if request could not be made
    /// @return success of the call
    /// @retval 0 the request was successfully made
    /// @retval -1 the request could not be made
    /// @note the probe must be frozen and in a raw data buffering mode in order for the call to succeed
    SOLUM_EXPORT int solumRequestRawData(long long int start, long long int end, int lzo, CusRawRequestFn fn);

    /// retrieves raw data from a previous request
    /// @param[out] data a pointer to a buffer that has been allocated to read the raw data into, this must be pre-allocated with
    ///             the size returned from a previous call to solumRequestRawData
    /// @param[in] fn result callback function, will return size of buffer required upon success, 0 if no raw data was buffered, or -1 if request could not be made,
    /// @param[in] progress download progress callback function that outputs the progress in percent
    /// @return success of the call
    /// @retval 0 the read request was successfully made
    /// @retval -1 the read request could not be made
    /// @note the probe must be frozen and a successful call to solumRequestRawData must have taken place in order for the call to succeed
    SOLUM_EXPORT int solumReadRawData(void** data, CusRawFn fn, CusProgressFn progress);

    /// sets a low level parameter to a specific value to gain access to lower level device control
    /// @param[in] prm the parameter to change
    /// @param[in] val the value to set the parameter to
    /// @return success of the call
    /// @retval 0 the call was successful
    /// @retval -1 the call was not successful
    /// @note see external documentation for supported parameters
    /// @warning changing parameters through this function may result in unstable operation, degradation of image quality, or operation outside of the safety limits that clarius tests to
    SOLUM_EXPORT int solumSetLowLevelParam(const char* prm, double val);

    /// enables or disables a low level parameter to gain access to lower device control
    /// @param[in] prm the parameter to change
    /// @param[in] en the enable flag, 0 to disable, 1 to enable
    /// @return success of the call
    /// @retval 0 the call was successful
    /// @retval -1 the call was not successful
    /// @note see external documentation for supported parameters
    /// @warning changing parameters through this function may result in unstable operation, degradation of image quality, or operation outside of the safety limits that clarius tests to
    SOLUM_EXPORT int solumEnableLowLevelParam(const char* prm, int en);

    /// sets a pulse shape parameter to a specific value to gain access to lower level device control
    /// @param[in] prm the parameter to change
    /// @param[in] shape the shape to set the pulse as
    /// @return success of the call
    /// @retval 0 the call was successful
    /// @retval -1 the call was not successful
    /// @note see external documentation for supported parameters
    /// @warning changing parameters through this function may result in unstable operation, degradation of image quality, or operation outside of the safety limits that clarius tests to
    SOLUM_EXPORT int solumSetLowLevelPulse(const char* prm, const char* shape);

    /// retrieves a low level parameter value
    /// @param[in] prm the parameter to retrieve the value for
    /// @return the parameter value, for boolean variables, the value will be 0 (disabled) or 1 (enabled)
    /// @retval -1 if the parameter value retrieval could not be made
    SOLUM_EXPORT double solumGetLowLevelParam(const char* prm);

    /// retrieves the acoustic indices for the loaded application, imaging mode, and parameter settings
    /// @param[out] indices the acoustic index values
    /// @return success of the call
    /// @retval 0 the call was successful
    /// @retval -1 the call was not successful
    SOLUM_EXPORT int solumGetAcousticIndices(CusAcoustic* indices);

    /// set the tee exam info for a connected probe
    /// @param[in] id the patient id
    /// @param[in] name the patient name
    /// @param[in] exam the exam id
    /// @return success of the call
    /// @retval 0 the function was successful
    /// @retval -1 the function was not successful
    SOLUM_EXPORT int solumSetTeeExamInfo(const char* id, const char* name, const char* exam);

#ifdef __cplusplus
}
#endif
