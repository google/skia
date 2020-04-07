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

    uint32_t uniqueID() const { return fUniqueID; }

private:
    friend class GrStrikeCache;

    GrTextStrike(const SkDescriptor& fontScalerKey);

    void setUniqueID(uint32_t uniqueID) { fUniqueID = uniqueID; }

    struct HashTraits {
        // GetKey and Hash for the the hash table.
        static const SkPackedGlyphID& GetKey(const GrGlyph* glyph) {
            return glyph->fPackedID;
        }

        static uint32_t Hash(SkPackedGlyphID key) {
            return SkChecksum::Mix(key.hash());
        }
    };

    uint32_t fUniqueID = SK_InvalidUniqueID;
    SkTHashTable<GrGlyph*, SkPackedGlyphID, HashTraits> fCache;
    SkAutoDescriptor fFontScalerKey;
    SkArenaAlloc fAlloc{512};
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
    sk_sp<GrTextStrike> findOrCreateStrike(const SkDescriptor& desc) {
        if (sk_sp<GrTextStrike>* cached = fCache.find(desc)) {
            return *cached;
        }
        return this->generateStrike(desc);
    }

    void freeAll();

    // This method unifies any GrTextStrikes created by a given DDL with those created by other
    // DDLs along with any GrTextStrikes created by the direct context.
    // Note that DDLs are only valid for the GrContext they are destined to be replayed in. Thus,
    // we don't have to worry about unique text strike IDs from another GrContext causing problems.
    // That is, we can copy the id from the direct context's GrTextStrike to the copy in the DDL
    // and leave it there forever.
    // The only quirk with this system is wrt GrContext::freeGpuResources. In that case 'freeAll'
    // is called - clearing out the strike cache. Any ids previously generated are still fine.
    // Aliasing could occur though: if one copy of a strike gets its id, gpu resources are freed,
    // then a second copy of the strike gets resolved - the two copies of the strike will get
    // different ids.
    void resolveUniqueID(GrTextStrike* strike) {
        SkASSERT(fOwnedByDirectContext);

        if (strike->fUniqueID != SK_InvalidUniqueID) {
            return;
        }

        sk_sp<GrTextStrike> tmp = this->findOrCreateStrike(*strike->fFontScalerKey.getDesc());
        strike->setUniqueID(tmp->uniqueID());
    }

private:
    friend class GrRecordingContext;

    GrStrikeCache(bool ownedByDirectContext) : fOwnedByDirectContext(ownedByDirectContext) {}

    sk_sp<GrTextStrike> generateStrike(const SkDescriptor& desc);

    struct DescriptorHashTraits {
        static const SkDescriptor& GetKey(const sk_sp<GrTextStrike>& strike) {
            return *strike->fFontScalerKey.getDesc();
        }
        static uint32_t Hash(const SkDescriptor& desc) { return desc.getChecksum(); }
    };

    using StrikeHash = SkTHashTable<sk_sp<GrTextStrike>, SkDescriptor, DescriptorHashTraits>;

    StrikeHash fCache;
    bool fOwnedByDirectContext;
};

#endif  // GrStrikeCache_DEFINED
