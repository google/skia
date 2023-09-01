/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/DitherUtils.h"

#ifndef SK_IGNORE_GPU_DITHER

#include "include/core/SkBitmap.h"
#include "include/core/SkColorType.h"

namespace skgpu {

float DitherRangeForConfig(SkColorType dstColorType) {
    SkASSERT(dstColorType != kUnknown_SkColorType);

    // We use 1 / (2^bitdepth-1) as the range since each channel can hold 2^bitdepth values
    switch (dstColorType) {
        // 4 bit
        case kARGB_4444_SkColorType:
            return 1 / 15.f;

        // 6 bit
        case kRGB_565_SkColorType:
            return 1 / 63.f;

        // 8 bit
        case kAlpha_8_SkColorType:
        case kGray_8_SkColorType:
        case kR8_unorm_SkColorType:
        case kR8G8_unorm_SkColorType:
        case kRGB_888x_SkColorType:
        case kRGBA_8888_SkColorType:
        case kSRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
            return 1 / 255.f;

        // 10 bit
        case kRGBA_1010102_SkColorType:
        case kBGRA_1010102_SkColorType:
        case kRGB_101010x_SkColorType:
        case kBGR_101010x_SkColorType:
        case kBGR_101010x_XR_SkColorType:
        case kRGBA_10x6_SkColorType:
            return 1 / 1023.f;

        // 16 bit
        case kA16_unorm_SkColorType:
        case kR16G16_unorm_SkColorType:
        case kR16G16B16A16_unorm_SkColorType:
            return 1 / 32767.f;

        // Unknown
        case kUnknown_SkColorType:
        // Half
        case kA16_float_SkColorType:
        case kR16G16_float_SkColorType:
        case kRGBA_F16_SkColorType:
        case kRGBA_F16Norm_SkColorType:
        // Float
        case kRGBA_F32_SkColorType:
            return 0.f; // no dithering
    }
    SkUNREACHABLE;
}

SkBitmap MakeDitherLUT() {
    static constexpr struct DitherTable {
        constexpr DitherTable() : data() {
            constexpr int kImgSize = 8; // if changed, also change value in sk_dither_shader

            for (int x = 0; x < kImgSize; ++x) {
                for (int y = 0; y < kImgSize; ++y) {
                    // The computation of 'm' and 'value' is lifted from CPU backend.
                    unsigned int m = (y & 1) << 5 | (x & 1) << 4 |
                                     (y & 2) << 2 | (x & 2) << 1 |
                                     (y & 4) >> 1 | (x & 4) >> 2;
                    float value = float(m) * 1.0 / 64.0 - 63.0 / 128.0;
                    // Bias by 0.5 to be in 0..1, mul by 255 and round to nearest int to make byte.
                    data[y * 8 + x] = (uint8_t)((value + 0.5) * 255.f + 0.5f);
                }
            }
        }
        uint8_t data[64];
    } gTable;

    SkBitmap bmp;
    bmp.setInfo(SkImageInfo::MakeA8(8, 8));
    bmp.setPixels(const_cast<uint8_t*>(gTable.data));
    bmp.setImmutable();
    return bmp;
}

}  // namespace skgpu

#endif // SK_IGNORE_GPU_DITHER
