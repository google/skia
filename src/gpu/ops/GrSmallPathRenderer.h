/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSmallPathRenderer_DEFINED
#define GrSmallPathRenderer_DEFINED

#include "src/gpu/GrDrawOpAtlas.h"
#include "src/gpu/GrOnFlushResourceProvider.h"
#include "src/gpu/GrPathRenderer.h"
#include "src/gpu/geometry/GrRect.h"
#include "src/gpu/geometry/GrShape.h"

#include "src/core/SkOpts.h"
#include "src/core/SkTDynamicHash.h"

class GrRecordingContext;

class ShapeData;
class ShapeDataKey;

class GrSmallPathRenderer : public GrPathRenderer, public GrOnFlushCallbackObject {
public:
    GrSmallPathRenderer();
    ~GrSmallPathRenderer() override;

    // GrOnFlushCallbackObject overrides
    //
    // Note: because this class is associated with a path renderer we want it to be removed from
    // the list of active OnFlushBackkbackObjects in an freeGpuResources call (i.e., we accept the
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

    using ShapeCache = SkTDynamicHash<ShapeData, ShapeDataKey>;
    typedef SkTInternalLList<ShapeData> ShapeDataList;

    static std::unique_ptr<GrDrawOp> createOp_TestingOnly(GrRecordingContext*,
                                                          GrPaint&&,
                                                          const GrShape&,
                                                          const SkMatrix& viewMatrix,
                                                          GrDrawOpAtlas* atlas,
                                                          ShapeCache*,
                                                          ShapeDataList*,
                                                          bool gammaCorrect,
                                                          const GrUserStencilSettings*);
    struct PathTestStruct;

private:
    class SmallPathOp;

    StencilSupport onGetStencilSupport(const GrShape&) const override {
        return GrPathRenderer::kNoSupport_StencilSupport;
    }

    CanDrawPath onCanDrawPath(const CanDrawPathArgs&) const override;

    bool onDrawPath(const DrawPathArgs&) override;

    static void HandleEviction(GrDrawOpAtlas::AtlasID, void*);

    std::unique_ptr<GrDrawOpAtlas> fAtlas;
    ShapeCache fShapeCache;
    ShapeDataList fShapeList;

    typedef GrPathRenderer INHERITED;
};

#endif
