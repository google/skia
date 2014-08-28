/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkForceLinking.h"
#include "SkImageDecoder.h"

// This method is required to fool the linker into not discarding the pre-main
// initialization and registration of the decoder classes. Passing true will
// cause memory leaks.
int SkForceLinking(bool doNotPassTrue) {
    if (doNotPassTrue) {
        SkASSERT(false);
        CreateJPEGImageDecoder();
        CreateWEBPImageDecoder();
        CreateBMPImageDecoder();
        CreateICOImageDecoder();
        CreateWBMPImageDecoder();
        // Only link hardware texture codecs on platforms that build them. See images.gyp
#ifndef SK_BUILD_FOR_ANDROID_FRAMEWORK
        CreatePKMImageDecoder();
        CreateKTXImageDecoder();
        CreateASTCImageDecoder();
#endif
        // Only link GIF and PNG on platforms that build them. See images.gyp
#if !defined(SK_BUILD_FOR_MAC) && !defined(SK_BUILD_FOR_WIN) && !defined(SK_BUILD_FOR_NACL) \
        && !defined(SK_BUILD_FOR_IOS)
        CreateGIFImageDecoder();
#endif
#if !defined(SK_BUILD_FOR_MAC) && !defined(SK_BUILD_FOR_WIN) && !defined(SK_BUILD_FOR_IOS)
        CreatePNGImageDecoder();
#endif
#if defined(SK_BUILD_FOR_IOS)
        CreatePNGImageEncoder_IOS();
#endif
        return -1;
    }
    return 0;
}
