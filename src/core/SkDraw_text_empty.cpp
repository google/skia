/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDraw.h"

bool SkDraw::ShouldDrawTextAsPaths(const SkPaint& paint, const SkMatrix& ctm, SkScalar sizeLimit) {
    SkASSERT_RELEASE(false);
    return false;
}
void SkDraw::blitARGB32Mask(const SkMask& mask, const SkPaint& paint) const {
    SkASSERT_RELEASE(false);
}

SkGlyphRunListPainter::PerMask SkDraw::drawOneMaskCreator(
        const SkPaint& paint, SkArenaAlloc* alloc) const {
    SkASSERT_RELEASE(false);
    return [](const SkMask&, const SkGlyph&, SkPoint){};
}

void SkDraw::drawGlyphRunList(
        const SkGlyphRunList& glyphRunList, SkGlyphRunListPainter* glyphPainter) const {
    SkASSERT_RELEASE(false);
}
