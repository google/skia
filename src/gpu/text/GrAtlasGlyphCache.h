/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAtlasGlyphCache_DEFINED
#define GrAtlasGlyphCache_DEFINED

#include "GrCaps.h"
#include "GrDrawOpAtlas.h"
#include "GrGlyph.h"
#include "GrOnFlushResourceProvider.h"
#include "SkArenaAlloc.h"
#include "SkGlyphCache.h"
#include "SkTDynamicHash.h"

class GrAtlasGlyphCache1;
class GrGpu;

/**
 *  The GrAtlasTextStrike manages a pool of CPU backing memory for GrGlyphs. This backing memory
 *  is indexed by a PackedID and SkGlyphCache. The SkGlyphCache is what actually creates the mask.
 *  The GrAtlasTextStrike may outlive the generating SkGlyphCache. However, it retains a copy
 *  of it's SkDescriptor as a key to access (or regenerate) the SkGlyphCache. GrAtlasTextStrike are
 *  created by and owned by a GrAtlasGlyphCache.
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
    bool addGlyphToAtlas(GrDeferredUploadTarget*, GrAtlasManager*, GrGlyph*, SkGlyphCache*,
                         GrMaskFormat expectedMaskFormat);

    // testing
    int countGlyphs() const { return fCache.count(); }

    // remove any references to this plot
    void removeID(GrDrawOpAtlas::AtlasID);

    // If a TextStrike is abandoned by the cache, then the caller must get a new strike
    bool isAbandoned1() const { return fIsAbandoned1; }

    static const SkDescriptor& GetKey(const GrAtlasTextStrike& ts) {
        return *ts.fFontScalerKey.getDesc();
    }

    static uint32_t Hash(const SkDescriptor& desc) { return desc.getChecksum(); }

private:
    SkTDynamicHash<GrGlyph, GrGlyph::PackedID> fCache;
    SkAutoDescriptor fFontScalerKey;
    SkArenaAlloc fPool1{512};

    int fAtlasedGlyphs;
    bool fIsAbandoned1;

    static const SkGlyph& GrToSkGlyph(SkGlyphCache* cache, GrGlyph::PackedID id) {
        return cache->getGlyphIDMetrics(GrGlyph::UnpackID(id),
                                        GrGlyph::UnpackFixedX(id),
                                        GrGlyph::UnpackFixedY(id));
    }

    GrGlyph* generateGlyph(const SkGlyph&, GrGlyph::PackedID, SkGlyphCache*);

    friend class GrAtlasGlyphCache1;
};

/**
 * GrAtlasGlyphCache manages strikes which are indexed by a SkGlyphCache. These strikes can then be
 * used to generate individual Glyph Masks.
 */
class GrAtlasGlyphCache1 {
public:
    GrAtlasGlyphCache1(SkScalar glyphSizeLimit);
    ~GrAtlasGlyphCache1();

    SkScalar getGlyphSizeLimit() const { return fGlyphSizeLimit; }

    void freeAll();

    // The user of the cache may hold a long-lived ref to the returned strike. However, actions by
    // another client of the cache may cause the strike to be purged while it is still reffed.
    // Therefore, the caller must check GrAtlasTextStrike::isAbandoned() if there are other
    // interactions with the cache since the strike was received.
    inline GrAtlasTextStrike* getStrike1(const SkGlyphCache* cache) {
        GrAtlasTextStrike* strike = fCache.find(cache->getDescriptor());
        if (nullptr == strike) {
            strike = this->generateStrike(cache);
        }
        return strike;
    }

    static void HandleEviction(GrDrawOpAtlas::AtlasID, void*);

private:
    GrAtlasTextStrike* generateStrike(const SkGlyphCache* cache) {
        GrAtlasTextStrike* strike = new GrAtlasTextStrike(cache->getDescriptor());
        fCache.add(strike);
        return strike;
    }

    using StrikeHash = SkTDynamicHash<GrAtlasTextStrike, SkDescriptor>;

    StrikeHash         fCache;
    GrAtlasTextStrike* fPreserveStrike;
    SkScalar           fGlyphSizeLimit;
};

 /** The GrAtlasManager classes manage the lifetime of and access to GrDrawOpAtlases.
  *  The restricted version is available at op creation time and only allows basic access
  *  to the proxies (so the created ops can reference them). The full GrAtlasManager class
  *  is only available and flush time and only via the GrOpFlushState.
  *
  *  This organization implies that all of the advanced atlasManager functionality (i.e.,
  *  adding
  */
class GrRestrictedAtlasManager : public GrOnFlushCallbackObject {
public:
    GrRestrictedAtlasManager(sk_sp<const GrCaps>, float maxTextureBytes,
                             GrDrawOpAtlas::AllowMultitexturing);
    ~GrRestrictedAtlasManager() override;

    // if getProxies returns nullptr, the client must not try to use other functions on the
    // GrAtlasGlyphCache which use the atlas.  This function *must* be called first, before other
    // functions which use the atlas.
    const sk_sp<GrTextureProxy>* getProxies(GrMaskFormat format, int* numProxies) {
        if (this->initAtlas(format)) {
            *numProxies = this->getAtlas(format)->pageCount1();
            return this->getAtlas(format)->getProxies();
        }
        *numProxies = 0;
        return nullptr;
    }

