/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/pdf/SkPDFMakeCIDGlyphWidthsArray.h"

#include "include/core/SkPaint.h"
#include "include/private/SkTo.h"
#include "src/core/SkStrike.h"
#include "src/core/SkStrikeSpec.h"
#include "src/pdf/SkPDFGlyphUse.h"

#include <vector>

// TODO(halcanary): Write unit tests for SkPDFMakeCIDGlyphWidthsArray().

// TODO(halcanary): The logic in this file originated in several
// disparate places.  I feel sure that someone could simplify this
// down to a single easy-to-read function.

namespace {

struct AdvanceMetric {
    enum MetricType {
        kDefault,  // Default advance: fAdvance.count = 1
        kRange,    // Advances for a range: fAdvance.count = fEndID-fStartID
        kRun       // fStartID-fEndID have same advance: fAdvance.count = 1
    };
    MetricType fType;
    uint16_t fStartId;
    uint16_t fEndId;
    std::vector<int16_t> fAdvance;
    AdvanceMetric(uint16_t startId) : fStartId(startId) {}
    AdvanceMetric(AdvanceMetric&&) = default;
    AdvanceMetric& operator=(AdvanceMetric&& other) = default;
    AdvanceMetric(const AdvanceMetric&) = delete;
    AdvanceMetric& operator=(const AdvanceMetric&) = delete;
};
const int16_t kInvalidAdvance = SK_MinS16;
const int16_t kDontCareAdvance = SK_MinS16 + 1;
} // namespace

// scale from em-units to base-1000, returning as a SkScalar
static SkScalar from_font_units(SkScalar scaled, uint16_t emSize) {
    if (emSize == 1000) {
        return scaled;
    } else {
        return scaled * 1000 / emSize;
    }
}

static SkScalar scale_from_font_units(int16_t val, uint16_t emSize) {
    return from_font_units(SkIntToScalar(val), emSize);
}

static void strip_uninteresting_trailing_advances_from_range(
        AdvanceMetric* range) {
    SkASSERT(range);

    int expectedAdvanceCount = range->fEndId - range->fStartId + 1;
    if (SkToInt(range->fAdvance.size()) < expectedAdvanceCount) {
        return;
    }

    for (int i = expectedAdvanceCount - 1; i >= 0; --i) {
        if (range->fAdvance[i] != kDontCareAdvance &&
            range->fAdvance[i] != kInvalidAdvance &&
            range->fAdvance[i] != 0) {
            range->fEndId = range->fStartId + i;
            break;
        }
    }
}

static void zero_wildcards_in_range(AdvanceMetric* range) {
    SkASSERT(range);
    if (range->fType != AdvanceMetric::kRange) {
        return;
    }
    SkASSERT(SkToInt(range->fAdvance.size()) == range->fEndId - range->fStartId + 1);

    // Zero out wildcards.
    for (size_t i = 0; i < range->fAdvance.size(); ++i) {
        if (range->fAdvance[i] == kDontCareAdvance) {
            range->fAdvance[i] = 0;
        }
    }
}

static void finish_range(
        AdvanceMetric* range,
        int endId,
        AdvanceMetric::MetricType type) {
    range->fEndId = endId;
    range->fType = type;
    strip_uninteresting_trailing_advances_from_range(range);
    size_t newLength;
    if (type == AdvanceMetric::kRange) {
        newLength = range->fEndId - range->fStartId + 1;
    } else {
        if (range->fEndId == range->fStartId) {
            range->fType = AdvanceMetric::kRange;
        }
        newLength = 1;
    }
    SkASSERT(range->fAdvance.size() >= newLength);
    range->fAdvance.resize(newLength);
    zero_wildcards_in_range(range);
}

static void compose_advance_data(const AdvanceMetric& range,
                                 uint16_t emSize,
                                 SkScalar* defaultAdvance,
                                 SkPDFArray* result) {
    switch (range.fType) {
        case AdvanceMetric::kDefault: {
            SkASSERT(range.fAdvance.size() == 1);
            *defaultAdvance = scale_from_font_units(range.fAdvance[0], emSize);
            break;
        }
        case AdvanceMetric::kRange: {
            auto advanceArray = SkPDFMakeArray();
            for (size_t j = 0; j < range.fAdvance.size(); j++)
                advanceArray->appendScalar(scale_from_font_units(range.fAdvance[j], emSize));
            result->appendInt(range.fStartId);
            result->appendObject(std::move(advanceArray));
            break;
        }
        case AdvanceMetric::kRun: {
            SkASSERT(range.fAdvance.size() == 1);
            result->appendInt(range.fStartId);
            result->appendInt(range.fEndId);
            result->appendScalar(scale_from_font_units(range.fAdvance[0], emSize));
            break;
        }
    }
}

