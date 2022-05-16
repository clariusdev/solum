#include "oemqt.h"
#include "display.h"
#include "3d.h"
#include "ui_oemqt.h"
#include <oem/oem.h>

#define IMU_TAB     3

static Oem* _me;

/// default constructor
/// @param[in] parent the parent object
Oem::Oem(QWidget *parent) : QMainWindow(parent), connected_(false), imaging_(false), ui_(new Ui::Oem)
{
    _me = this;
    ui_->setupUi(this);
    setWindowIcon(QIcon(":/res/logo.png"));
    image_ = new UltrasoundImage(this);
    signal_ = new RfSignal(this);
    ui_->image->addWidget(image_);
    ui_->image->addWidget(signal_);
    render_ = new ProbeRender(QGuiApplication::primaryScreen());
    ui_->render->addWidget(QWidget::createWindowContainer(render_));
    auto reset = new QPushButton(QStringLiteral("Reset"), this);
    ui_->render->addWidget(reset);
    render_->init(QStringLiteral("scanner.obj"));
    render_->show();
    ui_->rfzoom->setVisible(false);
    ui_->rfStream->setVisible(false);
    ui_->cfigain->setVisible(false);
    ui_->_tabs->setTabEnabled(IMU_TAB, false);

    QObject::connect(reset, &QPushButton::clicked, [this]()
    {
        render_->reset();
    });

    // load probes list
    cusOemProbes([](const char* list, int)
    {
        QApplication::postEvent(_me, new event::List(list, true));
    });

    ui_->modes->blockSignals(true);
    ui_->modes->addItem(QStringLiteral("B"));
    ui_->modes->addItem(QStringLiteral("SC"));
    ui_->modes->addItem(QStringLiteral("CFI"));
    ui_->modes->addItem(QStringLiteral("PDI"));
    ui_->modes->addItem(QStringLiteral("NE"));
    ui_->modes->addItem(QStringLiteral("RF"));
    ui_->modes->blockSignals(false);

    // connect status timer
    connect(&timer_, &QTimer::timeout, [this]()
    {
        CusStatusInfo st;
        if (cusOemStatusInfo(&st) == 0)
        {
            ui_->probeStatus->setText(QStringLiteral("Battery: %1%, Temp: %2%, FR: %3 Hz")
                .arg(st.battery).arg(st.temperature).arg(QString::number(st.frameRate, 'f', 0)));
        }
    });

    // connect ble device list
    connect(&ble_, &Ble::devices, [this](const QStringList& devs)
    {
        ui_->bleprobes->clear();
        for (auto d : devs)
            ui_->bleprobes->addItem(d);
        if (ui_->bleprobes->count())
            ui_->bleprobes->setCurrentIndex(0);
    });

    // power service ready
    connect(&ble_, &Ble::powerReady, [this](bool en)
    {
        ui_->poweron->setEnabled(en);
        ui_->poweroff->setEnabled(en);
        ui_->ring->setEnabled(en);
    });

    // wifi service ready
    connect(&ble_, &Ble::wifiReady, [this](bool en)
    {
        ui_->wifi->setEnabled(en);
        ui_->ap->setEnabled(en);
    });

    // power status sent
    connect(&ble_, &Ble::powered, [this](bool en)
    {
        ui_->status->showMessage(QStringLiteral("Powered: %1").arg(en ? QStringLiteral("On") : QStringLiteral("Off")));
    });

    // wifi info sent
    connect(&ble_, &Ble::wifiInfo, [this](const QString& info)
    {
        // yaml formatted network information
        QStringList network = info.split(QStringLiteral("\n"));
        auto getField = [&network](const QString& field) -> QString
        {
            QString ret;
            auto f = network.filter(field);
            if (f.size())
            {
                ret = f[0];
                ret.replace(field + QStringLiteral(" "), QString{});
            }
            return ret;
        };
        auto ip = getField(QStringLiteral("ip4:"));
        auto port = getField(QStringLiteral("ctl:"));
        auto ssid = getField(QStringLiteral("ssid:"));
        ui_->ip->setText(ip);
        ui_->port->setText(port);
        if (!ip.isEmpty() && !port.isEmpty())
            ui_->status->showMessage(QStringLiteral("Wi-Fi: %1 (%2) [SSID: %3]").arg(ip).arg(port).arg(ssid));
    });
}

/// destructor
Oem::~Oem()
{
    timer_.stop();
    delete ui_;
}

