/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontMgr_fontconfig_DEFINED
#define SkFontMgr_fontconfig_DEFINED

#include <fontconfig/fontconfig.h>
#include "include/core/SkRefCnt.h"
#include <memory>

class SkFontMgr;

/** Create a font manager around a FontConfig instance.
 *  If 'fc' is NULL, will use a new default config.
 *  Takes ownership of 'fc' and will call FcConfigDestroy on it.
 */
class SkFontScanner;
SK_API sk_sp<SkFontMgr> SkFontMgr_New_FontConfig(FcConfig* fc, std::unique_ptr<SkFontScanner> scanner);
SK_API sk_sp<SkFontMgr> SkFontMgr_New_FontConfig(FcConfig* fc);
#endif // #ifndef SkFontMgr_fontconfig_DEFINED
