/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This test only works with the GPU backend.

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkPoint3.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrContext.h"
#include "include/private/GrRecordingContext.h"
#include "include/private/GrSharedEnums.h"
#include "include/private/GrTypesPriv.h"
#include "include/private/SkColorData.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkPointPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrPaint.h"
#include "src/gpu/GrProcessorAnalysis.h"
#include "src/gpu/GrProcessorSet.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrRenderTargetContextPriv.h"
#include "src/gpu/GrUserStencilSettings.h"
#include "src/gpu/effects/GrBezierEffect.h"
#include "src/gpu/effects/GrPorterDuffXferProcessor.h"
#include "src/gpu/geometry/GrPathUtils.h"
#include "src/gpu/ops/GrDrawOp.h"
#include "src/gpu/ops/GrMeshDrawOp.h"
#include "src/gpu/ops/GrOp.h"
#include "src/gpu/ops/GrSimpleMeshDrawOpHelper.h"

#include <memory>
#include <utility>

class GrAppliedClip;

namespace skiagm {

class BezierTestOp : public GrMeshDrawOp {
public:
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }

    GrProcessorSet::Analysis finalize(
            const GrCaps& caps, const GrAppliedClip* clip, bool hasMixedSampledCoverage,
            GrClampType clampType) override {
        return fProcessorSet.finalize(
                fColor, GrProcessorAnalysisCoverage::kSingleChannel, clip,
                &GrUserStencilSettings::kUnused, hasMixedSampledCoverage, caps, clampType, &fColor);
    }

    void visitProxies(const VisitProxyFunc& func) const override {
        fProcessorSet.visitProxies(func);
    }

protected:
    BezierTestOp(GrClipEdgeType et, const SkRect& rect, const SkPMColor4f& color, int32_t classID)
            : INHERITED(classID)
            , fRect(rect)
            , fColor(color)
            , fEdgeType(et)
            , fProcessorSet(SkBlendMode::kSrc) {
        this->setBounds(rect, HasAABloat::kYes, IsHairline::kNo);
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        auto pipeline = GrSimpleMeshDrawOpHelper::CreatePipeline(flushState,
                                                                 std::move(fProcessorSet),
                                                                 GrPipeline::InputFlags::kNone);

        flushState->executeDrawsAndUploadsForMeshDrawOp(this, chainBounds, pipeline);
    }

    GrClipEdgeType edgeType() const { return fEdgeType; }

    const SkRect& rect() const { return fRect; }
    const SkPMColor4f& color() const { return fColor; }

private:
    SkRect fRect;
    SkPMColor4f fColor;
    GrClipEdgeType fEdgeType;
    GrProcessorSet fProcessorSet;

    typedef GrMeshDrawOp INHERITED;
};

/**
 * This GM directly exercises effects that draw Bezier curves in the GPU backend.
 */
class BezierConicTestOp : public BezierTestOp {
public:
    DEFINE_OP_CLASS_ID

    const char* name() const override { return "BezierConicTestOp"; }

    static std::unique_ptr<GrDrawOp> Make(GrRecordingContext* context,
                                          GrClipEdgeType et,
                                          const SkRect& rect,
                                          const SkPMColor4f& color,
                                          const SkMatrix& klm) {
        GrOpMemoryPool* pool = context->priv().opMemoryPool();

        return pool->allocate<BezierConicTestOp>(et, rect, color, klm);
    }

private:
    friend class ::GrOpMemoryPool; // for ctor

    BezierConicTestOp(GrClipEdgeType et, const SkRect& rect,
                      const SkPMColor4f& color, const SkMatrix& klm)
            : INHERITED(et, rect, color, ClassID()), fKLM(klm) {}

    struct Vertex {
        SkPoint fPosition;
        float   fKLM[4]; // The last value is ignored. The effect expects a vec4f.
    };

    void onPrepareDraws(Target* target) override {
        GrGeometryProcessor* gp = GrConicEffect::Make(target->allocator(), this->color(),
                                                      SkMatrix::I(), this->edgeType(),
                                                      target->caps(), SkMatrix::I(), false);
        if (!gp) {
            return;
        }

        SkASSERT(gp->vertexStride() == sizeof(Vertex));
        QuadHelper helper(target, sizeof(Vertex), 1);
        Vertex* verts = reinterpret_cast<Vertex*>(helper.vertices());
        if (!verts) {
            return;
        }
        SkRect rect = this->rect();
        SkPointPriv::SetRectTriStrip(&verts[0].fPosition, rect.fLeft, rect.fTop, rect.fRight,
                                     rect.fBottom, sizeof(Vertex));
        for (int v = 0; v < 4; ++v) {
            SkPoint3 pt3 = {verts[v].fPosition.x(), verts[v].fPosition.y(), 1.f};
            fKLM.mapHomogeneousPoints((SkPoint3* ) verts[v].fKLM, &pt3, 1);
        }

        helper.recordDraw(target, gp);
    }

