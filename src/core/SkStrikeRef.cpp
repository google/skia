/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStrikeRef.h"

#include "include/core/SkRect.h"
#include "include/core/SkSpan.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkStrike.h"
#include "src/core/SkZip.h"

#include <algorithm>

using namespace skia_private;

namespace {

constexpr SkRect scale_rect(SkRect r, SkScalar s) {
    return {r.fLeft * s, r.fTop * s, r.fRight * s, r.fBottom * s};
}

}  // namespace

SkStrikeRef::SkStrikeRef() = default;
SkStrikeRef::~SkStrikeRef() = default;
SkStrikeRef::SkStrikeRef(const SkStrikeRef&) = default;
SkStrikeRef& SkStrikeRef::operator=(const SkStrikeRef&) = default;
SkStrikeRef::SkStrikeRef(SkStrikeRef&&) = default;
SkStrikeRef& SkStrikeRef::operator=(SkStrikeRef&&) = default;

SkStrikeRef::SkStrikeRef(sk_sp<SkStrike> strike,
                         SkScalar strikeToSourceScale)
    : fStrike(std::move(strike))
    , fStrikeToSourceScale(strikeToSourceScale) {}

void SkStrikeRef::getWidths(SkSpan<const SkGlyphID> glyphs, SkSpan<SkScalar> widths) const {
    this->getWidthsBounds(glyphs, widths, {});
}

SkScalar SkStrikeRef::getWidth(SkGlyphID glyph) const {
    SkScalar width;
    this->getWidths({&glyph, 1}, {&width, 1});
    return width;
}

void SkStrikeRef::getWidthsBounds(SkSpan<const SkGlyphID> glyphs,
                                  SkSpan<SkScalar> widths,
                                  SkSpan<SkRect> bounds) const {
    SkASSERT(fStrike);

    AutoSTArray<64, const SkGlyph*> glyphPtrs(SkTo<int>(glyphs.size()));
    SkSpan<const SkGlyph*> results = fStrike->metrics(glyphs, glyphPtrs.get());

    if (bounds.size()) {
        const auto n = std::min(bounds.size(), results.size());
        for (auto [bound, glyph] : SkMakeZip(bounds.first(n), results.first(n))) {
            bound = scale_rect(glyph->rect(), fStrikeToSourceScale);
        }
    }

    if (widths.size()) {
        const auto n = std::min(widths.size(), results.size());
        for (auto [width, glyph] : SkMakeZip(widths.first(n), results.first(n))) {
            width = glyph->advanceX() * fStrikeToSourceScale;
        }
    }
}

void SkStrikeRef::getWidthsStrided(unsigned count,
                                   SkSpan<const uint32_t> glyphs,
                                   unsigned glyph_stride_32,
                                   SkSpan<SkScalar> advances,
                                   unsigned advance_stride_32) const {
    SkASSERT(fStrike);
    fStrike->getWidthsStrided(count, glyphs, glyph_stride_32, advances,
                              advance_stride_32, fStrikeToSourceScale);
}

void SkStrikeRef::getWidthsStrided(unsigned count,
                                   const uint32_t* first_glyph,
                                   unsigned glyph_stride_32,
                                   SkScalar* first_advance,
                                   unsigned advance_stride_32) const {
    if (count == 0) {
        return;
    }
    const size_t glyphCount = std::max<size_t>(1, size_t(count) * glyph_stride_32);
    const size_t advanceCount = std::max<size_t>(1, size_t(count) * advance_stride_32);
    // TODO(crbug.com/582862856): Remove once callers migrate to the SkSpan overload.
    this->getWidthsStrided(count,
                           SK_UNSAFE_TODO(SkSpan(first_glyph, glyphCount)),
                           glyph_stride_32,
                           SK_UNSAFE_TODO(SkSpan(first_advance, advanceCount)),
                           advance_stride_32);
}
