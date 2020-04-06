/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStrikeCache_DEFINED
#define GrStrikeCache_DEFINED

#include "include/private/SkTHash.h"
#include "src/codec/SkMasks.h"
#include "src/core/SkStrikeSpec.h"
#include "src/core/SkTDynamicHash.h"
#include "src/gpu/GrDrawOpAtlas.h"
#include "src/gpu/GrGlyph.h"

class GrAtlasManager;
class GrGpu;
class GrStrikeCache;
class SkBulkGlyphMetricsAndImages;

/**
 *  The GrTextStrike manages a pool of CPU backing memory for GrGlyphs. This backing memory
 *  is indexed by a PackedID and SkStrike. The SkStrike is what actually creates the mask.
 *  The GrTextStrike may outlive the generating SkStrike. However, it retains a copy
 *  of it's SkDescriptor as a key to access (or regenerate) the SkStrike. GrTextStrikes are
 *  created by and owned by a GrStrikeCache.
 */
class GrTextStrike : public SkNVRefCnt<GrTextStrike> {
public:
    GrTextStrike(const SkStrikeSpec&);

    GrGlyph* getGlyph(const SkGlyph& skGlyph);

    // returns true if glyph successfully added to texture atlas, false otherwise.  If the glyph's
    // mask format has changed, then addGlyphToAtlas will draw a clear box.  This will almost never
    // happen.
    // TODO we can handle some of these cases if we really want to, but the long term solution is to
    // get the actual glyph image itself when we get the glyph metrics.
    GrDrawOpAtlas::ErrorCode addGlyphToAtlas(const SkGlyph&,
                                             GrMaskFormat expectedMaskFormat,
                                             bool needsPadding,
                                             GrResourceProvider*,
                                             GrDeferredUploadTarget*,
                                             GrAtlasManager*,
                                             GrGlyph*);

    const SkStrikeSpec& strikeSpec() const { return fStrikeSpec; }

    SkScalar strikeToSourceRatio() const { return fStrikeSpec.strikeToSourceRatio(); }

private:
    struct HashTraits {
        // GetKey and Hash for the the hash table.
        static const SkPackedGlyphID& GetKey(const GrGlyph* glyph) {
            return glyph->fPackedID;
        }

        static uint32_t Hash(SkPackedGlyphID key) {
            return SkChecksum::Mix(key.hash());
        }
    };
    SkTHashTable<GrGlyph*, SkPackedGlyphID, HashTraits> fCache;
    SkStrikeSpec fStrikeSpec;
//    SkAutoDescriptor fFontScalerKey;
    SkArenaAlloc fAlloc{512};

    friend class GrStrikeCache;
};

/**
 * GrStrikeCache manages strikes which are indexed by a SkStrike. These strikes can then be
 * used to generate individual Glyph Masks.
 */
class GrStrikeCache {
public:
    ~GrStrikeCache();

    // The user of the cache may hold a long-lived ref to the returned strike. However, actions by
    // another client of the cache may cause the strike to be purged while it is still reffed.
    // Therefore, the caller must check GrTextStrike::isAbandoned() if there are other
    // interactions with the cache since the strike was received.
    sk_sp<GrTextStrike> findOrCreateGrStrike(const SkStrikeSpec& strikeSpec) {
        if (sk_sp<GrTextStrike>* cached = fCache.find(strikeSpec.descriptor())) {
            return *cached;
        }
        return this->generateStrike(strikeSpec);
    }

    void freeAll();

private:
    sk_sp<GrTextStrike> generateStrike(const SkStrikeSpec& strikeSpec) {
        sk_sp<GrTextStrike> strike = sk_make_sp<GrTextStrike>(strikeSpec);
        fCache.set(strike);
        return strike;
    }

    struct DescriptorHashTraits {
        static const SkDescriptor& GetKey(const sk_sp<GrTextStrike>& strike) {
            return strike->fStrikeSpec.descriptor();
        }
        static uint32_t Hash(const SkDescriptor& desc) { return desc.getChecksum(); }
    };

    using StrikeHash = SkTHashTable<sk_sp<GrTextStrike>, SkDescriptor, DescriptorHashTraits>;

    StrikeHash fCache;
};

#endif  // GrStrikeCache_DEFINED
