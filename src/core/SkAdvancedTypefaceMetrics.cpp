/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkAdvancedTypefaceMetrics.h"
#include "SkTypes.h"

SkAdvancedTypefaceMetrics::~SkAdvancedTypefaceMetrics() {}

const int16_t kInvalidAdvance = SK_MinS16;
const int16_t kDontCareAdvance = SK_MinS16 + 1;

static void stripUninterestingTrailingAdvancesFromRange(
        SkAdvancedTypefaceMetrics::WidthRange* range) {
    SkASSERT(range);

    int expectedAdvanceCount = range->fEndId - range->fStartId + 1;
    if (range->fAdvance.count() < expectedAdvanceCount) {
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

static void zeroWildcardsInRange(SkAdvancedTypefaceMetrics::WidthRange* range) {
    SkASSERT(range);
    if (range->fType != SkAdvancedTypefaceMetrics::WidthRange::kRange) {
        return;
    }
    SkASSERT(range->fAdvance.count() == range->fEndId - range->fStartId + 1);

    // Zero out wildcards.
    for (int i = 0; i < range->fAdvance.count(); ++i) {
        if (range->fAdvance[i] == kDontCareAdvance) {
            range->fAdvance[i] = 0;
        }
    }
}

void SkAdvancedTypefaceMetrics::FinishRange(
        SkAdvancedTypefaceMetrics::WidthRange* range,
        int endId,
        SkAdvancedTypefaceMetrics::WidthRange::MetricType type) {
    range->fEndId = endId;
    range->fType = type;
    stripUninterestingTrailingAdvancesFromRange(range);
    int newLength;
    if (type == SkAdvancedTypefaceMetrics::WidthRange::kRange) {
        newLength = range->fEndId - range->fStartId + 1;
    } else {
        if (range->fEndId == range->fStartId) {
            range->fType = SkAdvancedTypefaceMetrics::WidthRange::kRange;
        }
        newLength = 1;
    }
    SkASSERT(range->fAdvance.count() >= newLength);
    range->fAdvance.setCount(newLength);
    zeroWildcardsInRange(range);
}

void SkAdvancedTypefaceMetrics::setGlyphWidths(
        int num_glyphs,
        const uint32_t* subsetGlyphIDs,
        uint32_t subsetGlyphIDsLength,
        SkAdvancedTypefaceMetrics::GetAdvance getAdvance) {
    // Assuming that on average, the ASCII representation of an advance plus
    // a space is 8 characters and the ASCII representation of a glyph id is 3
    // characters, then the following cut offs for using different range types
    // apply:
    // The cost of stopping and starting the range is 7 characers
    //  a. Removing 4 0's or don't care's is a win
    // The cost of stopping and starting the range plus a run is 22
    // characters
    //  b. Removing 3 repeating advances is a win
    //  c. Removing 2 repeating advances and 3 don't cares is a win
    // When not currently in a range the cost of a run over a range is 16
    // characaters, so:
    //  d. Removing a leading 0/don't cares is a win because it is omitted
    //  e. Removing 2 repeating advances is a win

    WidthRange* prevRange = nullptr;
    int16_t lastAdvance = kInvalidAdvance;
    int repeatedAdvances = 0;
    int wildCardsInRun = 0;
    int trailingWildCards = 0;
    uint32_t subsetIndex = 0;

    // Limit the loop count to glyph id ranges provided.
    int firstIndex = 0;
    int lastIndex = num_glyphs;
    if (subsetGlyphIDs) {
        firstIndex = static_cast<int>(subsetGlyphIDs[0]);
        lastIndex =
                static_cast<int>(subsetGlyphIDs[subsetGlyphIDsLength - 1]) + 1;
    }
    WidthRange curRange(firstIndex);

    for (int gId = firstIndex; gId <= lastIndex; gId++) {
        int16_t advance = kInvalidAdvance;
        if (gId < lastIndex) {
            // Get glyph id only when subset is nullptr, or the id is in subset.
            SkASSERT(!subsetGlyphIDs || (subsetIndex < subsetGlyphIDsLength &&
                    static_cast<uint32_t>(gId) <= subsetGlyphIDs[subsetIndex]));
            if (!subsetGlyphIDs ||
                (subsetIndex < subsetGlyphIDsLength &&
                 static_cast<uint32_t>(gId) == subsetGlyphIDs[subsetIndex])) {
                SkAssertResult(getAdvance(gId, &advance));
                ++subsetIndex;
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
        } else if (curRange.fAdvance.count() ==
                   repeatedAdvances + 1 + wildCardsInRun) {  // All in run.
            if (lastAdvance == 0) {
                curRange.fStartId = gId;  // reset
                curRange.fAdvance.setCount(0);
                trailingWildCards = 0;
            } else if (repeatedAdvances + 1 >= 2 || trailingWildCards >= 4) {
                FinishRange(&curRange, gId - 1, WidthRange::kRun);
                prevRange = fGlyphWidths.emplace_back(std::move(curRange));
                curRange = WidthRange(gId);
                trailingWildCards = 0;
            }
            repeatedAdvances = 0;
            wildCardsInRun = trailingWildCards;
            trailingWildCards = 0;
        } else {
            if (lastAdvance == 0 &&
                    repeatedAdvances + 1 + wildCardsInRun >= 4) {
                FinishRange(&curRange,
                            gId - repeatedAdvances - wildCardsInRun - 2,
                            WidthRange::kRange);
                prevRange = fGlyphWidths.emplace_back(std::move(curRange));
                curRange = WidthRange(gId);
                trailingWildCards = 0;
            } else if (trailingWildCards >= 4 && repeatedAdvances + 1 < 2) {
                FinishRange(&curRange, gId - trailingWildCards - 1,
                            WidthRange::kRange);
                prevRange = fGlyphWidths.emplace_back(std::move(curRange));
                curRange = WidthRange(gId);
                trailingWildCards = 0;
            } else if (lastAdvance != 0 &&
                       (repeatedAdvances + 1 >= 3 ||
                        (repeatedAdvances + 1 >= 2 && wildCardsInRun >= 3))) {
                FinishRange(&curRange,
                            gId - repeatedAdvances - wildCardsInRun - 2,
                            WidthRange::kRange);
                (void)fGlyphWidths.emplace_back(std::move(curRange));
                curRange =
                        WidthRange(gId - repeatedAdvances - wildCardsInRun - 1);
                curRange.fAdvance.append(1, &lastAdvance);
                FinishRange(&curRange, gId - 1, WidthRange::kRun);
                prevRange = fGlyphWidths.emplace_back(std::move(curRange));
                curRange = WidthRange(gId);
                trailingWildCards = 0;
            }
            repeatedAdvances = 0;
            wildCardsInRun = trailingWildCards;
            trailingWildCards = 0;
        }
        curRange.fAdvance.append(1, &advance);
        if (advance != kDontCareAdvance) {
            lastAdvance = advance;
        }
    }
    if (curRange.fStartId == lastIndex) {
        SkASSERT(prevRange);
        if (!prevRange) {
            fGlyphWidths.reset();
            return;  // https://crbug.com/567031
        }
    } else {
        FinishRange(&curRange, lastIndex - 1, WidthRange::kRange);
        fGlyphWidths.emplace_back(std::move(curRange));
    }
}
