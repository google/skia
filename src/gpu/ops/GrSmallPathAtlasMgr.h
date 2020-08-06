/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSmallPathAtlasMgr_DEFINED
#define GrSmallPathAtlasMgr_DEFINED

#include "src/gpu/GrDrawOpAtlas.h"
#include "src/gpu/GrOnFlushResourceProvider.h"

class GrSmallPathShapeData;
class GrSmallPathShapeDataKey;
class GrStyledShape;

class GrSmallPathAtlasMgr : public GrOnFlushCallbackObject,
                            public GrDrawOpAtlas::EvictionCallback,
                            public GrDrawOpAtlas::GenerationCounter {
public:
    GrSmallPathAtlasMgr();
    ~GrSmallPathAtlasMgr() override;

    bool initAtlas(GrProxyProvider*, const GrCaps*);

    GrDrawOpAtlas* atlas() { return fAtlas.get(); }

    GrSmallPathShapeData* findOrCreate(const GrStyledShape&, int desiredDimension);
    GrSmallPathShapeData* findOrCreate(const GrStyledShape&, const SkMatrix& ctm);

    void updateCacheInfo(GrSmallPathShapeData*, GrDrawOpAtlas::AtlasLocator&, const SkRect& bounds);

    void setUseToken(GrSmallPathShapeData*, GrDeferredUploadToken);

    // GrOnFlushCallbackObject overrides
    //
    // Note: because this class is associated with a path renderer we want it to be removed from
    // the list of active OnFlushCallbackObjects in an freeGpuResources call (i.e., we accept the
    // default retainOnFreeGpuResources implementation).
    void preFlush(GrOnFlushResourceProvider* onFlushRP,
                  const uint32_t* /* opsTaskIDs */,
                  int /* numOpsTaskIDs */) override {
        if (fAtlas) {
            fAtlas->instantiate(onFlushRP);
        }
    }

    void postFlush(GrDeferredUploadToken startTokenForNextFlush,
                   const uint32_t* /* opsTaskIDs */,
                   int /* numOpsTaskIDs */) override {
        if (fAtlas) {
            fAtlas->compact(startTokenForNextFlush);
        }
    }

    const GrSurfaceProxyView* getViews(int* numActiveProxies) {
        *numActiveProxies = fAtlas->numActivePages();
        return fAtlas->getViews();
    }

private:
    GrSmallPathShapeData* findOrCreate(const GrSmallPathShapeDataKey&);

    using ShapeCache = SkTDynamicHash<GrSmallPathShapeData, GrSmallPathShapeDataKey>;
    typedef SkTInternalLList<GrSmallPathShapeData> ShapeDataList;

    void evict(GrDrawOpAtlas::PlotLocator) override;

    std::unique_ptr<GrDrawOpAtlas> fAtlas;
    ShapeCache fShapeCache;
    ShapeDataList fShapeList;
};

#endif // GrSmallPathAtlasMgr_DEFINED
