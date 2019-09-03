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

#include <algorithm>
#include <vector>

// TODO(halcanary): Write unit tests for SkPDFMakeCIDGlyphWidthsArray().

// TODO(halcanary): The logic in this file originated in several
// disparate places.  I feel sure that someone could simplify this
// down to a single easy-to-read function.

namespace {

// scale from em-units to base-1000, returning as a SkScalar
SkScalar from_font_units(SkScalar scaled, uint16_t emSize) {
    if (emSize == 1000) {
        return scaled;
    } else {
        return scaled * 1000 / emSize;
    }
}

SkScalar scale_from_font_units(int16_t val, uint16_t emSize) {
    return from_font_units(SkIntToScalar(val), emSize);
}

// Unfortunately poppler does not appear to respect the default width setting.
#if defined(SK_PDF_CAN_USE_DW)
int16_t findMode(SkSpan<const int16_t> advances) {
    if (advances.empty()) {
        return 0;
    }

    int16_t previousAdvance = advances[0];
    int16_t currentModeAdvance = advances[0];
    size_t currentCount = 1;
    size_t currentModeCount = 1;

    for (size_t i = 1; i < advances.size(); ++i) {
        if (advances[i] == previousAdvance) {
            ++currentCount;
        } else {
            if (currentCount > currentModeCount) {
                currentModeAdvance = previousAdvance;
                currentModeCount = currentCount;
            }
            previousAdvance = advances[i];
            currentCount = 1;
        }
    }

    return currentCount > currentModeCount ? previousAdvance : currentModeAdvance;
}
#endif
} // namespace

/** Retrieve advance data for glyphs. Used by the PDF backend. */
// TODO(halcanary): this function is complex enough to need its logic
// tested with unit tests.
std::unique_ptr<SkPDFArray> SkPDFMakeCIDGlyphWidthsArray(const SkTypeface& typeface,
                                                         const SkPDFGlyphUse& subset,
                                                         SkScalar* defaultAdvance) {
    // There are two ways of expressing advances
    //
    // range: " gfid [adv.ances adv.ances ... adv.ances]"
    //   run: " gfid gfid adv.ances"
    //
    // Assuming that on average
    // the ASCII representation of an advance plus a space is 10 characters
    // the ASCII representation of a glyph id plus a space is 4 characters
    // the ASCII representation of unused gid plus a space in a range is 2 characters
    //
    // When not in a range or run
    //  a. Skipping don't cares or defaults is a win (trivial)
    //  b. Run wins for 2+ repeats " gid gid adv.ances"
    //                             " gid [adv.ances adv.ances]"
    //     rule: 2+ repeats create run as long as possible, else start range
    //
    // When in a range
    // Cost of stopping and starting a range is 8 characters  "] gid ["
    //  c. Skipping defaults is always a win                  " adv.ances"
    //     rule: end range if default seen
    //  d. Skipping 4+ don't cares is a win                   " 0 0 0 0"
    //     rule: end range if 4+ don't cares
    // Cost of stop and start range plus run is 28 characters "] gid gid adv.ances gid ["
    //  e. Switching for 2+ repeats and 4+ don't cares wins   " 0 0 adv.ances 0 0 adv.ances"
    //     rule: end range for 2+ repeats with 4+ don't cares
    //  f. Switching for 3+ repeats wins                      " adv.ances adv.ances adv.ances"
    //     rule: end range for 3+ repeats

    int emSize;
    SkStrikeSpec strikeSpec = SkStrikeSpec::MakePDFVector(typeface, &emSize);
    SkBulkGlyphMetricsAndPaths paths{strikeSpec};

    auto result = SkPDFMakeArray();

    std::vector<SkGlyphID> glyphIDs;
    subset.getSetValues([&](unsigned index) {
        glyphIDs.push_back(SkToU16(index));
    });
    auto glyphs = paths.glyphs(SkMakeSpan(glyphIDs));

#if defined(SK_PDF_CAN_USE_DW)
    std::vector<int16_t> advances;
    advances.reserve(glyphs.size());
    for (const SkGlyph* glyph : glyphs) {
        advances.push_back((int16_t)glyph->advanceX());
    }
    std::sort(advances.begin(), advances.end());
    int16_t modeAdvance = findMode(SkMakeSpan(advances));
    *defaultAdvance = scale_from_font_units(modeAdvance, emSize);
#else
    *defaultAdvance = 0;
#endif

    for (size_t i = 0; i < glyphs.size(); ++i) {
        int16_t advance = (int16_t)glyphs[i]->advanceX();

#if defined(SK_PDF_CAN_USE_DW)
        // a. Skipping don't cares or defaults is a win (trivial)
        if (advance == modeAdvance) {
            continue;
        }
#endif

        // b. 2+ repeats create run as long as possible, else start range
        {
            size_t j = i + 1; // j is always one past the last known repeat
            for (; j < glyphs.size(); ++j) {
                int16_t next_advance = (int16_t)glyphs[j]->advanceX();
                if (advance != next_advance) {
                    break;
                }
            }
            if (j - i >= 2) {
                result->appendInt(glyphs[i]->getGlyphID());
                result->appendInt(glyphs[j - 1]->getGlyphID());
                result->appendScalar(scale_from_font_units(advance, emSize));
                i = j - 1;
                continue;
            }
        }

        {
            result->appendInt(glyphs[i]->getGlyphID());
            auto advanceArray = SkPDFMakeArray();
            advanceArray->appendScalar(scale_from_font_units(advance, emSize));
            size_t j = i + 1; // j is always one past the last output
            for (; j < glyphs.size(); ++j) {
                advance = (int16_t)glyphs[j]->advanceX();
#if defined(SK_PDF_CAN_USE_DW)
                // c. end range if default seen
                if (advance == modeAdvance) {
                    break;
                }
#endif

                int dontCares = glyphs[j]->getGlyphID() - glyphs[j - 1]->getGlyphID() - 1;
                // d. end range if 4+ don't cares
                if (dontCares >= 4) {
                    break;
                }

                int16_t next_advance = 0;
                // e. end range for 2+ repeats with 4+ don't cares
                if (j + 1 < glyphs.size()) {
                    next_advance = (int16_t)glyphs[j+1]->advanceX();
                    int next_dontCares = glyphs[j+1]->getGlyphID() - glyphs[j]->getGlyphID() - 1;
                    if (advance == next_advance && dontCares + next_dontCares >= 4) {
                        break;
                    }
                }

                // f. end range for 3+ repeats
                if (j + 2 < glyphs.size() && advance == next_advance) {
                    next_advance = (int16_t)glyphs[j+2]->advanceX();
                    if (advance == next_advance) {
                        break;
                    }
                }

                while (dontCares --> 0) {
                    advanceArray->appendScalar(0);
                }
                advanceArray->appendScalar(scale_from_font_units(advance, emSize));
            }
            result->appendObject(std::move(advanceArray));
            i = j - 1;
        }
    }

    return result;
}
