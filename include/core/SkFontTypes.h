/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontTypes_DEFINED
#define SkFontTypes_DEFINED

#include "SkScalar.h"
#include "SkTypeface.h"

enum SkTextEncoding : uint8_t {
    kUTF8_SkTextEncoding,
    kUTF16_SkTextEncoding,
    kUTF32_SkTextEncoding,
    kGlyphID_SkTextEncoding,
};

enum class SkFontHinting {
    kNone,      //!< glyph outlines unchanged
    kSlight,    //!< minimal modification to improve constrast
    kNormal,    //!< glyph outlines modified to improve constrast
    kFull,      //!< modifies glyph outlines for maximum constrast
};

#define kNo_SkFontHinting       SkFontHinting::kNone
#define kSlight_SkFontHinting   SkFontHinting::kSlight
#define kNormal_SkFontHinting   SkFontHinting::kNormal
#define kFull_SkFontHinting     SkFontHinting::kFull

#endif
