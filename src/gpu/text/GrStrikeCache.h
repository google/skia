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
#include "src/core/SkDescriptor.h"
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
    GrTextStrike(const SkDescriptor& fontScalerKey);

    GrGlyph* getGlyph(const SkGlyph& skGlyph);

    // This variant of the above function is called by GrAtlasTextOp. At this point, it is possible
    // that the maskformat of the glyph differs from what we expect.  In these cases we will just
    // draw a clear square.
    // skbug:4143 crbug:510931
    GrGlyph* getGlyph(SkPackedGlyphID packed, SkBulkGlyphMetricsAndImages* metricsAndImages);

    // returns true if glyph successfully added to texture atlas, false otherwise.  If the glyph's
    // mask format has changed, then addGlyphToAtlas will draw a clear box.  This will almost never
    // happen.
    // TODO we can handle some of these cases if we really want to, but the long term solution is to
    // get the actual glyph image itself when we get the glyph metrics.
    GrDrawOpAtlas::ErrorCode addGlyphToAtlas(GrResourceProvider*, GrDeferredUploadTarget*,
                                             GrStrikeCache*, GrAtlasManager*, GrGlyph*,
                                             SkBulkGlyphMetricsAndImages*,
                                             GrMaskFormat expectedMaskFormat,
                                             bool isScaledGlyph);

    // testing
    int countGlyphs() const { return fCache.count(); }

    // remove any references to this plot
    void removeID(GrDrawOpAtlas::AtlasID);

    // If a TextStrike is abandoned by the cache, then the caller must get a new strike
    bool isAbandoned() const { return fIsAbandoned; }

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
    SkAutoDescriptor fFontScalerKey;
    SkArenaAlloc fAlloc{512};

    int fAtlasedGlyphs{0};
    bool fIsAbandoned{false};

    friend class GrStrikeCache;
};

/**
 * GrStrikeCache manages strikes which are indexed by a SkStrike. These strikes can then be
 * used to generate individual Glyph Masks.
 */
class GrStrikeCache {
public:
    GrStrikeCache(const GrCaps* caps, size_t maxTextureBytes);
    ~GrStrikeCache();

    void setStrikeToPreserve(GrTextStrike* strike) { fPreserveStrike = strike; }

    // The user of the cache may hold a long-lived ref to the returned strike. However, actions by
    // another client of the cache may cause the strike to be purged while it is still reffed.
    // Therefore, the caller must check GrTextStrike::isAbandoned() if there are other
    // interactions with the cache since the strike was received.
    sk_sp<GrTextStrike> getStrike(const SkDescriptor& desc) {
        if (sk_sp<GrTextStrike>* cached = fCache.find(desc)) {
            return *cached;
        }
        return this->generateStrike(desc);
    }

    const SkMasks& getMasks() const { return *f565Masks; }

    void freeAll();

    static void HandleEviction(GrDrawOpAtlas::AtlasID, void*);

private:
    sk_sp<GrTextStrike> generateStrike(const SkDescriptor& desc) {
        sk_sp<GrTextStrike> strike = sk_make_sp<GrTextStrike>(desc);
        fCache.set(strike);
        return strike;
    }

    struct DescriptorHashTraits {
        static const SkDescriptor& GetKey(const sk_sp<GrTextStrike>& strike) {
            return *strike->fFontScalerKey.getDesc();
        }
        static uint32_t Hash(const SkDescriptor& desc) { return desc.getChecksum(); }
    };

    using StrikeHash = SkTHashTable<sk_sp<GrTextStrike>, SkDescriptor, DescriptorHashTraits>;

    StrikeHash fCache;
    GrTextStrike* fPreserveStrike;
    std::unique_ptr<const SkMasks> f565Masks;
};

#endif  // GrStrikeCache_DEFINED
