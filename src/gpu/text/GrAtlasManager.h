/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAtlasManager_DEFINED
#define GrAtlasManager_DEFINED

#include "GrDrawOpAtlas.h"
#include "GrOnFlushResourceProvider.h"

class GrAtlasGlypCache;
class GrTextStrike;
struct GrGlyph;

 /** The GrAtlasManager classes manage the lifetime of and access to GrDrawOpAtlases.
  *  The restricted version is available at op creation time and only allows basic access
  *  to the proxies (so the created ops can reference them). The full GrAtlasManager class
  *  is only available at flush time and only via the GrOpFlushState.
  *
  *  This organization implies that all of the advanced atlasManager functionality (i.e.,
  *  adding glyphs to the atlas) are only available at flush time.
  */
class GrRestrictedAtlasManager : public GrOnFlushCallbackObject {
public:
    GrRestrictedAtlasManager(sk_sp<const GrCaps>, float maxTextureBytes,
                             GrDrawOpAtlas::AllowMultitexturing);
    ~GrRestrictedAtlasManager() override;

    // if getProxies returns nullptr, the client must not try to use other functions on the
    // GrGlyphCache which use the atlas.  This function *must* be called first, before other
    // functions which use the atlas.
    const sk_sp<GrTextureProxy>* getProxies(GrMaskFormat format, unsigned int* numProxies) {
        if (this->initAtlas(format)) {
            *numProxies = this->getAtlas(format)->numActivePages();
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
    virtual bool initAtlas(GrMaskFormat) = 0;

    typedef GrOnFlushCallbackObject INHERITED;
};

//////////////////////////////////////////////////////////////////////////////////////////////////
class GrAtlasManager : public GrRestrictedAtlasManager {
public:
    GrAtlasManager(GrProxyProvider*, GrGlyphCache*,
                   float maxTextureBytes, GrDrawOpAtlas::AllowMultitexturing);

    void freeAll();

    bool hasGlyph(GrGlyph* glyph);

    // To ensure the GrDrawOpAtlas does not evict the Glyph Mask from its texture backing store,
    // the client must pass in the current op token along with the GrGlyph.
    // A BulkUseTokenUpdater is used to manage bulk last use token updating in the Atlas.
    // For convenience, this function will also set the use token for the current glyph if required
    // NOTE: the bulk uploader is only valid if the subrun has a valid atlasGeneration
    void addGlyphToBulkAndSetUseToken(GrDrawOpAtlas::BulkUseTokenUpdater*, GrGlyph*,
                                      GrDeferredUploadToken);

    void setUseTokenBulk(const GrDrawOpAtlas::BulkUseTokenUpdater& updater,
                         GrDeferredUploadToken token,
                         GrMaskFormat format) {
        this->getAtlas(format)->setLastUseTokenBulk(updater, token);
    }

    // add to texture atlas that matches this format
    bool addToAtlas(GrResourceProvider*, GrGlyphCache*, GrTextStrike*,
                    GrDrawOpAtlas::AtlasID*, GrDeferredUploadTarget*, GrMaskFormat,
                    int width, int height, const void* image, SkIPoint16* loc);

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
    bool initAtlas(GrMaskFormat) override;

    GrProxyProvider* fProxyProvider;
    GrGlyphCache* fGlyphCache;

    typedef GrRestrictedAtlasManager INHERITED;
};

#endif // GrAtlasManager_DEFINED
