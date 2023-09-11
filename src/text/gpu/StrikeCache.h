/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sktext_gpu_StrikeCache_DEFINED
#define sktext_gpu_StrikeCache_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkDescriptor.h"
#include "src/core/SkStrikeSpec.h"
#include "src/core/SkTHash.h"

#include <cstdint>

struct SkPackedGlyphID;

namespace sktext::gpu {

class Glyph;

// The TextStrike manages an SkArenaAlloc for Glyphs. The SkStrike is what actually creates
// the mask. The TextStrike may outlive the generating SkStrike. However, it retains a copy
// of it's SkDescriptor as a key to access (or regenerate) the SkStrike. TextStrikes are
// created by and owned by a StrikeCache.
class TextStrike : public SkNVRefCnt<TextStrike> {
public:
    TextStrike(const SkStrikeSpec& strikeSpec);

    Glyph* getGlyph(SkPackedGlyphID);
    const SkStrikeSpec& strikeSpec() const { return fStrikeSpec; }

private:
    // Key for retrieving the SkStrike for creating new atlas data.
    const SkStrikeSpec fStrikeSpec;

    struct HashTraits {
        static const SkPackedGlyphID& GetKey(const Glyph* glyph);
        static uint32_t Hash(SkPackedGlyphID key);
    };
    // Map SkPackedGlyphID -> Glyph*.
    skia_private::THashTable<Glyph*, SkPackedGlyphID, HashTraits> fCache;

    // Store for the glyph information.
    SkArenaAlloc fAlloc{512};

    friend class StrikeCache;
};

// StrikeCache manages strikes which are indexed by a SkStrike. These strikes can then be
// used to generate individual Glyph Masks.
class StrikeCache {
public:
    ~StrikeCache();

    // The user of the cache may hold a long-lived ref to the returned strike.
    sk_sp<TextStrike> findOrCreateStrike(const SkStrikeSpec& strikeSpec);

    void freeAll();

private:
    sk_sp<TextStrike> generateStrike(const SkStrikeSpec& strikeSpec);

    struct HashTraits {
        static const SkDescriptor& GetKey(const sk_sp<TextStrike>& strike);
        static uint32_t Hash(const SkDescriptor& strikeSpec);
    };

    using StrikeHash = skia_private::THashTable<sk_sp<TextStrike>, const SkDescriptor&, HashTraits>;

    StrikeHash fCache;
};

}  // namespace sktext::gpu

#endif  // sktext_gpu_StrikeCache_DEFINED
