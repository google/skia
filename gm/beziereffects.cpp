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
    BezierTestOp(sk_sp<const GrGeometryProcessor> gp, const SkRect& rect, const SkPMColor4f& color,
                 int32_t classID)
            : INHERITED(classID)
            , fRect(rect)
            , fColor(color)
            , fGeometryProcessor(std::move(gp))
            , fProcessorSet(SkBlendMode::kSrc) {
        this->setBounds(rect, HasAABloat::kYes, IsZeroArea::kNo);
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        flushState->executeDrawsAndUploadsForMeshDrawOp(
                this, chainBounds, std::move(fProcessorSet));
    }

    sk_sp<const GrGeometryProcessor> gp() const { return fGeometryProcessor; }

    const SkRect& rect() const { return fRect; }
    const SkPMColor4f& color() const { return fColor; }

private:
    SkRect fRect;
    SkPMColor4f fColor;
    sk_sp<const GrGeometryProcessor> fGeometryProcessor;
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
                                          sk_sp<const GrGeometryProcessor> gp,
                                          const SkRect& rect,
                                          const SkPMColor4f& color,
                                          const SkMatrix& klm) {
        GrOpMemoryPool* pool = context->priv().opMemoryPool();

        return pool->allocate<BezierConicTestOp>(std::move(gp), rect, color, klm);
    }

