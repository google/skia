/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageEncoderPriv.h"
#include "SkForceLinking.h"

// This method is required to fool the linker into not discarding the pre-main
// initialization and registration of the encoder classes. Passing true will
// cause memory leaks.
int SkForceLinking(bool doNotPassTrue) {
    if (doNotPassTrue) {
        SkASSERT(false);
#if defined(SK_HAS_JPEG_LIBRARY) && !defined(SK_USE_CG_ENCODER) && !defined(SK_USE_WIC_ENCODER)
        CreateJPEGImageEncoder();
#endif
#if defined(SK_HAS_WEBP_LIBRARY) && !defined(SK_USE_CG_ENCODER) && !defined(SK_USE_WIC_ENCODER)
        CreateWEBPImageEncoder();
#endif
#if defined(SK_HAS_PNG_LIBRARY) && !defined(SK_USE_CG_ENCODER) && !defined(SK_USE_WIC_ENCODER)
        CreatePNGImageEncoder();
#endif

        // Only link hardware texture codecs on platforms that build them. See images.gyp
#ifndef SK_BUILD_FOR_ANDROID_FRAMEWORK
        CreateKTXImageEncoder();
#endif

#if defined (SK_USE_CG_ENCODER)
        CreateImageEncoder_CG(SkEncodedImageFormat::kPNG);
#endif
#if defined (SK_USE_WIC_ENCODER)
        CreateImageEncoder_WIC(SkEncodedImageFormat::kPNG);
#endif
        return -1;
    }
    return 0;
}
