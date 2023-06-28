#pragma once

#include <span>
#include <solum/solum_def.h>

/// Common structure for Solum-provided image data.
struct SolumImage
{
    std::span<uint8_t> img_;
    int width_;
    int height_;
    int bpp_;
    CusImageFormat format_;
};
