/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "include/core/SkFontMgr.h"
#include "include/ports/SkFontMgr_empty.h"

#if defined(SK_SUPPORT_LEGACY_GLOBAL_SKFONTMGR)

sk_sp<SkFontMgr> SkFontMgr::Factory() {
    return SkFontMgr_New_Custom_Empty();
}

#endif  // defined(SK_SUPPORT_LEGACY_GLOBAL_SKFONTMGR)

sk_sp<SkFontMgr> SkNativeFontMgrFactory() {
    return SkFontMgr_New_Custom_Empty();
}
