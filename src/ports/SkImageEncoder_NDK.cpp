/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkStream.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/encode/SkPngEncoder.h"
#include "include/encode/SkWebpEncoder.h"
#include "include/private/base/SkTFitsIn.h"
#include "include/private/base/SkTo.h"
#include "src/encode/SkImageEncoderPriv.h"
#include "src/image/SkImage_Base.h"
#include "src/ports/SkNDKConversions.h"

static AndroidBitmapInfo info_for_pixmap(const SkPixmap& pmap) {
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
            SkDEBUGFAIL("unspecified alphaType");
            info.flags = ANDROID_BITMAP_FLAGS_ALPHA_OPAQUE;
            break;
    }
    return info;
}

static bool write_image_to_stream(SkWStream* stream,
                                  const SkPixmap& pmap,
                                  AndroidBitmapCompressFormat androidFormat,
                                  int quality) {
    AndroidBitmapInfo info = info_for_pixmap(pmap);

    auto write_to_stream = [](void* userContext, const void* data, size_t size) {
        return reinterpret_cast<SkWStream*>(userContext)->write(data, size);
    };

    return ANDROID_BITMAP_RESULT_SUCCESS == AndroidBitmap_compress(&info,
            SkNDKConversions::toDataSpace(pmap.colorSpace()), pmap.addr(), androidFormat, quality,
            reinterpret_cast<void*>(stream), write_to_stream);
}

namespace SkPngEncoder {
std::unique_ptr<SkEncoder> Make(SkWStream*, const SkPixmap&, const Options&) {
    SkDEBUGFAIL("Making an encoder is not supported via the NDK");
    return nullptr;
}

bool Encode(SkWStream* dst, const SkPixmap& src, const Options& options) {
    return write_image_to_stream(dst, src, ANDROID_BITMAP_COMPRESS_FORMAT_PNG, 100);
}

sk_sp<SkData> Encode(GrDirectContext* ctx, const SkImage* img, const Options& options) {
    if (!img) {
        return nullptr;
    }
    SkBitmap bm;
    if (!as_IB(img)->getROPixels(ctx, &bm)) {
        return nullptr;
    }
    SkDynamicMemoryWStream stream;
    if (Encode(&stream, bm.pixmap(), options)) {
        return stream.detachAsData();
    }
    return nullptr;
}
}  // namespace SkPngEncoder

namespace SkJpegEncoder {

bool Encode(SkWStream* dst, const SkPixmap& src, const Options& options) {
    return write_image_to_stream(dst, src, ANDROID_BITMAP_COMPRESS_FORMAT_JPEG, options.fQuality);
}

bool Encode(SkWStream*, const SkYUVAPixmaps&, const SkColorSpace*, const Options&) {
    SkDEBUGFAIL("encoding a YUVA pixmap is not supported via the NDK");
    return false;
}

sk_sp<SkData> Encode(GrDirectContext* ctx, const SkImage* img, const Options& options) {
    if (!img) {
        return nullptr;
    }
    SkBitmap bm;
    if (!as_IB(img)->getROPixels(ctx, &bm)) {
        return nullptr;
    }
    SkDynamicMemoryWStream stream;
    if (Encode(&stream, bm.pixmap(), options)) {
        return stream.detachAsData();
    }
    return nullptr;
}

std::unique_ptr<SkEncoder> Make(SkWStream*, const SkPixmap&, const Options&) {
    SkDEBUGFAIL("Making an encoder is not supported via the NDK");
    return nullptr;
}

std::unique_ptr<SkEncoder> Make(SkWStream*,
                                const SkYUVAPixmaps&,
                                const SkColorSpace*,
                                const Options&) {
    SkDEBUGFAIL("Making an encoder is not supported via the NDK");
    return nullptr;
}

}  // namespace SkJpegEncoder

namespace SkWebpEncoder {

bool Encode(SkWStream* dst, const SkPixmap& src, const Options& options) {
    if (options.fCompression == Compression::kLossless) {
        return write_image_to_stream(
                dst, src, ANDROID_BITMAP_COMPRESS_FORMAT_WEBP_LOSSLESS, options.fQuality);
    }
    return write_image_to_stream(
            dst, src, ANDROID_BITMAP_COMPRESS_FORMAT_WEBP_LOSSY, options.fQuality);
}

sk_sp<SkData> Encode(GrDirectContext* ctx, const SkImage* img, const Options& options) {
    if (!img) {
        return nullptr;
    }
    SkBitmap bm;
    if (!as_IB(img)->getROPixels(ctx, &bm)) {
        return nullptr;
    }
    SkDynamicMemoryWStream stream;
    if (Encode(&stream, bm.pixmap(), options)) {
        return stream.detachAsData();
    }
    return nullptr;
}

bool EncodeAnimated(SkWStream*, SkSpan<const SkEncoder::Frame>, const Options&) {
    SkDEBUGFAIL("Encoding Animated WebP images is not supported with the NDK.");
    return false;
}
}  // namespace SkWebpEncoder
