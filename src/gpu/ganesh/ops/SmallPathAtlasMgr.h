/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SmallPathAtlasMgr_DEFINED
#define SmallPathAtlasMgr_DEFINED

#if !defined(SK_ENABLE_OPTIMIZE_SIZE)

#include "src/base/SkTInternalLList.h"
#include "src/core/SkTDynamicHash.h"
#include "src/gpu/ganesh/GrDrawOpAtlas.h"
#include "src/gpu/ganesh/GrOnFlushResourceProvider.h"

class GrStyledShape;

namespace skgpu::ganesh {

class SmallPathShapeData;
class SmallPathShapeDataKey;

/**
 * This class manages the small path renderer's atlas. It solely operates at flush time. Thus
 * the small path renderer will generate ops at record time but the location of the ops' source
 * data and even the number of proxies to be used will not be determined until the recorded
 * DAGs/DDLs are (re)played.
 *
 * TODO: investigate fusing this class and the GrAtlasManager.
 */
class SmallPathAtlasMgr final : public GrOnFlushCallbackObject,
                                public skgpu::PlotEvictionCallback,
                                public skgpu::AtlasGenerationCounter {
public:
    SmallPathAtlasMgr();
    ~SmallPathAtlasMgr() override;

    void reset();

    bool initAtlas(GrProxyProvider*, const GrCaps*);

    SmallPathShapeData* findOrCreate(const GrStyledShape&, int desiredDimension);
    SmallPathShapeData* findOrCreate(const GrStyledShape&, const SkMatrix& ctm);

    GrDrawOpAtlas::ErrorCode addToAtlas(GrResourceProvider*,
                                        GrDeferredUploadTarget*,
                                        int width, int height, const void* image,
                                        skgpu::AtlasLocator*);

    void setUseToken(SmallPathShapeData*, skgpu::AtlasToken);

    // GrOnFlushCallbackObject overrides
    bool preFlush(GrOnFlushResourceProvider* onFlushRP) override {
#if defined(GR_TEST_UTILS)
        if (onFlushRP->failFlushTimeCallbacks()) {
            return false;
        }
#endif

        if (fAtlas) {
            fAtlas->instantiate(onFlushRP);
        }
        return true;
    }

    void postFlush(skgpu::AtlasToken startTokenForNextFlush) override {
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

    void deleteCacheEntry(SmallPathShapeData*);

private:
    SmallPathShapeData* findOrCreate(const SmallPathShapeDataKey&);

    void evict(skgpu::PlotLocator) override;

    using ShapeCache = SkTDynamicHash<SmallPathShapeData, SmallPathShapeDataKey>;
    typedef SkTInternalLList<SmallPathShapeData> ShapeDataList;

    std::unique_ptr<GrDrawOpAtlas> fAtlas;
    ShapeCache                     fShapeCache;
    ShapeDataList                  fShapeList;
};

}  // namespace skgpu::ganesh

#endif // SK_ENABLE_OPTIMIZE_SIZE

#endif // SmallPathAtlasMgr_DEFINED
