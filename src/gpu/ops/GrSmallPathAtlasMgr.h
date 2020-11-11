/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSmallPathAtlasMgr_DEFINED
#define GrSmallPathAtlasMgr_DEFINED

#include "src/core/SkTDynamicHash.h"
#include "src/core/SkTInternalLList.h"
#include "src/gpu/GrDrawOpAtlas.h"
#include "src/gpu/GrOnFlushResourceProvider.h"

class GrSmallPathShapeData;
class GrSmallPathShapeDataKey;
class GrStyledShape;

/**
 * This class manages the small path renderer's atlas. It solely operates at flush time. Thus
 * the small path renderer will generate ops at record time but the location of the ops' source
 * data and even the number of proxies to be used will not be determined until the recorded
 * DAGs/DDLs are (re)played.
 *
 * TODO: investigate fusing this class and the GrAtlasManager.
 */
class GrSmallPathAtlasMgr : public GrOnFlushCallbackObject,
                            public GrDrawOpAtlas::EvictionCallback,
                            public GrDrawOpAtlas::GenerationCounter {
public:
    GrSmallPathAtlasMgr();
    ~GrSmallPathAtlasMgr() override;

    void reset();

    bool initAtlas(GrProxyProvider*, const GrCaps*);

    GrSmallPathShapeData* findOrCreate(const GrStyledShape&, int desiredDimension);
    GrSmallPathShapeData* findOrCreate(const GrStyledShape&, const SkMatrix& ctm);

    GrDrawOpAtlas::ErrorCode addToAtlas(GrResourceProvider*,
                                        GrDeferredUploadTarget*,
                                        int width, int height, const void* image,
                                        GrDrawOpAtlas::AtlasLocator*);

    void setUseToken(GrSmallPathShapeData*, GrDeferredUploadToken);

    // GrOnFlushCallbackObject overrides
    void preFlush(GrOnFlushResourceProvider* onFlushRP,
                  SkSpan<const uint32_t> /* taskIDs */) override {
        if (fAtlas) {
            fAtlas->instantiate(onFlushRP);
        }
    }

    void postFlush(GrDeferredUploadToken startTokenForNextFlush,
                   SkSpan<const uint32_t> /* taskIDs */) override {
        if (fAtlas) {
            fAtlas->compact(startTokenForNextFlush);
        }
    }

    // This object has the same lifetime as the GrContext so we want it to survive freeGpuResources
    // calls
    bool retainOnFreeGpuResources() override { return true; }

    const GrSurfaceProxyView* getViews(int* numActiveProxies) {
        *numActiveProxies = fAtlas->numActivePages();
        return fAtlas->getViews();
    }

    void deleteCacheEntry(GrSmallPathShapeData*);

private:
    GrSmallPathShapeData* findOrCreate(const GrSmallPathShapeDataKey&);

    void evict(GrDrawOpAtlas::PlotLocator) override;

    using ShapeCache = SkTDynamicHash<GrSmallPathShapeData, GrSmallPathShapeDataKey>;
    typedef SkTInternalLList<GrSmallPathShapeData> ShapeDataList;

    std::unique_ptr<GrDrawOpAtlas> fAtlas;
    ShapeCache                     fShapeCache;
    ShapeDataList                  fShapeList;
};

#endif // GrSmallPathAtlasMgr_DEFINED
