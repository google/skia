/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAAFillRectOp.h"

#include "GrColor.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrMeshDrawOp.h"
#include "GrOpFlushState.h"
#include "GrResourceKey.h"
#include "GrResourceProvider.h"
#include "GrTypes.h"
#include "SkMatrix.h"
#include "SkRect.h"

GR_DECLARE_STATIC_UNIQUE_KEY(gAAFillRectIndexBufferKey);

static void set_inset_fan(SkPoint* pts, size_t stride, const SkRect& r, SkScalar dx, SkScalar dy) {
    pts->setRectFan(r.fLeft + dx, r.fTop + dy, r.fRight - dx, r.fBottom - dy, stride);
}

static const int kNumAAFillRectsInIndexBuffer = 256;
static const int kVertsPerAAFillRect = 8;
static const int kIndicesPerAAFillRect = 30;

const GrBuffer* get_index_buffer(GrResourceProvider* resourceProvider) {
    GR_DEFINE_STATIC_UNIQUE_KEY(gAAFillRectIndexBufferKey);

    // clang-format off
    static const uint16_t gFillAARectIdx[] = {
        0, 1, 5, 5, 4, 0,
        1, 2, 6, 6, 5, 1,
        2, 3, 7, 7, 6, 2,
        3, 0, 4, 4, 7, 3,
        4, 5, 6, 6, 7, 4,
    };
    // clang-format on

    GR_STATIC_ASSERT(SK_ARRAY_COUNT(gFillAARectIdx) == kIndicesPerAAFillRect);
    return resourceProvider->findOrCreatePatternedIndexBuffer(
            gFillAARectIdx, kIndicesPerAAFillRect, kNumAAFillRectsInIndexBuffer,
            kVertsPerAAFillRect, gAAFillRectIndexBufferKey);
}

static void generate_aa_fill_rect_geometry(intptr_t verts,
                                           size_t vertexStride,
                                           GrColor color,
                                           const SkMatrix& viewMatrix,
                                           const SkRect& rect,
                                           const SkRect& devRect,
                                           bool tweakAlphaForCoverage,
                                           const SkMatrix* localMatrix) {
    SkPoint* fan0Pos = reinterpret_cast<SkPoint*>(verts);
    SkPoint* fan1Pos = reinterpret_cast<SkPoint*>(verts + 4 * vertexStride);

    SkScalar inset;

    if (viewMatrix.rectStaysRect()) {
        inset = SkMinScalar(devRect.width(), SK_Scalar1);
        inset = SK_ScalarHalf * SkMinScalar(inset, devRect.height());

        set_inset_fan(fan0Pos, vertexStride, devRect, -SK_ScalarHalf, -SK_ScalarHalf);
        set_inset_fan(fan1Pos, vertexStride, devRect, inset, inset);
    } else {
        // compute transformed (1, 0) and (0, 1) vectors
        SkVector vec[2] = {{viewMatrix[SkMatrix::kMScaleX], viewMatrix[SkMatrix::kMSkewY]},
                           {viewMatrix[SkMatrix::kMSkewX], viewMatrix[SkMatrix::kMScaleY]}};

        SkScalar len1 = SkPoint::Normalize(&vec[0]);
        vec[0].scale(SK_ScalarHalf);
        SkScalar len2 = SkPoint::Normalize(&vec[1]);
        vec[1].scale(SK_ScalarHalf);

        inset = SkMinScalar(len1 * rect.width(), SK_Scalar1);
        inset = SK_ScalarHalf * SkMinScalar(inset, len2 * rect.height());

        // create the rotated rect
        fan0Pos->setRectFan(rect.fLeft, rect.fTop, rect.fRight, rect.fBottom, vertexStride);
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
            SkDebugf("View matrix is non-invertible, local coords will be wrong.");
            invViewMatrix = SkMatrix::I();
        }
        SkMatrix localCoordMatrix;
        localCoordMatrix.setConcat(*localMatrix, invViewMatrix);
        SkPoint* fan0Loc = reinterpret_cast<SkPoint*>(verts + sizeof(SkPoint) + sizeof(GrColor));
        localCoordMatrix.mapPointsWithStride(fan0Loc, fan0Pos, vertexStride, 8);
    }

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
            *reinterpret_cast<float*>(verts + i * vertexStride + coverageOffset) = innerCoverage;
        }
    }
}

