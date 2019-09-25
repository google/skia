/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#include "src/core/SkStrikeInterface.h"

#include "src/core/SkGlyphRunPainter.h"

bool SkStrikeInterface::CanDrawAsMask(const SkGlyph& glyph) {
    return glyph.maxDimension() <= SkStrikeCommon::kSkSideTooBigForAtlas;
}

bool SkStrikeInterface::CanDrawAsSDFT(const SkGlyph& glyph) {
    return glyph.maxDimension() <= SkStrikeCommon::kSkSideTooBigForAtlas
           && ~glyph.isColor();
}

bool SkStrikeInterface::CanDrawAsMaskPath(const SkGlyph& glyph) {
    SkASSERT(glyph.setPathHasBeenCalled());
    return !glyph.isColor() && glyph.path() != nullptr;
}

