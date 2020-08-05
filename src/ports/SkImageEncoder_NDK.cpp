/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkTFitsIn.h"
#include "include/private/SkTo.h"
#include "src/images/SkImageEncoderPriv.h"
#include "src/ports/SkNDKConversions.h"

bool SkEncodeImageWithNDK(SkWStream* stream, const SkPixmap& pmap, SkEncodedImageFormat format,
                          int quality) {
    // If any of these values is invalid (e.g. set to zero), the info will be rejected by
    // AndroidBitmap_compress.
    AndroidBitmapInfo info {
        .width  = SkTFitsIn<uint32_t>(pmap.width())    ? SkToU32(pmap.width())    : 0,
        .height = SkTFitsIn<uint32_t>(pmap.height())   ? SkToU32(pmap.height())   : 0,
        .stride = SkTFitsIn<uint32_t>(pmap.rowBytes()) ? SkToU32(pmap.rowBytes()) : 0,
        .format = SkNDKConversions::toAndroidBitmapFormat(pmap.colorType())
    };

    switch (pmap.alphaType()) {
        case kPremul_SkAlphaType:
            info.flags = ANDROID_BITMAP_FLAGS_ALPHA_PREMUL;
            break;
        case kOpaque_SkAlphaType:
            info.flags = ANDROID_BITMAP_FLAGS_ALPHA_OPAQUE;
            break;
        case kUnpremul_SkAlphaType:
            info.flags = ANDROID_BITMAP_FLAGS_ALPHA_UNPREMUL;
            break;
        default:
            return false;
    }

    AndroidBitmapCompressFormat androidFormat;
    switch (format) {
        case SkEncodedImageFormat::kJPEG:
            androidFormat = ANDROID_BITMAP_COMPRESS_FORMAT_JPEG;
            break;
        case SkEncodedImageFormat::kPNG:
            androidFormat = ANDROID_BITMAP_COMPRESS_FORMAT_PNG;
            break;
        case SkEncodedImageFormat::kWEBP:
            if (quality == 100) {
                // Mimic the behavior of SkImageEncoder.cpp. In LOSSLESS mode, libwebp
                // interprets quality as the amount of effort (time) to spend making
                // the encoded image smaller, while the visual quality remains constant.
                // This value of 75 (on a scale of 0 - 100, where 100 spends the most
                // time for the smallest encoding) matches WebPConfigInit.
                androidFormat = ANDROID_BITMAP_COMPRESS_FORMAT_WEBP_LOSSLESS;
                quality = 75;
            } else {
                androidFormat = ANDROID_BITMAP_COMPRESS_FORMAT_WEBP_LOSSY;
            }
            break;
        default:
            return false;
    }

    auto write_to_stream = [](void* userContext, const void* data, size_t size) {
        return reinterpret_cast<SkWStream*>(userContext)->write(data, size);
    };

    return ANDROID_BITMAP_RESULT_SUCCESS == AndroidBitmap_compress(&info,
            SkNDKConversions::toDataSpace(pmap.colorSpace()), pmap.addr(), androidFormat, quality,
            reinterpret_cast<void*>(stream), write_to_stream);
}
