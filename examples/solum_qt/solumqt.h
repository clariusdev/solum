#pragma once

#include "ble.h"
#include "ui_solumqt.h"
#include <solum/solum_def.h>

class UltrasoundImage;
class Spectrum;
class RfSignal;
class Prescan;
class ProbeRender;

#define CONNECT_EVENT   static_cast<QEvent::Type>(QEvent::User + 1)
#define CERT_EVENT      static_cast<QEvent::Type>(QEvent::User + 2)
#define POWER_EVENT     static_cast<QEvent::Type>(QEvent::User + 3)
#define SWUPDATE_EVENT  static_cast<QEvent::Type>(QEvent::User + 4)
#define LIST_EVENT      static_cast<QEvent::Type>(QEvent::User + 5)
#define IMAGE_EVENT     static_cast<QEvent::Type>(QEvent::User + 6)
#define PRESCAN_EVENT   static_cast<QEvent::Type>(QEvent::User + 7)
#define SPECTRUM_EVENT  static_cast<QEvent::Type>(QEvent::User + 8)
#define RF_EVENT        static_cast<QEvent::Type>(QEvent::User + 9)
#define IMAGING_EVENT   static_cast<QEvent::Type>(QEvent::User + 10)
#define BUTTON_EVENT    static_cast<QEvent::Type>(QEvent::User + 11)
#define ERROR_EVENT     static_cast<QEvent::Type>(QEvent::User + 12)
#define PROGRESS_EVENT  static_cast<QEvent::Type>(QEvent::User + 13)
#define TEE_EVENT       static_cast<QEvent::Type>(QEvent::User + 14)

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
        Cert(int daysValid) : QEvent(CERT_EVENT), daysValid_(daysValid) { }

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
        SwUpdate(CusSwUpdate res) : QEvent(SWUPDATE_EVENT), res_(res)  { }

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

    /// wrapper for error events that can be posted from the api callbacks
    class Error : public QEvent
    {
    public:
        /// default constructor
        /// @param[in] err the error message
        Error(const QString& err) : QEvent(ERROR_EVENT), error_(err) { }

        QString error_;     ///< the error message
    };

    /// wrapper for progress events that can be posted from the api callbacks
    class Progress : public QEvent
    {
    public:
        /// default constructor
        /// @param[in] progress the current progress
        Progress(int progress) : QEvent(PROGRESS_EVENT), progress_(progress) { }

        int progress_;  ///< the current progress
    };
}

struct ProbeData
{
    QString crt;
    QString model;
};

using Probes = std::map<QString, ProbeData>;

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
    void loadApplications(const QStringList& probes);
    void newProcessedImage(const void* img, int w, int h, int bpp, CusImageFormat format, int sz, bool overlay, const QQuaternion& imu);
    void newPrescanImage(const void* img, int w, int h, int bpp, int sz, CusImageFormat format);
    void newSpectrumImage(const void* img, int l, int s, int bps);
    void newRfImage(const void* rf, int l, int s, int ss);
    void reflectCertification();
    void reflectProbeModelAndWorkflows();
    void reflectMode(CusMode m);
    void setConnected(CusConnection res, int port, const QString& msg);
    void certification(int daysValid);
    void poweringDown(CusPowerDown res, int tm);
    void softwareUpdate(CusSwUpdate res);
    void imagingState(CusImagingState state, bool imaging);
    void onButton(CusButton btn, int clicks);
    void onTee(bool connected, const QString& serial, double timeRemaining);
    void setProgress(int progress);
    void addStatus(const QString &status);
    void addError(const QString &err);
    void getParams();
    void updateVelocity(CusMode mode);
    void connectPortChanged(QLineEdit* portEdit, QPushButton* connectButton);

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
    void onTcpConnect();
    void onImaging();
    void onUpdate();
    void onLoad();
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
    void onRfStream(int);
    void onPrescan(int);
    void onSplit(int);
    void tgcTop(int);
    void tgcMid(int);
    void tgcBottom(int);

private:
    QString bleConnectedProbe_;     ///< serial of the probe connected via BLE
    bool tcpConnected_ = false;     ///< TCP connection state
    bool imaging_;                  ///< imaging state
    bool teeConnected_;             ///< tee connected state
    QString tcpConnectedProbe_;     ///< serial of the probe connected via TCP
    Ui::Solum ui_;                  ///< ui controls, etc.
    UltrasoundImage* image_;        ///< image display
    UltrasoundImage* image2_;       ///< secondary image display
    Spectrum* spectrum_;            ///< spectrum display
    ProbeRender* render_;           ///< probe renderer
    RfSignal* signal_;              ///< rf signal display
    Prescan* prescan_;              ///< prescan display
    QTimer timer_;                  ///< timer for updating probe status
    QNetworkAccessManager cloud_;   ///< for accessing clarius cloud
    Ble ble_;                       ///< bluetooth module
    QStringList probesSupported_;   ///< result of solumProbes() API call
    Probes certified_;              ///< list of certified probes
    std::string activeWorkflow_;    ///< preserves parameters across imaging runs
    std::unique_ptr<QSettings> settings_;   ///< persistent settings
    QIntValidator* portValidator_;  ///< keeps port fields between 1 and 65535
    QString portError_;             ///< error message for port validation
};
