/*
 * Copyright 2009 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageEncoderPriv.h"

bool SkEncodeImage(SkWStream* dst, const SkPixmap& src,
                   SkEncodedImageFormat format, int quality) {
    #ifdef SK_USE_CG_ENCODER
        (void)quality;
        return SkEncodeImageWithCG(dst, src, format);
    #elif SK_USE_WIC_ENCODER
        return SkEncodeImageWithWIC(dst, src, format, quality);
    #else
        switch(format) {
            case SkEncodedImageFormat::kJPEG:
                return SkEncodeImageAsJPEG(dst, src, quality);
            case SkEncodedImageFormat::kPNG:
                return SkEncodeImageAsPNG(dst, src, SkEncodeOptions());
            case SkEncodedImageFormat::kWEBP:
                return SkEncodeImageAsWEBP(dst, src, quality);
            default:
                return false;
        }
    #endif
}
