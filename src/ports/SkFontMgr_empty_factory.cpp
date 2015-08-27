/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFontMgr.h"

SkFontMgr* SkFontMgr::Factory() {
    // Always return nullptr, an empty SkFontMgr will be used.
    return nullptr;
}
