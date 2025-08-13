#pragma once

#include "ble.h"
#include <solum/solum_def.h>

namespace Ui
{
    class Solum;
}

class UltrasoundImage;
class Spectrum;
class RfSignal;
class Prescan;
class ProbeRender;

#define CONNECT_EVENT           static_cast<QEvent::Type>(QEvent::User + 1)
#define CERT_EVENT              static_cast<QEvent::Type>(QEvent::User + 2)
#define POWER_EVENT             static_cast<QEvent::Type>(QEvent::User + 3)
#define SWUPDATE_EVENT          static_cast<QEvent::Type>(QEvent::User + 4)
#define LIST_EVENT              static_cast<QEvent::Type>(QEvent::User + 5)
#define IMAGE_EVENT             static_cast<QEvent::Type>(QEvent::User + 6)
#define PRESCAN_EVENT           static_cast<QEvent::Type>(QEvent::User + 7)
#define SPECTRUM_EVENT          static_cast<QEvent::Type>(QEvent::User + 8)
#define RF_EVENT                static_cast<QEvent::Type>(QEvent::User + 9)
#define IMAGING_EVENT           static_cast<QEvent::Type>(QEvent::User + 10)
#define BUTTON_EVENT            static_cast<QEvent::Type>(QEvent::User + 11)
#define ERROR_EVENT             static_cast<QEvent::Type>(QEvent::User + 12)
#define PROGRESS_EVENT          static_cast<QEvent::Type>(QEvent::User + 13)
#define TEE_EVENT               static_cast<QEvent::Type>(QEvent::User + 14)
#define IMU_EVENT               static_cast<QEvent::Type>(QEvent::User + 15)
#define RAWAVAIL_EVENT          static_cast<QEvent::Type>(QEvent::User + 16)
#define RAWREADY_EVENT          static_cast<QEvent::Type>(QEvent::User + 17)
#define RAWDOWNLOADED_EVENT     static_cast<QEvent::Type>(QEvent::User + 18)
#define IMU_PORT_EVENT          static_cast<QEvent::Type>(QEvent::User + 19)
#define BATTERY_HEALTH_EVENT    static_cast<QEvent::Type>(QEvent::User + 20)
#define ELEMENT_TEST_EVENT      static_cast<QEvent::Type>(QEvent::User + 21)

namespace event
{
    /// wrapper for connection events that can be posted from the api callbacks
    class Connection : public QEvent
    {
    public:
        /// default constructor
        /// @param[in] res the connection result
        /// @param[in] port the connection port
        /// @param[in] msg connection message
        Connection(CusConnection res, int port, const QString& msg) : QEvent(CONNECT_EVENT), result_(res), port_(port), message_(msg) { }

        CusConnection result_;  ///< connection result
        int port_;              ///< connection port
        QString message_;       ///< message
    };

    /// wrapper for certificate validation events that can be posted from the api callbacks
    class Cert : public QEvent
    {
    public:
        /// default constructor
        /// @param[in] daysValid days valid for certificate
        explicit Cert(int daysValid) : QEvent(CERT_EVENT), daysValid_(daysValid) { }

        int daysValid_;     ///< days valid
    };

    /// wrapper for power down events that can be posted from the api callbacks
    class PowerDown : public QEvent
    {
    public:
        /// default constructor
        /// @param[in] res the power down reason
        /// @param[in] tm any timeout associated
        PowerDown(CusPowerDown res, int tm) : QEvent(POWER_EVENT), res_(res), timeOut_(tm) { }

        CusPowerDown res_;  ///< power down reason
        int timeOut_;       ///< associated timeout
    };

    /// wrapper for software update that can be posted from the api callbacks
    class SwUpdate : public QEvent
    {
    public:
        /// default constructor
        /// @param[in] res the sw update code
        explicit SwUpdate(CusSwUpdate res) : QEvent(SWUPDATE_EVENT), res_(res)  { }

        CusSwUpdate res_;  ///< the software update code
    };

    /// wrapper for list events
    class List : public QEvent
    {
    public:
        /// default constructor
        List(const char* list, bool probes) : QEvent(LIST_EVENT), probes_(probes)
        {
            QString buf = QString::fromLatin1(list);
            list_ = buf.split(',');
        }

