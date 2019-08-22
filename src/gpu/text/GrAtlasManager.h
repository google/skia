/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAtlasManager_DEFINED
#define GrAtlasManager_DEFINED

#include "src/gpu/GrCaps.h"
#include "src/gpu/GrDrawOpAtlas.h"
#include "src/gpu/GrOnFlushResourceProvider.h"
#include "src/gpu/GrProxyProvider.h"

struct GrGlyph;
class GrTextStrike;

//////////////////////////////////////////////////////////////////////////////////////////////////
/** The GrAtlasManager manages the lifetime of and access to GrDrawOpAtlases.
 *  It is only available at flush and only via the GrOpFlushState.
 *
 *  This implies that all of the advanced atlasManager functionality (i.e.,
 *  adding glyphs to the atlas) are only available at flush time.
 */
class GrAtlasManager : public GrOnFlushCallbackObject {
public:
    GrAtlasManager(GrProxyProvider*, GrStrikeCache*,
                   size_t maxTextureBytes, GrDrawOpAtlas::AllowMultitexturing);
    ~GrAtlasManager() override;

    // Change an expected 565 mask format to 8888 if 565 is not supported (will happen when using
    // Metal on macOS). The actual conversion of the data is handled in get_packed_glyph_image() in
    // GrStrikeCache.cpp
    GrMaskFormat resolveMaskFormat(GrMaskFormat format) const {
        if (kA565_GrMaskFormat == format &&
            !fProxyProvider->caps()->getDefaultBackendFormat(GrColorType::kBGR_565,
                                                             GrRenderable::kNo).isValid()) {
            format = kARGB_GrMaskFormat;
        }
        return format;
    }

    // if getProxies returns nullptr, the client must not try to use other functions on the
    // GrStrikeCache which use the atlas.  This function *must* be called first, before other
    // functions which use the atlas. Note that we can have proxies available but none active
    // (i.e., none instantiated).
    const sk_sp<GrTextureProxy>* getProxies(GrMaskFormat format, unsigned int* numActiveProxies) {
        format = this->resolveMaskFormat(format);
        if (this->initAtlas(format)) {
            *numActiveProxies = this->getAtlas(format)->numActivePages();
            return this->getAtlas(format)->getProxies();
        }
        *numActiveProxies = 0;
        return nullptr;
    }

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
    GrDrawOpAtlas::ErrorCode addToAtlas(
                    GrResourceProvider*, GrStrikeCache*, GrTextStrike*,
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
                  SkTArray<std::unique_ptr<GrRenderTargetContext>>*) override {
        for (int i = 0; i < kMaskFormatCount; ++i) {
            if (fAtlases[i]) {
                fAtlases[i]->instantiate(onFlushResourceProvider);
            }
        }
    }

    void postFlush(GrDeferredUploadToken startTokenForNextFlush,
                   const uint32_t* opsTaskIDs, int numOpsTaskIDs) override {
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

    void setAtlasSizesToMinimum_ForTesting();
    void setMaxPages_TestingOnly(uint32_t maxPages);

private:
    bool initAtlas(GrMaskFormat);

    // There is a 1:1 mapping between GrMaskFormats and atlas indices
    static int MaskFormatToAtlasIndex(GrMaskFormat format) { return static_cast<int>(format); }
    static GrMaskFormat AtlasIndexToMaskFormat(int idx) { return static_cast<GrMaskFormat>(idx); }

    GrDrawOpAtlas* getAtlas(GrMaskFormat format) const {
        format = this->resolveMaskFormat(format);
        int atlasIndex = MaskFormatToAtlasIndex(format);
        SkASSERT(fAtlases[atlasIndex]);
        return fAtlases[atlasIndex].get();
    }

    GrDrawOpAtlas::AllowMultitexturing fAllowMultitexturing;
    std::unique_ptr<GrDrawOpAtlas> fAtlases[kMaskFormatCount];
    GrProxyProvider* fProxyProvider;
    sk_sp<const GrCaps> fCaps;
    GrStrikeCache* fGlyphCache;
    GrDrawOpAtlasConfig fAtlasConfig;

    typedef GrOnFlushCallbackObject INHERITED;
};

#endif // GrAtlasManager_DEFINED
