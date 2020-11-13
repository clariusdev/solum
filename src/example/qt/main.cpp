#include "oemqt.h"
#include <memory>
#include <oem/oem.h>
#include <iostream>

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
            QApplication::postEvent(Oem::instance(), new event::Connection(code, port, QString::fromLatin1(msg)));
        },
        // power down callback
        [](int code, int tm)
        {
            QApplication::postEvent(Oem::instance(), new event::PowerDown(code, tm));
        },
        // new image callback
        [](const void* img, const ClariusProcessedImageInfo* nfo, int, const ClariusPosInfo*)
        {
            // we need to perform a deep copy of the image data since we have to post the event (yes this happens a lot with this api)
            size_t sz = nfo->width * nfo->height * (nfo->bitsPerPixel / 8);
            if (_image.size() <  sz)
                _image.resize(sz);
            memcpy(_image.data(), img, sz);

            QApplication::postEvent(Oem::instance(), new event::Image(IMAGE_EVENT, _image.data(), nfo->width, nfo->height, nfo->bitsPerPixel));
        },
        // new raw data callback
        [](const void* data, const ClariusRawImageInfo* nfo, int, const ClariusPosInfo*)
        {
            // we need to perform a deep copy of the image data since we have to post the event (yes this happens a lot with this api)
            size_t sz = nfo->lines * nfo->samples * (nfo->bitsPerSample / 8);
            if (nfo->rf)
            {
                if (_rfData.size() <  sz)
                    _rfData.resize(sz);
                memcpy(_rfData.data(), data, sz);
                QApplication::postEvent(Oem::instance(), new event::RfImage(_rfData.data(), nfo->lines, nfo->samples, nfo->bitsPerSample, nfo->lateralSize, nfo->axialSize));
            }
            else
            {
                if (_prescanImage.size() <  sz)
                    _prescanImage.resize(sz);
                memcpy(_prescanImage.data(), data, sz);
                QApplication::postEvent(Oem::instance(), new event::PreScanImage(_prescanImage.data(), nfo->lines, nfo->samples, nfo->bitsPerSample, nfo->jpeg));
            }
        },
        // imaging state change callback
        [](int ready, int imaging)
        {
            // post event here, as the gui (statusbar) will be updated directly, and it needs to come from the application thread
            QApplication::postEvent(Oem::instance(), new event::Imaging(ready ? true : false, imaging ? true : false));
        },
        // button press callback
        [](int btn, int clicks)
        {
            // post event here, as the gui (statusbar) will be updated directly, and it needs to come from the application thread
            QApplication::postEvent(Oem::instance(), new event::Button(btn, clicks));
        },
        // error message callback
        [](const char* err)
        {
            // post event here, as the gui (statusbar) will be updated directly, and it needs to come from the application thread
            QApplication::postEvent(Oem::instance(), new event::Error(err));
        },
        width, height) != 0)
    {
        qDebug() << "error initializing listner";
        return -1;
    }

    Oem::instance()->show();
    return a.exec();
}
