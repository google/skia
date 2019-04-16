/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontTypes_DEFINED
#define SkFontTypes_DEFINED

enum class SkTextEncoding {
    kUTF8,      //!< uses bytes to represent UTF-8 or ASCII
    kUTF16,     //!< uses two byte words to represent most of Unicode
    kUTF32,     //!< uses four byte words to represent all of Unicode
    kGlyphID,   //!< uses two byte words to represent glyph indices
};

#define kUTF8_SkTextEncoding    SkTextEncoding::kUTF8
#define kUTF16_SkTextEncoding   SkTextEncoding::kUTF16
#define kUTF32_SkTextEncoding   SkTextEncoding::kUTF32
#define kGlyphID_SkTextEncoding SkTextEncoding::kGlyphID

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
