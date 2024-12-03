/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontMgr_FontConfigInterface_DEFINED
#define SkFontMgr_FontConfigInterface_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"

class SkFontMgr;
class SkFontConfigInterface;
class SkFontScanner;

/** Creates a SkFontMgr which wraps a SkFontConfigInterface. */
SK_API sk_sp<SkFontMgr> SkFontMgr_New_FCI(sk_sp<SkFontConfigInterface> fci);
SK_API sk_sp<SkFontMgr> SkFontMgr_New_FCI(sk_sp<SkFontConfigInterface> fci,
                                          std::unique_ptr<SkFontScanner> scanner);

#endif // #ifndef SkFontMgr_FontConfigInterface_DEFINED
