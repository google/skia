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

#include "src/core/SkTDynamicHash.h"
#include "src/core/SkTInternalLList.h"

class GrGlyph;
class GrStyledShape;
class GrTextStrike;

//class ShapeData;
//class ShapeDataKey;

#include "src/core/SkOpts.h"
#include "src/gpu/geometry/GrStyledShape.h"

class ShapeDataKey {
public:
    // add move variant!!
    ShapeDataKey() {}
    ShapeDataKey(const ShapeDataKey& that) { *this = that; }
    ShapeDataKey(const GrStyledShape& shape, uint32_t dim) { this->set(shape, dim); }
    ShapeDataKey(const GrStyledShape& shape, const SkMatrix& ctm) { this->set(shape, ctm); }

    ShapeDataKey& operator=(const ShapeDataKey& that) {
        fKey.reset(that.fKey.count());
        memcpy(fKey.get(), that.fKey.get(), fKey.count() * sizeof(uint32_t));
        return *this;
    }

    // for SDF paths
    void set(const GrStyledShape& shape, uint32_t dim) {
        // Shapes' keys are for their pre-style geometry, but by now we shouldn't have any
        // relevant styling information.
        SkASSERT(shape.style().isSimpleFill());
        SkASSERT(shape.hasUnstyledKey());
        int shapeKeySize = shape.unstyledKeySize();
        fKey.reset(1 + shapeKeySize);
        fKey[0] = dim;
        shape.writeUnstyledKey(&fKey[1]);
    }

    // for bitmap paths
    void set(const GrStyledShape& shape, const SkMatrix& ctm) {
        // Shapes' keys are for their pre-style geometry, but by now we shouldn't have any
        // relevant styling information.
        SkASSERT(shape.style().isSimpleFill());
        SkASSERT(shape.hasUnstyledKey());
        // We require the upper left 2x2 of the matrix to match exactly for a cache hit.
        SkScalar sx = ctm.get(SkMatrix::kMScaleX);
        SkScalar sy = ctm.get(SkMatrix::kMScaleY);
        SkScalar kx = ctm.get(SkMatrix::kMSkewX);
        SkScalar ky = ctm.get(SkMatrix::kMSkewY);
        SkScalar tx = ctm.get(SkMatrix::kMTransX);
        SkScalar ty = ctm.get(SkMatrix::kMTransY);
        // Allow 8 bits each in x and y of subpixel positioning.
        tx -= SkScalarFloorToScalar(tx);
        ty -= SkScalarFloorToScalar(ty);
        SkFixed fracX = SkScalarToFixed(tx) & 0x0000FF00;
        SkFixed fracY = SkScalarToFixed(ty) & 0x0000FF00;
        int shapeKeySize = shape.unstyledKeySize();
        fKey.reset(5 + shapeKeySize);
        fKey[0] = SkFloat2Bits(sx);
        fKey[1] = SkFloat2Bits(sy);
        fKey[2] = SkFloat2Bits(kx);
        fKey[3] = SkFloat2Bits(ky);
        fKey[4] = fracX | (fracY >> 8);
        shape.writeUnstyledKey(&fKey[5]);
    }

    bool operator==(const ShapeDataKey& that) const {
        return fKey.count() == that.fKey.count() &&
                0 == memcmp(fKey.get(), that.fKey.get(), sizeof(uint32_t) * fKey.count());
    }

    int count32() const { return fKey.count(); }
    const uint32_t* data() const { return fKey.get(); }

private:
    // The key is composed of the GrStyledShape's key, and either the dimensions of the DF
    // generated for the path (32x32 max, 64x64 max, 128x128 max) if an SDF image or
    // the matrix for the path with only fractional translation.
    SkAutoSTArray<24, uint32_t> fKey;
};

class ShapeData {
public:
    ShapeData(const ShapeDataKey& key) : fKey(key) {}

    const ShapeDataKey          fKey;
    SkRect                      fBounds;
    GrDrawOpAtlas::AtlasLocator fAtlasLocator;

    SK_DECLARE_INTERNAL_LLIST_INTERFACE(ShapeData);

    static inline const ShapeDataKey& GetKey(const ShapeData& data) {
        return data.fKey;
    }

    static inline uint32_t Hash(const ShapeDataKey& key) {
        return SkOpts::hash(key.data(), sizeof(uint32_t) * key.count32());
    }
};



