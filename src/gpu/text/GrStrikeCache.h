/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStrikeCache_DEFINED
#define GrStrikeCache_DEFINED

#include "include/private/SkTHash.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkStrikeSpec.h"

class GrGlyph;
class GrStrikeCache;


// The GrTextStrike manages an SkArenaAlloc for GrGlyphs. The SkStrike is what actually creates
// the mask. The GrTextStrike may outlive the generating SkStrike. However, it retains a copy
// of it's SkDescriptor as a key to access (or regenerate) the SkStrike. GrTextStrikes are
// created by and owned by a GrStrikeCache.
class GrTextStrike : public SkNVRefCnt<GrTextStrike> {
public:
    GrTextStrike(const SkStrikeSpec& strikeSpec);

    GrGlyph* getGlyph(SkPackedGlyphID);
    const SkStrikeSpec& strikeSpec() const { return fStrikeSpec; }

private:
    // Key for retrieving the SkStrike for creating new atlas data.
    const SkStrikeSpec fStrikeSpec;

    struct HashTraits {
        static const SkPackedGlyphID& GetKey(const GrGlyph* glyph);
        static uint32_t Hash(SkPackedGlyphID key);
    };
    // Map SkPackedGlyphID -> GrGlyph*.
    SkTHashTable<GrGlyph*, SkPackedGlyphID, HashTraits> fCache;

    // Store for the glyph information.
    SkArenaAlloc fAlloc{512};

    friend class GrStrikeCache;
};

// GrStrikeCache manages strikes which are indexed by a SkStrike. These strikes can then be
// used to generate individual Glyph Masks.
class GrStrikeCache {
public:
    ~GrStrikeCache();

    // The user of the cache may hold a long-lived ref to the returned strike.
    sk_sp<GrTextStrike> findOrCreateStrike(const SkStrikeSpec& strikeSpec);

    void freeAll();

private:
    sk_sp<GrTextStrike> generateStrike(const SkStrikeSpec& strikeSpec);

    struct HashTraits {
        static const SkDescriptor& GetKey(const sk_sp<GrTextStrike>& strike);
        static uint32_t Hash(const SkDescriptor& strikeSpec);
    };

    using StrikeHash = SkTHashTable<sk_sp<GrTextStrike>, const SkDescriptor&, HashTraits>;

    StrikeHash fCache;
};

#endif  // GrStrikeCache_DEFINED
