#include "oemqt.h"
#include "display.h"
#include "ui_oemqt.h"
#include <oem/oem.h>

std::unique_ptr<Oem> Oem::oem_;

/// retrieves the oem window instance
/// @return the oem window instance
Oem* Oem::instance()
{
    if (!oem_)
        oem_ = std::make_unique<Oem>();
    return oem_.get();
}

/// default constructor
/// @param[in] parent the parent object
Oem::Oem(QWidget *parent) : QMainWindow(parent), connected_(false), imaging_(false), ui_(new Ui::Oem)
{
    ui_->setupUi(this);
    image_ = new UltrasoundImage(this);
    signal_ = new RfSignal(this);
    ui_->image->addWidget(image_);
    ui_->image->addWidget(signal_);
    ui_->rfzoom->setVisible(false);
    ui_->cfigain->setVisible(false);

    // load probes list
    cusOemProbes([](const char* list, int)
    {
        QApplication::postEvent(Oem::instance(), new event::List(list, true));
    });

    ui_->modes->blockSignals(true);
    ui_->modes->addItem("B Mode");
    ui_->modes->addItem("RF");
    ui_->modes->addItem("CFI");
    ui_->modes->addItem("PDI");
    ui_->modes->blockSignals(false);

    // connect status timer
    connect(&timer_, &QTimer::timeout, [this]()
    {
        ClariusStatusInfo st;
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

    // connect power service ready
    connect(&ble_, &Ble::powerReady, [this](bool en)
    {
        ui_->poweron->setEnabled(en);
        ui_->poweroff->setEnabled(en);
    });

    // connect wifi service ready
    connect(&ble_, &Ble::wifiReady, [this](bool en)
    {
        ui_->wifi->setEnabled(en);
        ui_->ap->setEnabled(en);
    });

    // connect wifi info sent
    connect(&ble_, &Ble::wifiInfo, [this](const QString& info)
    {
        QStringList network = info.split(QStringLiteral(","));
        // field 1: ssid
        // field 2: password (for probe ap)
        // field 3: v4 ip address
        // field 4: v6 ip address
        // field 5: connection port
        if (network.size() >= 5)
        {
            ui_->ip->setText(network[2]);
            ui_->port->setText(network[4]);
        }
        ui_->status->showMessage(QStringLiteral("Wi-Fi: %1").arg(info));
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

/// called when the window is closing to clean up the clarius library
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

/// tries to reprogram probe to a new wifi network (router)
void Oem::onWiFi()
{
    ble_.requestWifi(QStringLiteral("%1,%2").arg(ui_->ssid->text()).arg(ui_->password->text()));
}

/// tries to repgram probe to it's own access point wifi
void Oem::onAp()
{
    ble_.requestWifi(QStringLiteral("p2pRequest,auto"));
}

/// handles custom events posted by oem api callbacks
/// @param[in] event the event to parse
/// @return handling status
bool Oem::event(QEvent *event)
{
    if (event->type() == CONNECT_EVENT)
    {
        auto evt = static_cast<event::Connection*>(event);
        setConnected(evt->code(), evt->port(), evt->message());
        return true;
    }
    else if (event->type() == CERT_EVENT)
    {
        auto evt = static_cast<event::Cert*>(event);
        certification(evt->daysValid());
        return true;
    }
    else if (event->type() == POWER_EVENT)
    {
        auto evt = static_cast<event::PowerDown*>(event);
        poweringDown(evt->code(), evt->timeOut());
        return true;
    }
    else if (event->type() == SWUPDATE_EVENT)
    {
        auto evt = static_cast<event::SwUpdate*>(event);
        softwareUpdate(evt->code());
        return true;
    }
    else if (event->type() == LIST_EVENT)
    {
        auto evt = static_cast<event::List*>(event);
        if (evt->probes())
            loadProbes(evt->list());
        else
            loadApplications(evt->list());
        return true;
    }
    else if (event->type() == IMAGE_EVENT)
    {
        auto evt = static_cast<event::Image*>(event);
        newProcessedImage(evt->data(), evt->width(), evt->height(), evt->bpp());
        return true;
    }
    else if (event->type() == PRESCAN_EVENT)
    {
        auto evt = static_cast<event::PreScanImage*>(event);
        newPrescanImage(evt->data(), evt->width(), evt->height(), evt->bpp(), evt->jpeg());
        return true;
    }
    else if (event->type() == RF_EVENT)
    {
        auto evt = static_cast<event::RfImage*>(event);
        newRfImage(evt->data(), evt->width(), evt->height(), evt->bpp() / 8);
        return true;
    }
    else if (event->type() == IMAGING_EVENT)
    {
        auto evt = static_cast<event::Imaging*>(event);
        imagingState(evt->ready(), evt->imaging());
        return true;
    }
    else if (event->type() == BUTTON_EVENT)
    {
        auto evt = static_cast<event::Button*>(event);
        onButton(evt->button(), evt->clicks());
        return true;
    }
    else if (event->type() == PROGRESS_EVENT)
    {
        setProgress((static_cast<event::Progress*>(event))->progress());
        return true;
    }
    else if (event->type() == ERROR_EVENT)
    {
        setError((static_cast<event::Error*>(event))->error());
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
/// @param[in] code the connection code
/// @param[in] port the connection port
/// @param[in] msg the associated message
void Oem::setConnected(int code, int port, const QString& msg)
{
    if (code == CONNECT_SUCCESS)
    {
        timer_.start(1000);
        connected_ = true;
        ui_->status->showMessage(QStringLiteral("Connected on port: %1").arg(port));
        ui_->connect->setText("Disconnect");
        ui_->update->setEnabled(true);
        ui_->load->setEnabled(true);
    }
    else if (code == CONNECT_DISCONNECT)
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
        imagingState(IMAGING_NOTREADY, false);
    }
    else if (code == CONNECT_FAILED || code == CONNECT_ERROR)
        ui_->status->showMessage(QStringLiteral("Error connecting: %1").arg(msg));
    else if (code == CONNECT_SWUPDATE)
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
/// @param[in] code the power down code
/// @param[in] tm the associated timeout
void Oem::poweringDown(int code, int tm)
{
    if (code == POWERDOWN_IDLE)
        ui_->status->showMessage(QStringLiteral("Idle power down in: %1s").arg(tm));
    else if (code == POWERDOWN_TOOHOT)
        ui_->status->showMessage(QStringLiteral("Heating power down in: %1s").arg(tm));
    else if (code == POWERDOWN_BATTERY)
        ui_->status->showMessage(QStringLiteral("Battery low power down in: %1s").arg(tm));
    else if (code == POWERDOWN_BUTTON)
        ui_->status->showMessage(QStringLiteral("Button press power down in: %1s").arg(tm));
}

/// called when there's a software upate notification
/// @param[in] code the software update code
void Oem::softwareUpdate(int code)
{
    if (code == SWUPDATE_SUCCESS)
        ui_->status->showMessage(QStringLiteral("Successfully updated software"));
    else if (code == SWUPDATE_CURRENT)
        ui_->status->showMessage(QStringLiteral("Software already up to date"));
    else
        ui_->status->showMessage(QStringLiteral("Software not updated: %1").arg(code));
}

/// called when the imaging state changes
/// @param[in] code the imaging ready code
/// @param[in] imaging the imaging state
void Oem::imagingState(int code, bool imaging)
{
    bool ready = (code == IMAGING_READY);
    ui_->freeze->setEnabled(ready ? true : false);
    ui_->decdepth->setEnabled(ready ? true : false);
    ui_->incdepth->setEnabled(ready ? true : false);
    ui_->gain->setEnabled(ready ? true : false);
    ui_->cfigain->setEnabled(ready ? true : false);
    ui_->rfzoom->setEnabled(ready ? true : false);
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
    else if (code == IMAGING_CERTEXPIRED)
        ui_->status->showMessage(QStringLiteral("Certificate needs updating prior to imaging"));
}

/// called when there is a button press on the ultrasound
/// @param[in] btn the button pressed
/// @param[in] clicks # of clicks used
void Oem::onButton(int btn, int clicks)
{
    ui_->status->showMessage(QStringLiteral("Button %1 Pressed, %2 Clicks").arg((btn == BUTTON_DOWN) ? QStringLiteral("Down") : QStringLiteral("Up")).arg(clicks));
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
/// @param[in] bpp the bits per pixel (should always be 8)
void Oem::newProcessedImage(const void* img, int w, int h, int bpp)
{
    image_->loadImage(img, w, h, bpp);
}

/// called when a new pre-scan image has been sent
/// @param[in] img the image data
/// @param[in] w width of the image
/// @param[in] h height of the image
/// @param[in] bpp the bits per pixel (should always be 8)
/// @param[in] jpg flag if the data is jpeg compressed
void Oem::newPrescanImage(const void* img, int w, int h, int bpp, bool jpg)
{
    Q_UNUSED(bpp)
    Q_UNUSED(jpg)
    prescan_ = QImage(reinterpret_cast<const uchar*>(img), w, h, QImage::Format_ARGB32);
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
        imagingState(IMAGING_READY, !imaging_);
}

/// initiates a software update
void Oem::onUpdate()
{
    if (!connected_)
        return;

    if (cusOemSoftwareUpdate(
        // software update result
        [](int code)
        {
            QApplication::postEvent(Oem::instance(), new event::SwUpdate(code));
        },
        // download progress
        [](int progress)
        {
            QApplication::postEvent(Oem::instance(), new event::Progress(progress));
        }) < 0)
        ui_->status->showMessage(QStringLiteral("Error requesting software update"));
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
            QApplication::postEvent(Oem::instance(), new event::List(list, false));
        });
    }
}

/// increases the depth
void Oem::incDepth()
{
    auto v = cusOemGetParam(PARAM_DEPTH);
    if (v != -1)
        cusOemSetParam(PARAM_DEPTH, v + 1.0);
}

/// decreases the depth
void Oem::decDepth()
{
    auto v = cusOemGetParam(PARAM_DEPTH);
    if (v > 1.0)
        cusOemSetParam(PARAM_DEPTH, v - 1.0);
}

/// called when gain adjusted
/// @param[in] gn the gain level
void Oem::onGain(int gn)
{
    cusOemSetParam(PARAM_GAIN, gn);
}

/// called when color gain adjusted
/// @param[in] gn the gain level
void Oem::onColorGain(int gn)
{
    cusOemSetParam(PARAM_CGAIN, gn);
}

/// sets the tgc top
/// @param[in] v the tgc value
void Oem::tgcTop(int v)
{
    ClariusTgc t;
    t.top = v;
    t.mid = ui_->tgcmid->value();
    t.bottom = ui_->tgcbottom->value();
    cusOemSetTgc(&t);
}

/// sets the tgc mid
/// @param[in] v the tgc value
void Oem::tgcMid(int v)
{
    ClariusTgc t;
    t.top = ui_->tgctop->value();
    t.mid = v;
    t.bottom = ui_->tgcbottom->value();
    cusOemSetTgc(&t);
}

/// sets the tgc bottom
/// @param[in] v the tgc value
void Oem::tgcBottom(int v)
{
    ClariusTgc t;
    t.top = ui_->tgctop->value();
    t.mid = ui_->tgcmid->value();
    t.bottom = v;
    cusOemSetTgc(&t);
}

/// get the initial parameter values
void Oem::getParams()
{
    auto v = cusOemGetParam(PARAM_DEPTH);
    if (v != -1 && image_)
        image_->setDepth(v);

    ClariusTgc t;
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
    if (cusOemSetMode(mode) < 0)
        ui_->status->showMessage(QStringLiteral("Error setting imaging mode"));
    else
    {
        signal_->setVisible(mode == MODE_RF);
        ui_->rfzoom->setVisible(mode == MODE_RF);
        ui_->cfigain->setVisible(mode == MODE_CFI || mode == MODE_PDI);
    }
}

/// called when rf zoom adjusted
void Oem::onZoom(int zoom)
{
    signal_->setZoom(zoom);
}
