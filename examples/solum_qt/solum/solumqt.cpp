#include "solumqt.h"
#include "display.h"
#include "3d.h"
#include <solum/solum.h>
#include <string>

static Solum* _me;

/// default constructor
/// @param[in] parent the parent object
Solum::Solum(QWidget *parent) : QMainWindow(parent), imaging_(false), teeConnected_(false)
{
    _me = this;
    ui_.setupUi(this);

    connect(ui_.igtlnode, &QLineEdit::textChanged, [this](auto& text)
    {
        igtl_.setNodeName(text);
        settings_->setValue("OpenIGTLink/node", text);
    });

    // Job button labels
    ui_.retrieve->setLabels(tr("Retrieve from Cloud"), tr("Retrieving..."));
    ui_.blesearch->setLabels(tr("Search"), tr("Searching..."));
    ui_.bleconnect->setLabels(tr("Connect"), tr("Connecting..."));

    // Port validation
    portValidator_ = new QIntValidator(1, std::numeric_limits<uint16_t>::max());
    portError_ = tr("must be between %1 and %2")
                     .arg(portValidator_->bottom())
                     .arg(portValidator_->top());

    connectPortChanged(ui_.igtlport, ui_.igtlserve);

    // UI polish handlers
    connect(ui_.token, &QLineEdit::textChanged, [this](auto& text)
    {
        ui_.retrieve->setEnabled(!text.isEmpty());
    });

    ui_.status->viewport()->setAutoFillBackground(false);
    ui_.progress->hide();
    setWindowIcon(QIcon(":/res/logo.png"));
    image_ = new UltrasoundImage(false, this);
    image2_ = new UltrasoundImage(true, this);
    image2_->setVisible(false);
    spectrum_ = new Spectrum(this);
    signal_ = new RfSignal(this);
    prescan_ = new Prescan(this);
    ui_.image->addWidget(image_);
    ui_.image->addWidget(prescan_);
    ui_.image->addWidget(spectrum_);
    ui_.image->addWidget(signal_);
    ui_.image2->addWidget(image2_);
    render_ = new ProbeRender(QGuiApplication::primaryScreen());
    ui_.render->addWidget(QWidget::createWindowContainer(render_));
    auto reset = new QPushButton(QStringLiteral("Reset"), this);
    ui_.render->addWidget(reset);
    render_->init(QStringLiteral("scanner.obj"));
    render_->show();
    reflectMode(BMode);
    ui_._tabs->setTabEnabled(ui_._tabs->indexOf(ui_._3d), false);

    // settings.ini file needs to be at a location the application can write to.
    // Writing directly into the application directory (using 'settings.ini') does not work on some systems (e.g. macos).
    QString p = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    settings_ = std::make_unique<QSettings>(p + "/settings.ini", QSettings::IniFormat);

    ui_.token->setText(settings_->value("token").toString());
    settings_->beginGroup("Probes");
    for (const auto& probe: settings_->childGroups())
    {
        settings_->beginGroup(probe);

        ProbeData probeData;
        probeData.crt = settings_->value("crt").toString();
        probeData.model = settings_->value("model").toString();
        certified_[probe] = probeData;

        settings_->endGroup();
    }
    settings_->endGroup();
    auto igtlport = settings_->value("OpenIGTLink/port").toString();
    if (!igtlport.isEmpty())
        ui_.igtlport->setText(igtlport);
    auto igtlnode = settings_->value("OpenIGTLink/node").toString();
    ui_.igtlnode->setText(igtlnode.isEmpty() ? "Ultrasound" : igtlnode);
    igtl_.setFlip(settings_->value("OpenIGTLink/flip").toBool());

    ui_.certtable->setColumnCount(4);
    ui_.certtable->setHorizontalHeaderLabels({tr("Serial number"),
                                              tr("Issue date"),
                                              tr("Expiry date"), tr("Valid")});

    // Avoids the need for calling resizeColumnsToContents(), which breaks in
    // conjunction with the `StretchLastSection` flag:
    //
    //     https://forum.qt.io/topic/129429
    ui_.certtable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // handle the reply from the call to clarius cloud to obtain json probe information
    connect(&cloud_, &QNetworkAccessManager::finished, [this](QNetworkReply* reply)
    {
        ui_.retrieve->ready();
        auto result = reply->readAll();
        auto doc = QJsonDocument::fromJson(result);
        // check for errors
        if (doc.isNull())
        {
            addStatus(tr("(Cloud) Error retrieving certificates"));
            return;
        }

        const size_t previouslyCertifiedCount = certified_.size();
        size_t newlyCertifiedCount = 0;

        certified_.clear();
        auto json = doc.object();
        if (json.contains("results") && json["results"].isArray())
        {
            settings_->beginGroup("Probes");

            auto probes = json["results"].toArray();
            for (auto i = 0u; i < probes.size(); i++)
            {
                ProbeData probeData;
                QString serial;

                auto probe = probes[i].toObject();
                if (probe.contains("crt") && probe.contains("device") && probe["device"].isObject())
                {
                    auto device = probe["device"].toObject();
                    if (device.contains("serial"))
                    {
                        serial = device["serial"].toString();
                        probeData.crt = probe["crt"].toString();

                        if (device.contains("model"))
                        {
                            probeData.model = device["model"].toString();
                        }
                    }
                }
                if (!serial.isEmpty() && !probeData.crt.isEmpty() && !probeData.model.isEmpty())
                {
                    settings_->beginGroup(serial);
                    if (settings_->value("crt") != probeData.crt)
                    {
                        settings_->setValue("crt", probeData.crt);
                        newlyCertifiedCount++;
                    }
                    settings_->setValue("model", probeData.model);
                    certified_[serial] = probeData;
                    settings_->endGroup();
                }
            }
            settings_->endGroup();

            if (previouslyCertifiedCount == 0)
            {
                if (newlyCertifiedCount == 0)
                    addStatus(tr("(Cloud) Found no valid OEM probes"));
                else
                   addStatus(tr("(Cloud) Found %1 valid OEM probes").arg(certified_.size()));
            }
            else
            {
                if (newlyCertifiedCount == 0)
                    addStatus(tr("(Cloud) Stored certificates are recent enough, not renewed"));
                else
                    addStatus(tr("(Cloud) Renewed certificates for %1 probes").arg(newlyCertifiedCount));
            }

            reflectCertification();
        }
    });

    connect(ui_.bleprobes, &QComboBox::currentTextChanged, [this](const auto& probe)
    {
        ui_.bleconnect->setEnabled(probe != this->bleConnectedProbe_);
    });

    QObject::connect(reset, &QPushButton::clicked, [this]()
    {
        render_->reset();
    });

    // load probes list
    solumProbes([](const char* list, int)
    {
        QApplication::postEvent(_me, new event::List(list, true));
    });

    ui_.modes->blockSignals(true);
    ui_.modes->addItem(QStringLiteral("B"));
    ui_.modes->addItem(QStringLiteral("SC"));
    ui_.modes->addItem(QStringLiteral("M"));
    ui_.modes->addItem(QStringLiteral("CFI"));
    ui_.modes->addItem(QStringLiteral("PDI"));
    ui_.modes->addItem(QStringLiteral("PW"));
    ui_.modes->addItem(QStringLiteral("NE"));
    ui_.modes->addItem(QStringLiteral("SI"));
    ui_.modes->addItem(QStringLiteral("RF"));
    ui_.modes->blockSignals(false);

    // Start the IGTL server at startup
    onIGTLServe();

    // connect status timer
    connect(&timer_, &QTimer::timeout, [this]()
    {
        CusStatusInfo st;
        if (solumStatusInfo(&st) == 0)
        {
            QString teeTime;
            if (teeConnected_)
                teeTime = QStringLiteral(", TEE: %1%").arg(st.teeTimeRemaining);

            ui_.probeStatus->setText(QStringLiteral("Battery: %1%, Temp: %2%, FR: %3 Hz%4")
                   .arg(st.battery).arg(st.temperature).arg(QString::number(st.frameRate, 'f', 0)).arg(teeTime));
        }
    });

    const auto bleReadyCheck = [this](const QString& probe)
    {
        if(ui_.wifi->isEnabled() && ui_.ring->isEnabled())
        {
            addStatus("(BLE) Service discovery finished");
            ui_.bleconnect->ready();

            ui_.bleconnect->setEnabled(probe != ui_.bleprobes->currentText());

            ui_.tcpbox->setTitle(tr("Connection to %1").arg(probe));
            ui_.tcpbox->setEnabled(true);
            bleConnectedProbe_ = probe;
            reflectProbeModelAndWorkflows();
            ui_.ip->setEnabled(false);
            ui_.ip->setText(tr("(waiting for BLE message)"));
            ui_.port->setEnabled(false);
            ui_.port->setText(tr("(waiting for BLE message)"));
        }
    };

    connect(&ble_, &Ble::connected, [this, bleReadyCheck](bool connected)
    {
        if(connected)
            addStatus("(BLE) Connected, discovering services...");
        else
        {
            addStatus("(BLE) Disconnected");
            bleConnectedProbe_.clear();
            ui_.ring->setEnabled(false);
            bleReadyCheck("");
        }
    });

    // connect ble device list
    connect(&ble_, &Ble::devices, [this](const QStringList& devs)
    {
        ui_.bleprobes->clear();
        for (const auto& d : devs)
        {
            addStatus(QStringLiteral("(BLE) Found %1").arg(d));
            ui_.bleprobes->addItem(d);
        }
        if (ui_.bleprobes->count())
            ui_.bleprobes->setCurrentIndex(0);
        else
            addStatus("(BLE) Found no devices");
        ui_.blesearch->ready();
    });

    // power service ready
    connect(&ble_, &Ble::powerReady, [this, bleReadyCheck](bool en, const QString& probe)
    {
        ui_.poweron->setEnabled(en);
        ui_.poweroff->setEnabled(en);
        ui_.ring->setEnabled(en);
        bleReadyCheck(probe);
    });

    // wifi service ready
    connect(&ble_, &Ble::wifiReady, [this, bleReadyCheck](bool en, const QString& probe)
    {
        ui_.networkConfig->setEnabled(en);
        bleReadyCheck(probe);
    });

    // power status sent
    connect(&ble_, &Ble::powered, [this](bool en)
    {
        addStatus(tr("Powered: %1").arg(en ? tr("On") : tr("Off")));
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
                ret = ret.replace("\"", QString{});
                ret.replace(field + QStringLiteral(" "), QString{});
            }
            return ret;
        };
        auto ip = getField(QStringLiteral("ip4:"));
        auto port = getField(QStringLiteral("ctl:"));
        auto ssid = getField(QStringLiteral("ssid:"));
        auto pw = getField(QStringLiteral("pw:"));
        if (!ip.isEmpty() && !port.isEmpty())
        {
            ui_.ip->setEnabled(true);
            ui_.ip->setText(ip);
            ui_.port->setEnabled(true);
            ui_.port->setText(port);
            ui_.tcpconnect->setEnabled(true);
            if (!pw.isEmpty())
                addStatus(tr("Wi-Fi: %1 (%2) [SSID: %3, Password: %4]").arg(ip).arg(port).arg(ssid).arg(pw));
            else
                addStatus(tr("Wi-Fi: %1 (%2) [SSID: %3]").arg(ip).arg(port).arg(ssid));
        }
    });

    connect(&igtl_, &SolumIGTL::clientConnected, [this](bool connected)
    {
        if(connected)
            ui_.igtlStatusClient->setText(tr("🟢 Client connected"));
        else
            ui_.igtlStatusClient->setText(tr("⌛ Waiting for client connection…"));
        ui_.igtlStatusFPS->clear();
    });

    connect(&igtl_, &SolumIGTL::msSinceLastFrame, [this](auto ms)
    {
        const auto fps = ((ms == 0) ? 0 : (1000.0 / ms));
        ui_.igtlStatusFPS->setText(tr("🎥 %1 fps").arg(fps, 0, 'f', 1));
    });

    imagingState(ImagingNotReady, false);

