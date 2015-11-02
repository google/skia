/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"
#if defined(SK_BUILD_FOR_WIN32)  // And !SKIA_GDI?

#include "SkFontMgr.h"
#include "SkTypeface_win.h"

SkFontMgr* SkFontMgr::Factory() {
    return SkFontMgr_New_DirectWrite();
}

#endif//defined(SK_BUILD_FOR_WIN32)
