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
        (void)quality;
        return SkEncodeImageWithCG(dst, src, format);
    #elif SK_USE_WIC_ENCODER
        return SkEncodeImageWithWIC(dst, src, format, 100);
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