#if QT_CONFIG(permissions)
    QBluetoothPermission bluetoothPermission;
    switch (qApp->checkPermission(bluetoothPermission)) {
    case Qt::PermissionStatus::Undetermined:
        qApp->requestPermission(bluetoothPermission, this, [this]()
                                {
                                    qDebug() << "bluetooth permission granted";
                                    ui_.blesearch->setEnabled(true);
                                    // Automatically trigger a BLE search at startup
                                    ui_.blesearch->click();
                                });
        return;
    case Qt::PermissionStatus::Denied:
        qDebug() << "bluetooth permission was denied";
        return;
    case Qt::PermissionStatus::Granted:
        // Automatically trigger a BLE search at startup
        ui_.blesearch->setEnabled(true);
        ui_.blesearch->click();
        break; // Proceed
    }
#endif
}

/// destructor
Solum::~Solum()
{
    timer_.stop();
}

/// loads a list of applications into selection box
/// @param[in] apps the applications list
void Solum::loadApplications(const QStringList& apps)
{
    ui_.workflows->clear();
    for (auto a : apps)
        ui_.workflows->addItem(a);
    if (ui_.workflows->count())
        ui_.workflows->setCurrentIndex(0);
}

/// called when the window is closing to clean up the library
void Solum::closeEvent(QCloseEvent*)
{
    if (tcpConnected_)
        solumDisconnect();
}

