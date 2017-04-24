/*
 * Copyright 2009 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageEncoderPriv.h"

bool SkEncodeImage(SkWStream* dst, const SkPixmap& src, SkEncodedImageFormat format,
                   const SkEncodeOptions& options) {
    if (SkTransferFunctionBehavior::kRespect == options.fUnpremulBehavior) {
        if (!src.colorSpace() || (!src.colorSpace()->gammaCloseToSRGB() &&
                                  !src.colorSpace()->gammaIsLinear())) {
            return false;
        }
    }

    #ifdef SK_USE_CG_ENCODER
        return SkEncodeImageWithCG(dst, src, format);
    #elif SK_USE_WIC_ENCODER
        int quality = 100;
        switch(format) {
            case SkEncodedImageFormat::kJPEG:
                quality = options.fFormat.fJPEG.fQuality;
                break;
            case SkEncodedImageFormat::kWEBP:
                quality = (int) options.fFormat.fWEBP.fQuality;
                break;
            default:
                break;
        }
        return SkEncodeImageWithWIC(dst, src, format, quality);
    #else
        switch(format) {
            case SkEncodedImageFormat::kJPEG:
                return SkEncodeImageAsJPEG(dst, src, options);
            case SkEncodedImageFormat::kPNG:
                return SkEncodeImageAsPNG(dst, src, options);
            case SkEncodedImageFormat::kWEBP:
                return SkEncodeImageAsWEBP(dst, src, options);
            default:
                return false;
        }
    #endif
}

sk_sp<SkData> SkEncodeImage(const SkPixmap& src, SkEncodedImageFormat format,
                            const SkEncodeOptions& options) {
    SkDynamicMemoryWStream dst;
    if (SkEncodeImage(&dst, src, format, options)) {
        return dst.detachAsData();
    }

    return nullptr;
}

bool SkEncodeImage(SkWStream* dst, const SkPixmap& src, SkEncodedImageFormat format, int quality) {
    SkEncodeOptions opts(format);
    opts.fUnpremulBehavior = SkTransferFunctionBehavior::kIgnore;
    switch (format) {
        case SkEncodedImageFormat::kJPEG:
            opts.fFormat.fJPEG.fQuality = quality;
            break;
        case SkEncodedImageFormat::kWEBP:
            opts.fFormat.fWEBP.fQuality = (float) quality;
            break;
        default:
            break;
    }

    return SkEncodeImage(dst, src, format, opts);
}