/// loads a list of probes into the selection box
/// @param[in] probes the probes list
void Oem::loadProbes(const QStringList& probes)
{
    ui_->probes->clear();
    for (auto p : probes)
        ui_->probes->addItem(p);
    if (ui_->probes->count())
        ui_->probes->setCurrentIndex(0);
}

/// loads a list of applications into selection box
/// @param[in] apps the applications list
void Oem::loadApplications(const QStringList& apps)
{
    ui_->workflows->clear();
    for (auto a : apps)
        ui_->workflows->addItem(a);
    if (ui_->workflows->count())
        ui_->workflows->setCurrentIndex(0);
}

/// called when the window is closing to clean up the library
void Oem::closeEvent(QCloseEvent*)
{
    if (connected_)
        cusOemDisconnect();

    cusOemDestroy();
}

/// initiates ble search
void Oem::onBleSearch()
{
    ble_.search();
}

/// called when a ble probe is selected
void Oem::onBleProbe(int index)
{
    ui_->bleconnect->setEnabled(index >= 0);
}

/// called when a ble connect is initiated
void Oem::onBleConnect()
{
    ble_.connectToProbe(ui_->bleprobes->currentText());
}

/// tries to power on probe
void Oem::onPowerOn()
{
    ble_.power(true);
}

/// tries to power off probe
void Oem::onPowerOff()
{
    ble_.power(false);
}

/// rings the probe
void Oem::onRing()
{
    ble_.ring();
}

/// tries to reprogram probe to a new wifi network (router)
void Oem::onWiFi()
{
    auto ssid = ui_->ssid->text();
    auto pw = ui_->password->text();
    if (ssid.isEmpty())
        return;

    QString req(QStringLiteral("ap: false\n"));
    req += QStringLiteral("ssid: %1\n").arg(ssid);
    if (!pw.isEmpty())
        req += QStringLiteral("password: %1\n").arg(pw);
    ble_.requestWifi(req);
}

/// tries to repgram probe to it's own access point wifi
void Oem::onAp()
{
    ble_.requestWifi(QStringLiteral("ap: true\nchannel: auto\n"));
}

/// handles custom events posted by oem api callbacks
/// @param[in] event the event to parse
/// @return handling status
bool Oem::event(QEvent *event)
{
    if (event->type() == CONNECT_EVENT)
    {
        auto evt = static_cast<event::Connection*>(event);
        setConnected(evt->result_, evt->port_, evt->message_);
        return true;
    }
    else if (event->type() == CERT_EVENT)
    {
        auto evt = static_cast<event::Cert*>(event);
        certification(evt->daysValid_);
        return true;
    }
    else if (event->type() == POWER_EVENT)
    {
        auto evt = static_cast<event::PowerDown*>(event);
        poweringDown(evt->res_, evt->timeOut_);
        return true;
    }
    else if (event->type() == SWUPDATE_EVENT)
    {
        auto evt = static_cast<event::SwUpdate*>(event);
        softwareUpdate(evt->res_);
        return true;
    }
    else if (event->type() == LIST_EVENT)
    {
        auto evt = static_cast<event::List*>(event);
        if (evt->probes_)
            loadProbes(evt->list_);
        else
            loadApplications(evt->list_);
        return true;
    }
    else if (event->type() == IMAGE_EVENT)
    {
        auto evt = static_cast<event::Image*>(event);
        newProcessedImage(evt->data_, evt->width_, evt->height_, evt->bpp_, evt->size_, evt->imu_);
        return true;
    }
    else if (event->type() == PRESCAN_EVENT)
    {
        auto evt = static_cast<event::Image*>(event);
        newPrescanImage(evt->data_, evt->width_, evt->height_, evt->bpp_, evt->size_);
        return true;
    }
    else if (event->type() == RF_EVENT)
    {
        auto evt = static_cast<event::RfImage*>(event);
        newRfImage(evt->data_, evt->width_, evt->height_, evt->bpp_ / 8);
        return true;
    }
    else if (event->type() == IMAGING_EVENT)
    {
        auto evt = static_cast<event::Imaging*>(event);
        imagingState(evt->state_, evt->imaging_);
        return true;
    }
    else if (event->type() == BUTTON_EVENT)
    {
        auto evt = static_cast<event::Button*>(event);
        onButton(evt->button_, evt->clicks_);
        return true;
    }
    else if (event->type() == PROGRESS_EVENT)
    {
        setProgress((static_cast<event::Progress*>(event))->progress_);
        return true;
    }
    else if (event->type() == ERROR_EVENT)
    {
        setError((static_cast<event::Error*>(event))->error_);
        return true;
    }

    return QMainWindow::event(event);
}