/// called for starting a cloud request to retrieve the probe credentials
void Solum::onRetrieve()
{
    auto token = ui_.token->text();
    if (token.isEmpty())
        return;

    addStatus("(Cloud) Retrieving probe credentials...");

    // Defer the actual retrieval, so that we immediately return and refresh
    // the UI.
    QTimer::singleShot(1, [=] ()
    {
        settings_->setValue("token", token);
        QNetworkRequest request(QUrl("https://cloud.clarius.com/api/public/v0/devices/oem/?limit=300&format=json"));
        request.setTransferTimeout(5000);
        QByteArray auth(QString("OEM-API-Key %1").arg(token).toUtf8());
        request.setRawHeader("Authorization", auth);
        cloud_.get(request);
    });
}

/// initiates ble search
void Solum::onBleSearch()
{
    addStatus("(BLE) Searching...");
    ble_.search();
}

/// called when a ble probe is selected
void Solum::onBleProbe(int index)
{
    ui_.bleconnect->setEnabled(index >= 0);
}

/// called when a ble connect is initiated
void Solum::onBleConnect()
{
    const auto& probe = ui_.bleprobes->currentText();
    addStatus(QStringLiteral("(BLE) Connecting to %1...").arg(probe));
    ble_.connectToProbe(probe);
}

