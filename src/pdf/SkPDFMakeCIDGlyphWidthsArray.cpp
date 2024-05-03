/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/pdf/SkPDFMakeCIDGlyphWidthsArray.h"

#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTo.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkStrikeSpec.h"
#include "src/pdf/SkPDFGlyphUse.h"
#include "src/pdf/SkPDFTypes.h"

#include <algorithm>
#include <cstddef>
#include <utility>
#include <vector>

namespace {

// Scale from em-units to 1000-units.
SkScalar from_font_units(SkScalar scaled, uint16_t emSize) {
    if (emSize == 1000) {
        return scaled;
    } else {
        return scaled * 1000 / emSize;
    }
}

SkScalar find_mode_or_0(SkSpan<const SkScalar> advances) {
    if (advances.empty()) {
        return 0;
    }

    SkScalar currentAdvance = advances[0];
    SkScalar currentModeAdvance = advances[0];
    size_t currentCount = 1;
    size_t currentModeCount = 1;

    for (size_t i = 1; i < advances.size(); ++i) {
        if (advances[i] == currentAdvance) {
            ++currentCount;
        } else {
            if (currentCount > currentModeCount) {
                currentModeAdvance = currentAdvance;
                currentModeCount = currentCount;
            }
            currentAdvance = advances[i];
            currentCount = 1;
        }
    }
    return currentCount > currentModeCount ? currentAdvance : currentModeAdvance;
}

} // namespace

std::unique_ptr<SkPDFArray> SkPDFMakeCIDGlyphWidthsArray(const SkTypeface& typeface,
                                                         const SkPDFGlyphUse& subset,
                                                         int32_t* defaultAdvance) {
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
    auto glyphs = paths.glyphs(SkSpan(glyphIDs));

    // C++20 = make_unique_for_overwrite<SkScalar[]>(glyphs.size());
    auto advances = std::unique_ptr<SkScalar[]>(new SkScalar[glyphs.size()]);

    // Find the pdf integer mode (most common pdf integer advance).
    // Unfortunately, poppler enforces DW (default width) must be an integer,
    // so only consider integer pdf advances when finding the mode.
    size_t numIntAdvances = 0;
    for (const SkGlyph* glyph : glyphs) {
        SkScalar currentAdvance = from_font_units(glyph->advanceX(), emSize);
        if ((int32_t)currentAdvance == currentAdvance) {
            advances[numIntAdvances++] = currentAdvance;
        }
    }
    std::sort(advances.get(), advances.get() + numIntAdvances);
    int32_t modeAdvance = (int32_t)find_mode_or_0(SkSpan(advances.get(), numIntAdvances));
    *defaultAdvance = modeAdvance;

    // Pre-convert to pdf advances.
    for (size_t i = 0; i < glyphs.size(); ++i) {
        advances[i] = from_font_units(glyphs[i]->advanceX(), emSize);
    }

    for (size_t i = 0; i < glyphs.size(); ++i) {
        SkScalar advance = advances[i];

        // a. Skipping don't cares or defaults is a win (trivial)
        if (advance == modeAdvance) {
            continue;
        }

        // b. 2+ repeats create run as long as possible, else start range
        {
            size_t j = i + 1; // j is always one past the last known repeat
            for (; j < glyphs.size(); ++j) {
                SkScalar next_advance = advances[j];
                if (advance != next_advance) {
                    break;
                }
            }
            if (j - i >= 2) {
                result->appendInt(glyphs[i]->getGlyphID());
                result->appendInt(glyphs[j - 1]->getGlyphID());
                result->appendScalar(advance);
                i = j - 1;
                continue;
            }
        }

        {
            result->appendInt(glyphs[i]->getGlyphID());
            auto advanceArray = SkPDFMakeArray();
            advanceArray->appendScalar(advance);
            size_t j = i + 1; // j is always one past the last output
            for (; j < glyphs.size(); ++j) {
                advance = advances[j];

                // c. end range if default seen
                if (advance == modeAdvance) {
                    break;
                }

                int dontCares = glyphs[j]->getGlyphID() - glyphs[j - 1]->getGlyphID() - 1;
                // d. end range if 4+ don't cares
                if (dontCares >= 4) {
                    break;
                }

                SkScalar next_advance = 0;
                // e. end range for 2+ repeats with 4+ don't cares
                if (j + 1 < glyphs.size()) {
                    next_advance = advances[j+1];
                    int next_dontCares = glyphs[j+1]->getGlyphID() - glyphs[j]->getGlyphID() - 1;
                    if (advance == next_advance && dontCares + next_dontCares >= 4) {
                        break;
                    }
                }

                // f. end range for 3+ repeats
                if (j + 2 < glyphs.size() && advance == next_advance) {
                    next_advance = advances[j+2];
                    if (advance == next_advance) {
                        break;
                    }
                }

                while (dontCares --> 0) {
                    advanceArray->appendScalar(0);
                }
                advanceArray->appendScalar(advance);
            }
            result->appendObject(std::move(advanceArray));
            i = j - 1;
        }
    }

    return result;
}
