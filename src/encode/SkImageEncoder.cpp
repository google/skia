/*
 * Copyright 2009 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkImageEncoder.h"

#include "include/codec/SkEncodedImageFormat.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkData.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypes.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/encode/SkPngEncoder.h"
#include "include/encode/SkWebpEncoder.h"

#if !defined(SK_DISABLE_LEGACY_IMAGE_ENCODER)

bool SkEncodeImage(SkWStream* dst, const SkBitmap& src, SkEncodedImageFormat f, int q) {
    SkPixmap pixmap;
    return src.peekPixels(&pixmap) && SkEncodeImage(dst, pixmap, f, q);
}

bool SkEncodeImage(SkWStream* dst, const SkPixmap& src,
                   SkEncodedImageFormat format, int quality) {
    switch (format) {
        case SkEncodedImageFormat::kJPEG: {
            SkJpegEncoder::Options opts;
            opts.fQuality = quality;
            return SkJpegEncoder::Encode(dst, src, opts);
        }
        case SkEncodedImageFormat::kPNG: {
            SkPngEncoder::Options opts;
            return SkPngEncoder::Encode(dst, src, opts);
        }
        case SkEncodedImageFormat::kWEBP: {
            SkWebpEncoder::Options opts;
            if (quality == 100) {
                opts.fCompression = SkWebpEncoder::Compression::kLossless;
                // Note: SkEncodeImage treats 0 quality as the lowest quality
                // (greatest compression) and 100 as the highest quality (least
                // compression). For kLossy, this matches libwebp's
                // interpretation, so it is passed directly to libwebp. But
                // with kLossless, libwebp always creates the highest quality
                // image. In this case, fQuality is reinterpreted as how much
                // effort (time) to put into making a smaller file. This API
                // does not provide a way to specify this value (though it can
                // be specified by using SkWebpEncoder::Encode) so we have to
                // pick one arbitrarily. This value matches that chosen by
                // blink::ImageEncoder::ComputeWebpOptions as well
                // WebPConfigInit.
                opts.fQuality = 75;
            } else {
                opts.fCompression = SkWebpEncoder::Compression::kLossy;
                opts.fQuality = quality;
            }
            return SkWebpEncoder::Encode(dst, src, opts);
        }
        default:
            return false;
    }
}

sk_sp<SkData> SkEncodePixmap(const SkPixmap& src, SkEncodedImageFormat format, int quality) {
    SkDynamicMemoryWStream stream;
    return SkEncodeImage(&stream, src, format, quality) ? stream.detachAsData() : nullptr;
}

sk_sp<SkData> SkEncodeBitmap(const SkBitmap& src, SkEncodedImageFormat format, int quality) {
    SkPixmap pixmap;
    return src.peekPixels(&pixmap) ? SkEncodePixmap(pixmap, format, quality) : nullptr;
}

#endif
