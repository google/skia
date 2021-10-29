/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkArenaAlloc.h"
#include "src/core/SkStrikeSpec.h"
#include "src/gpu/GrGlyph.h"
#include "src/gpu/text/GrStrikeCache.h"

GrStrikeCache::~GrStrikeCache() {
    this->freeAll();
}

void GrStrikeCache::freeAll() {
    fCache.reset();
}

sk_sp<GrTextStrike> GrStrikeCache::findOrCreateStrike(const SkStrikeSpec& strikeSpec) {
    if (sk_sp<GrTextStrike>* cached = fCache.find(strikeSpec.descriptor())) {
        return *cached;
    }
    return this->generateStrike(strikeSpec);
}

sk_sp<GrTextStrike> GrStrikeCache::generateStrike(const SkStrikeSpec& strikeSpec) {
    sk_sp<GrTextStrike> strike = sk_make_sp<GrTextStrike>(strikeSpec);
    fCache.set(strike);
    return strike;
}

const SkDescriptor& GrStrikeCache::HashTraits::GetKey(const sk_sp<GrTextStrike>& strike) {
    return strike->fStrikeSpec.descriptor();
}

uint32_t GrStrikeCache::HashTraits::Hash(const SkDescriptor& descriptor) {
    return descriptor.getChecksum();
}

GrTextStrike::GrTextStrike(const SkStrikeSpec& strikeSpec) : fStrikeSpec{strikeSpec} {}

GrGlyph* GrTextStrike::getGlyph(SkPackedGlyphID packedGlyphID) {
    GrGlyph* grGlyph = fCache.findOrNull(packedGlyphID);
    if (grGlyph == nullptr) {
        grGlyph = fAlloc.make<GrGlyph>(packedGlyphID);
        fCache.set(grGlyph);
    }
    return grGlyph;
}

const SkPackedGlyphID& GrTextStrike::HashTraits::GetKey(const GrGlyph* glyph) {
    return glyph->fPackedID;
}

uint32_t GrTextStrike::HashTraits::Hash(SkPackedGlyphID key) {
    return key.hash();
}