private:
    friend class ::GrOpMemoryPool; // for ctor

    BezierConicTestOp(sk_sp<const GrGeometryProcessor> gp, const SkRect& rect,
                      const SkPMColor4f& color, const SkMatrix& klm)
            : INHERITED(std::move(gp), rect, color, ClassID()), fKLM(klm) {}

    struct Vertex {
        SkPoint fPosition;
        float   fKLM[4]; // The last value is ignored. The effect expects a vec4f.
    };

    void onPrepareDraws(Target* target) override {
        SkASSERT(this->gp()->vertexStride() == sizeof(Vertex));
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

        helper.recordDraw(target, this->gp());
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
    SkString onShortName() override {
        return SkString("bezier_conic_effects");
    }

    SkISize onISize() override {
        return SkISize::Make(800, 800);
    }


    void onDraw(GrContext* context, GrRenderTargetContext* renderTargetContext,
                SkCanvas* canvas) override {
        struct Vertex {
            SkPoint fPosition;
            float   fKLM[4]; // The last value is ignored. The effect expects a vec4f.
        };

        constexpr int kNumConics = 10;
        SkRandom rand;

        // Mult by 3 for each edge effect type
        int numCols = SkScalarCeilToInt(SkScalarSqrt(SkIntToScalar(kNumConics*3)));
        int numRows = SkScalarCeilToInt(SkIntToScalar(kNumConics*3) / numCols);
        SkScalar w = SkIntToScalar(renderTargetContext->width()) / numCols;
        SkScalar h = SkIntToScalar(renderTargetContext->height()) / numRows;
        int row = 0;
        int col = 0;
        SkPMColor4f color = SkPMColor4f::FromBytes_RGBA(0xff000000);

        for (int i = 0; i < kNumConics; ++i) {
            SkPoint baseControlPts[] = {
                {rand.nextRangeF(0.f, w), rand.nextRangeF(0.f, h)},
                {rand.nextRangeF(0.f, w), rand.nextRangeF(0.f, h)},
                {rand.nextRangeF(0.f, w), rand.nextRangeF(0.f, h)}
            };
            SkScalar weight = rand.nextRangeF(0.f, 2.f);
            for(int edgeType = 0; edgeType < kGrClipEdgeTypeCnt; ++edgeType) {
                sk_sp<GrGeometryProcessor> gp;
                GrClipEdgeType et = (GrClipEdgeType)edgeType;
                gp = GrConicEffect::Make(color, SkMatrix::I(), et, *context->priv().caps(),
                                         SkMatrix::I(), false);
                if (!gp) {
                    continue;
                }

                SkScalar x = col * w;
                SkScalar y = row * h;
                SkPoint controlPts[] = {
                    {x + baseControlPts[0].fX, y + baseControlPts[0].fY},
                    {x + baseControlPts[1].fX, y + baseControlPts[1].fY},
                    {x + baseControlPts[2].fX, y + baseControlPts[2].fY}
                };
                SkConic dst[4];
                SkMatrix klm;
                int cnt = chop_conic(controlPts, dst, weight);
                GrPathUtils::getConicKLM(controlPts, weight, &klm);

                SkPaint ctrlPtPaint;
                ctrlPtPaint.setColor(rand.nextU() | 0xFF000000);
                for (int i = 0; i < 3; ++i) {
                    canvas->drawCircle(controlPts[i], 6.f, ctrlPtPaint);
                }

                SkPaint polyPaint;
                polyPaint.setColor(0xffA0A0A0);
                polyPaint.setStrokeWidth(0);
                polyPaint.setStyle(SkPaint::kStroke_Style);
                canvas->drawPoints(SkCanvas::kPolygon_PointMode, 3, controlPts, polyPaint);

                SkPaint choppedPtPaint;
                choppedPtPaint.setColor(~ctrlPtPaint.getColor() | 0xFF000000);

                for (int c = 0; c < cnt; ++c) {
                    SkPoint* pts = dst[c].fPts;
                    for (int i = 0; i < 3; ++i) {
                        canvas->drawCircle(pts[i], 3.f, choppedPtPaint);
                    }

                    SkRect bounds;
                    //SkPoint bPts[] = {{0.f, 0.f}, {800.f, 800.f}};
                    //bounds.set(bPts, 2);
                    bounds.setBounds(pts, 3);

                    SkPaint boundsPaint;
                    boundsPaint.setColor(0xff808080);
                    boundsPaint.setStrokeWidth(0);
                    boundsPaint.setStyle(SkPaint::kStroke_Style);
                    canvas->drawRect(bounds, boundsPaint);

                    std::unique_ptr<GrDrawOp> op = BezierConicTestOp::Make(context, gp, bounds,
                                                                           color, klm);
                    renderTargetContext->priv().testingOnly_addDrawOp(std::move(op));
                }
                ++col;
                if (numCols == col) {
                    col = 0;
                    ++row;
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
    int split_conic(const SkPoint src[3], SkConic dst[2], const SkScalar weight) {
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

    // Calls split_conic on the entire conic and then once more on each subsection.
    // Most cases will result in either 1 conic (chop point is not within t range)
    // or 3 points (split once and then one subsection is split again).
    int chop_conic(const SkPoint src[3], SkConic dst[4], const SkScalar weight) {
        SkConic dstTemp[2];
        int conicCnt = split_conic(src, dstTemp, weight);
        if (2 == conicCnt) {
            int conicCnt2 = split_conic(dstTemp[0].fPts, dst, dstTemp[0].fW);
            conicCnt = conicCnt2 + split_conic(dstTemp[1].fPts, &dst[conicCnt2], dstTemp[1].fW);
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
                                          sk_sp<const GrGeometryProcessor> gp,
                                          const SkRect& rect,
                                          const SkPMColor4f& color,
                                          const GrPathUtils::QuadUVMatrix& devToUV) {
        GrOpMemoryPool* pool = context->priv().opMemoryPool();

        return pool->allocate<BezierQuadTestOp>(std::move(gp), rect, color, devToUV);
    }

private:
    friend class ::GrOpMemoryPool; // for ctor

    BezierQuadTestOp(sk_sp<const GrGeometryProcessor> gp, const SkRect& rect,
                     const SkPMColor4f& color, const GrPathUtils::QuadUVMatrix& devToUV)
            : INHERITED(std::move(gp), rect, color, ClassID()), fDevToUV(devToUV) {}

    struct Vertex {
        SkPoint fPosition;
        float   fKLM[4]; // The last value is ignored. The effect expects a vec4f.
    };

    void onPrepareDraws(Target* target) override {
        SkASSERT(this->gp()->vertexStride() == sizeof(Vertex));
        QuadHelper helper(target, sizeof(Vertex), 1);
        Vertex* verts = reinterpret_cast<Vertex*>(helper.vertices());
        if (!verts) {
            return;
        }
        SkRect rect = this->rect();
        SkPointPriv::SetRectTriStrip(&verts[0].fPosition, rect, sizeof(Vertex));
        fDevToUV.apply(verts, 4, sizeof(Vertex), sizeof(SkPoint));
        helper.recordDraw(target, this->gp());
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
    SkString onShortName() override {
        return SkString("bezier_quad_effects");
    }

    SkISize onISize() override {
        return SkISize::Make(800, 800);
    }


    void onDraw(GrContext* context, GrRenderTargetContext* renderTargetContext,
                SkCanvas* canvas) override {
        struct Vertex {
            SkPoint fPosition;
            float   fUV[4]; // The last two values are ignored. The effect expects a vec4f.
        };

        constexpr int kNumQuads = 5;
        SkRandom rand;

        int numCols = SkScalarCeilToInt(SkScalarSqrt(SkIntToScalar(kNumQuads*3)));
        int numRows = SkScalarCeilToInt(SkIntToScalar(kNumQuads*3) / numCols);
        SkScalar w = SkIntToScalar(renderTargetContext->width()) / numCols;
        SkScalar h = SkIntToScalar(renderTargetContext->height()) / numRows;
        int row = 0;
        int col = 0;
        SkPMColor4f color = SkPMColor4f::FromBytes_RGBA(0xff000000);

        for (int i = 0; i < kNumQuads; ++i) {
            SkPoint baseControlPts[] = {
                {rand.nextRangeF(0.f, w), rand.nextRangeF(0.f, h)},
                {rand.nextRangeF(0.f, w), rand.nextRangeF(0.f, h)},
                {rand.nextRangeF(0.f, w), rand.nextRangeF(0.f, h)}
            };
            for(int edgeType = 0; edgeType < kGrClipEdgeTypeCnt; ++edgeType) {
                sk_sp<GrGeometryProcessor> gp;
                GrClipEdgeType et = (GrClipEdgeType)edgeType;
                gp = GrQuadEffect::Make(color, SkMatrix::I(), et, *context->priv().caps(),
                                        SkMatrix::I(), false);
                if (!gp) {
                    continue;
                }

                SkScalar x = col * w;
                SkScalar y = row * h;
                SkPoint controlPts[] = {
                    {x + baseControlPts[0].fX, y + baseControlPts[0].fY},
                    {x + baseControlPts[1].fX, y + baseControlPts[1].fY},
                    {x + baseControlPts[2].fX, y + baseControlPts[2].fY}
                };
                SkPoint chopped[5];
                int cnt = SkChopQuadAtMaxCurvature(controlPts, chopped);

                SkPaint ctrlPtPaint;
                ctrlPtPaint.setColor(rand.nextU() | 0xFF000000);
                for (int i = 0; i < 3; ++i) {
                    canvas->drawCircle(controlPts[i], 6.f, ctrlPtPaint);
                }

                SkPaint polyPaint;
                polyPaint.setColor(0xffA0A0A0);
                polyPaint.setStrokeWidth(0);
                polyPaint.setStyle(SkPaint::kStroke_Style);
                canvas->drawPoints(SkCanvas::kPolygon_PointMode, 3, controlPts, polyPaint);

                SkPaint choppedPtPaint;
                choppedPtPaint.setColor(~ctrlPtPaint.getColor() | 0xFF000000);

                for (int c = 0; c < cnt; ++c) {
                    SkPoint* pts = chopped + 2 * c;

                    for (int i = 0; i < 3; ++i) {
                        canvas->drawCircle(pts[i], 3.f, choppedPtPaint);
                    }

                    SkRect bounds;
                    bounds.setBounds(pts, 3);

                    SkPaint boundsPaint;
                    boundsPaint.setColor(0xff808080);
                    boundsPaint.setStrokeWidth(0);
                    boundsPaint.setStyle(SkPaint::kStroke_Style);
                    canvas->drawRect(bounds, boundsPaint);

                    GrPaint grPaint;
                    grPaint.setXPFactory(GrPorterDuffXPFactory::Get(SkBlendMode::kSrc));

                    GrPathUtils::QuadUVMatrix DevToUV(pts);

                    std::unique_ptr<GrDrawOp> op = BezierQuadTestOp::Make(context, gp,
                                                                          bounds, color, DevToUV);
                    renderTargetContext->priv().testingOnly_addDrawOp(std::move(op));
                }
                ++col;
                if (numCols == col) {
                    col = 0;
                    ++row;
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
