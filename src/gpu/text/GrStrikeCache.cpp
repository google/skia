/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkArenaAlloc.h"
#include "src/core/SkDistanceFieldGen.h"
#include "src/core/SkStrikeSpec.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrColor.h"
#include "src/gpu/GrDistanceFieldGenFromVector.h"
#include "src/gpu/text/GrAtlasManager.h"
#include "src/gpu/text/GrStrikeCache.h"

GrStrikeCache::~GrStrikeCache() {
    this->freeAll();
}

void GrStrikeCache::freeAll() {
    fCache.reset();
}

///////////////////////////////////////////////////////////////////////////////

/*
    The text strike is specific to a given font/style/matrix setup, which is
    represented by the GrHostFontScaler object we are given in getGlyph().

    We map a 32bit glyphID to a GrGlyph record, which in turn points to a
    atlas and a position within that texture.
 */

GrTextStrike::GrTextStrike(const SkDescriptor& key) : fFontScalerKey(key) {}

GrGlyph* GrTextStrike::getGlyph(SkPackedGlyphID packedGlyphID) {
    GrGlyph* grGlyph = fCache.findOrNull(packedGlyphID);
    if (grGlyph == nullptr) {
        grGlyph = fAlloc.make<GrGlyph>(packedGlyphID);
        fCache.set(grGlyph);
    }
    return grGlyph;
}

