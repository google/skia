/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAtlasGlyphCache_DEFINED
#define GrAtlasGlyphCache_DEFINED

#include "GrDrawOpAtlas.h"
#include "GrGlyph.h"
#include "SkArenaAlloc.h"
#include "SkGlyphCache.h"
#include "SkTDynamicHash.h"

class GrGlyphCache;
class GrAtlasManager;
class GrGpu;

/**
 *  The GrAtlasTextStrike manages a pool of CPU backing memory for GrGlyphs. This backing memory
 *  is indexed by a PackedID and SkGlyphCache. The SkGlyphCache is what actually creates the mask.
 *  The GrAtlasTextStrike may outlive the generating SkGlyphCache. However, it retains a copy
 *  of it's SkDescriptor as a key to access (or regenerate) the SkGlyphCache. GrAtlasTextStrike are
 *  created by and owned by a GrGlyphCache.
 */
class GrAtlasTextStrike : public SkNVRefCnt<GrAtlasTextStrike> {
public:
    GrAtlasTextStrike(const SkDescriptor& fontScalerKey);
    ~GrAtlasTextStrike();

    inline GrGlyph* getGlyph(const SkGlyph& skGlyph, GrGlyph::PackedID packed,
                             SkGlyphCache* cache) {
        GrGlyph* glyph = fCache.find(packed);
        if (nullptr == glyph) {
            glyph = this->generateGlyph(skGlyph, packed, cache);
        }
        return glyph;
    }

    // This variant of the above function is called by GrAtlasTextOp. At this point, it is possible
    // that the maskformat of the glyph differs from what we expect.  In these cases we will just
    // draw a clear square.
    // skbug:4143 crbug:510931
    inline GrGlyph* getGlyph(GrGlyph::PackedID packed,
                             GrMaskFormat expectedMaskFormat,
                             SkGlyphCache* cache) {
        GrGlyph* glyph = fCache.find(packed);
        if (nullptr == glyph) {
            // We could return this to the caller, but in practice it adds code complexity for
            // potentially little benefit(ie, if the glyph is not in our font cache, then its not
            // in the atlas and we're going to be doing a texture upload anyways).
            const SkGlyph& skGlyph = GrToSkGlyph(cache, packed);
            glyph = this->generateGlyph(skGlyph, packed, cache);
            glyph->fMaskFormat = expectedMaskFormat;
        }
        return glyph;
    }

    // returns true if glyph successfully added to texture atlas, false otherwise.  If the glyph's
    // mask format has changed, then addGlyphToAtlas will draw a clear box.  This will almost never
    // happen.
    // TODO we can handle some of these cases if we really want to, but the long term solution is to
    // get the actual glyph image itself when we get the glyph metrics.
    bool addGlyphToAtlas(GrResourceProvider*, GrDeferredUploadTarget*, GrGlyphCache*,
                         GrAtlasManager*, GrGlyph*,
                         SkGlyphCache*, GrMaskFormat expectedMaskFormat);

    // testing
    int countGlyphs() const { return fCache.count(); }

    // remove any references to this plot
    void removeID(GrDrawOpAtlas::AtlasID);

    // If a TextStrike is abandoned by the cache, then the caller must get a new strike
    bool isAbandoned() const { return fIsAbandoned; }

    static const SkDescriptor& GetKey(const GrAtlasTextStrike& ts) {
        return *ts.fFontScalerKey.getDesc();
    }

    static uint32_t Hash(const SkDescriptor& desc) { return desc.getChecksum(); }

private:
    SkTDynamicHash<GrGlyph, GrGlyph::PackedID> fCache;
    SkAutoDescriptor fFontScalerKey;
    SkArenaAlloc fPool{512};

    int fAtlasedGlyphs;
    bool fIsAbandoned;

    static const SkGlyph& GrToSkGlyph(SkGlyphCache* cache, GrGlyph::PackedID id) {
        return cache->getGlyphIDMetrics(GrGlyph::UnpackID(id),
                                        GrGlyph::UnpackFixedX(id),
                                        GrGlyph::UnpackFixedY(id));
    }

    GrGlyph* generateGlyph(const SkGlyph&, GrGlyph::PackedID, SkGlyphCache*);

    friend class GrGlyphCache;
};

/**
 * GrGlyphCache manages strikes which are indexed by a SkGlyphCache. These strikes can then be
 * used to generate individual Glyph Masks.
 */
class GrGlyphCache {
public:
    GrGlyphCache();
    ~GrGlyphCache();

    void setGlyphSizeLimit(SkScalar sizeLimit) { fGlyphSizeLimit = sizeLimit; }
    SkScalar getGlyphSizeLimit() const { return fGlyphSizeLimit; }

    void setStrikeToPreserve(GrAtlasTextStrike* strike) { fPreserveStrike = strike; }

    // The user of the cache may hold a long-lived ref to the returned strike. However, actions by
    // another client of the cache may cause the strike to be purged while it is still reffed.
    // Therefore, the caller must check GrAtlasTextStrike::isAbandoned() if there are other
    // interactions with the cache since the strike was received.
    inline GrAtlasTextStrike* getStrike(const SkGlyphCache* cache) {
        GrAtlasTextStrike* strike = fCache.find(cache->getDescriptor());
        if (nullptr == strike) {
            strike = this->generateStrike(cache);
        }
        return strike;
    }

    void freeAll();

    static void HandleEviction(GrDrawOpAtlas::AtlasID, void*);

private:
    GrAtlasTextStrike* generateStrike(const SkGlyphCache* cache) {
        GrAtlasTextStrike* strike = new GrAtlasTextStrike(cache->getDescriptor());
        fCache.add(strike);
        return strike;
    }

    using StrikeHash = SkTDynamicHash<GrAtlasTextStrike, SkDescriptor>;

    StrikeHash fCache;
    GrAtlasTextStrike* fPreserveStrike;
    SkScalar fGlyphSizeLimit;
};

#endif
