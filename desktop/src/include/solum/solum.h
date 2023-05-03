#pragma once
#include "solum_export.h"
#include "solum_cb.h"

extern "C"
{
    /// initializes the solum module
    /// @param[in] argc the argument count for input parameters to pass to the library
    /// @param[in] argv the arguments to pass to the library, possibly required for qt graphics buffer initialization
    /// @param[in] dir the directory to store security keys
    /// @param[in] connect connection status callback
    /// @param[in] cert certificate status callback
    /// @param[in] power probe power down callback
    /// @param[in] newProcessedImage new processed image callback (scan-converted image)
    /// @param[in] newRawImage new raw image callback - (pre scan-converted image)
    /// @param[in] newSpectralImage new processed spectral image callback
    /// @param[in] imaging imaging state callback
    /// @param[in] btn button press callback
    /// @param[in] err error message callback
    /// @param[in] width the width of the output buffer
    /// @param[in] height the height of the output buffer
    /// @return success of the call
    /// @retval 0 the initialization was successful
    /// @retval -1 the initialization was not successful
    /// @note must be called before any other functions will succeed
    SOLUM_EXPORT int solumInit(int argc, char** argv, const char* dir,
        CusConnectFn connect, CusCertFn cert, CusPowerDownFn power,
        CusNewProcessedImageFn newProcessedImage, CusNewRawImageFn newRawImage,
        CusNewSpectralImageFn newSpectralImage, CusImagingFn imaging, CusButtonFn btn, CusErrorFn err,
        int width, int height);

    /// cleans up memory allocated by the solum module
    /// @retval 0 the destroy attempt was successful
    /// @retval -1 the destroy attempt was not successful
    /// @note should be called prior to exiting the application
    SOLUM_EXPORT int solumDestroy();

    /// retrieves the firmware version for a given platform
    /// @param[in] platform the platform to retrieve the firmware version for
    /// @param[out] version holds the firmware version for the given platform
    /// @param[in] sz size of the version string buffer, suggest at least 32 bytes allocated
    /// @return success of the call
    /// @retval 0 the information was retrieved
    /// @retval -1 the information could not be retrieved
    SOLUM_EXPORT int solumFwVersion(CusPlatform platform, char* version, int sz);

    /// connects to a probe that is on the same network as the caller
    /// @param[in] ipAddress the ip address of the probe
    /// @param[in] port the probe's tcp port to connect to
    /// @return success of the call
    /// @retval 0 the connection attempt was successful
    /// @retval -1 the connection attempt was not successful
    SOLUM_EXPORT int solumConnect(const char* ipAddress, unsigned int port);

    /// disconnects from an existing connection
    /// @return success of the call
    /// @retval 0 disconnection was successful
    /// @retval -1 the disconnection was unsuccessful
    SOLUM_EXPORT int solumDisconnect();

    /// retrieves the current connected state of the module
    /// @return the connected state of the module
    /// @retval 0 there is currently no connection
    /// @retval 1 there is currently a connection
    /// @retval -1 the module is not initialized
    SOLUM_EXPORT int solumIsConnected();

    /// sets the certificate for the probe to be connected with
    /// @param[in] cert the certificate provided by clarius
    /// @return success of the call
    /// @retval 0 the certificate update attempt was successful
    /// @retval -1 the certificate update attempt was not successful
    SOLUM_EXPORT int solumSetCert(const char* cert);

    /// performs a software update once connected
    /// @param[in] fn the callback function that reports the status
    /// @param[in] progress software update progress callback
    /// @return success of the call
    /// @retval 0 the software is being sent
    /// @retval -1 the software could not be sent
    SOLUM_EXPORT int solumSoftwareUpdate(CusSwUpdateFn fn, CusProgressFn progress);

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
    SOLUM_EXPORT int solumPowerDown();

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

    /// retrieves the gate for the current mode if valid
    /// @param[out] lines holds the lines that can be drawn to portray the gate on the image
    /// @return success of the call
    /// @retval 0 gate was retrieved
    /// @retval -1 gate could not be retrieved
    SOLUM_EXPORT int solumGetGate(CusGateLines* lines);

    /// adjuss the gate based on the input provided
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
    SOLUM_EXPORT CusMode solumGetMode();

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

    /// makes a request for raw data from the probe
    /// @param[in] start the first frame to request, as determined by timestamp in nanoseconds, set to 0 along with end to requests all data in buffer
    /// @param[in] end the last frame to request, as determined by timestamp in nanoseconds, set to 0 along with start to requests all data in buffer
    /// @param[in] res result callback function, will return size of buffer required upon success, 0 if no raw data was buffered, or -1 if request could not be made,
    /// @return success of the call
    /// @retval 0 the request was successfully made
    /// @retval -1 the request could not be made
    /// @note the probe must be frozen and in a raw data buffering mode in order for the call to succeed
    SOLUM_EXPORT int solumRequestRawData(long long int start, long long int end, CusRawFn res);

    /// retrieves raw data from a previous request
    /// @param[out] data a pointer to a buffer that has been allocated to read the raw data into, this must be pre-allocated with
    ///             the size returned from a previous call to solumRequestRawData
    /// @param[in] res result callback function, will return size of buffer required upon success, 0 if no raw data was buffered, or -1 if request could not be made,
    /// @param[in] progress download progress callback function that outputs the progress in percent
    /// @return success of the call
    /// @retval 0 the read request was successfully made
    /// @retval -1 the read request could not be made
    /// @note the probe must be frozen and a successful call to solumRequestRawData must have taken place in order for the call to succeed
    SOLUM_EXPORT int solumReadRawData(void** data, CusRawFn res, CusProgressFn progress);
}