/// tries to power on probe
void Solum::onPowerOn()
{
    ble_.power(true);
}

/// tries to power off probe
void Solum::onPowerOff()
{
    ble_.power(false);
}

/// rings the probe
void Solum::onRing()
{
    ble_.ring();
}

/// tries to reprogram probe to a new wifi network (router)
void Solum::onWiFi()
{
    auto ssid = ui_.ssid->text();
    auto pw = ui_.password->text();
    if (ssid.isEmpty())
        return;

    QString req(QStringLiteral("ap: false\n"));
    req += QStringLiteral("ssid: %1\n").arg(ssid);
    if (!pw.isEmpty())
        req += QStringLiteral("pw: %1\n").arg(pw);
    ble_.requestWifi(req);
}

/// tries to repgram probe to it's own access point wifi
void Solum::onAp()
{
    ble_.requestWifi(QStringLiteral("ap: true\nch: auto\n"));
}

/// handles custom events posted by solum api callbacks
/// @param[in] event the event to parse
/// @return handling status
bool Solum::event(QEvent *event)
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
        {
            probesSupported_ = evt->list_;
            reflectCertification();
        }
        else
            loadApplications(evt->list_);
        return true;
    }
    else if (event->type() == IMAGE_EVENT)
    {
        newProcessedImage(*dynamic_cast<event::ProcessedImage*>(event));
        return true;
    }
    else if (event->type() == PRESCAN_EVENT)
    {
        newPrescanImage(*dynamic_cast<event::Image*>(event));
        return true;
    }
    else if (event->type() == SPECTRUM_EVENT)
    {
        newSpectrumImage(*dynamic_cast<event::SpectrumImage*>(event));
        return true;
    }
    else if (event->type() == RF_EVENT)
    {
        newRfImage(*dynamic_cast<event::RfImage*>(event));
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
        addError((static_cast<event::Error*>(event))->error_);
        return true;
    }
    else if (event->type() == TEE_EVENT)
    {
        auto evt = static_cast<event::Tee*>(event);
        onTee(evt->connected_, evt->serial_, evt->timeRemaining_);
        return true;
    }

    return QMainWindow::event(event);
}

/// called when reporting the result of a user action
/// @param[in] status the status message
void Solum::addStatus(const QString &status)
{
    ui_.status->appendPlainText(QStringLiteral("(%1) %2").arg(QTime::currentTime().toString()).arg(status));
}

/// called when the api returns an error
/// @param[in] err the error message
void Solum::addError(const QString &err)
{
    addStatus(tr("Error: %1").arg(err));
}

/// called when there's a new connection event
/// @param[in] res the connection result
/// @param[in] port the connection port
/// @param[in] msg the associated message
void Solum::setConnected(CusConnection res, int port, const QString& msg)
{
    if (res == ProbeConnected)
    {
        timer_.start(1000);
        tcpConnected_ = true;
        addStatus(tr("Connected on port: %1").arg(port));
        ui_.tcpconnect->setText(tr("Disconnect"));
        ui_.update->setEnabled(true);
        ui_.load->setEnabled(true);

        // load the certificate if it was already retrieved from the cloud
        if (certified_.count(bleConnectedProbe_))
            solumSetCert(certified_[bleConnectedProbe_].crt.toLatin1());

        if (tcpConnectedProbe_ != bleConnectedProbe_)
            activeWorkflow_.clear();
        tcpConnectedProbe_ = bleConnectedProbe_;
    }
    else if (res == ProbeDisconnected)
    {
        timer_.stop();
        tcpConnected_ = false;
        addStatus(tr("Disconnected"));
        ui_.tcpconnect->setText(tr("Connect"));
        ui_.probeStatus->clear();
        ui_.imaging->setEnabled(false);
        ui_.update->setEnabled(false);
        ui_.load->setEnabled(false);
        // disable controls upon disconnect
        imagingState(ImagingNotReady, false);
    }
    else if (res == ConnectionFailed || res == ConnectionError)
        addStatus(tr("Error connecting: %1").arg(msg));
    else if (res == SwUpdateRequired)
        addStatus(tr("Software update required prior to imaging"));
}

