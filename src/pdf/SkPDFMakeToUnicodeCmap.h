/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPDFMakeToUnicodeCmap_DEFINED
#define SkPDFMakeToUnicodeCmap_DEFINED

#include "include/core/SkTypes.h"
#include "src/core/SkTHash.h"

#include <memory>

class SkDynamicMemoryWStream;
class SkPDFGlyphUse;
class SkStreamAsset;
class SkString;

std::unique_ptr<SkStreamAsset> SkPDFMakeToUnicodeCmap(
        const SkUnichar* glyphToUnicode,
        const skia_private::THashMap<SkGlyphID, SkString>& glyphToUnicodeEx,
        const SkPDFGlyphUse* subset,
        bool multiByteGlyphs,
        SkGlyphID firstGlyphID,
        SkGlyphID lastGlyphID);

// Exposed for unit testing.
void SkPDFAppendCmapSections(const SkUnichar* glyphToUnicode,
                             const skia_private::THashMap<SkGlyphID, SkString>& glyphToUnicodeEx,
                             const SkPDFGlyphUse* subset,
                             SkDynamicMemoryWStream* cmap,
                             bool multiByteGlyphs,
                             SkGlyphID firstGlyphID,
                             SkGlyphID lastGlyphID);

#endif  // SkPDFMakeToUnicodeCmap_DEFINED
