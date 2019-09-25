/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#include "src/core/SkStrikeForGPU.h"

#include "src/core/SkGlyphRunPainter.h"

bool SkStrikeForGPU::CanDrawAsMask(const SkGlyph& glyph) {
    return glyph.maxDimension() <= SkStrikeCommon::kSkSideTooBigForAtlas;
}

bool SkStrikeForGPU::CanDrawAsSDFT(const SkGlyph& glyph) {
    return glyph.maxDimension() <= SkStrikeCommon::kSkSideTooBigForAtlas
           && glyph.maskFormat() == SkMask::kSDF_Format;
}

bool SkStrikeForGPU::CanDrawAsPath(const SkGlyph& glyph) {
    SkASSERT(glyph.isColor() || glyph.setPathHasBeenCalled());
    return !glyph.isColor() && glyph.path() != nullptr;
}