    SkMatrix fKLM;

    static constexpr int kVertsPerCubic = 4;
    static constexpr int kIndicesPerCubic = 6;

    typedef BezierTestOp INHERITED;
};


/**
 * This GM directly exercises effects that draw Bezier curves in the GPU backend.
 */
class BezierConicEffects : public GpuGM {
public:
    BezierConicEffects() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    static const int kNumConics = 10;
    static const int kCellWidth = 128;
    static const int kCellHeight = 128;

    SkString onShortName() override {
        return SkString("bezier_conic_effects");
    }

    SkISize onISize() override {
        return SkISize::Make(kGrClipEdgeTypeCnt*kCellWidth, kNumConics*kCellHeight);
    }

    void onDraw(GrContext* context, GrRenderTargetContext* renderTargetContext,
                SkCanvas* canvas) override {

        const SkScalar w = kCellWidth, h = kCellHeight;
        const SkPMColor4f kOpaqueBlack = SkPMColor4f::FromBytes_RGBA(0xff000000);

        const SkPoint baseControlPts[kNumConics][3] = {
            { { 0.31f * w, 0.01f * h}, { 0.48f * w, 0.74f * h }, { 0.19f * w, 0.33f * h } },
            { { 0.00f * w, 0.07f * h}, { 0.30f * w, 0.70f * h }, { 0.47f * w, 0.37f * h } },
            { { 0.15f * w, 0.23f * h}, { 0.49f * w, 0.87f * h }, { 0.85f * w, 0.66f * h } },
            { { 0.09f * w, 0.15f * h}, { 0.42f * w, 0.33f * h }, { 0.17f * w, 0.38f * h } },
            { { 0.98f * w, 0.54f * h}, { 0.83f * w, 0.91f * h }, { 0.62f * w, 0.40f * h } },
            { { 0.96f * w, 0.65f * h}, { 0.03f * w, 0.79f * h }, { 0.24f * w, 0.56f * h } },
            { { 0.57f * w, 0.12f * h}, { 0.33f * w, 0.67f * h }, { 0.59f * w, 0.33f * h } },
            { { 0.12f * w, 0.72f * h}, { 0.69f * w, 0.85f * h }, { 0.46f * w, 0.32f * h } },
            { { 0.27f * w, 0.49f * h}, { 0.41f * w, 0.02f * h }, { 0.11f * w, 0.42f * h } },
            { { 0.40f * w, 0.13f * h}, { 0.83f * w, 0.30f * h }, { 0.31f * w, 0.68f * h } },
        };
        const SkScalar weights[kNumConics] = { 0.62f, 0.01f, 0.95f, 1.48f, 0.37f,
                                               0.66f, 0.15f, 0.14f, 0.61f, 1.4f };

        SkPaint ctrlPtPaint;
        ctrlPtPaint.setColor(SK_ColorRED);

        SkPaint choppedPtPaint;
        choppedPtPaint.setColor(~ctrlPtPaint.getColor() | 0xFF000000);

        SkPaint polyPaint;
        polyPaint.setColor(0xffA0A0A0);
        polyPaint.setStrokeWidth(0);
        polyPaint.setStyle(SkPaint::kStroke_Style);

        SkPaint boundsPaint;
        boundsPaint.setColor(0xff808080);
        boundsPaint.setStrokeWidth(0);
        boundsPaint.setStyle(SkPaint::kStroke_Style);


        for (int row = 0; row < kNumConics; ++row) {
            for(int col = 0; col < kGrClipEdgeTypeCnt; ++col) {
                GrClipEdgeType et = (GrClipEdgeType) col;

                SkScalar x = col * w;
                SkScalar y = row * h;
                SkPoint controlPts[] = {
                    {x + baseControlPts[row][0].fX, y + baseControlPts[row][0].fY},
                    {x + baseControlPts[row][1].fX, y + baseControlPts[row][1].fY},
                    {x + baseControlPts[row][2].fX, y + baseControlPts[row][2].fY}
                };

                for (int i = 0; i < 3; ++i) {
                    canvas->drawCircle(controlPts[i], 6.f, ctrlPtPaint);
                }

                canvas->drawPoints(SkCanvas::kPolygon_PointMode, 3, controlPts, polyPaint);

                SkConic dst[4];
                SkMatrix klm;
                int cnt = ChopConic(controlPts, dst, weights[row]);
                GrPathUtils::getConicKLM(controlPts, weights[row], &klm);

                for (int c = 0; c < cnt; ++c) {
                    SkPoint* pts = dst[c].fPts;
                    for (int i = 0; i < 3; ++i) {
                        canvas->drawCircle(pts[i], 3.f, choppedPtPaint);
                    }

                    SkRect bounds;
                    bounds.setBounds(pts, 3);

                    canvas->drawRect(bounds, boundsPaint);

                    std::unique_ptr<GrDrawOp> op = BezierConicTestOp::Make(context, et, bounds,
                                                                           kOpaqueBlack, klm);
                    renderTargetContext->priv().testingOnly_addDrawOp(std::move(op));
                }
            }
        }
    }

private:
    // Uses the max curvature function for quads to estimate
    // where to chop the conic. If the max curvature is not
    // found along the curve segment it will return 1 and
    // dst[0] is the original conic. If it returns 2 the dst[0]
    // and dst[1] are the two new conics.
    static int SplitConic(const SkPoint src[3], SkConic dst[2], const SkScalar weight) {
        SkScalar t = SkFindQuadMaxCurvature(src);
        if (t == 0 || t == 1) {
            if (dst) {
                dst[0].set(src, weight);
            }
            return 1;
        } else {
            if (dst) {
                SkConic conic;
                conic.set(src, weight);
                if (!conic.chopAt(t, dst)) {
                    dst[0].set(src, weight);
                    return 1;
                }
            }
            return 2;
        }
    }

