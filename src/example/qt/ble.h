#pragma once

typedef QVector<QPair<QString,QBluetoothDeviceInfo>> DiscoveredDevices;

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

private slots:
    void searchComplete();
    void onConnected();
    void onService(const QBluetoothUuid& u);
    void onDiscoveryFinished();

signals:
    void devices(const QStringList&);
    void powerReady(bool);
    void wifiReady(bool);
    void wifiInfo(const QString&);

private:
    DiscoveredDevices probes_;
    QBluetoothDeviceDiscoveryAgent search_;
    std::unique_ptr<QLowEnergyController> probe_;
    std::unique_ptr<QLowEnergyService> power_;
    std::unique_ptr<QLowEnergyService> wifi_;
};
