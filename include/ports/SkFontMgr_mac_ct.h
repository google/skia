/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontMgr_mac_ct_DEFINED
#define SkFontMgr_mac_ct_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"

class SkFontMgr;

/** Create a font manager for CoreText. */
SK_API sk_sp<SkFontMgr> SkFontMgr_New_CoreText();

#endif // SkFontMgr_mac_ct_DEFINED
