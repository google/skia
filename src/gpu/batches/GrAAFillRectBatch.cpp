/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAAFillRectBatch.h"

#include "GrColor.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrResourceKey.h"
#include "GrResourceProvider.h"
#include "GrTInstanceBatch.h"
#include "GrTypes.h"
#include "SkMatrix.h"
#include "SkRect.h"

GR_DECLARE_STATIC_UNIQUE_KEY(gAAFillRectIndexBufferKey);

static void set_inset_fan(SkPoint* pts, size_t stride,
                          const SkRect& r, SkScalar dx, SkScalar dy) {
    pts->setRectFan(r.fLeft + dx, r.fTop + dy,
                    r.fRight - dx, r.fBottom - dy, stride);
}

static const int kNumAAFillRectsInIndexBuffer = 256;
static const int kVertsPerAAFillRect = 8;
static const int kIndicesPerAAFillRect = 30;

const GrBuffer* get_index_buffer(GrResourceProvider* resourceProvider) {
    GR_DEFINE_STATIC_UNIQUE_KEY(gAAFillRectIndexBufferKey);

    static const uint16_t gFillAARectIdx[] = {
        0, 1, 5, 5, 4, 0,
        1, 2, 6, 6, 5, 1,
        2, 3, 7, 7, 6, 2,
        3, 0, 4, 4, 7, 3,
        4, 5, 6, 6, 7, 4,
    };
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(gFillAARectIdx) == kIndicesPerAAFillRect);
    return resourceProvider->findOrCreateInstancedIndexBuffer(gFillAARectIdx,
        kIndicesPerAAFillRect, kNumAAFillRectsInIndexBuffer, kVertsPerAAFillRect,
        gAAFillRectIndexBufferKey);
}

static const GrGeometryProcessor* create_fill_rect_gp(
                                       const SkMatrix& viewMatrix,
                                       const GrXPOverridesForBatch& overrides,
                                       GrDefaultGeoProcFactory::LocalCoords::Type localCoordsType) {
    using namespace GrDefaultGeoProcFactory;

    Color color(Color::kAttribute_Type);
    Coverage::Type coverageType;
    // TODO remove coverage if coverage is ignored
    /*if (coverageIgnored) {
        coverageType = Coverage::kNone_Type;
    } else*/ if (overrides.canTweakAlphaForCoverage()) {
        coverageType = Coverage::kSolid_Type;
    } else {
        coverageType = Coverage::kAttribute_Type;
    }
    Coverage coverage(coverageType);

    // We assume the caller has inverted the viewmatrix
    if (LocalCoords::kHasExplicit_Type == localCoordsType) {
        LocalCoords localCoords(localCoordsType);
        return GrDefaultGeoProcFactory::Create(color, coverage, localCoords, SkMatrix::I());
    } else {
        LocalCoords localCoords(overrides.readsLocalCoords() ? localCoordsType :
                                                               LocalCoords::kUnused_Type);
        return CreateForDeviceSpace(color, coverage, localCoords, viewMatrix);
    }
}