/** Retrieve advance data for glyphs. Used by the PDF backend. */
// TODO(halcanary): this function is complex enough to need its logic
// tested with unit tests.
std::unique_ptr<SkPDFArray> SkPDFMakeCIDGlyphWidthsArray(const SkTypeface& typeface,
                                                         const SkPDFGlyphUse* subset,
                                                         SkScalar* defaultAdvance) {
    // Assuming that on average, the ASCII representation of an advance plus
    // a space is 8 characters and the ASCII representation of a glyph id is 3
    // characters, then the following cut offs for using different range types
    // apply:
    // The cost of stopping and starting the range is 7 characters
    //  a. Removing 4 0's or don't care's is a win
    // The cost of stopping and starting the range plus a run is 22
    // characters
    //  b. Removing 3 repeating advances is a win
    //  c. Removing 2 repeating advances and 3 don't cares is a win
    // When not currently in a range the cost of a run over a range is 16
    // characters, so:
    //  d. Removing a leading 0/don't cares is a win because it is omitted
    //  e. Removing 2 repeating advances is a win

    int emSize;
    SkStrikeSpec strikeSpec = SkStrikeSpec::MakePDFVector(typeface, &emSize);
    SkBulkGlyphMetricsAndPaths paths{strikeSpec};

    auto result = SkPDFMakeArray();
    int num_glyphs = SkToInt(typeface.countGlyphs());

    bool prevRange = false;

    int16_t lastAdvance = kInvalidAdvance;
    int repeatedAdvances = 0;
    int wildCardsInRun = 0;
    int trailingWildCards = 0;

    // Limit the loop count to glyph id ranges provided.
    int lastIndex = num_glyphs;
    if (subset) {
        while (!subset->has(lastIndex - 1) && lastIndex > 0) {
            --lastIndex;
        }
    }
    AdvanceMetric curRange(0);

    SkAutoTArray<SkGlyphID> glyphIDs{lastIndex + 1};
    for (int gId = 0; gId <= lastIndex; gId++) {
        glyphIDs[gId] = gId;
    }

    auto glyphs = paths.glyphs(SkMakeSpan(glyphIDs.get(), lastIndex + 1));

    for (int gId = 0; gId <= lastIndex; gId++) {
        int16_t advance = kInvalidAdvance;
        if (gId < lastIndex) {
            if (!subset || 0 == gId || subset->has(gId)) {
                advance = (int16_t)glyphs[gId]->advanceX();
            } else {
                advance = kDontCareAdvance;
            }
        }
        if (advance == lastAdvance) {
            repeatedAdvances++;
            trailingWildCards = 0;
        } else if (advance == kDontCareAdvance) {
            wildCardsInRun++;
            trailingWildCards++;
        } else if (SkToInt(curRange.fAdvance.size()) ==
                   repeatedAdvances + 1 + wildCardsInRun) {  // All in run.
            if (lastAdvance == 0) {
                curRange.fStartId = gId;  // reset
                curRange.fAdvance.resize(0);
                trailingWildCards = 0;
            } else if (repeatedAdvances + 1 >= 2 || trailingWildCards >= 4) {
                finish_range(&curRange, gId - 1, AdvanceMetric::kRun);
                compose_advance_data(curRange, emSize, defaultAdvance, result.get());
                prevRange = true;
                curRange = AdvanceMetric(gId);
                trailingWildCards = 0;
            }
            repeatedAdvances = 0;
            wildCardsInRun = trailingWildCards;
            trailingWildCards = 0;
        } else {
            if (lastAdvance == 0 &&
                    repeatedAdvances + 1 + wildCardsInRun >= 4) {
                finish_range(&curRange,
                            gId - repeatedAdvances - wildCardsInRun - 2,
                            AdvanceMetric::kRange);
                compose_advance_data(curRange, emSize, defaultAdvance, result.get());
                prevRange = true;
                curRange = AdvanceMetric(gId);
                trailingWildCards = 0;
            } else if (trailingWildCards >= 4 && repeatedAdvances + 1 < 2) {
                finish_range(&curRange, gId - trailingWildCards - 1,
                            AdvanceMetric::kRange);
                compose_advance_data(curRange, emSize, defaultAdvance, result.get());
                prevRange = true;
                curRange = AdvanceMetric(gId);
                trailingWildCards = 0;
            } else if (lastAdvance != 0 &&
                       (repeatedAdvances + 1 >= 3 ||
                        (repeatedAdvances + 1 >= 2 && wildCardsInRun >= 3))) {
                finish_range(&curRange,
                            gId - repeatedAdvances - wildCardsInRun - 2,
                            AdvanceMetric::kRange);
                compose_advance_data(curRange, emSize, defaultAdvance, result.get());
                curRange =
                        AdvanceMetric(gId - repeatedAdvances - wildCardsInRun - 1);
                curRange.fAdvance.push_back(lastAdvance);
                finish_range(&curRange, gId - 1, AdvanceMetric::kRun);
                compose_advance_data(curRange, emSize, defaultAdvance, result.get());
                prevRange = true;
                curRange = AdvanceMetric(gId);
                trailingWildCards = 0;
            }
            repeatedAdvances = 0;
            wildCardsInRun = trailingWildCards;
            trailingWildCards = 0;
        }
        curRange.fAdvance.push_back(advance);
        if (advance != kDontCareAdvance) {
            lastAdvance = advance;
        }
    }
    if (curRange.fStartId == lastIndex) {
        if (!prevRange) {
            return nullptr;  // https://crbug.com/567031
        }
    } else {
        finish_range(&curRange, lastIndex - 1, AdvanceMetric::kRange);
        compose_advance_data(curRange, emSize, defaultAdvance, result.get());
    }
    return result;
}