    // Calls SplitConic on the entire conic and then once more on each subsection.
    // Most cases will result in either 1 conic (chop point is not within t range)
    // or 3 points (split once and then one subsection is split again).
    static int ChopConic(const SkPoint src[3], SkConic dst[4], const SkScalar weight) {
        SkConic dstTemp[2];
        int conicCnt = SplitConic(src, dstTemp, weight);
        if (2 == conicCnt) {
            int conicCnt2 = SplitConic(dstTemp[0].fPts, dst, dstTemp[0].fW);
            conicCnt = conicCnt2 + SplitConic(dstTemp[1].fPts, &dst[conicCnt2], dstTemp[1].fW);
        } else {
            dst[0] = dstTemp[0];
        }
        return conicCnt;
    }

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

class BezierQuadTestOp : public BezierTestOp {
public:
    DEFINE_OP_CLASS_ID
    const char* name() const override { return "BezierQuadTestOp"; }

    static std::unique_ptr<GrDrawOp> Make(GrContext* context,
                                          GrClipEdgeType et,
                                          const SkRect& rect,
                                          const SkPMColor4f& color,
                                          const GrPathUtils::QuadUVMatrix& devToUV) {
        GrOpMemoryPool* pool = context->priv().opMemoryPool();

        return pool->allocate<BezierQuadTestOp>(et, rect, color, devToUV);
    }

private:
    friend class ::GrOpMemoryPool; // for ctor

    BezierQuadTestOp(GrClipEdgeType et, const SkRect& rect,
                     const SkPMColor4f& color, const GrPathUtils::QuadUVMatrix& devToUV)
            : INHERITED(et, rect, color, ClassID()), fDevToUV(devToUV) {}

    struct Vertex {
        SkPoint fPosition;
        float   fKLM[4]; // The last value is ignored. The effect expects a vec4f.
    };

    void onPrepareDraws(Target* target) override {
        GrGeometryProcessor* gp = GrQuadEffect::Make(target->allocator(), this->color(),
                                                     SkMatrix::I(), this->edgeType(),
                                                     target->caps(), SkMatrix::I(), false);
        if (!gp) {
            return;
        }

        SkASSERT(gp->vertexStride() == sizeof(Vertex));
        QuadHelper helper(target, sizeof(Vertex), 1);
        Vertex* verts = reinterpret_cast<Vertex*>(helper.vertices());
        if (!verts) {
            return;
        }
        SkRect rect = this->rect();
        SkPointPriv::SetRectTriStrip(&verts[0].fPosition, rect, sizeof(Vertex));
        fDevToUV.apply(verts, 4, sizeof(Vertex), sizeof(SkPoint));
        helper.recordDraw(target, gp);
    }

