#include "openigtlink.h"

SolumIGTL::SolumIGTL()
{
    QObject::connect(&clientConnectTimer_, &QTimer::timeout, [this]()
    {
        client_ = server_->WaitForConnection(1); // 0 would wait forever.
        if (client_.IsNotNull())
        {
            emit clientConnected(true);
            clientConnectTimer_.stop();
        }
    });
}

bool SolumIGTL::serve(uint16_t port)
{
    server_ = igtl::ServerSocket::New();
    if (server_->CreateServer(port) < 0)
        return false;
    reconnectClient();
    return true;
}

void SolumIGTL::close()
{
    disconnectClient();
    server_->CloseSocket();
}

void SolumIGTL::disconnectClient()
{
    clientConnectTimer_.stop();
    if (client_)
        client_->CloseSocket();
    emit clientConnected(false);
}

void SolumIGTL::reconnectClient()
{
    disconnectClient();
    clientConnectTimer_.start(100);
}

bool SolumIGTL::isServing() const
{
    return (server_ && server_->GetConnected());
}

bool SolumIGTL::isClientConnected() const
{
    return (client_ && client_->GetConnected());
}