/// called when a new certification message has been sent
/// @param[in] daysValid # of days valid for certificate
void Solum::certification(int daysValid)
{
    if (daysValid == CERT_INVALID)
        ui_.cert->setText(QStringLiteral("Invalid"));
    else if (!daysValid)
        ui_.cert->setText(QStringLiteral("Expired"));
    else
        ui_.cert->setText(QStringLiteral("%1 Days").arg(daysValid));
}

/// called when there's a power down event
/// @param[in] res the power down reason
/// @param[in] tm the associated timeout
void Solum::poweringDown(CusPowerDown res, int tm)
{
    if (res == Idle)
        addStatus(tr("Idle power down in: %1s").arg(tm));
    else if (res == TooHot)
        addStatus(tr("Heating power down in: %1s").arg(tm));
    else if (res == LowBattery)
        addStatus(tr("Battery low power down in: %1s").arg(tm));
    else if (res == ButtonOff)
        addStatus(tr("Button press power down in: %1s").arg(tm));
}

/// called when there's a software update notification
/// @param[in] res the software update result
void Solum::softwareUpdate(CusSwUpdate res)
{
    if (res == SwUpdateSuccess)
        addStatus(tr("Successfully updated software"));
    else if (res == SwUpdateCurrent)
        addStatus(tr("Software already up to date"));
    else
        addStatus(tr("Software not updated: %1").arg(res));
}

/// called when the imaging state changes
/// @param[in] state the imaging ready code
/// @param[in] imaging the imaging state
void Solum::imagingState(CusImagingState state, bool imaging)
{
    bool ready = (state != ImagingNotReady);
    ui_.imaging->setEnabled(ready ? true : false);
    ui_.autogain->setEnabled(ready ? true : false);
    ui_.autofocus->setEnabled(ready ? true : false);
    ui_.decdepth->setEnabled(ready ? true : false);
    ui_.incdepth->setEnabled(ready ? true : false);
    ui_.gain->setEnabled(ready ? true : false);
    ui_.focus->setEnabled((ready && !ui_.autofocus->isChecked()) ? true : false);
    ui_.cfigain->setEnabled(ready ? true : false);
    ui_.opacity->setEnabled(ready ? true : false);
    ui_.rfzoom->setEnabled(ready ? true : false);
    ui_.rfStream->setEnabled(ready ? true : false);
    ui_.prescan->setEnabled(ready ? true : false);
    ui_.split->setEnabled(ready ? true : false);
    ui_.imu->setEnabled(ready ? true : false);
    ui_.tgc->setEnabled(ready && !ui_.autogain->isChecked());

    ui_.modes->setEnabled(imaging ? true : false);
    if (!imaging)
    {
        ui_.modes->blockSignals(true);
        ui_.modes->setCurrentText(QStringLiteral("B"));
        ui_.modes->blockSignals(false);
    }

    if (!imaging || !tcpConnected_)
        ui_.imaging->setLabels(tr("Run"), tr("Starting imaging..."));
    else
        ui_.imaging->setLabels(tr("Stop"), tr("Stopping imaging..."));

    if (tcpConnected_)
    {
        if (imaging_ != imaging)
            ui_.imaging->ready();
        addStatus(tr("Image: %1").arg(imaging ? tr("Running") : tr("Frozen")));
    }
    else
    {
        ui_.imaging->ready();
        ui_.imaging->setEnabled(false);
    }

    if (ready)
    {
        imaging_ = imaging;
        getParams();
        image_->checkRoi();
        image_->checkGate();
    }
    else if (state == CertExpired)
        addStatus(tr("Certificate needs updating prior to imaging"));
}

/// called when there is a button press on the ultrasound
/// @param[in] btn the button pressed
/// @param[in] clicks # of clicks used
void Solum::onButton(CusButton btn, int clicks)
{
    QString text;
    switch (btn)
    {
        case ButtonDown: text = QStringLiteral("Down"); break;
        case ButtonUp: text = QStringLiteral("Up"); break;
        case ButtonHandle: text = QStringLiteral("Handle"); break;
    }

    addStatus(tr("Button %1 Pressed, %2 Clicks").arg(text).arg(clicks));
}

/// called when there is a tee connect or disconnect
/// @param[in] connected flag if the probe is connected or not
/// @param[in] serial the serial number if the probe is connected
/// @param[in] timeRemaining the time remaining in percent if the probe is connected
void Solum::onTee(bool connected, const QString& serial, double timeRemaining)
{
    teeConnected_ = connected;
    if (connected)
        addStatus(tr("TEE Connected: %1 @ %2% Remaining").arg(serial).arg(timeRemaining));
    else
        addStatus(tr("TEE Disconnected"));
}

/// called when the download progress changes
/// @param[in] progress the current progress
void Solum::setProgress(int progress)
{
    ui_.progress->show();
    ui_.progress->setValue(progress);
}