class GrSmallPathAtlasMgr : public GrOnFlushCallbackObject,
                            public GrDrawOpAtlas::EvictionCallback,
                            public GrDrawOpAtlas::GenerationCounter {
public:
    GrSmallPathAtlasMgr();
    ~GrSmallPathAtlasMgr();

    bool initAtlas(GrProxyProvider*, const GrCaps*);

    GrDrawOpAtlas* atlas() { return fAtlas.get(); }

    ShapeData* findOrCreate(const GrStyledShape&, int desiredDimension);
    ShapeData* findOrCreate(const GrStyledShape&, const SkMatrix& ctm);

    void updateCacheInfo(ShapeData*, GrDrawOpAtlas::AtlasLocator&, const SkRect& bounds);

    void setUseToken(ShapeData*, GrDeferredUploadToken);

    // GrOnFlushCallbackObject overrides
    //
    // Note: because this class is associated with a path renderer we want it to be removed from
    // the list of active OnFlushCallbackObjects in an freeGpuResources call (i.e., we accept the
    // default retainOnFreeGpuResources implementation).
    void preFlush(GrOnFlushResourceProvider* onFlushRP, const uint32_t*, int) override {
        if (fAtlas) {
            fAtlas->instantiate(onFlushRP);
        }
    }

    void postFlush(GrDeferredUploadToken startTokenForNextFlush,
                   const uint32_t* /*opsTaskIDs*/, int /*numOpsTaskIDs*/) override {
        if (fAtlas) {
            fAtlas->compact(startTokenForNextFlush);
        }
    }

    const GrSurfaceProxyView* getViews(unsigned int* numActiveProxies) {
        *numActiveProxies = fAtlas->numActivePages();
        return fAtlas->getViews();
    }

protected:

private:
    ShapeData* findOrCreate(const ShapeDataKey&);

    using ShapeCache = SkTDynamicHash<ShapeData, ShapeDataKey>;
    typedef SkTInternalLList<ShapeData> ShapeDataList;

    void evict(GrDrawOpAtlas::PlotLocator) override;

    std::unique_ptr<GrDrawOpAtlas> fAtlas;
    ShapeCache fShapeCache;
    ShapeDataList fShapeList;
};

//////////////////////////////////////////////////////////////////////////////////////////////////
/** The GrAtlasManager manages the lifetime of and access to GrDrawOpAtlases.
 *  It is only available at flush and only via the GrOpFlushState.
 *
 *  This implies that all of the advanced atlasManager functionality (i.e.,
 *  adding glyphs to the atlas) are only available at flush time.
 */
class GrAtlasManager : public GrOnFlushCallbackObject, public GrDrawOpAtlas::GenerationCounter {
public:
    GrAtlasManager(GrProxyProvider*, size_t maxTextureBytes, GrDrawOpAtlas::AllowMultitexturing);
    ~GrAtlasManager() override;

    // if getViews returns nullptr, the client must not try to use other functions on the
    // GrStrikeCache which use the atlas.  This function *must* be called first, before other
    // functions which use the atlas. Note that we can have proxies available but none active
    // (i.e., none instantiated).
    const GrSurfaceProxyView* getViews1(GrMaskFormat format, unsigned int* numActiveProxies) {
        format = this->resolveMaskFormat(format);
        if (this->initAtlas1(format)) {
            *numActiveProxies = this->getAtlas(format)->numActivePages();
            return this->getAtlas(format)->getViews();
        }
        *numActiveProxies = 0;
        return nullptr;
    }

    void freeAll();

    bool hasGlyph(GrMaskFormat, GrGlyph*);

    // If bilerpPadding == true then addGlyphToAtlas adds a 1 pixel border to the glyph before
    // inserting it into the atlas.
    GrDrawOpAtlas::ErrorCode addGlyphToAtlas(const SkGlyph& skGlyph,
                                             GrGlyph* grGlyph,
                                             int srcPadding,
                                             GrResourceProvider* resourceProvider,
                                             GrDeferredUploadTarget* uploadTarget,
                                             bool bilerpPadding = false);

    // To ensure the GrDrawOpAtlas does not evict the Glyph Mask from its texture backing store,
    // the client must pass in the current op token along with the GrGlyph.
    // A BulkUseTokenUpdater is used to manage bulk last use token updating in the Atlas.
    // For convenience, this function will also set the use token for the current glyph if required
    // NOTE: the bulk uploader is only valid if the subrun has a valid atlasGeneration
    void addGlyphToBulkAndSetUseToken(GrDrawOpAtlas::BulkUseTokenUpdater*, GrMaskFormat, GrGlyph*,
                                      GrDeferredUploadToken);

    void setUseTokenBulk(const GrDrawOpAtlas::BulkUseTokenUpdater& updater,
                         GrDeferredUploadToken token,
                         GrMaskFormat format) {
        this->getAtlas(format)->setLastUseTokenBulk(updater, token);
    }

    // add to texture atlas that matches this format
    GrDrawOpAtlas::ErrorCode addToAtlas(GrResourceProvider*, GrDeferredUploadTarget*, GrMaskFormat,
                                        int width, int height, const void* image,
                                        GrDrawOpAtlas::AtlasLocator*);

    // Some clients may wish to verify the integrity of the texture backing store of the
    // GrDrawOpAtlas. The atlasGeneration returned below is a monotonically increasing number which
    // changes every time something is removed from the texture backing store.
    uint64_t atlasGeneration(GrMaskFormat format) const {
        return this->getAtlas(format)->atlasGeneration();
    }

    // GrOnFlushCallbackObject overrides

    void preFlush(GrOnFlushResourceProvider* onFlushRP, const uint32_t*, int) override {
        for (int i = 0; i < kMaskFormatCount; ++i) {
            if (fAtlases[i]) {
                fAtlases[i]->instantiate(onFlushRP);
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
    void dump(GrDirectContext*) const;
#endif

    void setAtlasDimensionsToMinimum_ForTesting();
    void setMaxPages_TestingOnly(uint32_t maxPages);

private:
    bool initAtlas1(GrMaskFormat);
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
    static_assert(kMaskFormatCount == 3);
    GrProxyProvider* fProxyProvider;
    sk_sp<const GrCaps> fCaps;
    GrDrawOpAtlasConfig fAtlasConfig;

    typedef GrOnFlushCallbackObject INHERITED;
};

#endif // GrAtlasManager_DEFINED