    GrPathUtils::QuadUVMatrix fDevToUV;

    static constexpr int kVertsPerCubic = 4;
    static constexpr int kIndicesPerCubic = 6;

    typedef BezierTestOp INHERITED;
};

/**
 * This GM directly exercises effects that draw Bezier quad curves in the GPU backend.
 */
class BezierQuadEffects : public GpuGM {
public:
    BezierQuadEffects() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    static const int kNumQuads = 5;
    static const int kCellWidth = 128;
    static const int kCellHeight = 128;

    SkString onShortName() override {
        return SkString("bezier_quad_effects");
    }

    SkISize onISize() override {
        return SkISize::Make(kGrClipEdgeTypeCnt*kCellWidth, kNumQuads*kCellHeight);
    }

    void onDraw(GrContext* context, GrRenderTargetContext* renderTargetContext,
                SkCanvas* canvas) override {

        const SkScalar w = kCellWidth, h = kCellHeight;
        const SkPMColor4f kOpaqueBlack = SkPMColor4f::FromBytes_RGBA(0xff000000);

        const SkPoint baseControlPts[kNumQuads][3] = {
            { { 0.31f * w, 0.01f * h}, { 0.48f * w, 0.74f * h }, { 0.19f * w, 0.33f * h } },
            { { 0.00f * w, 0.07f * h}, { 0.30f * w, 0.70f * h }, { 0.47f * w, 0.37f * h } },
            { { 0.15f * w, 0.23f * h}, { 0.49f * w, 0.87f * h }, { 0.85f * w, 0.66f * h } },
            { { 0.09f * w, 0.15f * h}, { 0.42f * w, 0.33f * h }, { 0.17f * w, 0.38f * h } },
            { { 0.98f * w, 0.54f * h}, { 0.83f * w, 0.91f * h }, { 0.62f * w, 0.40f * h } },
        };

        SkPaint ctrlPtPaint;
        ctrlPtPaint.setColor(SK_ColorRED);

        SkPaint choppedPtPaint;
        choppedPtPaint.setColor(~ctrlPtPaint.getColor() | 0xFF000000);

        SkPaint polyPaint;
        polyPaint.setColor(0xffA0A0A0);
        polyPaint.setStrokeWidth(0);
        polyPaint.setStyle(SkPaint::kStroke_Style);

        SkPaint boundsPaint;
        boundsPaint.setColor(0xff808080);
        boundsPaint.setStrokeWidth(0);
        boundsPaint.setStyle(SkPaint::kStroke_Style);

        for (int row = 0; row < kNumQuads; ++row) {
            for(int col = 0; col < kGrClipEdgeTypeCnt; ++col) {
                GrClipEdgeType et = (GrClipEdgeType) col;

                SkScalar x = col * w;
                SkScalar y = row * h;
                SkPoint controlPts[] = {
                    {x + baseControlPts[row][0].fX, y + baseControlPts[row][0].fY},
                    {x + baseControlPts[row][1].fX, y + baseControlPts[row][1].fY},
                    {x + baseControlPts[row][2].fX, y + baseControlPts[row][2].fY}
                };

                for (int i = 0; i < 3; ++i) {
                    canvas->drawCircle(controlPts[i], 6.f, ctrlPtPaint);
                }

                canvas->drawPoints(SkCanvas::kPolygon_PointMode, 3, controlPts, polyPaint);

                SkPoint chopped[5];
                int cnt = SkChopQuadAtMaxCurvature(controlPts, chopped);

                for (int c = 0; c < cnt; ++c) {
                    SkPoint* pts = chopped + 2 * c;

                    for (int i = 0; i < 3; ++i) {
                        canvas->drawCircle(pts[i], 3.f, choppedPtPaint);
                    }

                    SkRect bounds;
                    bounds.setBounds(pts, 3);

                    canvas->drawRect(bounds, boundsPaint);

                    GrPathUtils::QuadUVMatrix DevToUV(pts);

                    std::unique_ptr<GrDrawOp> op = BezierQuadTestOp::Make(context, et, bounds,
                                                                          kOpaqueBlack, DevToUV);
                    renderTargetContext->priv().testingOnly_addDrawOp(std::move(op));
                }
            }
        }
    }

private:
    typedef GM INHERITED;
};

DEF_GM(return new BezierConicEffects;)
DEF_GM(return new BezierQuadEffects;)
}