static void generate_aa_fill_rect_geometry(intptr_t verts,
                                           size_t vertexStride,
                                           GrColor color,
                                           const SkMatrix& viewMatrix,
                                           const SkRect& rect,
                                           const SkRect& devRect,
                                           const GrXPOverridesForBatch& overrides,
                                           const SkMatrix* localMatrix) {
    SkPoint* fan0Pos = reinterpret_cast<SkPoint*>(verts);
    SkPoint* fan1Pos = reinterpret_cast<SkPoint*>(verts + 4 * vertexStride);

    SkScalar inset = SkMinScalar(devRect.width(), SK_Scalar1);
    inset = SK_ScalarHalf * SkMinScalar(inset, devRect.height());

    if (viewMatrix.rectStaysRect()) {
        set_inset_fan(fan0Pos, vertexStride, devRect, -SK_ScalarHalf, -SK_ScalarHalf);
        set_inset_fan(fan1Pos, vertexStride, devRect, inset,  inset);
    } else {
        // compute transformed (1, 0) and (0, 1) vectors
        SkVector vec[2] = {
          { viewMatrix[SkMatrix::kMScaleX], viewMatrix[SkMatrix::kMSkewY] },
          { viewMatrix[SkMatrix::kMSkewX],  viewMatrix[SkMatrix::kMScaleY] }
        };

        vec[0].normalize();
        vec[0].scale(SK_ScalarHalf);
        vec[1].normalize();
        vec[1].scale(SK_ScalarHalf);

        // create the rotated rect
        fan0Pos->setRectFan(rect.fLeft, rect.fTop,
                            rect.fRight, rect.fBottom, vertexStride);
        viewMatrix.mapPointsWithStride(fan0Pos, vertexStride, 4);

        // Now create the inset points and then outset the original
        // rotated points

        // TL
        *((SkPoint*)((intptr_t)fan1Pos + 0 * vertexStride)) =
            *((SkPoint*)((intptr_t)fan0Pos + 0 * vertexStride)) + vec[0] + vec[1];
        *((SkPoint*)((intptr_t)fan0Pos + 0 * vertexStride)) -= vec[0] + vec[1];
        // BL
        *((SkPoint*)((intptr_t)fan1Pos + 1 * vertexStride)) =
            *((SkPoint*)((intptr_t)fan0Pos + 1 * vertexStride)) + vec[0] - vec[1];
        *((SkPoint*)((intptr_t)fan0Pos + 1 * vertexStride)) -= vec[0] - vec[1];
        // BR
        *((SkPoint*)((intptr_t)fan1Pos + 2 * vertexStride)) =
            *((SkPoint*)((intptr_t)fan0Pos + 2 * vertexStride)) - vec[0] - vec[1];
        *((SkPoint*)((intptr_t)fan0Pos + 2 * vertexStride)) += vec[0] + vec[1];
        // TR
        *((SkPoint*)((intptr_t)fan1Pos + 3 * vertexStride)) =
            *((SkPoint*)((intptr_t)fan0Pos + 3 * vertexStride)) - vec[0] + vec[1];
        *((SkPoint*)((intptr_t)fan0Pos + 3 * vertexStride)) += vec[0] - vec[1];
    }

    if (localMatrix) {
        SkMatrix invViewMatrix;
        if (!viewMatrix.invert(&invViewMatrix)) {
            SkASSERT(false);
            invViewMatrix = SkMatrix::I();
        }
        SkMatrix localCoordMatrix;
        localCoordMatrix.setConcat(*localMatrix, invViewMatrix);
        SkPoint* fan0Loc = reinterpret_cast<SkPoint*>(verts + sizeof(SkPoint) + sizeof(GrColor));
        localCoordMatrix.mapPointsWithStride(fan0Loc, fan0Pos, vertexStride, 8);
    }

    bool tweakAlphaForCoverage = overrides.canTweakAlphaForCoverage();

    // Make verts point to vertex color and then set all the color and coverage vertex attrs
    // values.
    verts += sizeof(SkPoint);

    // The coverage offset is always the last vertex attribute
    intptr_t coverageOffset = vertexStride - sizeof(GrColor) - sizeof(SkPoint);
    for (int i = 0; i < 4; ++i) {
        if (tweakAlphaForCoverage) {
            *reinterpret_cast<GrColor*>(verts + i * vertexStride) = 0;
        } else {
            *reinterpret_cast<GrColor*>(verts + i * vertexStride) = color;
            *reinterpret_cast<float*>(verts + i * vertexStride + coverageOffset) = 0;
        }
    }

    int scale;
    if (inset < SK_ScalarHalf) {
        scale = SkScalarFloorToInt(512.0f * inset / (inset + SK_ScalarHalf));
        SkASSERT(scale >= 0 && scale <= 255);
    } else {
        scale = 0xff;
    }

    verts += 4 * vertexStride;

    float innerCoverage = GrNormalizeByteToFloat(scale);
    GrColor scaledColor = (0xff == scale) ? color : SkAlphaMulQ(color, scale);

    for (int i = 0; i < 4; ++i) {
        if (tweakAlphaForCoverage) {
            *reinterpret_cast<GrColor*>(verts + i * vertexStride) = scaledColor;
        } else {
            *reinterpret_cast<GrColor*>(verts + i * vertexStride) = color;
            *reinterpret_cast<float*>(verts + i * vertexStride +
                                      coverageOffset) = innerCoverage;
        }
    }
}

// Common functions
class AAFillRectBatchBase {
public:
    static const int kVertsPerInstance = kVertsPerAAFillRect;
    static const int kIndicesPerInstance = kIndicesPerAAFillRect;

    static void InitInvariantOutputCoverage(GrInitInvariantOutput* out) {
        out->setUnknownSingleComponent();
    }

    static const GrBuffer* GetIndexBuffer(GrResourceProvider* rp) {
        return get_index_buffer(rp);
    }

    template <class Geometry>
    static void SetBounds(const Geometry& geo, SkRect* outBounds) {
        *outBounds = geo.fDevRect;
    }

    template <class Geometry>
    static void UpdateBoundsAfterAppend(const Geometry& geo, SkRect* outBounds) {
        outBounds->join(geo.fDevRect);
    }
};

