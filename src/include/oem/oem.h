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
    /// @return success of the call
    /// @retval 0 the initialization was successful
    /// @retval -1 the initialization was not successful
    /// @note must be called before any other functions will succeed
    OEM_EXPORT int cusOemInit(int argc, char** argv, const char* dir,
        ClariusConnectFn connect, ClariusCertFn cert, ClariusPowerDownFn power,
        ClariusNewProcessedImageFn newProcessedImage, ClariusNewRawImageFn newRawImage,
        ClariusNewSpectralImageFn newSpectralImage, ClariusImagingFn imaging, ClariusButtonFn btn, ClariusErrorFn err,
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
    /// @param[in] cert the certificate provided by Clarius
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
    OEM_EXPORT int cusOemSoftwareUpdate(ClariusSwUpdateFn fn, ClariusProgressFn progress);

    /// retrieves the available probe models the api supports
    /// @param[in] fn the callback function that reports the list
    /// @return success of the call
    /// @retval 0 the information was retrieved
    /// @retval -1 the information could not be retrieved
    OEM_EXPORT int cusOemProbes(ClariusListFn fn);

    /// retrieves the available applications for a specific probe model
    /// @param[in] probe the probe model to retrieve applications for
    /// @param[in] fn the callback function that reports the list
    /// @return success of the call
    /// @retval 0 the information was retrieved
    /// @retval -1 the information could not be retrieved
    OEM_EXPORT int cusOemApplications(const char* probe, ClariusListFn fn);

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
    OEM_EXPORT int cusOemStatusInfo(ClariusStatusInfo* info);

    /// retrieves the current probe information
    /// @param[out] info the probe information
    /// @return success of the call
    /// @retval 0 the information was retrieved
    /// @retval -1 the information could not be retrieved
    OEM_EXPORT int cusOemProbeInfo(ClariusProbeInfo* info);

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
    /// imaging is enabled, two callbacks will be generated, one with the greyscale frame, and the other with the overlay
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

    /// sets an imaging parameter
    /// @param[in] param the parameter to set
    /// @param[in] val the value to set the parameter to
    /// @return success of the call
    /// @retval 0 parameter set request successfully made
    /// @retval -1 parameter set request could not be made
    OEM_EXPORT int cusOemSetParam(int param, double val);

    /// retrieves an imaging parameter value
    /// @param[in] param the parameter to retrieve the value for
    /// @return the parameter value
    /// @retval -1 if the parameter value retrieval could not be made
    OEM_EXPORT double cusOemGetParam(int param);

    /// sets the tgc
    /// @param[in] tgc the value to set the tgc to
    /// @return success of the call
    /// @retval 0 tgc set request successfully made
    /// @retval -1 tgc set request could not be made
    OEM_EXPORT int cusOemSetTgc(const ClariusTgc* tgc);

    /// retrieves the tgc values
    /// @param[out] tgc holds the tgc values
    /// @return success of the call
    /// @retval 0 tgc set request successfully made
    /// @retval -1 tgc set request could not be made
    OEM_EXPORT int cusOemGetTgc(ClariusTgc* tgc);

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
    /// @param[in] fn set to ROI_MOVE to move the top/left roi position to x/y without changing dimensions
    ///               set to ROI_SIZE to adjust the roi bottom/right to x/y and change dimensions
    /// @return success of the call
    /// @retval 0 roi could be moved
    /// @retval -1 roi could not be moved
    OEM_EXPORT int cusOemAdjustRoi(int x, int y, int fn);

    /// sets an imaging mode
    /// @param[in] mode the imaging mode to set
    /// @return success of the call
    /// @retval 0 mode set request successfully made
    /// @retval -1 mode set request could not be made
    OEM_EXPORT int cusOemSetMode(int mode);

    /// retrieves the current imaging mode
    /// @return the current imaging mode
    /// @retval -1 if the mode retrieval could not be made
    OEM_EXPORT int cusOemGetMode();

    /// enables the 5v output on or off
    /// @param[in] en the enable state, set to 1 to turn 5v on, or 0 to turn off
    /// @return success of the call
    /// @retval 0 enable request successfully made
    /// @retval -1 enable request could not be made
    OEM_EXPORT int cusOemEnable5v(int en);
}
