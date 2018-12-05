/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontTypes_DEFINED
#define SkFontTypes_DEFINED

#include "SkTypes.h"

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