class AAFillRectBatchNoLocalMatrixImp : public AAFillRectBatchBase {
public:
    struct Geometry {
        SkMatrix fViewMatrix;
        SkRect fRect;
        SkRect fDevRect;
        GrColor fColor;
    };

    static const char* Name() { return "AAFillRectBatchNoLocalMatrix"; }

    static SkString DumpInfo(const Geometry& geo, int index) {
        SkString str;
        str.appendf("%d: Color: 0x%08x, Rect [L: %.2f, T: %.2f, R: %.2f, B: %.2f]\n",
                    index,
                    geo.fColor,
                    geo.fRect.fLeft, geo.fRect.fTop, geo.fRect.fRight, geo.fRect.fBottom);
        return str;
    }

    static bool CanCombine(const Geometry& mine, const Geometry& theirs,
                           const GrXPOverridesForBatch& overrides) {
        // We apply the viewmatrix to the rect points on the cpu.  However, if the pipeline uses
        // local coords then we won't be able to batch.  We could actually upload the viewmatrix
        // using vertex attributes in these cases, but haven't investigated that
        return !overrides.readsLocalCoords() || mine.fViewMatrix.cheapEqualTo(theirs.fViewMatrix);
    }

    static const GrGeometryProcessor* CreateGP(const Geometry& geo,
                                               const GrXPOverridesForBatch& overrides) {
        const GrGeometryProcessor* gp =
                create_fill_rect_gp(geo.fViewMatrix, overrides,
                                    GrDefaultGeoProcFactory::LocalCoords::kUsePosition_Type);

        SkASSERT(overrides.canTweakAlphaForCoverage() ?
                 gp->getVertexStride() == sizeof(GrDefaultGeoProcFactory::PositionColorAttr) :
                 gp->getVertexStride() ==
                         sizeof(GrDefaultGeoProcFactory::PositionColorCoverageAttr));
        return gp;
    }

    static void Tesselate(intptr_t vertices, size_t vertexStride, const Geometry& geo,
                          const GrXPOverridesForBatch& overrides) {
        generate_aa_fill_rect_geometry(vertices, vertexStride,
                                       geo.fColor, geo.fViewMatrix, geo.fRect, geo.fDevRect,
                                       overrides, nullptr);
    }
};

class AAFillRectBatchLocalMatrixImp : public AAFillRectBatchBase {
public:
    struct Geometry {
        SkMatrix fViewMatrix;
        SkMatrix fLocalMatrix;
        SkRect fRect;
        SkRect fDevRect;
        GrColor fColor;
    };

    static const char* Name() { return "AAFillRectBatchLocalMatrix"; }

    static SkString DumpInfo(const Geometry& geo, int index) {
        SkString str;
        str.appendf("%d: Color: 0x%08x, Rect [L: %.2f, T: %.2f, R: %.2f, B: %.2f]\n",
                    index,
                    geo.fColor,
                    geo.fRect.fLeft, geo.fRect.fTop, geo.fRect.fRight, geo.fRect.fBottom);
        return str;
    }

    static bool CanCombine(const Geometry& mine, const Geometry& theirs,
                           const GrXPOverridesForBatch& overrides) {
        return true;
    }

    static const GrGeometryProcessor* CreateGP(const Geometry& geo,
                                               const GrXPOverridesForBatch& overrides) {
        const GrGeometryProcessor* gp =
                create_fill_rect_gp(geo.fViewMatrix, overrides,
                                    GrDefaultGeoProcFactory::LocalCoords::kHasExplicit_Type);

        SkASSERT(overrides.canTweakAlphaForCoverage() ?
                 gp->getVertexStride() ==
                         sizeof(GrDefaultGeoProcFactory::PositionColorLocalCoordAttr) :
                 gp->getVertexStride() ==
                         sizeof(GrDefaultGeoProcFactory::PositionColorLocalCoordCoverage));
        return gp;
    }

    static void Tesselate(intptr_t vertices, size_t vertexStride, const Geometry& geo,
                          const GrXPOverridesForBatch& overrides) {
        generate_aa_fill_rect_geometry(vertices, vertexStride,
                                       geo.fColor, geo.fViewMatrix, geo.fRect, geo.fDevRect,
                                       overrides, &geo.fLocalMatrix);
    }
};

typedef GrTInstanceBatch<AAFillRectBatchNoLocalMatrixImp> AAFillRectBatchNoLocalMatrix;
typedef GrTInstanceBatch<AAFillRectBatchLocalMatrixImp> AAFillRectBatchLocalMatrix;

