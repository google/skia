/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFontMgr.h"
#include "SkFontMgr_custom.h"

SkFontMgr* SkFontMgr::Factory() {
    return SkFontMgr_New_Custom_Empty();
}
