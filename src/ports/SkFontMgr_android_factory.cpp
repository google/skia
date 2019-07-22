/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#if defined(SK_BUILD_FOR_ANDROID)

#include "include/core/SkFontMgr.h"
#include "include/ports/SkFontMgr_android.h"

#if defined(SK_SUPPORT_LEGACY_GLOBAL_SKFONTMGR)

sk_sp<SkFontMgr> SkFontMgr::Factory() {
    return SkFontMgr_New_Android(nullptr);
}

#endif  // defined(SK_SUPPORT_LEGACY_GLOBAL_SKFONTMGR)

sk_sp<SkFontMgr> SkNativeFontMgrFactory() {
    return SkFontMgr_New_Android(nullptr);
}

#endif//defined(SK_BUILD_FOR_ANDROID)
