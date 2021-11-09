#include "oemqt.h"
#include <memory>
#include <oem/oem.h>
#include <iostream>

static std::unique_ptr<Oem> _oem;
static std::vector<char> _image;
static std::vector<char> _prescanImage;
static std::vector<char> _rfData;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    const int width  = 640; // width of the rendered image
    const int height = 480; // height of the rendered image

    if (cusOemInit(argc, argv, QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString().c_str(),
        // connection callback
        [](int code, int port, const char* msg)
        {
            QApplication::postEvent(_oem.get(), new event::Connection(code, port, QString::fromLatin1(msg)));
        },
        // cert callback
        [](int daysValid)
        {
            QApplication::postEvent(_oem.get(), new event::Cert(daysValid));
        },
        // power down callback
        [](int code, int tm)
        {
            QApplication::postEvent(_oem.get(), new event::PowerDown(code, tm));
        },
        // new image callback
        [](const void* img, const ClariusProcessedImageInfo* nfo, int npos, const ClariusPosInfo* pos)
        {
            int sz = nfo->imageSize;
            // we need to perform a deep copy of the image data since we have to post the event (yes this happens a lot with this api)
            if (_image.size() < static_cast<size_t>(sz))
                _image.resize(sz);
            memcpy(_image.data(), img, sz);
            QQuaternion imu;
            if (npos && pos)
                imu = QQuaternion(static_cast<float>(pos[0].qw), static_cast<float>(pos[0].qx), static_cast<float>(pos[0].qy), static_cast<float>(pos[0].qz));

            QApplication::postEvent(_oem.get(), new event::Image(IMAGE_EVENT, _image.data(), nfo->width, nfo->height, nfo->bitsPerPixel, sz, imu));
        },
        // new raw data callback
        [](const void* data, const ClariusRawImageInfo* nfo, int, const ClariusPosInfo*)
        {
            // we need to perform a deep copy of the image data since we have to post the event (yes this happens a lot with this api)
            int sz = nfo->lines * nfo->samples * (nfo->bitsPerSample / 8);
            if (nfo->rf)
            {
                if (_rfData.size() < static_cast<size_t>(sz))
                    _rfData.resize(sz);
                memcpy(_rfData.data(), data, sz);
                QApplication::postEvent(_oem.get(), new event::RfImage(_rfData.data(), nfo->lines, nfo->samples, nfo->bitsPerSample, sz,
                                                                            nfo->lateralSize, nfo->axialSize));
            }
            else
            {
                // image may be a jpeg, adjust the size
                if (nfo->jpeg)
                    sz = nfo->jpeg;
                if (_prescanImage.size() < static_cast<size_t>(sz))
                    _prescanImage.resize(sz);
                memcpy(_prescanImage.data(), data, sz);
                QApplication::postEvent(_oem.get(), new event::Image(PRESCAN_EVENT, _prescanImage.data(), nfo->lines, nfo->samples,
                                                                          nfo->bitsPerSample, sz, QQuaternion()));
            }
        },
        nullptr,
        // imaging state change callback
        [](int ready, int imaging)
        {
            // post event here, as the gui (statusbar) will be updated directly, and it needs to come from the application thread
            QApplication::postEvent(_oem.get(), new event::Imaging(ready, imaging ? true : false));
        },
        // button press callback
        [](int btn, int clicks)
        {
            // post event here, as the gui (statusbar) will be updated directly, and it needs to come from the application thread
            QApplication::postEvent(_oem.get(), new event::Button(btn, clicks));
        },
        // error message callback
        [](const char* err)
        {
            // post event here, as the gui (statusbar) will be updated directly, and it needs to come from the application thread
            QApplication::postEvent(_oem.get(), new event::Error(err));
        },
        width, height) != 0)
    {
        qDebug() << "error initializing listner";
        return -1;
    }

    _oem = std::make_unique<Oem>();
    _oem->show();
    return a.exec();
}