class AAFillRectOp final : public GrLegacyMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID

    AAFillRectOp(GrColor color,
                 const SkMatrix& viewMatrix,
                 const SkRect& rect,
                 const SkRect& devRect,
                 const SkMatrix* localMatrix)
            : INHERITED(ClassID()) {
        if (localMatrix) {
            void* mem = fRectData.push_back_n(sizeof(RectWithLocalMatrixInfo));
            new (mem) RectWithLocalMatrixInfo(color, viewMatrix, rect, devRect, *localMatrix);
        } else {
            void* mem = fRectData.push_back_n(sizeof(RectInfo));
            new (mem) RectInfo(color, viewMatrix, rect, devRect);
        }
        IsZeroArea zeroArea =
                (!rect.width() || !rect.height()) ? IsZeroArea::kYes : IsZeroArea::kNo;
        this->setBounds(devRect, HasAABloat::kYes, zeroArea);
        fRectCnt = 1;
    }

    const char* name() const override { return "AAFillRectOp"; }

    SkString dumpInfo() const override {
        SkString str;
        str.append(INHERITED::dumpInfo());
        str.appendf("# combined: %d\n", fRectCnt);
        const RectInfo* info = this->first();
        for (int i = 0; i < fRectCnt; ++i) {
            const SkRect& rect = info->rect();
            str.appendf("%d: Color: 0x%08x, Rect [L: %.2f, T: %.2f, R: %.2f, B: %.2f]\n", i,
                        info->color(), rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
            info = this->next(info);
        }
        str.append(DumpPipelineInfo(*this->pipeline()));
        return str;
    }

    void applyPipelineOptimizations(const PipelineOptimizations& optimizations) override {
        GrColor color;
        if (optimizations.getOverrideColorIfSet(&color)) {
            this->first()->setColor(color);
        }
        fCanTweakAlphaForCoverage = optimizations.canTweakAlphaForCoverage();
        fNeedsLocalCoords = optimizations.readsLocalCoords();
    }

private:
    void getProcessorAnalysisInputs(GrProcessorAnalysisColor* color,
                                    GrProcessorAnalysisCoverage* coverage) const override {
        color->setToConstant(this->first()->color());
        *coverage = GrProcessorAnalysisCoverage::kSingleChannel;
    }

    void onPrepareDraws(Target* target) const override {
        using namespace GrDefaultGeoProcFactory;

        Color color(Color::kPremulGrColorAttribute_Type);
        Coverage::Type coverageType;
        if (fCanTweakAlphaForCoverage) {
            coverageType = Coverage::kSolid_Type;
        } else {
            coverageType = Coverage::kAttribute_Type;
        }
        LocalCoords lc =
                fNeedsLocalCoords ? LocalCoords::kHasExplicit_Type : LocalCoords::kUnused_Type;
        sk_sp<GrGeometryProcessor> gp =
                GrDefaultGeoProcFactory::Make(color, coverageType, lc, SkMatrix::I());
        if (!gp) {
            SkDebugf("Couldn't create GrGeometryProcessor\n");
            return;
        }

        size_t vertexStride = gp->getVertexStride();

        sk_sp<const GrBuffer> indexBuffer(get_index_buffer(target->resourceProvider()));
        PatternHelper helper(kTriangles_GrPrimitiveType);
        void* vertices =
                helper.init(target, vertexStride, indexBuffer.get(), kVertsPerAAFillRect,
                            kIndicesPerAAFillRect, fRectCnt);
        if (!vertices || !indexBuffer) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        const RectInfo* info = this->first();
        const SkMatrix* localMatrix = nullptr;
        for (int i = 0; i < fRectCnt; i++) {
            intptr_t verts =
                    reinterpret_cast<intptr_t>(vertices) + i * kVertsPerAAFillRect * vertexStride;
            if (fNeedsLocalCoords) {
                if (info->hasLocalMatrix()) {
                    localMatrix = &static_cast<const RectWithLocalMatrixInfo*>(info)->localMatrix();
                } else {
                    localMatrix = &SkMatrix::I();
                }
            }
            generate_aa_fill_rect_geometry(verts, vertexStride, info->color(), info->viewMatrix(),
                                           info->rect(), info->devRect(), fCanTweakAlphaForCoverage,
                                           localMatrix);
            info = this->next(info);
        }
        helper.recordDraw(target, gp.get(), this->pipeline());
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        AAFillRectOp* that = t->cast<AAFillRectOp>();
        if (!GrPipeline::CanCombine(*this->pipeline(), this->bounds(), *that->pipeline(),
                                    that->bounds(), caps)) {
            return false;
        }

        SkASSERT(fNeedsLocalCoords == that->fNeedsLocalCoords);

        // In the event of two ops, one who can tweak, one who cannot, we just fall back to not
        // tweaking.
        if (fCanTweakAlphaForCoverage && !that->fCanTweakAlphaForCoverage) {
            fCanTweakAlphaForCoverage = false;
        }

        fRectData.push_back_n(that->fRectData.count(), that->fRectData.begin());
        fRectCnt += that->fRectCnt;
        this->joinBounds(*that);
        return true;
    }

    struct RectInfo {
    public:
        RectInfo(GrColor color, const SkMatrix& viewMatrix, const SkRect& rect,
                 const SkRect& devRect)
                : RectInfo(color, viewMatrix, rect, devRect, HasLocalMatrix::kNo) {}
        bool hasLocalMatrix() const { return HasLocalMatrix::kYes == fHasLocalMatrix; }
        GrColor color() const { return fColor; }
        const SkMatrix& viewMatrix() const { return fViewMatrix; }
        const SkRect& rect() const { return fRect; }
        const SkRect& devRect() const { return fDevRect; }

        void setColor(GrColor color) { fColor = color; }

    protected:
        enum class HasLocalMatrix : uint32_t { kNo, kYes };

        RectInfo(GrColor color, const SkMatrix& viewMatrix, const SkRect& rect,
                 const SkRect& devRect, HasLocalMatrix hasLM)
                : fHasLocalMatrix(hasLM)
                , fColor(color)
                , fViewMatrix(viewMatrix)
                , fRect(rect)
                , fDevRect(devRect) {}

        HasLocalMatrix fHasLocalMatrix;
        GrColor fColor;
        SkMatrix fViewMatrix;
        SkRect fRect;
        SkRect fDevRect;
    };

    struct RectWithLocalMatrixInfo : public RectInfo {
    public:
        RectWithLocalMatrixInfo(GrColor color, const SkMatrix& viewMatrix, const SkRect& rect,
                                const SkRect& devRect, const SkMatrix& localMatrix)
                : RectInfo(color, viewMatrix, rect, devRect, HasLocalMatrix::kYes)
                , fLocalMatrix(localMatrix) {}
        const SkMatrix& localMatrix() const { return fLocalMatrix; }

    private:
        SkMatrix fLocalMatrix;
    };

    RectInfo* first() { return reinterpret_cast<RectInfo*>(fRectData.begin()); }
    const RectInfo* first() const { return reinterpret_cast<const RectInfo*>(fRectData.begin()); }
    const RectInfo* next(const RectInfo* prev) const {
        intptr_t next =
                reinterpret_cast<intptr_t>(prev) +
                (prev->hasLocalMatrix() ? sizeof(RectWithLocalMatrixInfo) : sizeof(RectInfo));
        return reinterpret_cast<const RectInfo*>(next);
    }

    bool fNeedsLocalCoords;
    bool fCanTweakAlphaForCoverage;
    SkSTArray<4 * sizeof(RectWithLocalMatrixInfo), uint8_t, true> fRectData;
    int fRectCnt;

    typedef GrLegacyMeshDrawOp INHERITED;
};

