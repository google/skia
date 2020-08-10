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

/**
 * This class manages the small path renderer's atlas.
 */
class GrSmallPathAtlasMgr : public GrOnFlushCallbackObject,
                            public GrDrawOpAtlas::EvictionCallback,
                            public GrDrawOpAtlas::GenerationCounter{
public:
    GrSmallPathAtlasMgr();
    ~GrSmallPathAtlasMgr() override;

    bool initAtlas(GrProxyProvider*, const GrCaps*);

    GrDrawOpAtlas* atlas() { return fAtlas.get(); }

    const GrSurfaceProxyView* getViews(int* numActiveProxies) {
        *numActiveProxies = fAtlas->numActivePages();
        return fAtlas->getViews();
    }

    void setUseToken(GrSmallPathShapeData*, GrDeferredUploadToken);

    // GrOnFlushCallbackObject overrides
    //
    // Note: because this class is associated with a path renderer we want it to be removed from
    // the list of active OnFlushBackkbackObjects in an freeGpuResources call (i.e., we accept the
    // default retainOnFreeGpuResources implementation).
    void preFlush(GrOnFlushResourceProvider* onFlushRP,
                  const uint32_t* /*opsTaskIDs*/, int /*numOpsTaskIDs*/) override {
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

    GrSmallPathShapeData* findOrCreate(const GrSmallPathShapeDataKey& key);

    void deleteCacheEntry(GrSmallPathShapeData*);

private:
    using ShapeCache = SkTDynamicHash<GrSmallPathShapeData, GrSmallPathShapeDataKey>;
    typedef SkTInternalLList<GrSmallPathShapeData> ShapeDataList;

    void evict(GrDrawOpAtlas::PlotLocator plotLocator) override;

    std::unique_ptr<GrDrawOpAtlas> fAtlas;
    ShapeCache                     fShapeCache;
    ShapeDataList                  fShapeList;
};

#endif // GrSmallPathAtlasMgr_DEFINED
