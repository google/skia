/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This test only works with the GPU backend.

#include "gm.h"

#if SK_SUPPORT_GPU

#include "GrContext.h"
#include "GrOpFlushState.h"
#include "GrPathUtils.h"
#include "GrRenderTargetContextPriv.h"
#include "GrTest.h"
#include "SkColorPriv.h"
#include "SkGeometry.h"
#include "effects/GrBezierEffect.h"
#include "ops/GrMeshDrawOp.h"

namespace skiagm {

class BezierTestOp : public GrMeshDrawOp {
public:
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }

    RequiresDstTexture finalize(const GrCaps& caps, const GrAppliedClip* clip) override {
        auto analysis = fProcessorSet.finalize(fColor, GrProcessorAnalysisCoverage::kSingleChannel,
                                               clip, false, caps, &fColor);
        return analysis.requiresDstTexture() ? RequiresDstTexture::kYes : RequiresDstTexture::kNo;
    }

protected:
    BezierTestOp(sk_sp<GrGeometryProcessor> gp, const SkRect& rect, GrColor color, int32_t classID)
            : INHERITED(classID)
            , fRect(rect)
            , fColor(color)
            , fGeometryProcessor(std::move(gp))
            , fProcessorSet(SkBlendMode::kSrc) {
        this->setBounds(rect, HasAABloat::kYes, IsZeroArea::kNo);
    }

    const GrPipeline* makePipeline(Target* target) const {
        return target->makePipeline(0, &fProcessorSet);
    }

    const GrGeometryProcessor* gp() const { return fGeometryProcessor.get(); }

    const SkRect& rect() const { return fRect; }
    GrColor color() const { return fColor; }

private:
    bool onCombineIfPossible(GrOp* op, const GrCaps& caps) override { return false; }

    SkRect fRect;
    GrColor fColor;
    sk_sp<GrGeometryProcessor> fGeometryProcessor;
    GrProcessorSet fProcessorSet;

    typedef GrMeshDrawOp INHERITED;
};

class BezierCubicTestOp : public BezierTestOp {
public:
    DEFINE_OP_CLASS_ID

    const char* name() const override { return "BezierCubicTestOp"; }

    static std::unique_ptr<GrDrawOp> Make(sk_sp<GrGeometryProcessor> gp, const SkRect& rect,
                                          GrColor color) {
        return std::unique_ptr<GrDrawOp>(new BezierCubicTestOp(std::move(gp), rect, color));
    }

private:
    BezierCubicTestOp(sk_sp<GrGeometryProcessor> gp, const SkRect& rect, GrColor color)
            : INHERITED(std::move(gp), rect, color, ClassID()) {}

    void onPrepareDraws(Target* target) const override {
        QuadHelper helper;
        size_t vertexStride = this->gp()->getVertexStride();
        SkASSERT(vertexStride == sizeof(SkPoint));
        SkPoint* pts = reinterpret_cast<SkPoint*>(helper.init(target, vertexStride, 1));
        if (!pts) {
            return;
        }
        SkRect rect = this->rect();
        pts[0].setRectFan(rect.fLeft, rect.fTop, rect.fRight, rect.fBottom, vertexStride);
        helper.recordDraw(target, this->gp(), this->makePipeline(target));
    }

    static constexpr int kVertsPerCubic = 4;
    static constexpr int kIndicesPerCubic = 6;

    typedef BezierTestOp INHERITED;
};

/**
 * This GM directly exercises effects that draw Bezier curves in the GPU backend.
 */
class BezierCubicEffects : public GM {
public:
    BezierCubicEffects() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString onShortName() override {
        return SkString("bezier_cubic_effects");
    }

    SkISize onISize() override {
        return SkISize::Make(800, 800);
    }

