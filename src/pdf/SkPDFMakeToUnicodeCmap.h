/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPDFMakeToUnicodeCmap_DEFINED
#define SkPDFMakeToUnicodeCmap_DEFINED

#include "SkTDArray.h"
#include "SkPDFFont.h"
#include "SkStream.h"

sk_sp<SkPDFStream> SkPDFMakeToUnicodeCmap(
        const SkTDArray<SkUnichar>& glyphToUnicode,
        const SkBitSet* subset,
        bool multiByteGlyphs,
        SkGlyphID firstGlyphID,
        SkGlyphID lastGlyphID);

// Exposed for unit testing.
void SkPDFAppendCmapSections(const SkTDArray<SkUnichar>& glyphToUnicode,
                             const SkBitSet* subset,
                             SkDynamicMemoryWStream* cmap,
                             bool multiByteGlyphs,
                             SkGlyphID firstGlyphID,
                             SkGlyphID lastGlyphID);

#endif  // SkPDFMakeToUnicodeCmap_DEFINED
