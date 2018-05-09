/*
 * Copyright 2018 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGlyphRunInfo.h"
#include "SkUtils.h"

#include <algorithm>

namespace {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wuninitialized"
    struct GlyphIndex {
        GlyphIndex() : glyph(glyph), index(index) {}
        GlyphIndex(SkGlyphID glyph_, uint16_t index_) : glyph(glyph_), index(index_) {}
        SkGlyphID glyph;
        uint16_t index;
        constexpr bool operator < (const GlyphIndex& rhs) const { return glyph < rhs.glyph; }
    };

    struct UnicharIndex {
        UnicharIndex() : unichar(unichar), index(index) {}
        UnicharIndex(SkGlyphID unichar_, uint16_t index_) : unichar(unichar_), index(index_) {}
        SkUnichar unichar;
        uint16_t index;
        constexpr bool operator < (const UnicharIndex& rhs) const { return unichar < rhs.unichar; }
    };
#pragma clang diagnostic pop
}

SkGlyphRunInfo SkGlyphRunInfo::Make(uint16_t count, const SkGlyphID* glyphs) {
    std::vector<GlyphIndex> glyphIndex(count);

    for (uint16_t i = 0; i < count; i++) {
        glyphIndex[i] = GlyphIndex{glyphs[i], i};
    }

    std::sort(glyphIndex.begin(), glyphIndex.end());

    std::vector<DenseIndex> denseIndex(count);
    std::vector<DrawableGlyph> unique;

    uint16_t last = 0;
    uint16_t uniqueIndex = 0;
    for (auto& gi : glyphIndex) {
        if (last != gi.glyph) {
            unique.emplace_back(gi.glyph);
            last = gi.glyph;
            uniqueIndex = SkTo<uint16_t>(unique.size() - 1);
        }
        denseIndex[gi.index] = uniqueIndex;
    }

    return SkGlyphRunInfo{std::move(denseIndex), std::move(unique)};
}

SkGlyphRunInfo SkGlyphRunInfo::Make(size_t byteLength, const void* utfN,
                                    const SkTypeface& typeface, SkTypeface::Encoding encoding) {
    auto count = SkUTFN_CountUnichars(encoding, utfN, byteLength);
    SkASSERT(count < std::numeric_limits<uint16_t>::max());

    std::vector<SkUnichar> unicharIndex(count);



    for (uint16_t i = 0; i < count; i++) {
        unicharIndex[i] = UnicharIndex{};
    }

}

SkGlyphRunInfo::SkGlyphRunInfo(
    std::vector<SkGlyphRunInfo::DenseIndex>&& denseIndex,
    std::vector<DrawableGlyph>&& unique)
    : fDenseIndex{std::move(denseIndex)}, fUnique{std::move(unique)} { }