/// called when a new image has been sent
/// @param[in] evt the processed image event
void Solum::newProcessedImage(const event::ProcessedImage& evt)
{
    igtl_.sendImage(evt.img_, evt.micronsPerPixel_);

    if (evt.overlay_)
        image2_->loadImage(evt.img_);
    else
        image_->loadImage(evt.img_);

    if (!evt.imu_.isNull())
        render_->update(evt.imu_);
}

/// called when a new pre-scan image has been sent
/// @param[in] evt the pre-scan image event
void Solum::newPrescanImage(const event::Image& evt)
{
    prescan_->loadImage(evt.img_);
}

/// called when a new spectrum image has been sent
/// @param[in] evt the spectrum image event
void Solum::newSpectrumImage(const event::SpectrumImage& evt)
{
    spectrum_->loadImage(evt.data_, evt.lines_, evt.samples_, evt.bps_);
}

/// called when new rf data has been sent
/// @param[in] evt the RF image event
void Solum::newRfImage(const event::RfImage& evt)
{
    signal_->loadSignal(evt.img_.img_.data(), evt.img_.width_, evt.img_.height_, evt.img_.bpp_ / 8);
}

void Solum::reflectCertification()
{
    // Check for unsupported probe models
    {
        auto it = certified_.begin();
        while(it != certified_.end())
        {
            if (!probesSupported_.contains(it->second.model))
            {
                addStatus(tr("%1: Probe model \"%2\" not supported by this version of Solum").arg(it->first).arg(it->second.model));
                it = certified_.erase(it);
            }
            else
                it++;
        }
    }

    ui_.certtable->setRowCount(int(certified_.size()));
    int row = 0;
    bool tcpMakesSense = false;
    for (const auto& probe : certified_)
    {
        QSslCertificate cert(probe.second.crt.toLatin1());

        auto issued = cert.effectiveDate().toString(Qt::RFC2822Date);
        auto expiry = cert.expiryDate().toString(Qt::RFC2822Date);
        auto now = QDateTime::currentDateTime();
        auto valid = ((cert.effectiveDate() <= now) && (cert.expiryDate() >= now));
        tcpMakesSense |= valid;

        auto itemSerial = new QTableWidgetItem(probe.first);
        auto itemIssued = new QTableWidgetItem(issued);
        auto itemExpiry = new QTableWidgetItem(expiry);
        auto itemValid = new QTableWidgetItem(valid ? "✅" : "❌");
        itemValid->setTextAlignment(Qt::AlignCenter);

        ui_.certtable->setItem(row, 0, itemSerial);
        ui_.certtable->setItem(row, 1, itemIssued);
        ui_.certtable->setItem(row, 2, itemExpiry);
        ui_.certtable->setItem(row, 3, itemValid);
        row++;
    }
    ui_._tabs->setTabEnabled(ui_._tabs->indexOf(ui_.tcp), tcpMakesSense);
    reflectProbeModelAndWorkflows();
}

/// called when the connect/disconnect button is clicked
void Solum::onTcpConnect()
{
    if (!tcpConnected_)
    {

        const std::string ip = ui_.ip->text().toStdString();
        const CusConnectionParams connectionParams = {
            ip.c_str(),
            ui_.port->text().toUInt(),
            0
        };

        if (solumConnect(&connectionParams) < 0)
            addStatus(tr("Connection failed"));
        else
            addStatus(tr("Trying connection"));
    }
    else
    {
        if (solumDisconnect() < 0)
            addStatus(tr("Disconnect failed"));
    }
}

/// handles starting and stopping imaging with the selected workflow
void Solum::onImaging()
{
    if (!tcpConnected_)
        return;

    if (solumRun(imaging_ ? 0 : 1) < 0)
        addStatus(tr("Error requesting imaging run/stop"));
    else
    {
        imagingState(ImagingReady, !imaging_);
        if (imaging_)
            spectrum_->reset();
    }
}

/// initiates a software update
void Solum::onUpdate()
{
    if (!tcpConnected_)
        return;

    auto filePath = QFileDialog::getOpenFileName(this,
        QStringLiteral("Choose Firmware Package"), QString(), QStringLiteral("Zip Files (*.zip)"));
    if (filePath.isEmpty())
        return;

    if (solumSoftwareUpdate(
        filePath.toStdString().c_str(),
        // software update result
        [](CusSwUpdate res)
        {
            QApplication::postEvent(_me, new event::SwUpdate(res));
        },
        // download progress
        [](int progress)
        {
            QApplication::postEvent(_me, new event::Progress(progress));
        }, 0) < 0)
        addStatus(tr("Error requesting software update"));
}

