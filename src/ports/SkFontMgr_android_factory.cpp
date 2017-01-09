/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkTypes.h"
#if defined(SK_BUILD_FOR_ANDROID)

#include "SkFontMgr.h"
#include "SkFontMgr_android.h"

#ifdef SK_LEGACY_FONTMGR_FACTORY
SkFontMgr* SkFontMgr::Factory() {
#else
sk_sp<SkFontMgr> SkFontMgr::Factory() {
#endif
    return SkFontMgr_New_Android(nullptr);
}

#endif//defined(SK_BUILD_FOR_ANDROID)
