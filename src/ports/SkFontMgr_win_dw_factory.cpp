/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#if defined(SK_BUILD_FOR_WIN)  // And !SKIA_GDI?

#include "include/core/SkFontMgr.h"
#include "include/ports/SkTypeface_win.h"

#if !defined(SK_DISABLE_LEGACY_FONTMGR_REFDEFAULT)
sk_sp<SkFontMgr> SkFontMgr::Factory() {
    return SkFontMgr_New_DirectWrite();
}
#endif

#endif//defined(SK_BUILD_FOR_WIN)
