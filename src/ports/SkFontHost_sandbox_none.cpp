/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFontHost.h"
#include "SkTypeface.h"
#include "SkTypeface_win.h"

//static
void SkFontHost::EnsureTypefaceAccessible(const SkTypeface& typeface) {
    //No sandbox, nothing to do.
}
