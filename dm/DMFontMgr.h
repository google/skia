/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DMFontMgr_DEFINED
#define DMFontMgr_DEFINED

#include "SkFontMgr.h"

// An SkFontMgr that always uses sk_tool_utils::create_portable_typeface().

namespace DM {
    sk_sp<SkFontMgr> MakeFontMgr();
}  // namespace DM

#endif//DMFontMgr_DEFINED
