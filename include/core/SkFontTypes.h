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

enum SkTextEncoding {
    kUTF8_SkTextEncoding,
    kUTF16_SkTextEncoding,
    kUTF32_SkTextEncoding,
    kGlyphID_SkTextEncoding,
};

enum SkFontHinting {
    kNo_SkFontHinting     = 0, //!< glyph outlines unchanged
    kSlight_SkFontHinting = 1, //!< minimal modification to improve constrast
    kNormal_SkFontHinting = 2, //!< glyph outlines modified to improve constrast
    kFull_SkFontHinting   = 3, //!< modifies glyph outlines for maximum constrast
};

#endif
