#pragma once

typedef QVector<QPair<QString,QBluetoothDeviceInfo>> DiscoveredDevices;

/// power states the probe will publish
enum class PowerState
{
    Off = 0,        ///< probe powered off
    On,             ///< probe powered on
    LowBattery,     ///< cannot boot, low battery
    TooHot,         ///< cannot boot, too hot
    ErrorBooting,   ///< cannot boot, other error
    Booted          ///< probe booted, setting up wi-fi
};

/// bluetooth module
class Ble : public QObject
{
    Q_OBJECT

public:
    Ble();
    virtual ~Ble() override;

    void search();
    bool connectToProbe(const QString& name);
    bool disconnectFromProbe();
    bool power(bool en);
    bool requestWifi(const QString& info);
    bool ring();

private:
    bool ping();

private slots:
    void searchComplete();
    void onConnected();
    void onService(const QBluetoothUuid& u);
    void onDiscoveryFinished();

signals:
    void devices(const QStringList&);
    void powerReady(bool);
    void powered(PowerState);
    void wifiReady(bool);
    void wifiInfo(const QString&);

private:
    DiscoveredDevices probes_;
    QBluetoothDeviceDiscoveryAgent search_;
    std::unique_ptr<QLowEnergyController> probe_;
    std::unique_ptr<QLowEnergyService> power_;
    std::unique_ptr<QLowEnergyService> wifi_;
    std::unique_ptr<QLowEnergyService> ias_;
    QTimer ping_;
};
