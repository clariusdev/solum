#pragma once

#include "ble.h"

namespace Ui
{
    class Oem;
}

class UltrasoundImage;
class RfSignal;
class ProbeRender;

#define CONNECT_EVENT   static_cast<QEvent::Type>(QEvent::User + 1)
#define CERT_EVENT      static_cast<QEvent::Type>(QEvent::User + 2)
#define POWER_EVENT     static_cast<QEvent::Type>(QEvent::User + 3)
#define SWUPDATE_EVENT  static_cast<QEvent::Type>(QEvent::User + 4)
#define LIST_EVENT      static_cast<QEvent::Type>(QEvent::User + 5)
#define IMAGE_EVENT     static_cast<QEvent::Type>(QEvent::User + 6)
#define PRESCAN_EVENT   static_cast<QEvent::Type>(QEvent::User + 7)
#define RF_EVENT        static_cast<QEvent::Type>(QEvent::User + 8)
#define IMAGING_EVENT   static_cast<QEvent::Type>(QEvent::User + 9)
#define BUTTON_EVENT    static_cast<QEvent::Type>(QEvent::User + 10)
#define ERROR_EVENT     static_cast<QEvent::Type>(QEvent::User + 11)
#define PROGRESS_EVENT  static_cast<QEvent::Type>(QEvent::User + 12)

namespace event
{
    /// wrapper for connection events that can be posted from the api callbacks
    class Connection : public QEvent
    {
    public:
        /// default constructor
        /// @param[in] code the connection code
        /// @param[in] port the connection port
        /// @param[in] msg connection message
        Connection(int code, int port, const QString& msg) : QEvent(CONNECT_EVENT), code_(code), port_(port), message_(msg) { }

        int code_;          ///< connection code
        int port_;          ///< connection port
        QString message_;   ///< message
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
        /// @param[in] code the power down code
        /// @param[in] tm any timeout associated
        PowerDown(int code, int tm) : QEvent(POWER_EVENT), code_(code), timeOut_(tm) { }

        int code_;      ///< power down code
        int timeOut_;   ///< associated timeout
    };

    /// wrapper for software update that can be posted from the api callbacks
    class SwUpdate : public QEvent
    {
    public:
        /// default constructor
        /// @param[in] code the sw update code
        SwUpdate(int code) : QEvent(SWUPDATE_EVENT), code_(code)  { }

        int code_;  ///< the software update code
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
        /// @param[in] data the image data
        /// @param[in] w the image width
        /// @param[in] h the image height
        /// @param[in] bpp the image bits per pixel
        /// @param[in] imu latest imu data if sent
        Image(QEvent::Type evt, const void* data, int w, int h, int bpp, const QQuaternion& imu) : QEvent(evt),
            data_(data), width_(w), height_(h), bpp_(bpp), imu_(imu) { }

        const void* data_;  ///< pointer to the image data
        int width_;         ///< width of the image
        int height_;        ///< height of the image
        int bpp_;           ///< bits per pixel of the image (should always be 32)
        QQuaternion imu_;   ///< latest imu position
    };

    /// wrapper for new data events that can be posted from the api callbacks
    class PreScanImage : public Image
    {
    public:
        /// default constructor
        /// @param[in] data the image data
        /// @param[in] w the image width
        /// @param[in] h the image height
        /// @param[in] bpp the image bits per sample
        /// @param[in] jpg the jpeg compression flag for the data
        PreScanImage(const void* data, int w, int h, int bpp, int jpg) : Image(PRESCAN_EVENT, data, w, h, bpp, QQuaternion()), jpeg_(jpg) { }

        bool jpeg_; ///< size of jpeg compressed image
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
        /// @param[in] lateral lateral spacing between lines
        /// @param[in] axial sample size
        RfImage(const void* data, int l, int s, int bps, double lateral, double axial) : Image(RF_EVENT, data, l, s, bps, QQuaternion()), lateral_(lateral), axial_(axial) { }

        double lateral_;    ///< spacing between each line
        double axial_;      ///< sample size
    };

    /// wrapper for imaging state events that can be posted from the api callbacks
    class Imaging : public QEvent
    {
    public:
        /// default constructor
        /// @param[in] rdy the ready state
        /// @param[in] imaging the imaging state
        Imaging(int rdy, bool imaging) : QEvent(IMAGING_EVENT), ready_(rdy), imaging_(imaging) { }

        int ready_;     ///< the ready state
        bool imaging_;  ///< the imaging state
    };

    /// wrapper for button press events that can be posted from the api callbacks
    class Button : public QEvent
    {
    public:
        /// default constructor
        /// @param[in] btn the button pressed
        /// @param[in] clicks # of clicks
        Button(int btn, int clicks) : QEvent(BUTTON_EVENT), button_(btn), clicks_(clicks) { }

        int button_;    ///< button pressed, 0 = up, 1 = down
        int clicks_;    ///< # of clicks
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

/// oem gui application
class Oem : public QMainWindow
{
    Q_OBJECT

public:
    explicit Oem(QWidget *parent = nullptr);
    ~Oem() override;

    static Oem* instance();

protected:
    virtual bool event(QEvent *event) override;
    virtual void closeEvent(QCloseEvent *event) override;

private:
    void loadProbes(const QStringList& probes);
    void loadApplications(const QStringList& probes);
    void newProcessedImage(const void* img, int w, int h, int bpp, const QQuaternion& imu);
    void newPrescanImage(const void* img, int w, int h, int bpp, bool jpg);
    void newRfImage(const void* rf, int l, int s, int ss);
    void setConnected(int code, int port, const QString& msg);
    void certification(int daysValid);
    void poweringDown(int code, int tm);
    void softwareUpdate(int code);
    void imagingState(int code, bool imaging);
    void onButton(int btn, int clicks);
    void setProgress(int progress);
    void setError(const QString& err);
    void getParams();

public slots:
    void onBleProbe(int);
    void onBleConnect();
    void onBleSearch();
    void onPowerOn();
    void onPowerOff();
    void onWiFi();
    void onAp();
    void onConnect();
    void onFreeze();
    void onUpdate();
    void onUpdateCert();
    void onLoad();
    void onProbeSelected(const QString& probe);
    void onMode(int);
    void onZoom(int);
    void incDepth();
    void decDepth();
    void onGain(int);
    void onColorGain(int);
    void onAutoGain(int);
    void onImu(int);
    void tgcTop(int);
    void tgcMid(int);
    void tgcBottom(int);

private:
    bool connected_;            ///< connection state
    bool imaging_;              ///< imaging state
    Ui::Oem *ui_;               ///< ui controls, etc.
    UltrasoundImage* image_;    ///< image display
    ProbeRender* render_;       ///< probe renderer
    RfSignal* signal_;          ///< rf signal display
    QImage prescan_;            ///< pre-scan converted image
    QTimer timer_;              ///< timer for updating probe status
    Ble ble_;                   ///< bluetooth module
    static std::unique_ptr<Oem> oem_;
};
