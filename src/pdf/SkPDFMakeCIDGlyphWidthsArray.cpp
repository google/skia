/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/pdf/SkPDFMakeCIDGlyphWidthsArray.h"

#include "include/private/SkTo.h"
#include "src/core/SkStrike.h"
#include "src/core/SkStrikeSpec.h"
#include "src/pdf/SkPDFGlyphUse.h"

#include <vector>

static void append(SkPDFArray* dst, int first, int last, float advance, int emSize) {
    dst->appendInt(first);
    dst->appendInt(last);
    dst->appendScalar(emSize != 1000 ? (advance * 1000) / emSize : advance);
}

/** Retrieve advance data for glyphs. Used by the PDF backend. */
std::unique_ptr<SkPDFArray> SkPDFMakeCIDGlyphWidthsArray(const SkTypeface& typeface,
                                                         const SkPDFGlyphUse* subset,
                                                         SkScalar* defaultAdvance) {
    auto result = SkPDFMakeArray();
    int emSize = 1000;
    SkStrikeSpec strikeSpec = SkStrikeSpec::MakePDFVector(typeface, &emSize);
    SkBulkGlyphMetricsAndPaths paths{strikeSpec};
    std::vector<SkGlyphID> glyphIDs;
    if (!subset->has(0)) {
        glyphIDs.push_back(0);
    }
    subset->getSetValues([&](unsigned i) { glyphIDs.push_back(SkToU16(i)); });
    result->reserve(SkToInt(glyphIDs.size() * 3));
    auto glyphs = paths.glyphs(SkMakeSpan(glyphIDs));
    int firstGlyphID = 0, lastGlyphID = 0;
    float advance = glyphs[0]->advanceX();

    for (size_t idx = 1; idx < glyphIDs.size(); ++idx) {
        if (glyphs[idx]->advanceX() != advance) {
            append(result.get(), firstGlyphID, lastGlyphID, advance, emSize);
            advance = glyphs[idx]->advanceX();
            firstGlyphID = glyphIDs[idx];
        }
        lastGlyphID = glyphIDs[idx];
    }
    append(result.get(), firstGlyphID, lastGlyphID, advance, emSize);
    *defaultAdvance = 0;
    return result;
}
