/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBWFillRectBatch.h"

#include "GrBatchFlushState.h"
#include "GrColor.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrPrimitiveProcessor.h"
#include "GrQuad.h"
#include "GrResourceProvider.h"
#include "GrTInstanceBatch.h"
#include "GrVertexBatch.h"

// Common functions
class BWFillRectBatchBase {
public:
    static const int kVertsPerInstance = 4;
    static const int kIndicesPerInstance = 6;

    static const GrIndexBuffer* GetIndexBuffer(GrResourceProvider* rp) {
        return rp->refQuadIndexBuffer();
    }

    template <typename Geometry>
    static void SetBounds(const Geometry& geo, SkRect* outBounds) {
        geo.fViewMatrix.mapRect(outBounds, geo.fRect);
    }
};

/** We always use per-vertex colors so that rects can be batched across color changes. Sometimes
    we  have explicit local coords and sometimes not. We *could* always provide explicit local
    coords and just duplicate the positions when the caller hasn't provided a local coord rect,
    but we haven't seen a use case which frequently switches between local rect and no local
    rect draws.

    The color param is used to determine whether the opaque hint can be set on the draw state.
    The caller must populate the vertex colors itself.

    The vertex attrib order is always pos, color, [local coords].
 */
static const GrGeometryProcessor* create_gp(const SkMatrix& viewMatrix,
                                            bool readsCoverage,
                                            bool hasExplicitLocalCoords,
                                            const SkMatrix* localMatrix) {
    using namespace GrDefaultGeoProcFactory;
    Color color(Color::kAttribute_Type);
    Coverage coverage(readsCoverage ? Coverage::kSolid_Type : Coverage::kNone_Type);

    // if we have a local rect, then we apply the localMatrix directly to the localRect to
    // generate vertex local coords
    if (hasExplicitLocalCoords) {
        LocalCoords localCoords(LocalCoords::kHasExplicit_Type);
        return GrDefaultGeoProcFactory::Create(color, coverage, localCoords, SkMatrix::I());
    } else {
        LocalCoords localCoords(LocalCoords::kUsePosition_Type, localMatrix ? localMatrix : NULL);
        return GrDefaultGeoProcFactory::CreateForDeviceSpace(color, coverage, localCoords,
                                                             viewMatrix);
    }
}

static void tesselate(intptr_t vertices,
                      size_t vertexStride,
                      GrColor color,
                      const SkMatrix& viewMatrix,
                      const SkRect& rect,
                      const SkRect* localRect,
                      const SkMatrix* localMatrix) {
    SkPoint* positions = reinterpret_cast<SkPoint*>(vertices);

    positions->setRectFan(rect.fLeft, rect.fTop,
                          rect.fRight, rect.fBottom, vertexStride);
    viewMatrix.mapPointsWithStride(positions, vertexStride, BWFillRectBatchBase::kVertsPerInstance);

    // TODO we should only do this if local coords are being read
    if (localRect) {
        static const int kLocalOffset = sizeof(SkPoint) + sizeof(GrColor);
        SkPoint* coords = reinterpret_cast<SkPoint*>(vertices + kLocalOffset);
        coords->setRectFan(localRect->fLeft, localRect->fTop,
                           localRect->fRight, localRect->fBottom,
                           vertexStride);
        if (localMatrix) {
            localMatrix->mapPointsWithStride(coords, vertexStride,
                                             BWFillRectBatchBase::kVertsPerInstance);
        }
    }

    static const int kColorOffset = sizeof(SkPoint);
    GrColor* vertColor = reinterpret_cast<GrColor*>(vertices + kColorOffset);
    for (int j = 0; j < 4; ++j) {
        *vertColor = color;
        vertColor = (GrColor*) ((intptr_t) vertColor + vertexStride);
    }
}

class BWFillRectBatchNoLocalMatrixImp : public BWFillRectBatchBase {
public:
    struct Geometry {
        SkMatrix fViewMatrix;
        SkRect fRect;
        GrColor fColor;
    };

    static const char* Name() { return "BWFillRectBatchNoLocalMatrix"; }

