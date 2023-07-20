#include "openigtlink.h"

bool SolumIGTL::serve(uint16_t port)
{
    server_ = igtl::ServerSocket::New();
    if (server_->CreateServer(port) < 0)
        return false;
    return true;
}

void SolumIGTL::close()
{
    server_->CloseSocket();
}

bool SolumIGTL::isServing() const
{
    return (server_ && server_->GetConnected());
}
