#pragma once

#include "igtlServerSocket.h"

/// OpenIGTLink module
class SolumIGTL : public QObject
{
    Q_OBJECT;

public:
    bool serve(uint16_t port); /// Returns `false` on failure.
    void close();

    bool isServing() const;

private:
    igtl::ServerSocket::Pointer server_;
};
