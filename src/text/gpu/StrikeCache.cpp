/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/base/SkArenaAlloc.h"
#include "src/core/SkStrikeSpec.h"
#include "src/text/gpu/Glyph.h"
#include "src/text/gpu/StrikeCache.h"

namespace sktext::gpu {

StrikeCache::~StrikeCache() {
    this->freeAll();
}

void StrikeCache::freeAll() {
    fCache.reset();
}

sk_sp<TextStrike> StrikeCache::findOrCreateStrike(const SkStrikeSpec& strikeSpec) {
    if (sk_sp<TextStrike>* cached = fCache.find(strikeSpec.descriptor())) {
        return *cached;
    }
    return this->generateStrike(strikeSpec);
}

sk_sp<TextStrike> StrikeCache::generateStrike(const SkStrikeSpec& strikeSpec) {
    sk_sp<TextStrike> strike = sk_make_sp<TextStrike>(strikeSpec);
    fCache.set(strike);
    return strike;
}

const SkDescriptor& StrikeCache::HashTraits::GetKey(const sk_sp<TextStrike>& strike) {
    return strike->fStrikeSpec.descriptor();
}

uint32_t StrikeCache::HashTraits::Hash(const SkDescriptor& descriptor) {
    return descriptor.getChecksum();
}

TextStrike::TextStrike(const SkStrikeSpec& strikeSpec) : fStrikeSpec{strikeSpec} {}

Glyph* TextStrike::getGlyph(SkPackedGlyphID packedGlyphID) {
    Glyph* glyph = fCache.findOrNull(packedGlyphID);
    if (glyph == nullptr) {
        glyph = fAlloc.make<Glyph>(packedGlyphID);
        fCache.set(glyph);
    }
    return glyph;
}

const SkPackedGlyphID& TextStrike::HashTraits::GetKey(const Glyph* glyph) {
    return glyph->fPackedID;
}

uint32_t TextStrike::HashTraits::Hash(SkPackedGlyphID key) {
    return key.hash();
}

}  // namespace sktext::gpu
