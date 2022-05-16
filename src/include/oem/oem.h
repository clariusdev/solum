#pragma once
#include "oem_export.h"
#include "oem_def.h"

extern "C"
{
    /// initializes the oem module
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
    OEM_EXPORT int cusOemInit(int argc, char** argv, const char* dir,
        CusConnectFn connect, CusCertFn cert, CusPowerDownFn power,
        CusNewProcessedImageFn newProcessedImage, CusNewRawImageFn newRawImage,
        CusNewSpectralImageFn newSpectralImage, CusImagingFn imaging, CusButtonFn btn, CusErrorFn err,
        int width, int height);

    /// cleans up memory allocated by the oem module
    /// @retval 0 the destroy attempt was successful
    /// @retval -1 the destroy attempt was not successful
    /// @note should be called prior to exiting the application
    OEM_EXPORT int cusOemDestroy();

    /// connects to a probe that is on the same network as the caller
    /// @param[in] ipAddress the ip address of the probe
    /// @param[in] port the probe's tcp port to connect to
    /// @return success of the call
    /// @retval 0 the connection attempt was successful
    /// @retval -1 the connection attempt was not successful
    OEM_EXPORT int cusOemConnect(const char* ipAddress, unsigned int port);

    /// disconnects from an existing connection
    /// @return success of the call
    /// @retval 0 disconnection was successful
    /// @retval -1 the disconnection was unsuccessful
    OEM_EXPORT int cusOemDisconnect();

    /// retrieves the current connected state of the module
    /// @return the connected state of the module
    /// @retval 0 there is currently no connection
    /// @retval 1 there is currently a connection
    /// @retval -1 the module is not initialized
    OEM_EXPORT int cusOemIsConnected();

    /// sets the certificate for the probe to be connected with
    /// @param[in] cert the certificate provided by clarius
    /// @return success of the call
    /// @retval 0 the certificate update attempt was successful
    /// @retval -1 the certificate update attempt was not successful
    OEM_EXPORT int cusOemSetCert(const char* cert);

    /// performs a software update once connected
    /// @param[in] fn the callback function that reports the status
    /// @param[in] progress software update progress callback
    /// @return success of the call
    /// @retval 0 the software is being sent
    /// @retval -1 the software could not be sent
    OEM_EXPORT int cusOemSoftwareUpdate(CusSwUpdateFn fn, CusProgressFn progress);

    /// retrieves the available probe models the api supports
    /// @param[in] fn the callback function that reports the list
    /// @return success of the call
    /// @retval 0 the information was retrieved
    /// @retval -1 the information could not be retrieved
    OEM_EXPORT int cusOemProbes(CusListFn fn);

    /// retrieves the available applications for a specific probe model
    /// @param[in] probe the probe model to retrieve applications for
    /// @param[in] fn the callback function that reports the list
    /// @return success of the call
    /// @retval 0 the information was retrieved
    /// @retval -1 the information could not be retrieved
    OEM_EXPORT int cusOemApplications(const char* probe, CusListFn fn);

    /// loads an application
    /// @param[in] probe the probe model to load
    /// @param[in] workflow the workflow to load
    /// @return success of the call
    /// @retval 0 the application was loaded
    /// @retval -1 the application could not be loaded
    OEM_EXPORT int cusOemLoadApplication(const char* probe, const char* workflow);

    /// retrieves the current probe status if there's a connection
    /// @param[out] info the status information
    /// @return success of the call
    /// @retval 0 the information was retrieved
    /// @retval -1 the information could not be retrieved
    OEM_EXPORT int cusOemStatusInfo(CusStatusInfo* info);

    /// retrieves the current probe information
    /// @param[out] info the probe information
    /// @return success of the call
    /// @retval 0 the information was retrieved
    /// @retval -1 the information could not be retrieved
    OEM_EXPORT int cusOemProbeInfo(CusProbeInfo* info);

    /// sets the dimensions of the output display for scan conversion
    /// @param[in] w the number of pixels in the horizontal direction
    /// @param[in] h the number of pixels in the vertical direction
    /// @return success of the call
    /// @retval 0 the output size was successfully programmed
    /// @retval -1 the output size could not be set
    /// @note the output will always result in a 1:1 pixel ratio, depending on geometry of scanning array, and parameters
    ///       the frame will have various sizes of black borders around the image
    OEM_EXPORT int cusOemSetOutputSize(int w, int h);

    /// sets a flag to separate overlays into separate images, for example if color/power Doppler or strain
    /// imaging is enabled, two callbacks will be generated, one with the grayscale frame, and the other with the overlay
    /// @param[in] en the enable flag for separating overlays
    /// @return success of the call
    /// @retval 0 the flag was successfully programmed
    /// @retval -1 the flag could not be set
    OEM_EXPORT int cusOemSeparateOverlays(int en);

    /// runs or stops imaging
    /// @param[in] run the run state to set, 0 to stop imaging, 1 to start imaging
    /// @return success of the call
    /// @retval 0 run request was successfully made
    /// @retval -1 the run request could not be made
    OEM_EXPORT int cusOemRun(int run);

    /// sets the internal probe settings to be applied upon a connection or when an existing connection exists
    /// @param[in] settings the structure containing the probe settings
    /// @return success of the call
    /// @retval 0 the settings were successfully programmed
    /// @retval -1 the settings could not be programmed
    OEM_EXPORT int cusOemSetProbeSettings(const CusProbeSettings* settings);

    /// sets an imaging parameter
    /// @param[in] param the parameter to set
    /// @param[in] val the value to set the parameter to
    /// @return success of the call
    /// @retval 0 parameter set request successfully made
    /// @retval -1 parameter set request could not be made
    OEM_EXPORT int cusOemSetParam(CusParam param, double val);

    /// retrieves an imaging parameter value
    /// @param[in] param the parameter to retrieve the value for
    /// @return the parameter value
    /// @retval -1 if the parameter value retrieval could not be made
    OEM_EXPORT double cusOemGetParam(CusParam param);

    /// sets the tgc
    /// @param[in] tgc the value to set the tgc to
    /// @return success of the call
    /// @retval 0 tgc set request successfully made
    /// @retval -1 tgc set request could not be made
    OEM_EXPORT int cusOemSetTgc(const CusTgc* tgc);

    /// retrieves the tgc values
    /// @param[out] tgc holds the tgc values
    /// @return success of the call
    /// @retval 0 tgc set request successfully made
    /// @retval -1 tgc set request could not be made
    OEM_EXPORT int cusOemGetTgc(CusTgc* tgc);

    /// retrieves the roi for the current mode if valid
    /// @param[out] points holds a vector of points in x/y format
    /// @param[in] count the number of points to generate (points buffer must be count x 2 or larger)
    /// @return success of the call
    /// @retval 0 roi was retrieved
    /// @retval -1 roi could not be retrieved
    OEM_EXPORT int cusOemGetRoi(double* points, int count);

    /// moves the top/left of the roi to a specific point
    /// @param[in] x the horizontal pixel position
    /// @param[in] y the vertical pixel position
    /// @param[in] fn roi function
    /// @return success of the call
    /// @retval 0 roi could be adjusted
    /// @retval -1 roi could not be adjusted
    OEM_EXPORT int cusOemAdjustRoi(int x, int y, CusRoi fn);

    /// sets an imaging mode
    /// @param[in] mode the imaging mode to set
    /// @return success of the call
    /// @retval 0 mode set request successfully made
    /// @retval -1 mode set request could not be made
    OEM_EXPORT int cusOemSetMode(CusMode mode);

    /// retrieves the current imaging mode
    /// @return the current imaging mode
    OEM_EXPORT CusMode cusOemGetMode();

    /// enables the 5v output on or off
    /// @param[in] en the enable state, set to 1 to turn 5v on, or 0 to turn off
    /// @return success of the call
    /// @retval 0 enable request successfully made
    /// @retval -1 enable request could not be made
    OEM_EXPORT int cusOemEnable5v(int en);

    /// sets the format for processed images, by default the format will be uncompressed argb
    /// @param[in] format the format of the image
    /// @return success of the call
    /// @retval 0 the format was successfully set
    /// @retval -1 the format could not be set
    OEM_EXPORT int cusOemSetFormat(CusImageFormat format);

    /// makes a request for raw data from the probe
    /// @param[in] start the first frame to request, as determined by timestamp in nanoseconds, set to 0 along with end to requets all data in buffer
    /// @param[in] end the last frame to request, as determined by timestamp in nanoseconds, set to 0 along with start to requets all data in buffer
    /// @param[in] res result callback function, will return size of buffer required upon success, 0 if no raw data was buffered, or -1 if request could not be made,
    /// @return success of the call
    /// @retval 0 the request was successfully made
    /// @retval -1 the request could not be made
    /// @note the probe must be frozen and in a raw data buffering mode in order for the call to succeed
    OEM_EXPORT int cusOemRequestRawData(long long int start, long long int end, CusRawFn res);

    /// retrieves raw data from a previous request
    /// @param[out] data a pointer to a buffer that has been allocated to read the raw data into, this must be pre-allocated with
    ///             the size returned from a previous call to cusOemRequestRawData
    /// @param[in] res result callback function, will return size of buffer required upon success, 0 if no raw data was buffered, or -1 if request could not be made,
    /// @param[in] progress download progress callback function that outputs the progress in percent
    /// @return success of the call
    /// @retval 0 the read request was successfully made
    /// @retval -1 the read request could not be made
    /// @note the probe must be frozen and a successful call to cusOemRequestRawData must have taken place in order for the call to succeed
    OEM_EXPORT int cusOemReadRawData(void** data, CusRawFn res, CusProgressFn progress);
}