/// called when the api returns an error
/// @param[in] err the error message
void Oem::setError(const QString& err)
{
    ui_->status->showMessage(QStringLiteral("Error: %1").arg(err));
}

/// called when there's a new connection event
/// @param[in] res the connection result
/// @param[in] port the connection port
/// @param[in] msg the associated message
void Oem::setConnected(CusConnection res, int port, const QString& msg)
{
    if (res == ProbeConnected)
    {
        timer_.start(1000);
        connected_ = true;
        ui_->status->showMessage(QStringLiteral("Connected on port: %1").arg(port));
        ui_->connect->setText("Disconnect");
        ui_->update->setEnabled(true);
        ui_->load->setEnabled(true);
    }
    else if (res == ProbeDisconnected)
    {
        timer_.stop();
        connected_ = false;
        ui_->status->showMessage(QStringLiteral("Disconnected"));
        ui_->connect->setText(QStringLiteral("Connect"));
        ui_->cert->clear();
        ui_->freeze->setEnabled(false);
        ui_->update->setEnabled(false);
        ui_->load->setEnabled(false);
        // disable controls upon disconnect
        imagingState(ImagingNotReady, false);
    }
    else if (res == ConnectionFailed || res == ConnectionError)
        ui_->status->showMessage(QStringLiteral("Error connecting: %1").arg(msg));
    else if (res == SwUpdateRequired)
        ui_->status->showMessage(QStringLiteral("Software update required prior to imaging"));
}

/// called when a new certification message has been sent
/// @param[in] daysValid # of days valid for certificate
void Oem::certification(int daysValid)
{
    if (daysValid == CERT_INVALID)
        ui_->cert->setText(QStringLiteral("Invalid"));
    else if (!daysValid)
        ui_->cert->setText(QStringLiteral("Expired"));
    else
        ui_->cert->setText(QStringLiteral("%1 Days").arg(daysValid));
}

/// called when there's a power down event
/// @param[in] res the power down reason
/// @param[in] tm the associated timeout
void Oem::poweringDown(CusPowerDown res, int tm)
{
    if (res == Idle)
        ui_->status->showMessage(QStringLiteral("Idle power down in: %1s").arg(tm));
    else if (res == TooHot)
        ui_->status->showMessage(QStringLiteral("Heating power down in: %1s").arg(tm));
    else if (res == LowBattery)
        ui_->status->showMessage(QStringLiteral("Battery low power down in: %1s").arg(tm));
    else if (res == ButtonOff)
        ui_->status->showMessage(QStringLiteral("Button press power down in: %1s").arg(tm));
}

/// called when there's a software upate notification
/// @param[in] res the software update result
void Oem::softwareUpdate(CusSwUpdate res)
{
    if (res == SwUpdateSuccess)
        ui_->status->showMessage(QStringLiteral("Successfully updated software"));
    else if (res == SwUpdateCurrent)
        ui_->status->showMessage(QStringLiteral("Software already up to date"));
    else
        ui_->status->showMessage(QStringLiteral("Software not updated: %1").arg(res));
}

/// called when the imaging state changes
/// @param[in] state the imaging ready code
/// @param[in] imaging the imaging state
void Oem::imagingState(CusImagingState state, bool imaging)
{
    bool ready = (state == ImagingReady);
    ui_->freeze->setEnabled(ready ? true : false);
    ui_->decdepth->setEnabled(ready ? true : false);
    ui_->incdepth->setEnabled(ready ? true : false);
    ui_->gain->setEnabled(ready ? true : false);
    ui_->cfigain->setEnabled(ready ? true : false);
    ui_->rfzoom->setEnabled(ready ? true : false);
    ui_->rfStream->setEnabled(ready ? true : false);
    ui_->imu->setEnabled(ready ? true : false);
    ui_->autogain->setEnabled(ready ? true : false);
    ui_->tgctop->setEnabled(ready ? true : false);
    ui_->tgcmid->setEnabled(ready ? true : false);
    ui_->tgcbottom->setEnabled(ready ? true : false);
    ui_->modes->setEnabled(ready ? true : false);

    ui_->status->showMessage(QStringLiteral("Image: %1").arg(imaging ? QStringLiteral("Running") : QStringLiteral("Frozen")));
    if (ready)
    {
        ui_->freeze->setText(imaging ? QStringLiteral("Stop") : QStringLiteral("Run"));
        imaging_ = imaging;
        getParams();
        image_->checkRoi();
    }
    else if (state == CertExpired)
        ui_->status->showMessage(QStringLiteral("Certificate needs updating prior to imaging"));
}

