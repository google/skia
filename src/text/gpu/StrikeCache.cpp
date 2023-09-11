/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/text/gpu/StrikeCache.h"

#include "include/private/base/SkAssert.h"
#include "include/private/chromium/SkChromeRemoteGlyphCache.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkStrikeSpec.h"
#include "src/text/StrikeForGPU.h"
#include "src/text/gpu/Glyph.h"

#include <optional>
#include <utility>

class SkStrike;

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

namespace sktext {
std::optional<SkStrikePromise> SkStrikePromise::MakeFromBuffer(
        SkReadBuffer& buffer, const SkStrikeClient* client, SkStrikeCache* strikeCache) {
    std::optional<SkAutoDescriptor> descriptor = SkAutoDescriptor::MakeFromBuffer(buffer);
    if (!buffer.validate(descriptor.has_value())) {
        return std::nullopt;
    }

    // If there is a client, then this from a different process. Translate the SkTypefaceID from
    // the strike server (Renderer) process to strike client (GPU) process.
    if (client != nullptr) {
        if (!client->translateTypefaceID(&descriptor.value())) {
            return std::nullopt;
        }
    }

    sk_sp<SkStrike> strike = strikeCache->findStrike(*descriptor->getDesc());
    SkASSERT(strike != nullptr);
    if (!buffer.validate(strike != nullptr)) {
        return std::nullopt;
    }

    return SkStrikePromise{std::move(strike)};
}
}  // namespace sktext