        QStringList list_;  ///< resultant list
        bool probes_;       ///< flag for probes vs applications
    };

    /// wrapper for new image events that can be posted from the api callbacks
    class Image : public QEvent
    {
    public:
        /// default constructor
        /// @param[in] evt the event type
        /// @param[in] data the image data
        /// @param[in] w the image width
        /// @param[in] h the image height
        /// @param[in] bpp the image bits per pixel
        /// @param[in] format the image format
        /// @param[in] sz total size of the image
        /// @param[in] overlay flag if the image came from a separated overlay
        /// @param[in] imu latest imu data if sent
        Image(QEvent::Type evt, const void* data, int w, int h, int bpp, CusImageFormat format, int sz, bool overlay, const QQuaternion& imu) : QEvent(evt),
            data_(data), width_(w), height_(h), bpp_(bpp), format_(format), size_(sz), overlay_(overlay), imu_(imu) { }

        const void* data_;      ///< pointer to the image data
        int width_;             ///< width of the image
        int height_;            ///< height of the image
        int bpp_ ;              ///< bits per pixel
        CusImageFormat format_; ///< image format
        int size_;              ///< total size of image
        bool overlay_;          ///< flag if the image came from a separated overlay
        QQuaternion imu_;       ///< latest imu position
    };

    /// wrapper for new spectrum events that can be posted from the api callbacks
    class SpectrumImage : public QEvent
    {
    public:
        /// default constructor
        /// @param[in] data the spectrum data
        /// @param[in] l the # of lines in the spectrum
        /// @param[in] s the # of samples in the spectrum
        /// @param[in] bps the image bits per sample
        SpectrumImage(const void* data, int l, int s, int bps) : QEvent(SPECTRUM_EVENT),
            data_(data), lines_(l), samples_(s), bps_(bps) { }

        const void* data_;  ///< pointer to the image data
        int lines_;         ///< # of lines in the spectrum
        int samples_;       ///< # of samples in the spectrum
        int bps_ ;          ///< bits per sample
    };

    /// wrapper for new rf events that can be posted from the api callbacks
    class RfImage : public Image
    {
    public:
        /// default constructor
        /// @param[in] data the rf data
        /// @param[in] l # of rf lines
        /// @param[in] s # of samples per line
        /// @param[in] bps bits per sample
        /// @param[in] sz total size of the image
        /// @param[in] lateral lateral spacing between lines
        /// @param[in] axial sample size
        RfImage(const void* data, int l, int s, int bps, int sz, double lateral, double axial) : Image(RF_EVENT, data, l, s, bps, Uncompressed, sz, false, QQuaternion()), lateral_(lateral), axial_(axial) { }

        double lateral_;    ///< spacing between each line
        double axial_;      ///< sample size
    };

    /// wrapper for imaging state events that can be posted from the api callbacks
    class Imaging : public QEvent
    {
    public:
        /// default constructor
        /// @param[in] state the ready state
        /// @param[in] imaging the imaging state
        Imaging(CusImagingState state, bool imaging) : QEvent(IMAGING_EVENT), state_(state), imaging_(imaging) { }

        CusImagingState state_; ///< the imaging ready state
        bool imaging_;          ///< the imaging state
    };

    /// wrapper for button press events that can be posted from the api callbacks
    class Button : public QEvent
    {
    public:
        /// default constructor
        /// @param[in] btn the button pressed
        /// @param[in] clicks # of clicks
        Button(CusButton btn, int clicks) : QEvent(BUTTON_EVENT), button_(btn), clicks_(clicks) { }

        CusButton button_;  ///< button pressed
        int clicks_;        ///< # of clicks
    };

    /// wrapper for button press events that can be posted from the api callbacks
    class Tee : public QEvent
    {
    public:
        /// default constructor
        /// @param[in] connected the connected flag
        /// @param[in] serial the serial number
        /// @param[in] timeRemaining the time remaining in percent
        /// @param[in] id the patient id
        /// @param[in] name the patient name
        /// @param[in] exam the exam id
        Tee(bool connected, const QString& serial, double timeRemaining, const QString& id, const QString& name, const QString& exam) :
            QEvent(TEE_EVENT), connected_(connected), serial_(serial), timeRemaining_(timeRemaining), id_(id), name_(name), exam_(exam) { }

