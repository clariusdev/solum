#pragma once

#include "igtlImageMessage.h"
#include "igtlServerSocket.h"
#include "image.h"

/// OpenIGTLink module
class SolumIGTL : public QObject
{
    Q_OBJECT;

public:
    SolumIGTL();

    bool serve(uint16_t port); /// Returns `false` on failure.
    void close();

    void setNodeName(const QString& name);
    void sendImage(const SolumImage& img, double micronsPerPixel);

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
    std::string nodeName_;

    // Sharing an allocation for all images of the same size.
    igtl::ImageMessage::Pointer msg_;
};