/// called when there is a button press on the ultrasound
/// @param[in] btn the button pressed
/// @param[in] clicks # of clicks used
void Oem::onButton(CusButton btn, int clicks)
{
    ui_->status->showMessage(QStringLiteral("Button %1 Pressed, %2 Clicks").arg((btn == ButtonDown) ? QStringLiteral("Down") : QStringLiteral("Up")).arg(clicks));
}

/// called when the download progress changes
/// @param[in] progress the current progress
void Oem::setProgress(int progress)
{
    ui_->progress->setValue(progress);
}

/// called when a new image has been sent
/// @param[in] img the image data
/// @param[in] w width of the image
/// @param[in] h height of the image
/// @param[in] bpp the bits per pixel
/// @param[in] sz size of the image in bytes
/// @param[in] imu the imu data if valid
void Oem::newProcessedImage(const void* img, int w, int h, int bpp, int sz, const QQuaternion& imu)
{
    image_->loadImage(img, w, h, bpp, sz);
    if (!imu.isNull())
        render_->update(imu);
}

/// called when a new pre-scan image has been sent
/// @param[in] img the image data
/// @param[in] w width of the image
/// @param[in] h height of the image
/// @param[in] bpp the bits per pixel
/// @param[in] sz size of the image in bytes
void Oem::newPrescanImage(const void* img, int w, int h, int bpp, int sz)
{
    if (sz == (w * h * (bpp / 8)))
        prescan_ = QImage(reinterpret_cast<const uchar*>(img), w, h, QImage::Format_ARGB32);
    else
        prescan_.loadFromData(static_cast<const uchar*>(img), sz, "JPG");
}

/// called when new rf data has been sent
/// @param[in] rf the rf data
/// @param[in] l # of rf lines
/// @param[in] s # of rf samples per line
/// @param[in] ss sample size (should always be 2)
void Oem::newRfImage(const void* rf, int l, int s, int ss)
{
    signal_->loadSignal(rf, l, s, ss);
}

/// called when the connect/disconnect button is clicked
void Oem::onConnect()
{
    if (!connected_)
    {
        if (cusOemConnect(ui_->ip->text().toStdString().c_str(), ui_->port->text().toInt()) < 0)
            ui_->status->showMessage(QStringLiteral("Connection failed"));
        else
            ui_->status->showMessage(QStringLiteral("Trying connection"));
    }
    else
    {
        if (cusOemDisconnect() < 0)
            ui_->status->showMessage(QStringLiteral("Disconnect failed"));
    }
}

/// called when the freeze button is clicked
void Oem::onFreeze()
{
    if (!connected_)
        return;

    if (cusOemRun(imaging_ ? 0 : 1) < 0)
        ui_->status->showMessage(QStringLiteral("Error requesting imaging run/stop"));
    else
        imagingState(ImagingReady, !imaging_);
}

/// initiates a software update
void Oem::onUpdate()
{
    if (!connected_)
        return;

    if (cusOemSoftwareUpdate(
        // software update result
        [](CusSwUpdate res)
        {
            QApplication::postEvent(_me, new event::SwUpdate(res));
        },
        // download progress
        [](int progress)
        {
            QApplication::postEvent(_me, new event::Progress(progress));
        }) < 0)
        ui_->status->showMessage(QStringLiteral("Error requesting software update"));
}

/// called to load a certificate
void Oem::onUpdateCert()
{
    auto cert = QFileDialog::getOpenFileName(this,
        QStringLiteral("Load Certificate"), QString(), QStringLiteral("Certs (*.pem *.crt);;All Files (*)"));
    if (cert.isEmpty())
        return;

    QFile f(cert);
    if (f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&f);
        stream.setCodec("UTF-8");
        auto text = stream.readAll();
        cusOemSetCert(text.toStdString().c_str());
    }
}