    void onDraw(SkCanvas* canvas) override {
        GrRenderTargetContext* renderTargetContext =
            canvas->internal_private_accessTopLayerRenderTargetContext();
        if (!renderTargetContext) {
            skiagm::GM::DrawGpuOnlyMessage(canvas);
            return;
        }

        GrContext* context = canvas->getGrContext();
        if (!context) {
            return;
        }

        struct Vertex {
            SkPoint fPosition;
            float   fKLM[4]; // The last value is ignored. The effect expects a vec4f.
        };

        constexpr int kNumCubics = 15;
        SkRandom rand;

        // Mult by 3 for each edge effect type
        int numCols = SkScalarCeilToInt(SkScalarSqrt(SkIntToScalar(kNumCubics*3)));
        int numRows = SkScalarCeilToInt(SkIntToScalar(kNumCubics*3) / numCols);
        SkScalar w = SkIntToScalar(renderTargetContext->width()) / numCols;
        SkScalar h = SkIntToScalar(renderTargetContext->height()) / numRows;
        int row = 0;
        int col = 0;
        constexpr GrColor color = 0xff000000;

        for (int i = 0; i < kNumCubics; ++i) {
            SkPoint baseControlPts[] = {
                {rand.nextRangeF(0.f, w), rand.nextRangeF(0.f, h)},
                {rand.nextRangeF(0.f, w), rand.nextRangeF(0.f, h)},
                {rand.nextRangeF(0.f, w), rand.nextRangeF(0.f, h)},
                {rand.nextRangeF(0.f, w), rand.nextRangeF(0.f, h)}
            };
            for(GrPrimitiveEdgeType edgeType : {kFillBW_GrProcessorEdgeType,
                                                kFillAA_GrProcessorEdgeType,
                                                kHairlineAA_GrProcessorEdgeType}) {
                SkScalar x = col * w;
                SkScalar y = row * h;
                SkPoint controlPts[] = {
                    {x + baseControlPts[0].fX, y + baseControlPts[0].fY},
                    {x + baseControlPts[1].fX, y + baseControlPts[1].fY},
                    {x + baseControlPts[2].fX, y + baseControlPts[2].fY},
                    {x + baseControlPts[3].fX, y + baseControlPts[3].fY}
                };
                SkPoint chopped[10];
                SkMatrix klm;
                int loopIndex;
                int cnt = GrPathUtils::chopCubicAtLoopIntersection(controlPts,
                                                                   chopped,
                                                                   &klm,
                                                                   &loopIndex);

                SkPaint ctrlPtPaint;
                ctrlPtPaint.setColor(rand.nextU() | 0xFF000000);
                canvas->drawCircle(controlPts[0], 8.f, ctrlPtPaint);
                for (int i = 1; i < 4; ++i) {
                    canvas->drawCircle(controlPts[i], 6.f, ctrlPtPaint);
                }

                SkPaint polyPaint;
                polyPaint.setColor(0xffA0A0A0);
                polyPaint.setStrokeWidth(0);
                polyPaint.setStyle(SkPaint::kStroke_Style);
                canvas->drawPoints(SkCanvas::kPolygon_PointMode, 4, controlPts, polyPaint);

                SkPaint choppedPtPaint;
                choppedPtPaint.setColor(~ctrlPtPaint.getColor() | 0xFF000000);

                for (int c = 0; c < cnt; ++c) {
                    SkPoint* pts = chopped + 3 * c;

                    for (int i = 0; i < 4; ++i) {
                        canvas->drawCircle(pts[i], 3.f, choppedPtPaint);
                    }

                    SkRect bounds;
                    bounds.set(pts, 4);

                    SkPaint boundsPaint;
                    boundsPaint.setColor(0xff808080);
                    boundsPaint.setStrokeWidth(0);
                    boundsPaint.setStyle(SkPaint::kStroke_Style);
                    canvas->drawRect(bounds, boundsPaint);


                    bool flipKL = (c == loopIndex && cnt != 3);
                    sk_sp<GrGeometryProcessor> gp = GrCubicEffect::Make(color, SkMatrix::I(), klm,
                                                                        flipKL, edgeType,
                                                                        *context->caps());
                    if (!gp) {
                        break;
                    }

                    std::unique_ptr<GrDrawOp> op =
                            BezierCubicTestOp::Make(std::move(gp), bounds, color);
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

//////////////////////////////////////////////////////////////////////////////

class BezierConicTestOp : public BezierTestOp {
public:
    DEFINE_OP_CLASS_ID

    const char* name() const override { return "BezierConicTestOp"; }

    static std::unique_ptr<GrDrawOp> Make(sk_sp<GrGeometryProcessor> gp, const SkRect& rect,
                                          GrColor color, const SkMatrix& klm) {
        return std::unique_ptr<GrMeshDrawOp>(
                new BezierConicTestOp(std::move(gp), rect, color, klm));
    }

private:
    BezierConicTestOp(sk_sp<GrGeometryProcessor> gp, const SkRect& rect, GrColor color,
                      const SkMatrix& klm)
            : INHERITED(std::move(gp), rect, color, ClassID()), fKLM(klm) {}

    struct Vertex {
        SkPoint fPosition;
        float   fKLM[4]; // The last value is ignored. The effect expects a vec4f.
    };

    void onPrepareDraws(Target* target) const override {
        QuadHelper helper;
        size_t vertexStride = this->gp()->getVertexStride();
        SkASSERT(vertexStride == sizeof(Vertex));
        Vertex* verts = reinterpret_cast<Vertex*>(helper.init(target, vertexStride, 1));
        if (!verts) {
            return;
        }
        SkRect rect = this->rect();
        verts[0].fPosition.setRectFan(rect.fLeft, rect.fTop, rect.fRight, rect.fBottom,
                                      sizeof(Vertex));
        for (int v = 0; v < 4; ++v) {
            SkScalar pt3[3] = {verts[v].fPosition.x(), verts[v].fPosition.y(), 1.f};
            fKLM.mapHomogeneousPoints(verts[v].fKLM, pt3, 1);
        }
        helper.recordDraw(target, this->gp(), this->makePipeline(target));
    }

    SkMatrix fKLM;

    static constexpr int kVertsPerCubic = 4;
    static constexpr int kIndicesPerCubic = 6;

    typedef BezierTestOp INHERITED;
};


/**
 * This GM directly exercises effects that draw Bezier curves in the GPU backend.
 */
class BezierConicEffects : public GM {
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


    void onDraw(SkCanvas* canvas) override {
        GrRenderTargetContext* renderTargetContext =
            canvas->internal_private_accessTopLayerRenderTargetContext();
        if (!renderTargetContext) {
            skiagm::GM::DrawGpuOnlyMessage(canvas);
            return;
        }

        GrContext* context = canvas->getGrContext();
        if (!context) {
            return;
        }

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
        constexpr GrColor color = 0xff000000;

        for (int i = 0; i < kNumConics; ++i) {
            SkPoint baseControlPts[] = {
                {rand.nextRangeF(0.f, w), rand.nextRangeF(0.f, h)},
                {rand.nextRangeF(0.f, w), rand.nextRangeF(0.f, h)},
                {rand.nextRangeF(0.f, w), rand.nextRangeF(0.f, h)}
            };
            SkScalar weight = rand.nextRangeF(0.f, 2.f);
            for(int edgeType = 0; edgeType < kGrProcessorEdgeTypeCnt; ++edgeType) {
                sk_sp<GrGeometryProcessor> gp;
                GrPrimitiveEdgeType et = (GrPrimitiveEdgeType)edgeType;
                gp = GrConicEffect::Make(color, SkMatrix::I(), et,
                                         *context->caps(), SkMatrix::I(), false);
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
                    bounds.set(pts, 3);

                    SkPaint boundsPaint;
                    boundsPaint.setColor(0xff808080);
                    boundsPaint.setStrokeWidth(0);
                    boundsPaint.setStyle(SkPaint::kStroke_Style);
                    canvas->drawRect(bounds, boundsPaint);

                    std::unique_ptr<GrDrawOp> op = BezierConicTestOp::Make(gp, bounds, color, klm);
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
        if (t == 0) {
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

    static std::unique_ptr<GrDrawOp> Make(sk_sp<GrGeometryProcessor> gp, const SkRect& rect,
                                          GrColor color, const GrPathUtils::QuadUVMatrix& devToUV) {
        return std::unique_ptr<GrDrawOp>(new BezierQuadTestOp(std::move(gp), rect, color, devToUV));
    }

private:
    BezierQuadTestOp(sk_sp<GrGeometryProcessor> gp, const SkRect& rect, GrColor color,
                     const GrPathUtils::QuadUVMatrix& devToUV)
            : INHERITED(std::move(gp), rect, color, ClassID()), fDevToUV(devToUV) {}

    struct Vertex {
        SkPoint fPosition;
        float   fKLM[4]; // The last value is ignored. The effect expects a vec4f.
    };

    void onPrepareDraws(Target* target) const override {
        QuadHelper helper;
        size_t vertexStride = this->gp()->getVertexStride();
        SkASSERT(vertexStride == sizeof(Vertex));
        Vertex* verts = reinterpret_cast<Vertex*>(helper.init(target, vertexStride, 1));
        if (!verts) {
            return;
        }
        SkRect rect = this->rect();
        verts[0].fPosition.setRectFan(rect.fLeft, rect.fTop, rect.fRight, rect.fBottom,
                                      sizeof(Vertex));
        fDevToUV.apply<4, sizeof(Vertex), sizeof(SkPoint)>(verts);
        helper.recordDraw(target, this->gp(), this->makePipeline(target));
    }

    GrPathUtils::QuadUVMatrix fDevToUV;

    static constexpr int kVertsPerCubic = 4;
    static constexpr int kIndicesPerCubic = 6;

    typedef BezierTestOp INHERITED;
};

/**
 * This GM directly exercises effects that draw Bezier quad curves in the GPU backend.
 */
class BezierQuadEffects : public GM {
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


    void onDraw(SkCanvas* canvas) override {
        GrRenderTargetContext* renderTargetContext =
            canvas->internal_private_accessTopLayerRenderTargetContext();
        if (!renderTargetContext) {
            skiagm::GM::DrawGpuOnlyMessage(canvas);
            return;
        }

        GrContext* context = canvas->getGrContext();
        if (!context) {
            return;
        }

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
        constexpr GrColor color = 0xff000000;

        for (int i = 0; i < kNumQuads; ++i) {
            SkPoint baseControlPts[] = {
                {rand.nextRangeF(0.f, w), rand.nextRangeF(0.f, h)},
                {rand.nextRangeF(0.f, w), rand.nextRangeF(0.f, h)},
                {rand.nextRangeF(0.f, w), rand.nextRangeF(0.f, h)}
            };
            for(int edgeType = 0; edgeType < kGrProcessorEdgeTypeCnt; ++edgeType) {
                sk_sp<GrGeometryProcessor> gp;
                GrPrimitiveEdgeType et = (GrPrimitiveEdgeType)edgeType;
                gp = GrQuadEffect::Make(color, SkMatrix::I(), et,
                                        *context->caps(), SkMatrix::I(), false);
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
                    bounds.set(pts, 3);

                    SkPaint boundsPaint;
                    boundsPaint.setColor(0xff808080);
                    boundsPaint.setStrokeWidth(0);
                    boundsPaint.setStyle(SkPaint::kStroke_Style);
                    canvas->drawRect(bounds, boundsPaint);

                    GrPaint grPaint;
                    grPaint.setXPFactory(GrPorterDuffXPFactory::Get(SkBlendMode::kSrc));

                    GrPathUtils::QuadUVMatrix DevToUV(pts);

                    std::unique_ptr<GrDrawOp> op =
                            BezierQuadTestOp::Make(gp, bounds, color, DevToUV);
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

DEF_GM(return new BezierCubicEffects;)
DEF_GM(return new BezierConicEffects;)
DEF_GM(return new BezierQuadEffects;)
}

#endif