    SkScalar getGlyphSizeLimit() const { return fGlyphSizeLimit; }

protected:
    // There is a 1:1 mapping between GrMaskFormats and atlas indices
    static int MaskFormatToAtlasIndex(GrMaskFormat format) {
        static const int sAtlasIndices[] = {
            kA8_GrMaskFormat,
            kA565_GrMaskFormat,
            kARGB_GrMaskFormat,
        };
        static_assert(SK_ARRAY_COUNT(sAtlasIndices) == kMaskFormatCount, "array_size_mismatch");

        SkASSERT(sAtlasIndices[format] < kMaskFormatCount);
        return sAtlasIndices[format];
    }

    GrDrawOpAtlas* getAtlas(GrMaskFormat format) const {
        int atlasIndex = MaskFormatToAtlasIndex(format);
        SkASSERT(fAtlases[atlasIndex]);
        return fAtlases[atlasIndex].get();
    }

    sk_sp<const GrCaps> fCaps;
    GrDrawOpAtlas::AllowMultitexturing fAllowMultitexturing;
    std::unique_ptr<GrDrawOpAtlas> fAtlases[kMaskFormatCount];
    GrDrawOpAtlasConfig fAtlasConfigs[kMaskFormatCount];
    SkScalar fGlyphSizeLimit;

private:
    bool initAtlas(GrMaskFormat);

    typedef GrOnFlushCallbackObject INHERITED;
};

class GrAtlasManager : public GrRestrictedAtlasManager {
public:
    GrAtlasManager(sk_sp<const GrCaps>, float maxTextureBytes, GrDrawOpAtlas::AllowMultitexturing);

    void freeAll();

    bool hasGlyph(GrGlyph* glyph) {
        SkASSERT(glyph);
        return this->getAtlas(glyph->fMaskFormat)->hasID(glyph->fID);
    }

    // To ensure the GrDrawOpAtlas does not evict the Glyph Mask from its texture backing store,
    // the client must pass in the current op token along with the GrGlyph.
    // A BulkUseTokenUpdater is used to manage bulk last use token updating in the Atlas.
    // For convenience, this function will also set the use token for the current glyph if required
    // NOTE: the bulk uploader is only valid if the subrun has a valid atlasGeneration
    void addGlyphToBulkAndSetUseToken(GrDrawOpAtlas::BulkUseTokenUpdater* updater, GrGlyph* glyph,
                                      GrDeferredUploadToken token) {
        SkASSERT(glyph);
        updater->add(glyph->fID);
        this->getAtlas(glyph->fMaskFormat)->setLastUseToken(glyph->fID, token);
    }

    void setUseTokenBulk(const GrDrawOpAtlas::BulkUseTokenUpdater& updater,
                         GrDeferredUploadToken token,
                         GrMaskFormat format) {
        this->getAtlas(format)->setLastUseTokenBulk(updater, token);
    }

    // add to texture atlas that matches this format
    bool addToAtlas2(GrAtlasTextStrike* strike, GrDrawOpAtlas::AtlasID* id,
                    GrDeferredUploadTarget* target, GrMaskFormat format, int width, int height,
                    const void* image, SkIPoint16* loc) {
        // TODO: need to restore this!
        //fPreserveStrike = strike;
        return this->getAtlas(format)->addToAtlas1(id, target, width, height, image, loc);
    }

    // Some clients may wish to verify the integrity of the texture backing store of the
    // GrDrawOpAtlas. The atlasGeneration returned below is a monotonically increasing number which
    // changes every time something is removed from the texture backing store.
    uint64_t atlasGeneration(GrMaskFormat format) const {
        return this->getAtlas(format)->atlasGeneration();
    }

    // GrOnFlushCallbackObject overrides

    void preFlush(GrOnFlushResourceProvider* onFlushResourceProvider, const uint32_t*, int,
                  SkTArray<sk_sp<GrRenderTargetContext>>*) override {
        for (int i = 0; i < kMaskFormatCount; ++i) {
            if (fAtlases[i]) {
                fAtlases[i]->instantiate(onFlushResourceProvider);
            }
        }
    }

    void postFlush(GrDeferredUploadToken startTokenForNextFlush,
                   const uint32_t* opListIDs, int numOpListIDs) override {
        for (int i = 0; i < kMaskFormatCount; ++i) {
            if (fAtlases[i]) {
                fAtlases[i]->compact(startTokenForNextFlush);
            }
        }
    }

    // The AtlasGlyph cache always survives freeGpuResources so we want it to remain in the active
    // OnFlushCallbackObject list
    bool retainOnFreeGpuResources() override { return true; }

    ///////////////////////////////////////////////////////////////////////////
    // Functions intended debug only
#ifdef SK_DEBUG
    void dump(GrContext* context) const;
#endif

    void setAtlasSizes_ForTesting(const GrDrawOpAtlasConfig configs[3]);

private:
    typedef GrRestrictedAtlasManager INHERITED;
};

#endif
