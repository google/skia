/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// All prior contents of this file have been divided among the following files.
// This file remains while builds are migrated.

// The Chromium and Pdfium builds will filter out src/utils/mac/SkCTFontSmoothBehavior.cpp
// on ios, but not on macos. These should be the only builds targeting this file
// that need to be updated in lock-step. So that we can update their build once, hack this in.
#include "include/core/SkTypes.h"
#if defined(SK_BUILD_FOR_IOS)
#include "src/utils/mac/SkCTFontSmoothBehavior.cpp"
#endif
#include "src/ports/SkFontMgr_mac_ct.cpp"
#include "src/ports/SkTypeface_mac_ct.cpp"
#include "src/ports/SkScalerContext_mac_ct.cpp"
#include "src/ports/SkFontMgr_mac_ct_factory.cpp"
