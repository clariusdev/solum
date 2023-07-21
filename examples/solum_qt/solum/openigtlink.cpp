#include "openigtlink.h"

SolumIGTL::SolumIGTL()
{
    msg_ = igtl::ImageMessage::New();
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

void SolumIGTL::setNodeName(const QString &name)
{
    nodeName_ = name.toStdString();
}

void SolumIGTL::sendImage(const SolumImage& img, double micronsPerPixel)
{
    if (!isClientConnected())
        return;

    assert((img.bpp_ == 32) && (img.format_ == Uncompressed));

    int wPrev, hPrev, zPrev;
    msg_->GetDimensions(wPrev, hPrev, zPrev);
    if ((wPrev != img.width_) || (hPrev != img.height_))
    {
        msg_->SetDimensions(img.width_, img.height_, 1);
        msg_->SetOrigin(0, 0, 0);
        msg_->SetScalarType(igtl::ImageMessage::TYPE_UINT32);
        msg_->SetSubVolume(img.width_, img.height_, 1, 0, 0, 0); // determines the buffer size!
        msg_->SetDeviceName(nodeName_);
        msg_->AllocateScalars();
    }

    // Also necessary to force a repack below.
    msg_->SetMessageID(msg_->GetMessageID() + 1);

    // Required for reslicing to not crash. The microns per pixel will change
    // as the imaging depth changes, independently of the image resolution.
    const auto spacing = (micronsPerPixel / 1000.0);
    msg_->SetSpacing(spacing, spacing, 1.0f);

    // Even C++23 does not have output ranges anyway...
    // (https://thephd.dev/output-ranges)
    memcpy(msg_->GetScalarPointer(), img.img_.data(), img.img_.size_bytes());

    msg_->Pack();
    if (client_->Send(msg_->GetPackPointer(), msg_->GetPackSize()) == 0)
        reconnectClient();
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
