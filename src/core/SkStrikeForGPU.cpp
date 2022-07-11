/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkStrikeForGPU.h"

#include "src/core/SkGlyphRunPainter.h"

namespace sktext::gpu {
bool StrikeForGPU::CanDrawAsMask(const SkGlyph& glyph) {
    return FitsInAtlas(glyph);
}

bool StrikeForGPU::CanDrawAsSDFT(const SkGlyph& glyph) {
    return FitsInAtlas(glyph) && glyph.maskFormat() == SkMask::kSDF_Format;
}

bool StrikeForGPU::CanDrawAsPath(const SkGlyph& glyph) {
    SkASSERT(glyph.setPathHasBeenCalled());
    return glyph.path() != nullptr;
}

bool StrikeForGPU::FitsInAtlas(const SkGlyph& glyph) {
    return glyph.maxDimension() <= SkStrikeCommon::kSkSideTooBigForAtlas;
}
}  // namespace sktext::gpu