/// initiates a workflow load
void Oem::onLoad()
{
    if (!connected_)
        return;

    if (cusOemLoadApplication(ui_->probes->currentText().toStdString().c_str(), ui_->workflows->currentText().toStdString().c_str()) < 0)
        ui_->status->showMessage(QStringLiteral("Error requesting application load"));
}

/// called when user selects a new probe definition
/// @param[in] probe the probe selected
void Oem::onProbeSelected(const QString &probe)
{
    if (!probe.isEmpty())
    {
        cusOemApplications(probe.toStdString().c_str(), [](const char* list, int)
        {
            QApplication::postEvent(_me, new event::List(list, false));
        });
    }
}

/// increases the depth
void Oem::incDepth()
{
    auto v = cusOemGetParam(ImageDepth);
    if (v != -1)
        cusOemSetParam(ImageDepth, v + 1.0);
}

/// decreases the depth
void Oem::decDepth()
{
    auto v = cusOemGetParam(ImageDepth);
    if (v > 1.0)
        cusOemSetParam(ImageDepth, v - 1.0);
}

/// called when gain adjusted
/// @param[in] gn the gain level
void Oem::onGain(int gn)
{
    cusOemSetParam(Gain, gn);
}

/// called when color gain adjusted
/// @param[in] gn the gain level
void Oem::onColorGain(int gn)
{
    cusOemSetParam(ColorGain, gn);
}

/// called when auto gain enable adjusted
/// @param[in] state checkbox state
void Oem::onAutoGain(int state)
{
    cusOemSetParam(AutoGain, (state == Qt::Checked) ? 1 : 0);
}

/// called when imu enable adjusted
/// @param[in] state checkbox state
void Oem::onImu(int state)
{
    ui_->_tabs->setTabEnabled(IMU_TAB, (state == Qt::Checked));
    cusOemSetParam(ImuStreaming, (state == Qt::Checked) ? 1 : 0);
}

/// called when rf stream enable adjusted
/// @param[in] state checkbox state
void Oem::onRfStream(int state)
{
    cusOemSetParam(RfStreaming, (state == Qt::Checked) ? 1 : 0);
}

/// sets the tgc top
/// @param[in] v the tgc value
void Oem::tgcTop(int v)
{
    CusTgc t;
    t.top = v;
    t.mid = ui_->tgcmid->value();
    t.bottom = ui_->tgcbottom->value();
    cusOemSetTgc(&t);
}

/// sets the tgc mid
/// @param[in] v the tgc value
void Oem::tgcMid(int v)
{
    CusTgc t;
    t.top = ui_->tgctop->value();
    t.mid = v;
    t.bottom = ui_->tgcbottom->value();
    cusOemSetTgc(&t);
}

/// sets the tgc bottom
/// @param[in] v the tgc value
void Oem::tgcBottom(int v)
{
    CusTgc t;
    t.top = ui_->tgctop->value();
    t.mid = ui_->tgcmid->value();
    t.bottom = v;
    cusOemSetTgc(&t);
}

/// get the initial parameter values
void Oem::getParams()
{
    auto v = cusOemGetParam(ImageDepth);
    if (v != -1 && image_)
        image_->setDepth(v);

    v = cusOemGetParam(AutoGain);
    ui_->autogain->setChecked(v > 0);
    v = cusOemGetParam(ImuStreaming);
    ui_->imu->setChecked(v > 0);
    v = cusOemGetParam(RfStreaming);
    ui_->rfStream->setChecked(v > 0);

    CusTgc t;
    if (cusOemGetTgc(&t) == 0)
    {
        ui_->tgctop->setValue(static_cast<int>(t.top));
        ui_->tgcmid->setValue(static_cast<int>(t.mid));
        ui_->tgcbottom->setValue(static_cast<int>(t.bottom));
    }
}

/// called on a mode change
void Oem::onMode(int mode)
{
    auto m = static_cast<CusMode>(mode);
    if (cusOemSetMode(m) < 0)
        ui_->status->showMessage(QStringLiteral("Error setting imaging mode"));
    else
    {
        signal_->setVisible(m == RfMode);
        ui_->rfzoom->setVisible(m == RfMode);
        ui_->rfStream->setVisible(m == RfMode);
        ui_->cfigain->setVisible(m == ColorMode || m == PowerMode);
    }
}

/// called when rf zoom adjusted
void Oem::onZoom(int zoom)
{
    signal_->setZoom(zoom);
}