namespace GrAAFillRectOp {

std::unique_ptr<GrLegacyMeshDrawOp> Make(GrColor color,
                                         const SkMatrix& viewMatrix,
                                         const SkRect& rect,
                                         const SkRect& devRect) {
    return std::unique_ptr<GrLegacyMeshDrawOp>(
            new AAFillRectOp(color, viewMatrix, rect, devRect, nullptr));
}

std::unique_ptr<GrLegacyMeshDrawOp> Make(GrColor color,
                                         const SkMatrix& viewMatrix,
                                         const SkMatrix& localMatrix,
                                         const SkRect& rect,
                                         const SkRect& devRect) {
    return std::unique_ptr<GrLegacyMeshDrawOp>(
            new AAFillRectOp(color, viewMatrix, rect, devRect, &localMatrix));
}

std::unique_ptr<GrLegacyMeshDrawOp> Make(GrColor color,
                                         const SkMatrix& viewMatrix,
                                         const SkMatrix& localMatrix,
                                         const SkRect& rect) {
    SkRect devRect;
    viewMatrix.mapRect(&devRect, rect);
    return Make(color, viewMatrix, localMatrix, rect, devRect);
}

std::unique_ptr<GrLegacyMeshDrawOp> MakeWithLocalRect(GrColor color,
                                                      const SkMatrix& viewMatrix,
                                                      const SkRect& rect,
                                                      const SkRect& localRect) {
    SkRect devRect;
    viewMatrix.mapRect(&devRect, rect);
    SkMatrix localMatrix;
    if (!localMatrix.setRectToRect(rect, localRect, SkMatrix::kFill_ScaleToFit)) {
        return nullptr;
    }
    return Make(color, viewMatrix, localMatrix, rect, devRect);
}
};

///////////////////////////////////////////////////////////////////////////////////////////////////

#if GR_TEST_UTILS

#include "GrDrawOpTest.h"

GR_LEGACY_MESH_DRAW_OP_TEST_DEFINE(AAFillRectOp) {
    GrColor color = GrRandomColor(random);
    SkMatrix viewMatrix = GrTest::TestMatrixInvertible(random);
    SkRect rect = GrTest::TestRect(random);
    SkRect devRect = GrTest::TestRect(random);
    return GrAAFillRectOp::Make(color, viewMatrix, rect, devRect);
}

GR_LEGACY_MESH_DRAW_OP_TEST_DEFINE(AAFillRectOpLocalMatrix) {
    GrColor color = GrRandomColor(random);
    SkMatrix viewMatrix = GrTest::TestMatrixInvertible(random);
    SkMatrix localMatrix = GrTest::TestMatrix(random);
    SkRect rect = GrTest::TestRect(random);
    SkRect devRect = GrTest::TestRect(random);
    return GrAAFillRectOp::Make(color, viewMatrix, localMatrix, rect, devRect);
}

#endif
