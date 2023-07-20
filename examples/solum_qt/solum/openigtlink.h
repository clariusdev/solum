#pragma once

#include "igtlServerSocket.h"

/// OpenIGTLink module
class SolumIGTL : public QObject
{
    Q_OBJECT;

public:
    SolumIGTL();

    bool serve(uint16_t port); /// Returns `false` on failure.
    void close();

    bool isServing() const;
    bool isClientConnected() const;

signals:
    void clientConnected(bool);

private:
    void disconnectClient();
    void reconnectClient();

    igtl::ServerSocket::Pointer server_;
    igtl::Socket::Pointer client_;
    QTimer clientConnectTimer_;
};