inline static void append_to_batch(AAFillRectBatchNoLocalMatrix* batch, GrColor color,
                                   const SkMatrix& viewMatrix, const SkRect& rect,
                                   const SkRect& devRect) {
    AAFillRectBatchNoLocalMatrix::Geometry& geo = batch->geoData()->push_back();
    geo.fColor = color;
    geo.fViewMatrix = viewMatrix;
    geo.fRect = rect;
    geo.fDevRect = devRect;
}

inline static void append_to_batch(AAFillRectBatchLocalMatrix* batch, GrColor color,
                                   const SkMatrix& viewMatrix, const SkMatrix& localMatrix,
                                   const SkRect& rect, const SkRect& devRect) {
    AAFillRectBatchLocalMatrix::Geometry& geo = batch->geoData()->push_back();
    geo.fColor = color;
    geo.fViewMatrix = viewMatrix;
    geo.fLocalMatrix = localMatrix;
    geo.fRect = rect;
    geo.fDevRect = devRect;
}

namespace GrAAFillRectBatch {

GrDrawBatch* Create(GrColor color,
                    const SkMatrix& viewMatrix,
                    const SkRect& rect,
                    const SkRect& devRect) {
    AAFillRectBatchNoLocalMatrix* batch = AAFillRectBatchNoLocalMatrix::Create();
    append_to_batch(batch, color, viewMatrix, rect, devRect);
    batch->init();
    return batch;
}

GrDrawBatch* Create(GrColor color,
                    const SkMatrix& viewMatrix,
                    const SkMatrix& localMatrix,
                    const SkRect& rect,
                    const SkRect& devRect) {
    AAFillRectBatchLocalMatrix* batch = AAFillRectBatchLocalMatrix::Create();
    append_to_batch(batch, color, viewMatrix, localMatrix, rect, devRect);
    batch->init();
    return batch;
}

GrDrawBatch* Create(GrColor color,
                    const SkMatrix& viewMatrix,
                    const SkMatrix& localMatrix,
                    const SkRect& rect) {
    SkRect devRect;
    viewMatrix.mapRect(&devRect, rect);
    return Create(color, viewMatrix, localMatrix, rect, devRect);
}

GrDrawBatch* CreateWithLocalRect(GrColor color,
                                 const SkMatrix& viewMatrix,
                                 const SkRect& rect,
                                 const SkRect& localRect) {
    SkRect devRect;
    viewMatrix.mapRect(&devRect, rect);
    SkMatrix localMatrix;
    if (!localMatrix.setRectToRect(rect, localRect, SkMatrix::kFill_ScaleToFit)) {
        return nullptr;
    }
    return Create(color, viewMatrix, localMatrix, rect, devRect);
}

void Append(GrBatch* origBatch,
            GrColor color,
            const SkMatrix& viewMatrix,
            const SkRect& rect,
            const SkRect& devRect) {
    AAFillRectBatchNoLocalMatrix* batch = origBatch->cast<AAFillRectBatchNoLocalMatrix>();
    append_to_batch(batch, color, viewMatrix, rect, devRect);
    batch->updateBoundsAfterAppend();
}

void Append(GrBatch* origBatch,
            GrColor color,
            const SkMatrix& viewMatrix,
            const SkMatrix& localMatrix,
            const SkRect& rect,
            const SkRect& devRect) {
    AAFillRectBatchLocalMatrix* batch = origBatch->cast<AAFillRectBatchLocalMatrix>();
    append_to_batch(batch, color, viewMatrix, localMatrix, rect, devRect);
    batch->updateBoundsAfterAppend();
}

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GR_TEST_UTILS

#include "GrBatchTest.h"

DRAW_BATCH_TEST_DEFINE(AAFillRectBatch) {
    GrColor color = GrRandomColor(random);
    SkMatrix viewMatrix = GrTest::TestMatrixInvertible(random);
    SkRect rect = GrTest::TestRect(random);
    SkRect devRect = GrTest::TestRect(random);
    return GrAAFillRectBatch::Create(color, viewMatrix, rect, devRect);
}

DRAW_BATCH_TEST_DEFINE(AAFillRectBatchLocalMatrix) {
    GrColor color = GrRandomColor(random);
    SkMatrix viewMatrix = GrTest::TestMatrixInvertible(random);
    SkMatrix localMatrix = GrTest::TestMatrix(random);
    SkRect rect = GrTest::TestRect(random);
    SkRect devRect = GrTest::TestRect(random);
    return GrAAFillRectBatch::Create(color, viewMatrix, localMatrix, rect, devRect);
}

#endif