    static bool CanCombine(const Geometry& mine, const Geometry& theirs,
                           const GrPipelineOptimizations& opts) {
        // We apply the viewmatrix to the rect points on the cpu.  However, if the pipeline uses
        // local coords then we won't be able to batch.  We could actually upload the viewmatrix
        // using vertex attributes in these cases, but haven't investigated that
        return !opts.readsLocalCoords() || mine.fViewMatrix.cheapEqualTo(theirs.fViewMatrix);
    }

    static const GrGeometryProcessor* CreateGP(const Geometry& geo,
                                               const GrPipelineOptimizations& opts) {
        const GrGeometryProcessor* gp = create_gp(geo.fViewMatrix, opts.readsCoverage(), false,
                                                  NULL);

        SkASSERT(gp->getVertexStride() == sizeof(GrDefaultGeoProcFactory::PositionColorAttr));
        return gp;
    }

    static void Tesselate(intptr_t vertices, size_t vertexStride, const Geometry& geo,
                          const GrPipelineOptimizations& opts) {
        tesselate(vertices, vertexStride, geo.fColor, geo.fViewMatrix, geo.fRect, NULL, NULL);
    }
};

class BWFillRectBatchLocalMatrixImp : public BWFillRectBatchBase {
public:
    struct Geometry {
        SkMatrix fViewMatrix;
        SkMatrix fLocalMatrix;
        SkRect fRect;
        GrColor fColor;
    };

    static const char* Name() { return "BWFillRectBatchLocalMatrix"; }

    static bool CanCombine(const Geometry& mine, const Geometry& theirs,
                           const GrPipelineOptimizations& opts) {
        // We apply the viewmatrix to the rect points on the cpu.  However, if the pipeline uses
        // local coords then we won't be able to batch.  We could actually upload the viewmatrix
        // using vertex attributes in these cases, but haven't investigated that
        return !opts.readsLocalCoords() || mine.fViewMatrix.cheapEqualTo(theirs.fViewMatrix);
    }

    static const GrGeometryProcessor* CreateGP(const Geometry& geo,
                                               const GrPipelineOptimizations& opts) {
        const GrGeometryProcessor* gp = create_gp(geo.fViewMatrix, opts.readsCoverage(), false,
                                                  &geo.fLocalMatrix);

        SkASSERT(gp->getVertexStride() == sizeof(GrDefaultGeoProcFactory::PositionColorAttr));
        return gp;
    }

    static void Tesselate(intptr_t vertices, size_t vertexStride, const Geometry& geo,
                          const GrPipelineOptimizations& opts) {
        tesselate(vertices, vertexStride, geo.fColor, geo.fViewMatrix, geo.fRect, NULL,
                  &geo.fLocalMatrix);
    }
};

class BWFillRectBatchLocalRectImp : public BWFillRectBatchBase {
public:
    struct Geometry {
        SkMatrix fViewMatrix;
        SkRect fRect;
        SkRect fLocalRect;
        GrColor fColor;
    };

    static const char* Name() { return "BWFillRectBatchLocalRect"; }

    static bool CanCombine(const Geometry& mine, const Geometry& theirs,
                           const GrPipelineOptimizations& opts) {
        return true;
    }

    static const GrGeometryProcessor* CreateGP(const Geometry& geo,
                                               const GrPipelineOptimizations& opts) {
        const GrGeometryProcessor* gp = create_gp(geo.fViewMatrix, opts.readsCoverage(), true,
                                                  NULL);

        SkASSERT(gp->getVertexStride() ==
                sizeof(GrDefaultGeoProcFactory::PositionColorLocalCoordAttr));
        return gp;
    }

    static void Tesselate(intptr_t vertices, size_t vertexStride, const Geometry& geo,
                          const GrPipelineOptimizations& opts) {
        tesselate(vertices, vertexStride, geo.fColor, geo.fViewMatrix, geo.fRect, &geo.fLocalRect,
                  NULL);
    }
};

class BWFillRectBatchLocalMatrixLocalRectImp : public BWFillRectBatchBase {
public:
    struct Geometry {
        SkMatrix fViewMatrix;
        SkMatrix fLocalMatrix;
        SkRect fRect;
        SkRect fLocalRect;
        GrColor fColor;
    };

