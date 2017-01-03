/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFontMgr.h"
#include "SkFontMgr_fontconfig.h"
#include "SkTypes.h"

#ifdef SK_LEGACY_FONTMGR_FACTORY
SkFontMgr* SkFontMgr::Factory() {
#else
sk_sp<SkFontMgr> SkFontMgr::Factory() {
#endif
    return SkFontMgr_New_FontConfig(nullptr);
}
