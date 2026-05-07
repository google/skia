/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStrikeRef.h"

#include "include/core/SkRect.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkZip.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkStrike.h"

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
