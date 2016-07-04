/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAAFillRectBatch.h"

#include "GrBatchFlushState.h"
#include "GrColor.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrResourceKey.h"
#include "GrResourceProvider.h"
#include "GrTypes.h"
#include "SkMatrix.h"
#include "SkRect.h"
#include "GrVertexBatch.h"

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

static sk_sp<GrGeometryProcessor> create_fill_rect_gp(
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
        return GrDefaultGeoProcFactory::Make(color, coverage, localCoords, SkMatrix::I());
    } else {
        LocalCoords localCoords(overrides.readsLocalCoords() ? localCoordsType :
                                                               LocalCoords::kUnused_Type);
        return MakeForDeviceSpace(color, coverage, localCoords, viewMatrix);
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

    SkScalar inset;

    if (viewMatrix.rectStaysRect()) {
        inset = SkMinScalar(devRect.width(), SK_Scalar1);
        inset = SK_ScalarHalf * SkMinScalar(inset, devRect.height());

        set_inset_fan(fan0Pos, vertexStride, devRect, -SK_ScalarHalf, -SK_ScalarHalf);
        set_inset_fan(fan1Pos, vertexStride, devRect, inset,  inset);
    } else {
        // compute transformed (1, 0) and (0, 1) vectors
        SkVector vec[2] = {
          { viewMatrix[SkMatrix::kMScaleX], viewMatrix[SkMatrix::kMSkewY] },
          { viewMatrix[SkMatrix::kMSkewX],  viewMatrix[SkMatrix::kMScaleY] }
        };

        SkScalar len1 = SkPoint::Normalize(&vec[0]);
        vec[0].scale(SK_ScalarHalf);
        SkScalar len2 = SkPoint::Normalize(&vec[1]);
        vec[1].scale(SK_ScalarHalf);

        inset = SkMinScalar(len1 * rect.width(), SK_Scalar1);
        inset = SK_ScalarHalf * SkMinScalar(inset, len2 * rect.height());

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

class AAFillRectNoLocalMatrixBatch : public GrVertexBatch {
public:
    DEFINE_BATCH_CLASS_ID
    AAFillRectNoLocalMatrixBatch(GrColor color,
                                 const SkMatrix& viewMatrix,
                                 const SkRect& rect,
                                 const SkRect& devRect) : INHERITED(ClassID()) {
        fRects.emplace_back(RectInfo{color, viewMatrix, rect, devRect});
        fBounds = devRect;
    }

    const char* name() const override { return "AAFillRectBatchNoLocalMatrix"; }

    SkString dumpInfo() const override {
        SkString str;
        str.appendf("# batched: %d\n", fRects.count());
        for (int i = 0; i < fRects.count(); ++i) {
            const RectInfo& info = fRects[i];
            str.appendf("%d: Color: 0x%08x, Rect [L: %.2f, T: %.2f, R: %.2f, B: %.2f]\n",
                        i, info.fColor,
                        info.fRect.fLeft, info.fRect.fTop, info.fRect.fRight, info.fRect.fBottom);
        }
        str.append(INHERITED::dumpInfo());
        return str;
    }

    void computePipelineOptimizations(GrInitInvariantOutput* color,
                                      GrInitInvariantOutput* coverage,
                                      GrBatchToXPOverrides* overrides) const override {
        // When this is called on a batch, there is only one rect
        color->setKnownFourComponents(fRects[0].fColor);
        coverage->setUnknownSingleComponent();
    }

    void initBatchTracker(const GrXPOverridesForBatch& overrides) override {
        overrides.getOverrideColorIfSet(&fRects[0].fColor);
        fOverrides = overrides;
    }

private:
    AAFillRectNoLocalMatrixBatch() : INHERITED(ClassID()) {}

    void onPrepareDraws(Target* target) const override {
        sk_sp<GrGeometryProcessor> gp =
                create_fill_rect_gp(fRects[0].fViewMatrix, fOverrides,
                                    GrDefaultGeoProcFactory::LocalCoords::kUsePosition_Type);
        if (!gp) {
            SkDebugf("Couldn't create GrGeometryProcessor\n");
            return;
        }
        SkASSERT(fOverrides.canTweakAlphaForCoverage() ?
                     gp->getVertexStride() == sizeof(GrDefaultGeoProcFactory::PositionColorAttr) :
                     gp->getVertexStride() ==
                         sizeof(GrDefaultGeoProcFactory::PositionColorCoverageAttr));

        size_t vertexStride = gp->getVertexStride();

        SkAutoTUnref<const GrBuffer> indexBuffer(get_index_buffer(target->resourceProvider()));
        InstancedHelper helper;
        void* vertices = helper.init(target, kTriangles_GrPrimitiveType, vertexStride,
                                     indexBuffer, kVertsPerAAFillRect,
                                     kIndicesPerAAFillRect, fRects.count());
        if (!vertices || !indexBuffer) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        for (int i = 0; i < fRects.count(); i++) {
            intptr_t verts = reinterpret_cast<intptr_t>(vertices) +
                             i * kVertsPerAAFillRect * vertexStride;
            generate_aa_fill_rect_geometry(verts, vertexStride,
                                           fRects[i].fColor, fRects[i].fViewMatrix,
                                           fRects[i].fRect, fRects[i].fDevRect, fOverrides,
                                           nullptr);
        }
        helper.recordDraw(target, gp.get());
    }

    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override {
        AAFillRectNoLocalMatrixBatch* that = t->cast<AAFillRectNoLocalMatrixBatch>();
        if (!GrPipeline::CanCombine(*this->pipeline(), this->bounds(), *that->pipeline(),
                                    that->bounds(), caps)) {
            return false;
        }

        // We apply the viewmatrix to the rect points on the cpu.  However, if the pipeline uses
        // local coords then we won't be able to batch.  We could actually upload the viewmatrix
        // using vertex attributes in these cases, but haven't investigated that
        if (fOverrides.readsLocalCoords() &&
            !fRects[0].fViewMatrix.cheapEqualTo(that->fRects[0].fViewMatrix)) {
            return false;
        }

        // In the event of two batches, one who can tweak, one who cannot, we just fall back to
        // not tweaking
        if (fOverrides.canTweakAlphaForCoverage() && !that->fOverrides.canTweakAlphaForCoverage()) {
            fOverrides = that->fOverrides;
        }

        fRects.push_back_n(that->fRects.count(), that->fRects.begin());
        this->joinBounds(that->bounds());
        return true;
    }

    struct RectInfo {
        GrColor fColor;
        SkMatrix fViewMatrix;
        SkRect fRect;
        SkRect fDevRect;
    };

    GrXPOverridesForBatch fOverrides;
    SkSTArray<1, RectInfo, true> fRects;

    typedef GrVertexBatch INHERITED;
};

class AAFillRectLocalMatrixBatch : public GrVertexBatch {
public:
    DEFINE_BATCH_CLASS_ID

    AAFillRectLocalMatrixBatch(GrColor color,
                               const SkMatrix& viewMatrix,
                               const SkMatrix& localMatrix,
                               const SkRect& rect,
                               const SkRect& devRect) : INHERITED(ClassID()) {
        fRects.emplace_back(RectInfo{color, viewMatrix, localMatrix, rect, devRect});
        fBounds = devRect;
    }

    const char* name() const override { return "AAFillRectBatchLocalMatrix"; }

    SkString dumpInfo() const override {
        SkString str;
        str.appendf("# batched: %d\n", fRects.count());
        for (int i = 0; i < fRects.count(); ++i) {
            const RectInfo& info = fRects[i];
            str.appendf("%d: Color: 0x%08x, Rect [L: %.2f, T: %.2f, R: %.2f, B: %.2f]\n",
                        i, info.fColor,
                        info.fRect.fLeft, info.fRect.fTop, info.fRect.fRight, info.fRect.fBottom);
        }
        str.append(INHERITED::dumpInfo());
        return str;
    }

    void computePipelineOptimizations(GrInitInvariantOutput* color,
                                      GrInitInvariantOutput* coverage,
                                      GrBatchToXPOverrides* overrides) const override {
        // When this is called on a batch, there is only one rect
        color->setKnownFourComponents(fRects[0].fColor);
        coverage->setUnknownSingleComponent();
    }

    void initBatchTracker(const GrXPOverridesForBatch& overrides) override {
        overrides.getOverrideColorIfSet(&fRects[0].fColor);
        fOverrides = overrides;
    }

private:
    AAFillRectLocalMatrixBatch() : INHERITED(ClassID()) {}

    void onPrepareDraws(Target* target) const override {
        sk_sp<GrGeometryProcessor> gp =
                create_fill_rect_gp(fRects[0].fViewMatrix, fOverrides,
                                    GrDefaultGeoProcFactory::LocalCoords::kHasExplicit_Type);
        if (!gp) {
            SkDebugf("Couldn't create GrGeometryProcessor\n");
            return;
        }
        SkASSERT(fOverrides.canTweakAlphaForCoverage() ?
                 gp->getVertexStride() ==
                 sizeof(GrDefaultGeoProcFactory::PositionColorLocalCoordAttr) :
                 gp->getVertexStride() ==
                 sizeof(GrDefaultGeoProcFactory::PositionColorLocalCoordCoverage));

        size_t vertexStride = gp->getVertexStride();

        SkAutoTUnref<const GrBuffer> indexBuffer(get_index_buffer(target->resourceProvider()));
        InstancedHelper helper;
        void* vertices = helper.init(target, kTriangles_GrPrimitiveType, vertexStride,
                                     indexBuffer, kVertsPerAAFillRect,
                                     kIndicesPerAAFillRect, fRects.count());
        if (!vertices || !indexBuffer) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        for (int i = 0; i < fRects.count(); i++) {
            intptr_t verts = reinterpret_cast<intptr_t>(vertices) +
                             i * kVertsPerAAFillRect * vertexStride;
            generate_aa_fill_rect_geometry(verts, vertexStride, fRects[i].fColor,
                                           fRects[i].fViewMatrix, fRects[i].fRect,
                                           fRects[i].fDevRect, fOverrides,
                                           &fRects[i].fLocalMatrix);
        }
        helper.recordDraw(target, gp.get());
    }

    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override {
        AAFillRectLocalMatrixBatch* that = t->cast<AAFillRectLocalMatrixBatch>();
        if (!GrPipeline::CanCombine(*this->pipeline(), this->bounds(), *that->pipeline(),
                                    that->bounds(), caps)) {
            return false;
        }

        // In the event of two batches, one who can tweak, one who cannot, we just fall back to
        // not tweaking
        if (fOverrides.canTweakAlphaForCoverage() && !that->fOverrides.canTweakAlphaForCoverage()) {
            fOverrides = that->fOverrides;
        }

        fRects.push_back_n(that->fRects.count(), that->fRects.begin());
        this->joinBounds(that->bounds());
        return true;
    }

    struct RectInfo {
        GrColor fColor;
        SkMatrix fViewMatrix;
        SkMatrix fLocalMatrix;
        SkRect fRect;
        SkRect fDevRect;
    };

    GrXPOverridesForBatch fOverrides;
    SkSTArray<1, RectInfo, true> fRects;

    typedef GrVertexBatch INHERITED;
};

namespace GrAAFillRectBatch {

GrDrawBatch* Create(GrColor color,
                    const SkMatrix& viewMatrix,
                    const SkRect& rect,
                    const SkRect& devRect) {
    return new AAFillRectNoLocalMatrixBatch(color, viewMatrix, rect, devRect);
}

GrDrawBatch* Create(GrColor color,
                    const SkMatrix& viewMatrix,
                    const SkMatrix& localMatrix,
                    const SkRect& rect,
                    const SkRect& devRect) {
    return new AAFillRectLocalMatrixBatch(color, viewMatrix, localMatrix, rect, devRect);
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