        bool connected_;        ///< connected flag
        QString serial_;        ///< serial number
        double timeRemaining_;  ///< time remaining in percent
        QString id_;            ///< patient id
        QString name_;          ///< patient name
        QString exam_;          ///< exam id
    };

    /// wrapper for new imu data events that can be posted from the api callbacks
    class ImuPort : public QEvent
    {
    public:
        /// default constructor
        /// @param[in] port latest imu port
        explicit ImuPort(int port) : QEvent(IMU_PORT_EVENT), port_(port) { }

        int port_;   ///< latest imu port
    };

    /// wrapper for new imu data events that can be posted from the api callbacks
    class Imu : public QEvent
    {
    public:
        /// default constructor
        /// @param[in] imu latest imu data
        explicit Imu(const QQuaternion& imu) : QEvent(IMU_EVENT), imu_(imu) { }

        QQuaternion imu_;   ///< latest imu position
    };


    /// wrapper for error events that can be posted from the api callbacks
    class Error : public QEvent
    {
    public:
        /// default constructor
        /// @param[in] err the error message
        explicit Error(CusErrorCode code, const QString& err) : QEvent(ERROR_EVENT), code_(code), error_(err) { }

        CusErrorCode code_;     ///< error code
        QString error_;         ///< error message
    };

    /// wrapper for progress events that can be posted from the api callbacks
    class Progress : public QEvent
    {
    public:
        /// default constructor
        /// @param[in] selection progress bar to move
        /// @param[in] progress the current progress
        Progress(int selection, int progress) : QEvent(PROGRESS_EVENT), selection_(selection), progress_(progress) { }

        int selection_; ///< progress bar selection
        int progress_;  ///< the current progress
    };

    /// wrapper for raw availability events that can be posted from the api callbacks
    class RawAvailability : public QEvent
    {
    public:
        /// default constructor
        /// @param[in] res the result of the api call
        /// @param[in] b number of b raw frames available
        /// @param[in] iqrf number of iq/rf raw frames available
        RawAvailability(int res, int b, int iqrf) : QEvent(RAWAVAIL_EVENT), res_(res), b_(b), iqrf_(iqrf) { }

        int res_;   ///< result
        int b_;     ///< b frames
        int iqrf_;  ///< iqrf frames
    };

    /// wrapper for raw ready events that can be posted from the api callbacks
    class RawReady : public QEvent
    {
    public:
        /// default constructor
        /// @param[in] sz size of the buffer created
        /// @param[in] ext extension of the file package
        RawReady(int sz, const QString& ext) : QEvent(RAWREADY_EVENT), sz_(sz), ext_(ext) { }

        int sz_;        ///< size of the package
        QString ext_;   ///< package extension
    };

    /// wrapper for raw downloaded events that can be posted from the api callbacks
    class RawDownloaded: public QEvent
    {
    public:
        /// default constructor
        /// @param[in] res result of the download
        explicit RawDownloaded(int res) : QEvent(RAWDOWNLOADED_EVENT), res_(res) { }

        int res_;   ///< result of the download
    };

    /// wrapper for battery health events that can be posted from the api callbacks
    class BatteryHealth : public QEvent
    {
    public:
        /// default constructor
        /// @param[in] res the result of the api call
        /// @param[in] val the health value
        BatteryHealth(CusBatteryHealth res, double val) : QEvent(BATTERY_HEALTH_EVENT), res_(res), val_(val) { }

        CusBatteryHealth res_;  ///< result
        int val_;               ///< health value
    };

    /// wrapper for element test events that can be posted from the api callbacks
    class ElementTest : public QEvent
    {
    public:
        /// default constructor
        /// @param[in] res the result of the api call
        /// @param[in] val the test result value
        ElementTest(CusElementTest res, double val) : QEvent(ELEMENT_TEST_EVENT), res_(res), val_(val) { }

        CusElementTest res_;    ///< result
        int val_;               ///< test result value
    };
}

