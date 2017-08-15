/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkTypes.h"
#if 1 //defined(SK_BUILD_FOR_ANDROID)

#include "SkFontMgr.h"
#include "SkFontMgr_android.h"

sk_sp<SkFontMgr> SkFontMgr::Factory() {
    SkFontMgr_Android_CustomFonts foo;

    foo.fSystemFontUse = SkFontMgr_Android_CustomFonts::kOnlyCustom;
#if 0
    foo.fBasePath = "C:\\src\\skia.1\\android-fonts\\";
    foo.fFontsXml = "c:\\src\\skia.1\\fonts.xml";
#else
    foo.fBasePath = "/Users/robertphillips/src/skia.0/android-fonts/";
    foo.fFontsXml = "/Users/robertphillips/src/skia.0/fonts.xml";
#endif
    foo.fFallbackFontsXml = nullptr;
    foo.fIsolated = false;

    return SkFontMgr_New_Android(&foo);
}

#endif//defined(SK_BUILD_FOR_ANDROID)
