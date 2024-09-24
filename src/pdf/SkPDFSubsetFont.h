// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkPDFSubsetFont_DEFINED
#define SkPDFSubsetFont_DEFINED

#include "include/core/SkData.h" // IWYU pragma: keep
#include "include/core/SkRefCnt.h"

class SkPDFGlyphUse;
class SkTypeface;

/** Subset the typeface's data to only include the glyphs used.
 *  The glyph ids will remain the same.
 *
 *  @return The subset font data, or nullptr if it cannot be subset.
 */
sk_sp<SkData> SkPDFSubsetFont(const SkTypeface& typeface, const SkPDFGlyphUse& glyphUsage);

#endif  // SkPDFSubsetFont_DEFINED