/// holds raw data information
class RawData
{
public:
    RawData() : size_(0), ptr_(nullptr) { }

    QString file_;
    int size_;
    QByteArray data_;
    char* ptr_;
};

using Probes = std::map<QString,QString>;

/// solum gui application
class Solum : public QMainWindow
{
    Q_OBJECT

public:
    explicit Solum(QWidget *parent = nullptr);
    ~Solum() override;

protected:
    virtual bool event(QEvent *event) override;
    virtual void closeEvent(QCloseEvent *event) override;

private:
    void loadProbes(const QStringList& probes);
    void loadApplications(const QStringList& probes);
    void newProcessedImage(const void* img, int w, int h, int bpp, CusImageFormat format, int sz, bool overlay, const QQuaternion& imu);
    void newPrescanImage(const void* img, int w, int h, int bpp, int sz, CusImageFormat format);
    void newSpectrumImage(const void* img, int l, int s, int bps);
    void newRfImage(const void* rf, int l, int s, int ss);
    void newImuData(const QQuaternion& imu);
    void setConnected(CusConnection res, int port, const QString& msg);
    void certification(int daysValid);
    void poweringDown(CusPowerDown res, int tm);
    void softwareUpdate(CusSwUpdate res);
    void imagingState(CusImagingState state, bool imaging);
    void onButton(CusButton btn, int clicks);
    void onTee(bool connected, const QString& serial, double timeRemaining);
    void onRawAvailabilityResult(int res, int b, int iqrf);
    void onRawReadyToDownload(int sz, const QString& ext);
    void onRawDownloaded(int res);
    void onBatteryHealthResult(CusBatteryHealth res, double val);
    void onElementTestResult(CusElementTest res, double val);
    void setProgress(int selection, int progress);
    void setError(const QString& err);
    void getParams();
    void updateVelocity(CusMode mode);

public slots:
    void onRetrieve();
    void onBleProbe(int);
    void onBleConnect();
    void onBleSearch();
    void onPowerOn();
    void onPowerOff();
    void onRing();
    void onWiFi();
    void onAp();
    void onConnect();
    void onFreeze();
    void onUpdate();
    void onUpdateCert();
    void onBatteryHealth();
    void onLoad();
    void onProbeSelected(const QString& probe);
    void onMode(int);
    void onZoom(int);
    void incDepth();
    void decDepth();
    void onGain(int);
    void onFocus(int);
    void onColorGain(int);
    void onOpacity(int);
    void onAutoGain(int);
    void onAutoFocus(int);
    void onImu(int);
    void onPrescan(int);
    void onSplit(int);
    void tgcTop(int);
    void tgcMid(int);
    void tgcBottom(int);
    void onFormat(int);
    void onRfStream(int);
    void onRawBuffer(int);
    void onRawAvailability();
    void onRawDownload();
    void onLowLevelFetch();
    void onLowLevelSet();
    void onLowLevelToggle();

private:
    bool connected_;                ///< connection state
    bool imaging_;                  ///< imaging state
    bool teeConnected_;             ///< tee connected state
    uint32_t imuSamples_;           ///< keeps track of samples collected
    uint64_t acquired_;             ///< tracks acquired bytes
    Ui::Solum *ui_;                 ///< ui controls, etc.
    UltrasoundImage* image_;        ///< image display
    UltrasoundImage* image2_;       ///< secondary image display
    Spectrum* spectrum_;            ///< spectrum display
    ProbeRender* render_;           ///< probe renderer
    RfSignal* signal_;              ///< rf signal display
    Prescan* prescan_;              ///< prescan display
    QTimer timer_;                  ///< timer for updating probe status
    QTimer brTimer_;                ///< timer for updating bit rate
    QElapsedTimer elapsed_;         ///< holds elapsed time for bit rate calculations
    QNetworkAccessManager cloud_;   ///< for accessing clarius cloud
    Ble ble_;                       ///< bluetooth module
    Probes certified_;              ///< list of certified probes
    RawData rawData_;               ///< holds raw data info
    CusAcoustic acoustic_;          ///< holds latest acoustic data
    std::unique_ptr<QSettings> settings_;   ///< persistent settings
};