/// initiates a workflow load
void Solum::onLoad()
{
    if (!tcpConnected_)
        return;

    // capture the currently selected one, just in case the user changes it
    // before the timer
    const auto probeModel = ui_.tcpprobe->text().toStdString();
    const auto workflow = ui_.workflows->currentText().toStdString();

    if (solumLoadApplication(probeModel.c_str(), workflow.c_str()) < 0)
        addStatus(tr("Error requesting application load"));
    // update depth range on a successful load
    else
    {
        // wait a second for the application load to propagate internally before fetching the range
        // ideally the api would provide a callback for when the application is fully loaded (ofi)
        QTimer::singleShot(1000, this, [this, workflow] ()
        {
            CusRange range;
            if (solumGetRange(ImageDepth, &range) == 0)
            {
                ui_.maxdepth->setText(
                    tr("Range: %1 - %2cm").arg(range.min).arg(range.max)
                );
                activeWorkflow_ = workflow;
            }
        });
    }
}

/// updates the probe model and workflow fields
/// @param[in] probe the probe selected
void Solum::reflectProbeModelAndWorkflows()
{
    if (certified_.count(bleConnectedProbe_))
    {
        const auto& model = certified_[bleConnectedProbe_].model;
        ui_.tcpprobe->setText(model);
        ui_.tcpprobe->setEnabled(true);
        solumApplications(model.toStdString().c_str(), [](const char* list, int)
        {
            QApplication::postEvent(_me, new event::List(list, false));
        });
    }
}

/// shows or hides parameter widgets depending on the given mode
/// @param[in] m an imaging mode
void Solum::reflectMode(CusMode m)
{
    spectrum_->setVisible(m == MMode || m == PwMode);
    signal_->setVisible(m == RfMode);
    ui_.cfigain->setVisible(m == ColorMode || m == PowerMode);
    ui_.cfigain_label->setVisible(m == ColorMode || m == PowerMode);
    ui_.velocity->setVisible(m == ColorMode || m == PwMode);
    ui_.opacity->setVisible(m == Strain);
    ui_.opacity_label->setVisible(m == Strain);
    ui_.rfzoom->setVisible(m == RfMode);
    ui_.rf_label->setVisible(m == RfMode);
    ui_.rfStream->setVisible(m == RfMode);
    ui_.split->setVisible(m == ColorMode || m == PowerMode || m == Strain);
}

/// increases the depth
void Solum::incDepth()
{
    auto v = solumGetParam(ImageDepth);
    if (v != -1)
        solumSetParam(ImageDepth, v + 1.0);
}

/// decreases the depth
void Solum::decDepth()
{
    auto v = solumGetParam(ImageDepth);
    if (v > 1.0)
        solumSetParam(ImageDepth, v - 1.0);
}

/// called when gain adjusted
/// @param[in] gn the gain level
void Solum::onGain(int gn)
{
    solumSetParam(Gain, gn);
}

/// called when manual focus adjusted
/// @param[in] fd the focus depth
void Solum::onFocus(int fd)
{
    auto v = solumGetParam(ImageDepth);
    if (fd < v)
        solumSetParam(FocusDepth, fd);
}

/// called when color gain adjusted
/// @param[in] gn the gain level
void Solum::onColorGain(int gn)
{
    solumSetParam(ColorGain, gn);
}

/// called when strain opacity adjusted
/// @param[in] gn the opacity level
void Solum::onOpacity(int gn)
{
    solumSetParam(StrainOpacity, gn);
}

/// called when auto gain enable adjusted
/// @param[in] state checkbox state
void Solum::onAutoGain(int state)
{
    bool en = (state == Qt::Checked);
    solumSetParam(AutoGain, en ? 1 : 0);
    ui_.tgctop->setEnabled(en ? false: true);
    ui_.tgcmid->setEnabled(en ? false: true);
    ui_.tgcbottom->setEnabled(en ? false: true);
    // reset the tgc sliders
    ui_.tgctop->setValue(0);
    ui_.tgcmid->setValue(0);
    ui_.tgcbottom->setValue(0);
}

/// called when auto focus enable adjusted
/// @param[in] state checkbox state
void Solum::onAutoFocus(int state)
{
    bool en = (state == Qt::Checked);
    solumSetParam(AutoFocus, en ? 1 : 0);
    ui_.focus->setEnabled(en ? false: true);
}

/// called when imu enable adjusted
/// @param[in] state checkbox state
void Solum::onImu(int state)
{
    ui_._tabs->setTabEnabled(ui_._tabs->indexOf(ui_._3d), (state == Qt::Checked));
    solumSetParam(ImuStreaming, (state == Qt::Checked) ? 1 : 0);
}

/// called when rf stream enable adjusted
/// @param[in] state checkbox state
void Solum::onRfStream(int state)
{
    solumSetParam(RfStreaming, (state == Qt::Checked) ? 1 : 0);
}

/// called when separate overlays is changed
/// @param[in] state checkbox state
void Solum::onSplit(int state)
{
    bool en = (state == Qt::Checked);
    solumSeparateOverlays(en ? 1 : 0);
    image2_->setVisible(en);
}

