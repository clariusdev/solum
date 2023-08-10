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
            msSinceLastFrame_ = 0;
            fpsSignalTimer_.start(1000);
        }
    });

    QObject::connect(&fpsSignalTimer_, &QTimer::timeout, [this]()
    {
        if (!imageTimer_.isValid())
            emit msSinceLastFrame(0);
        else
        {
            const auto elapsed = imageTimer_.elapsed();
            emit msSinceLastFrame((elapsed >= 1000) ? elapsed : msSinceLastFrame_);
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

void SolumIGTL::setFlip(bool flip)
{
    flip_ = flip;
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

    if (flip_)
    {
        // Copy the image upside down
        const auto stride = (img.width_ * (img.bpp_ / 8));
        auto* dst_row = reinterpret_cast<std::byte *>(msg_->GetScalarPointer());
        auto* src_row = (img.img_.data() + img.img_.size_bytes() - stride);
        for (int y = 0; y < img.height_; y++)
        {
            memcpy(dst_row, src_row, stride);
            dst_row += stride;
            src_row -= stride;
        }
    }
    else
    {
        // Even C++23 does not have output ranges anyway...
        // (https://thephd.dev/output-ranges)
        memcpy(msg_->GetScalarPointer(), img.img_.data(), img.img_.size_bytes());
    }

    msg_->Pack();
    if (client_->Send(msg_->GetPackPointer(), msg_->GetPackSize()) == 0)
        reconnectClient();

    if (!imageTimer_.isValid())
        imageTimer_.start();
    else
    {
        constexpr double SMOOTHING = 0.75;
        static_assert(SMOOTHING < 1.0);
        msSinceLastFrame_ = ((msSinceLastFrame_ * SMOOTHING) +
                             (imageTimer_.restart() * (1.0 - SMOOTHING)));
    }
}

void SolumIGTL::disconnectClient()
{
    clientConnectTimer_.stop();
    fpsSignalTimer_.stop();
    imageTimer_.invalidate();
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
