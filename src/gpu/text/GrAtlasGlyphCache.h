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
#include "SkGlyphCache.h"
#include "SkTDynamicHash.h"
#include "SkVarAlloc.h"

class GrAtlasGlyphCache;
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
    /** Owner is the cache that owns this strike. */
    GrAtlasTextStrike(GrAtlasGlyphCache* owner, const SkDescriptor& fontScalerKey);
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
    bool addGlyphToAtlas(GrDrawOp::Target*, GrGlyph*, SkGlyphCache*,
                         GrMaskFormat expectedMaskFormat);

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
    SkVarAlloc fPool;

    GrAtlasGlyphCache* fAtlasGlyphCache;
    int fAtlasedGlyphs;
    bool fIsAbandoned;

    static const SkGlyph& GrToSkGlyph(SkGlyphCache* cache, GrGlyph::PackedID id) {
        return cache->getGlyphIDMetrics(GrGlyph::UnpackID(id),
                                        GrGlyph::UnpackFixedX(id),
                                        GrGlyph::UnpackFixedY(id));
    }

    GrGlyph* generateGlyph(const SkGlyph&, GrGlyph::PackedID, SkGlyphCache*);

    friend class GrAtlasGlyphCache;
};

/**
 * GrAtlasGlyphCache manages strikes which are indexed by a SkGlyphCache. These strikes can then be
 * used to generate individual Glyph Masks. The GrAtlasGlyphCache also manages GrDrawOpAtlases,
 * though this is more or less transparent to the client(aside from atlasGeneration, described
 * below).
 */
class GrAtlasGlyphCache {
public:
    GrAtlasGlyphCache(GrContext*);
    ~GrAtlasGlyphCache();
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

    // if getProxy returns nullptr, the client must not try to use other functions on the
    // GrAtlasGlyphCache which use the atlas.  This function *must* be called first, before other
    // functions which use the atlas.
    sk_sp<GrTextureProxy> getProxy(GrMaskFormat format) {
        if (this->initAtlas(format)) {
            return this->getAtlas(format)->getProxy();
        }
        return nullptr;
    }

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
                                      GrDrawOpUploadToken token) {
        SkASSERT(glyph);
        updater->add(glyph->fID);
        this->getAtlas(glyph->fMaskFormat)->setLastUseToken(glyph->fID, token);
    }

    void setUseTokenBulk(const GrDrawOpAtlas::BulkUseTokenUpdater& updater,
                         GrDrawOpUploadToken token,
                         GrMaskFormat format) {
        this->getAtlas(format)->setLastUseTokenBulk(updater, token);
    }

    // add to texture atlas that matches this format
    bool addToAtlas(GrAtlasTextStrike* strike, GrDrawOpAtlas::AtlasID* id, GrDrawOp::Target* target,
                    GrMaskFormat format, int width, int height, const void* image,
                    SkIPoint16* loc) {
        fPreserveStrike = strike;
        return this->getAtlas(format)->addToAtlas(id, target, width, height, image, loc);
    }

    // Some clients may wish to verify the integrity of the texture backing store of the
    // GrDrawOpAtlas. The atlasGeneration returned below is a monotonically increasing number which
    // changes every time something is removed from the texture backing store.
    uint64_t atlasGeneration(GrMaskFormat format) const {
        return this->getAtlas(format)->atlasGeneration();
    }

    int log2Width(GrMaskFormat format) { return fAtlasConfigs[format].fLog2Width; }
    int log2Height(GrMaskFormat format) { return fAtlasConfigs[format].fLog2Height; }

    ///////////////////////////////////////////////////////////////////////////
    // Functions intended debug only
#ifdef SK_DEBUG
    void dump() const;
#endif

    void setAtlasSizes_ForTesting(const GrDrawOpAtlasConfig configs[3]);

    GrContext* context() const { return fContext; }

private:
    static GrPixelConfig MaskFormatToPixelConfig(GrMaskFormat format, const GrCaps& caps) {
        switch (format) {
            case kA8_GrMaskFormat:
                return kAlpha_8_GrPixelConfig;
            case kA565_GrMaskFormat:
                return kRGB_565_GrPixelConfig;
            case kARGB_GrMaskFormat:
                return caps.srgbSupport() ? kSRGBA_8888_GrPixelConfig : kRGBA_8888_GrPixelConfig;
            default:
                SkDEBUGFAIL("unsupported GrMaskFormat");
                return kAlpha_8_GrPixelConfig;
        }
    }

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

    bool initAtlas(GrMaskFormat);

    GrAtlasTextStrike* generateStrike(const SkGlyphCache* cache) {
        GrAtlasTextStrike* strike = new GrAtlasTextStrike(this, cache->getDescriptor());
        fCache.add(strike);
        return strike;
    }

    GrDrawOpAtlas* getAtlas(GrMaskFormat format) const {
        int atlasIndex = MaskFormatToAtlasIndex(format);
        SkASSERT(fAtlases[atlasIndex]);
        return fAtlases[atlasIndex].get();
    }

    static void HandleEviction(GrDrawOpAtlas::AtlasID, void*);

    using StrikeHash = SkTDynamicHash<GrAtlasTextStrike, SkDescriptor>;
    GrContext* fContext;
    StrikeHash fCache;
    std::unique_ptr<GrDrawOpAtlas> fAtlases[kMaskFormatCount];
    GrAtlasTextStrike* fPreserveStrike;
    GrDrawOpAtlasConfig fAtlasConfigs[kMaskFormatCount];
};

#endif
