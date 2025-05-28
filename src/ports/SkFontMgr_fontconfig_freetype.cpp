/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontScanner.h"
#include "include/core/SkRefCnt.h"
#include "include/ports/SkFontMgr_fontconfig.h"
#include "include/ports/SkFontScanner_FreeType.h"

#if !defined(SK_DISABLE_LEGACY_FONTCONFIG_FACTORY)

sk_sp<SkFontMgr> SkFontMgr_New_FontConfig(FcConfig* fc) {
    return SkFontMgr_New_FontConfig(fc, SkFontScanner_Make_FreeType());
}

#endif
