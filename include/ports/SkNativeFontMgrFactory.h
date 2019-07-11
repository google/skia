/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkNativeFontMgrFactory_DEFINED
#define SkNativeFontMgrFactory_DEFINED

#include "include/core/SkFontMgr.h"

/** Create a new font manager. The implementation depends on which definition of this function has
    been compiled into the binary. See implementations in src/ports/SkFontMgr_*_factory.cpp and
    src/ports/SkFontHost_mac.cpp.
*/
SK_API sk_sp<SkFontMgr> SkNativeFontMgrFactory();

#endif // SkNativeFontMgrFactory_DEFINED
