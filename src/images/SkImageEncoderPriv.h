/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageEncoderPriv_DEFINED
#define SkImageEncoderPriv_DEFINED

#include "SkImageEncoder.h"

struct SkEncodeOptions {
    SkTransferFunctionBehavior fUnpremulBehavior = SkTransferFunctionBehavior::kIgnore;
};

#ifdef SK_HAS_JPEG_LIBRARY
    bool SkEncodeImageAsJPEG(SkWStream*, const SkPixmap&, const SkEncodeOptions&);
    bool SkEncodeImageAsJPEG(SkWStream*, const SkPixmap&, int quality);
#else
    #define SkEncodeImageAsJPEG(...) false
#endif

#ifdef SK_HAS_PNG_LIBRARY
    bool SkEncodeImageAsPNG(SkWStream*, const SkPixmap&, const SkEncodeOptions&);
#else
    #define SkEncodeImageAsPNG(...) false
#endif

#ifdef SK_HAS_WEBP_LIBRARY
    bool SkEncodeImageAsWEBP(SkWStream*, const SkPixmap&, const SkEncodeOptions&);
    bool SkEncodeImageAsWEBP(SkWStream*, const SkPixmap&, int quality);
#else
    #define SkEncodeImageAsWEBP(...) false
#endif

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
    bool SkEncodeImageWithCG(SkWStream*, const SkPixmap&, SkEncodedImageFormat);
#else
    #define SkEncodeImageWithCG(...) false
#endif

#ifdef SK_BUILD_FOR_WIN
    bool SkEncodeImageWithWIC(SkWStream*, const SkPixmap&, SkEncodedImageFormat, int quality);
#else
    #define SkEncodeImageWithWIC(...) false
#endif

#endif // SkImageEncoderPriv_DEFINED
