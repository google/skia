/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)

#include "include/core/SkFontMgr.h"
#include "include/ports/SkFontMgr_mac_ct.h"

#if !defined(SK_DISABLE_LEGACY_FONTMGR_FACTORY)
sk_sp<SkFontMgr> SkFontMgr::Factory() {
    return SkFontMgr_New_CoreText(nullptr);
}
#endif

#endif//defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
