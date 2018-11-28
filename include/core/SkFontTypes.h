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

// TODO: add to clients, and then remove from here.
#define SK_SUPPORT_LEGACY_TEXTENCODINGENUM

#ifdef SK_SUPPORT_LEGACY_TEXTENCODINGENUM
enum SkTextEncoding : uint8_t {
    kUTF8_SkTextEncoding,
    kUTF16_SkTextEncoding,
    kUTF32_SkTextEncoding,
    kGlyphID_SkTextEncoding,
};
#else
enum class SkTextEncoding {
    kUTF8,
    kUTF16,
    kUTF32,
    kGlyphID,
};
#define kUTF8_SkTextEncoding    SkTextEncoding::kUTF8
#define kUTF16_SkTextEncoding   SkTextEncoding::kUTF16
#define kUTF32_SkTextEncoding   SkTextEncoding::kUTF32
#define kGlyphID_SkTextEncoding SkTextEncoding::kGlyphID
#endif

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
