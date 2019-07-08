/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef TestFontMgr_DEFINED
#define TestFontMgr_DEFINED

#include "include/core/SkFontMgr.h"

// An SkFontMgr that always uses ToolUtils::create_portable_typeface().

namespace ToolUtils {
sk_sp<SkFontMgr> MakePortableFontMgr();

/** This is a magic function that causes other magic functions to use
    MakePortableFontMgr(). You must only call it near the beginning of main().
*/
void SetGlobalPortableFontMgr();

}  // namespace ToolUtils

#endif  // TestFontMgr_DEFINED