/// called when separate overlays is changed
/// @param[in] state checkbox state
void Solum::onPrescan(int state)
{
    bool en = (state == Qt::Checked);
    prescan_->setVisible(en);
}

/// sets the tgc top
/// @param[in] v the tgc value
void Solum::tgcTop(int v)
{
    CusTgc t;
    t.top = v;
    t.mid = ui_.tgcmid->value();
    t.bottom = ui_.tgcbottom->value();
    solumSetTgc(&t);
}

/// sets the tgc mid
/// @param[in] v the tgc value
void Solum::tgcMid(int v)
{
    CusTgc t;
    t.top = ui_.tgctop->value();
    t.mid = v;
    t.bottom = ui_.tgcbottom->value();
    solumSetTgc(&t);
}

/// sets the tgc bottom
/// @param[in] v the tgc value
void Solum::tgcBottom(int v)
{
    CusTgc t;
    t.top = ui_.tgctop->value();
    t.mid = ui_.tgcmid->value();
    t.bottom = v;
    solumSetTgc(&t);
}

/// get the initial parameter values
void Solum::getParams()
{
    auto v = solumGetParam(ImageDepth);
    if (v != -1 && image_)
        image_->setDepth(v);

    v = solumGetParam(AutoGain);
    ui_.autogain->setChecked(v > 0);
    v = solumGetParam(AutoFocus);
    ui_.autofocus->setChecked(v > 0);
    v = solumGetParam(ImuStreaming);
    ui_.imu->setChecked(v > 0);
    v = solumGetParam(RfStreaming);
    ui_.rfStream->setChecked(v > 0);

    CusTgc t;
    if (solumGetTgc(&t) == 0)
    {
        ui_.tgctop->setValue(static_cast<int>(t.top));
        ui_.tgcmid->setValue(static_cast<int>(t.mid));
        ui_.tgcbottom->setValue(static_cast<int>(t.bottom));
    }
}

/// called on a mode change
void Solum::onMode(int mode)
{
    auto m = static_cast<CusMode>(mode);
    if (solumSetMode(m) < 0)
        addStatus(tr("Error setting imaging mode"));
    else
    {
        reflectMode(m);
        updateVelocity(m);
    }
}

/// updates the velocity range on the interface
/// @param[in] mode the current imaging mode
/// @note called on a mode change, but should also be called if a prf adjustment occurs
void Solum::updateVelocity(CusMode mode)
{
    // wait a second for the mode load to propagate internally before fetching the velociy
    if (mode == ColorMode || mode == PwMode)
    {
        QTimer::singleShot(1000, this, [this] ()
        {
            auto v = solumGetParam(DopplerVelocity);
            if (v)
            {
                ui_.velocity->setText(QStringLiteral("+/- %1cm/s").arg(v));
            }
        });
    }
}

/// called when rf zoom adjusted
void Solum::onZoom(int zoom)
{
    signal_->setZoom(zoom);
}

void Solum::connectPortChanged(QLineEdit* portEdit, QPushButton* connectButton)
{
    portEdit->setValidator(portValidator_);
    connect(portEdit, &QLineEdit::textChanged, [=]()
    {
        const auto pos = portEdit->mapToGlobal(QPoint(0, 0));
        if (portEdit->hasAcceptableInput())
        {
            portEdit->setStyleSheet("");
            connectButton->setEnabled(true);
            QToolTip::showText(pos, "", portEdit);
        }
        else
        {
            portEdit->setStyleSheet("QLineEdit { border: 1px solid red }");
            connectButton->setEnabled(false);
            if (portEdit->isVisible())
                QToolTip::showText(pos, portError_, portEdit);
        }
    });
}

void Solum::onIGTLServe()
{
    const auto port = ui_.igtlport->text().toInt();
    if (!igtl_.isServing())
    {
        settings_->setValue("OpenIGTLink/port", port);
        if (igtl_.serve(port))
        {
            ui_.igtlStatusServer->setText(tr("🟢 Server is running"));
            ui_.igtlport->setEnabled(false);
            ui_.igtlserve->setText(tr("Stop"));
        }
        else
            ui_.igtlStatusServer->setText(
                tr("❌ Socket could not be opened on port %1").arg(port));
    }
    else
    {
        igtl_.close();
        ui_.igtlStatusServer->setText(tr("🔴 Server is stopped"));
        ui_.igtlStatusClient->clear();
        ui_.igtlStatusFPS->clear();
        ui_.igtlport->setEnabled(true);
        ui_.igtlserve->setText(tr("Serve"));
    }
}

void Solum::onIGTLFlip(int check)
{
    const auto value = (check == Qt::Checked);
    igtl_.setFlip(value);
    settings_->setValue("OpenIGTLink/flip", value);
}
