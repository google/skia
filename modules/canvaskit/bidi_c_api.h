/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef bidi_c_api_DEFINED
#define bidi_c_api_DEFINED

#include "modules/skunicode/include/SkUnicode.h"

#if defined(SK_UNICODE_BIDI_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_bidi.h"
#else
#error "SkUnicode bidi component is required but missing"
#endif

#include <algorithm>
#include <cstdint>

namespace skbidi {

inline SkUnicode* getBidiUnicode() {
    static SkUnicode* unicode = SkUnicodes::Bidi::Make().release();
    return unicode;
}

inline int getBidiRegions(const uint16_t* text, int textCount, int dir, int32_t* out_regions) {
    SkBidiIterator::Direction direction =
            dir == 1 ? SkBidiIterator::Direction::kLTR : SkBidiIterator::Direction::kRTL;
    int regionCount = 0;
    getBidiUnicode()->forEachBidiRegion(text,
                                        textCount,
                                        direction,
                                        [&](SkBidiIterator::Position start,
                                            SkBidiIterator::Position end,
                                            SkBidiIterator::Level level) {
                                            out_regions[regionCount * 3 + 0] = start;
                                            out_regions[regionCount * 3 + 1] = end;
                                            out_regions[regionCount * 3 + 2] = level;
                                            regionCount++;
                                        });
    return regionCount;
}

inline int reorderVisual(const SkUnicode::BidiLevel* runLevels,
                         int levelsCount,
                         int32_t* logicalFromVisual) {
    if (levelsCount <= 0 || !runLevels || !logicalFromVisual) {
        return 0;
    }
    // ubidi_reorderVisual returns void and leaves the output buffer untouched if any level
    // exceeds UBIDI_MAX_EXPLICIT_LEVEL + 1 (126). Sentinel-initialize index 0 to detect
    // early bailout and zero-fill the output so callers never read uninitialized memory
    logicalFromVisual[0] = -1;
    getBidiUnicode()->reorderVisual(runLevels, levelsCount, logicalFromVisual);
    if (logicalFromVisual[0] < 0) {
        std::fill_n(logicalFromVisual, levelsCount, 0);
        return 0;
    }
    return 1;
}

inline int computeCodeUnitFlags(const uint16_t* text, int textCount, uint16_t* out_flags) {
    skia_private::TArray<SkUnicode::CodeUnitFlags, true> flags;
    flags.resize(textCount);
    // SkUnicode::computeCodeUnitFlags expects char16_t* but does not mutate the buffer when
    // replaceTabs is false
    char16_t* utf16 = reinterpret_cast<char16_t*>(const_cast<uint16_t*>(text));
    if (!getBidiUnicode()->computeCodeUnitFlags(utf16, textCount, false, &flags)) {
        return 0;
    }
    for (int i = 0; i < textCount; i++) {
        out_flags[i] = static_cast<uint16_t>(flags[i]);
    }
    return 1;
}

}  // namespace skbidi

#endif  // bidi_c_api_DEFINED
