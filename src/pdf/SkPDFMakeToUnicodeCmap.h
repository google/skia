/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPDFMakeToUnicodeCmap_DEFINED
#define SkPDFMakeToUnicodeCmap_DEFINED

#include "include/core/SkStream.h"
#include "src/pdf/SkPDFFont.h"

std::unique_ptr<SkStreamAsset> SkPDFMakeToUnicodeCmap(
        const SkUnichar* glyphToUnicode,
        const SkPDFGlyphUse* subset,
        bool multiByteGlyphs,
        SkGlyphID firstGlyphID,
        SkGlyphID lastGlyphID);

// Exposed for unit testing.
void SkPDFAppendCmapSections(const SkUnichar* glyphToUnicode,
                             const SkPDFGlyphUse* subset,
                             SkDynamicMemoryWStream* cmap,
                             bool multiByteGlyphs,
                             SkGlyphID firstGlyphID,
                             SkGlyphID lastGlyphID);

#endif  // SkPDFMakeToUnicodeCmap_DEFINED
