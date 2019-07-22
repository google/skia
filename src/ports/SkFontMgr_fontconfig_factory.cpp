/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "include/core/SkFontMgr.h"
#include "include/ports/SkFontMgr_fontconfig.h"

#if defined(SK_SUPPORT_LEGACY_GLOBAL_SKFONTMGR)

sk_sp<SkFontMgr> SkFontMgr::Factory() {
    return SkFontMgr_New_FontConfig(nullptr);
}

#endif  // defined(SK_SUPPORT_LEGACY_GLOBAL_SKFONTMGR)

sk_sp<SkFontMgr> SkNativeFontMgrFactory() {
    return SkFontMgr_New_FontConfig(nullptr);
}