    static const char* Name() { return "BWFillRectBatchLocalMatrixLocalRect"; }

    static bool CanCombine(const Geometry& mine, const Geometry& theirs,
                           const GrPipelineOptimizations& opts) {
        return true;
    }

    static const GrGeometryProcessor* CreateGP(const Geometry& geo,
                                               const GrPipelineOptimizations& opts) {
        const GrGeometryProcessor* gp = create_gp(geo.fViewMatrix, opts.readsCoverage(), true,
                                                  NULL);

        SkASSERT(gp->getVertexStride() ==
                sizeof(GrDefaultGeoProcFactory::PositionColorLocalCoordAttr));
        return gp;
    }

    static void Tesselate(intptr_t vertices, size_t vertexStride, const Geometry& geo,
                          const GrPipelineOptimizations& opts) {
        tesselate(vertices, vertexStride, geo.fColor, geo.fViewMatrix, geo.fRect, &geo.fLocalRect,
                  &geo.fLocalMatrix);
    }
};

typedef GrTInstanceBatch<BWFillRectBatchNoLocalMatrixImp> BWFillRectBatchSimple;
typedef GrTInstanceBatch<BWFillRectBatchLocalMatrixImp> BWFillRectBatchLocalMatrix;
typedef GrTInstanceBatch<BWFillRectBatchLocalRectImp> BWFillRectBatchLocalRect;
typedef GrTInstanceBatch<BWFillRectBatchLocalMatrixLocalRectImp> BWFillRectBatchLocalMatrixLocalRect;

namespace GrBWFillRectBatch {
GrDrawBatch* Create(GrColor color,
                    const SkMatrix& viewMatrix,
                    const SkRect& rect,
                    const SkRect* localRect,
                    const SkMatrix* localMatrix) {
    // TODO bubble these up as separate calls
    if (localRect && localMatrix) {
        BWFillRectBatchLocalMatrixLocalRect* batch = BWFillRectBatchLocalMatrixLocalRect::Create();
        BWFillRectBatchLocalMatrixLocalRect::Geometry& geo = *batch->geometry();
        geo.fColor = color;
        geo.fViewMatrix = viewMatrix;
        geo.fLocalMatrix = *localMatrix;
        geo.fRect = rect;
        geo.fLocalRect = *localRect;
        batch->init();
        return batch;
    } else if (localRect) {
        BWFillRectBatchLocalRect* batch = BWFillRectBatchLocalRect::Create();
        BWFillRectBatchLocalRect::Geometry& geo = *batch->geometry();
        geo.fColor = color;
        geo.fViewMatrix = viewMatrix;
        geo.fRect = rect;
        geo.fLocalRect = *localRect;
        batch->init();
        return batch;
    } else if (localMatrix) {
        BWFillRectBatchLocalMatrix* batch = BWFillRectBatchLocalMatrix::Create();
        BWFillRectBatchLocalMatrix::Geometry& geo = *batch->geometry();
        geo.fColor = color;
        geo.fViewMatrix = viewMatrix;
        geo.fLocalMatrix = *localMatrix;
        geo.fRect = rect;
        batch->init();
        return batch;
    } else {
        BWFillRectBatchSimple* batch = BWFillRectBatchSimple::Create();
        BWFillRectBatchSimple::Geometry& geo = *batch->geometry();
        geo.fColor = color;
        geo.fViewMatrix = viewMatrix;
        geo.fRect = rect;
        batch->init();
        return batch;
    }
}
};

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GR_TEST_UTILS

#include "GrBatchTest.h"

DRAW_BATCH_TEST_DEFINE(RectBatch) {
    GrColor color = GrRandomColor(random);
    SkRect rect = GrTest::TestRect(random);
    SkRect localRect = GrTest::TestRect(random);
    SkMatrix viewMatrix = GrTest::TestMatrixInvertible(random);
    SkMatrix localMatrix = GrTest::TestMatrix(random);

    bool hasLocalRect = random->nextBool();
    bool hasLocalMatrix = random->nextBool();
    return GrBWFillRectBatch::Create(color, viewMatrix, rect, hasLocalRect ? &localRect : nullptr,
                                     hasLocalMatrix ? &localMatrix : nullptr);
}

#endif
