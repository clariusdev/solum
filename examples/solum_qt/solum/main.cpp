#include "solumqt.h"
#include <memory>
#include <solum/solum.h>
#include <iostream>

static std::unique_ptr<Solum> _solum;
static std::vector<char> _image;
static std::vector<char> _prescanImage;
static std::vector<char> _spectrum;
static std::vector<char> _rfData;

void print_firmware_version()
{
    char buffer [64];
    auto get_version = [&buffer](CusPlatform platform) -> QString
    {
        if (CUS_SUCCESS != solumFwVersion(platform, buffer, std::size(buffer)))
            return QStringLiteral("error");
        buffer [std::size(buffer)-1] = '\0';
        return QString::fromLatin1(buffer);
    };
    qDebug() << '\n';
    qDebug() << "V1 firmware version:" << get_version(CusPlatform::V1);
    qDebug() << "HD firmware version:" << get_version(CusPlatform::HD);
    qDebug() << "HD3 firmware version:" << get_version(CusPlatform::HD3);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // When using the default `windowsvista` style on Windows 10 in Dark Mode,
    // Qt 6.5.1 renders the labels of disabled QPushButtons with the same
    // black color as enabled ones, making it hard to distinguish the two:
    //
    //     https://bugreports.qt.io/browse/QTBUG-115036
    //
    // The `fusion` style is bundled with Qt as well, does not have this issue,
    // and looks much nicer in general. We only override the specific
    // `windowsvista` style though: Other styles don't have this issue, and we
    // do want to retain a potential system-wide Qt style setting, especially
    // on Linux.
    //
    // QStyle::name() is also only available on Qt 6.1 and above.
#if QT_VERSION >= QT_VERSION_CHECK(6, 1, 0)
    if (a.style()->name() == "windowsvista") {
        QApplication::setStyle("fusion");
    }
#endif

    QCoreApplication::setOrganizationName(QStringLiteral("Siemens Healthineers AG"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("healthineers.com"));
    QCoreApplication::setApplicationName(QStringLiteral("Clarius Server"));

    const int width  = 640; // width of the rendered image
    const int height = 480; // height of the rendered image

    _solum = std::make_unique<Solum>();

    if (solumInit(argc, argv, QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString().c_str(),
        // connection callback
        [](CusConnection res, int port, const char* msg)
        {
            QApplication::postEvent(_solum.get(), new event::Connection(res, port, QString::fromLatin1(msg)));
        },
        // cert callback
        [](int daysValid)
        {
            QApplication::postEvent(_solum.get(), new event::Cert(daysValid));
        },
        // power down callback
        [](CusPowerDown res, int tm)
        {
            QApplication::postEvent(_solum.get(), new event::PowerDown(res, tm));
        },
        // new image callback
        [](const void* img, const CusProcessedImageInfo* nfo, int npos, const CusPosInfo* pos)
        {
            size_t sz = nfo->imageSize;
            // we need to perform a deep copy of the image data since we have to post the event (yes this happens a lot with this api)
            if (_image.size() < static_cast<size_t>(sz))
                _image.resize(sz);
            memcpy(_image.data(), img, sz);
            QQuaternion imu;
            if (npos && pos)
                imu = QQuaternion(static_cast<float>(pos[0].qw), static_cast<float>(pos[0].qx), static_cast<float>(pos[0].qy), static_cast<float>(pos[0].qz));

            SolumImage solumImage = {
                .img_ = { reinterpret_cast<uint8_t *>(_image.data()), sz },
                .width_ = nfo->width,
                .height_ = nfo->height,
                .bpp_ = nfo->bitsPerPixel,
                .format_ = nfo->format,
            };

            QApplication::postEvent(_solum.get(), new event::ProcessedImage(
                IMAGE_EVENT, solumImage, nfo->overlay, imu, nfo->micronsPerPixel, nfo->originX, nfo->originY
            ));
        },
        // new raw data callback
        [](const void* data, const CusRawImageInfo* nfo, int, const CusPosInfo*)
        {
            SolumImage solumImage = {
                .width_ = nfo->lines,
                .height_ = nfo->samples,
                .bpp_ = nfo->bitsPerSample,
                .format_ = nfo->jpeg ? Jpeg : Uncompressed8Bit,
            };

            // we need to perform a deep copy of the image data since we have to post the event (yes this happens a lot with this api)
            size_t sz = nfo->lines * nfo->samples * (nfo->bitsPerSample / 8);
            if (nfo->rf)
            {
                if (_rfData.size() < static_cast<size_t>(sz))
                    _rfData.resize(sz);
                memcpy(_rfData.data(), data, sz);
                solumImage.img_ = { reinterpret_cast<uint8_t *>(_rfData.data()), sz },
                QApplication::postEvent(_solum.get(), new event::RfImage(solumImage, nfo->lateralSize, nfo->axialSize));
            }
            else
            {
                // image may be a jpeg, adjust the size
                if (nfo->jpeg)
                    sz = nfo->jpeg;
                if (_prescanImage.size() < static_cast<size_t>(sz))
                    _prescanImage.resize(sz);
                memcpy(_prescanImage.data(), data, sz);
                solumImage.img_ = { reinterpret_cast<uint8_t *>(_prescanImage.data()), sz },
                QApplication::postEvent(_solum.get(), new event::Image(PRESCAN_EVENT, solumImage, false));
            }
        },
        // new spectrum callback
        [](const void* img, const CusSpectralImageInfo* nfo)
        {
            size_t sz = nfo->lines * nfo->samples * (nfo->bitsPerSample / 8);
            // we need to perform a deep copy of the spectrum data since we have to post the event (yes this happens a lot with this api)
            if (_spectrum.size() < sz)
                _spectrum.resize(sz);
            memcpy(_spectrum.data(), img, sz);
            QApplication::postEvent(_solum.get(), new event::SpectrumImage(_spectrum.data(), nfo->lines, nfo->samples, nfo->bitsPerSample));
        },
        // new imu port callback
        [](int port)
        {
            QApplication::postEvent(_solum.get(), new event::ImuPort(port));
        },
        // new imu data callback
        [](const CusPosInfo* pos)
        {
            QQuaternion imu;
            if (pos)
                imu = QQuaternion(static_cast<float>(pos->qw), static_cast<float>(pos->qx), static_cast<float>(pos->qy), static_cast<float>(pos->qz));
            QApplication::postEvent(_solum.get(), new event::Imu(IMU_EVENT, imu));
        },
        // imaging state change callback
        [](CusImagingState state, int imaging)
        {
            // post event here, as the gui (statusbar) will be updated directly, and it needs to come from the application thread
            QApplication::postEvent(_solum.get(), new event::Imaging(state, imaging ? true : false));
        },
        // button press callback
        [](CusButton btn, int clicks)
        {
            // post event here, as the gui (statusbar) will be updated directly, and it needs to come from the application thread
            QApplication::postEvent(_solum.get(), new event::Button(btn, clicks));
        },
        // error message callback
        [](const char* err)
        {
            // post event here, as the gui (statusbar) will be updated directly, and it needs to come from the application thread
            QApplication::postEvent(_solum.get(), new event::Error(err));
        },
        width, height) != 0)
    {
        qDebug() << "error initializing listener";
        return -1;
    }

    solumSetTeeFn(
        [](bool connected, const char* serial, double timeRemaining, const char* id, const char* name, const char* exam)
        {
            // post event here, as the gui (statusbar) will be updated directly, and it needs to come from the application thread
            QApplication::postEvent(_solum.get(), new event::Tee(connected, QString::fromLatin1(serial), timeRemaining,
                QString::fromLatin1(id), QString::fromLatin1(name), QString::fromLatin1(exam)));
        });

    print_firmware_version();

    // CONFIGURE PROBE SETTINGS
    CusProbeSettings settings;
    settings.contactDetection = 0;
    settings.autoFreeze = 0;
    settings.keepAwake = 0;
    settings.deepSleep = 0;
    settings.wifiOptimization = 0;
    if (solumSetProbeSettings(&settings) != 0)
    {
        std::cerr << "Failed to set Clarius OEM probe settings" << std::endl;
    }

    _solum->show();
    const int result = a.exec();
    solumDestroy();
    _solum.reset();
    return result;
}
