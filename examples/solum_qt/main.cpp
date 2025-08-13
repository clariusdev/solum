#include "solumqt.h"
#include <memory>
#include <solum/solum.h>
#include <iostream>

static std::unique_ptr<Solum> _solum;
static std::vector<char> _image;
static std::vector<char> _prescanImage;
static std::vector<char> _spectrum;
static std::vector<char> _rfData;

void printFirmwareVersions()
{
    char buffer[64];
    auto get_version = [&buffer](CusPlatform platform) -> QString
    {
        if (CUS_SUCCESS != solumFwVersion(platform, buffer, static_cast<int>(std::size(buffer))))
            return QStringLiteral("Error");

        buffer [std::size(buffer) - 1] = '\0';
        return QString::fromLatin1(buffer);
    };

    qDebug() << "Firmware Versions";
    qDebug() << "V1:" << get_version(CusPlatform::V1);
    qDebug() << "HD:" << get_version(CusPlatform::HD);
    qDebug() << "HD3:" << get_version(CusPlatform::HD3);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName(QStringLiteral("Clarius"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("clarius.com"));
    QCoreApplication::setApplicationName(QStringLiteral("Solum Demo"));

    const int width  = 640; // width of the rendered image
    const int height = 480; // height of the rendered image

    _solum = std::make_unique<Solum>();

    auto storeDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toLocal8Bit();
    auto initParams = solumDefaultInitParams();
    initParams.args.argc = argc;
    initParams.args.argv = argv;
    initParams.storeDir = storeDir.constData();
    initParams.width = width;
    initParams.height = height;

    initParams.connectFn =
        [](CusConnection res, int port, const char* msg)
        {
            QApplication::postEvent(_solum.get(), new event::Connection(res, port, QString::fromLatin1(msg)));
        };

    initParams.certFn =
        [](int daysValid)
        {
            QApplication::postEvent(_solum.get(), new event::Cert(daysValid));
        };

    initParams.powerDownFn =
        [](CusPowerDown res, int tm)
        {
            QApplication::postEvent(_solum.get(), new event::PowerDown(res, tm));
        };

    initParams.newProcessedImageFn =
        [](const void* img, const CusProcessedImageInfo* nfo, int npos, const CusPosInfo* pos)
        {
            int sz = nfo->imageSize;
            // we need to perform a deep copy of the image data since we have to post the event (yes this happens a lot with this api)
            if (_image.size() < static_cast<size_t>(sz))
                _image.resize(sz);
            std::memcpy(_image.data(), img, sz);
            QQuaternion imu;
            imu.setScalar(0.0);
            if (npos && pos)
                imu = QQuaternion(static_cast<float>(pos[0].qw), static_cast<float>(pos[0].qx), static_cast<float>(pos[0].qy), static_cast<float>(pos[0].qz));

            QApplication::postEvent(_solum.get(), new event::Image(IMAGE_EVENT, _image.data(), nfo->width, nfo->height, nfo->bitsPerPixel, nfo->format, sz, nfo->overlay, imu));
        };

    initParams.newRawImageFn =
        [](const void* data, const CusRawImageInfo* nfo, int, const CusPosInfo*)
        {
            // we need to perform a deep copy of the image data since we have to post the event (yes this happens a lot with this api)
            int sz = nfo->lines * nfo->samples * (nfo->bitsPerSample / 8);
            if (nfo->rf)
            {
                if (_rfData.size() < static_cast<size_t>(sz))
                    _rfData.resize(sz);
                std::memcpy(_rfData.data(), data, sz);
                QApplication::postEvent(_solum.get(), new event::RfImage(_rfData.data(), nfo->lines, nfo->samples, nfo->bitsPerSample, sz,
                                                                            nfo->lateralSize, nfo->axialSize));
            }
            else
            {
                // image may be a jpeg, adjust the size
                if (nfo->jpeg)
                    sz = nfo->jpeg;
                if (_prescanImage.size() < static_cast<size_t>(sz))
                    _prescanImage.resize(sz);
                std::memcpy(_prescanImage.data(), data, sz);
                QApplication::postEvent(_solum.get(), new event::Image(PRESCAN_EVENT, _prescanImage.data(), nfo->lines, nfo->samples,
                                                                       nfo->bitsPerSample, nfo->jpeg ? Jpeg : Uncompressed8Bit, sz, false, QQuaternion()));
            }
        };

    initParams.newSpectralImageFn =
        [](const void* img, const CusSpectralImageInfo* nfo)
        {
            size_t sz = nfo->lines * nfo->samples * (nfo->bitsPerSample / 8);
            // we need to perform a deep copy of the spectrum data since we have to post the event (yes this happens a lot with this api)
            if (_spectrum.size() < sz)
                _spectrum.resize(sz);
            std::memcpy(_spectrum.data(), img, sz);
            QApplication::postEvent(_solum.get(), new event::SpectrumImage(_spectrum.data(), nfo->lines, nfo->samples, nfo->bitsPerSample));
    };

    initParams.newImuPortFn =
        [](int port)
        {
            QApplication::postEvent(_solum.get(), new event::ImuPort(port));
        };

    initParams.newImuDataFn =
        [](const CusPosInfo* pos)
        {
            QQuaternion imu;
            imu.setScalar(0.0);
            if (pos)
                imu = QQuaternion(static_cast<float>(pos->qw), static_cast<float>(pos->qx), static_cast<float>(pos->qy), static_cast<float>(pos->qz));
            QApplication::postEvent(_solum.get(), new event::Imu(imu));
        };

    initParams.imagingFn =
        [](CusImagingState state, int imaging)
        {
            // post event here, as the gui (statusbar) will be updated directly, and it needs to come from the application thread
            QApplication::postEvent(_solum.get(), new event::Imaging(state, imaging ? true : false));
        };

    initParams.buttonFn =
        [](CusButton btn, int clicks)
        {
            // post event here, as the gui (statusbar) will be updated directly, and it needs to come from the application thread
            QApplication::postEvent(_solum.get(), new event::Button(btn, clicks));
        };

    initParams.errorFn =
        [](CusErrorCode code, const char* err)
        {
            // post event here, as the gui (statusbar) will be updated directly, and it needs to come from the application thread
            QApplication::postEvent(_solum.get(), new event::Error(code, err));
    };

    initParams.elemTestFn  =
        [](CusElementTest res, double val)
    {
        // post event here, as the gui (statusbar) will be updated directly, and it needs to come from the application thread
        QApplication::postEvent(_solum.get(), new event::ElementTest(res, val));
    };

    if (solumInit(&initParams) != CUS_SUCCESS)
    {
        qDebug() << "error initializing solum";
        return -1;
    }

    solumSetTeeFn(
        [](bool connected, const char* serial, double timeRemaining, const char* id, const char* name, const char* exam)
        {
            // post event here, as the gui (statusbar) will be updated directly, and it needs to come from the application thread
            QApplication::postEvent(_solum.get(), new event::Tee(connected, QString::fromLatin1(serial), timeRemaining,
                QString::fromLatin1(id), QString::fromLatin1(name), QString::fromLatin1(exam)));
        });

    printFirmwareVersions();

    _solum->show();
    const int result = a.exec();
    solumDestroy();
    _solum.reset();
    return result;
}
