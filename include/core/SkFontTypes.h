/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkFontTypes_DEFINED
#define SkFontTypes_DEFINED

#include <cstdint>

enum SkTextEncoding {
    kUTF8_SkTextEncoding,
    kUTF16_SkTextEncoding,
    kUTF32_SkTextEncoding,
    kGlyphID_SkTextEncoding,
};

/** 32 bit integer to hold a unicode value */
using SkUnichar = int32_t;

/** 16 bit unsigned integer to hold a glyph index */
using SkGlyphID = uint16_t;

#endif  // SkFontTypes_DEFINED
