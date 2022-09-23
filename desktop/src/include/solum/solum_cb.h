#pragma once

#include "solum_def.h"

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
