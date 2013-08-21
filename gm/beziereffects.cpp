
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This test only works with the GPU backend.

#include "gm.h"

#if SK_SUPPORT_GPU && 0 // Can be enabled when cubic effect is checked in.

#include "GrContext.h"
#include "GrPathUtils.h"
#include "GrTest.h"
#include "SkColorPriv.h"
#include "SkDevice.h"

// Position & KLM line eq values. These are the vertex attributes for Bezier curves. The last value
// of the Vec4f is ignored.
extern const GrVertexAttrib kAttribs[] = {
    {kVec2f_GrVertexAttribType, 0, kPosition_GrVertexAttribBinding},
    {kVec4f_GrVertexAttribType, sizeof(GrPoint), kEffect_GrVertexAttribBinding}
};

static inline SkScalar eval_line(const SkPoint& p, const SkScalar lineEq[3], SkScalar sign) {
    return sign * (lineEq[0] * p.fX + lineEq[1] * p.fY + lineEq[2]);
}

namespace skiagm {
/**
 * This GM directly exercises effects that draw Bezier curves in the GPU backend.
 */
class BezierEffects : public GM {
public:
    BezierEffects() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    virtual SkString onShortName() SK_OVERRIDE {
        return SkString("bezier_effects");
    }

    virtual SkISize onISize() SK_OVERRIDE {
        return make_isize(800, 800);
    }

    virtual uint32_t onGetFlags() const SK_OVERRIDE {
        // This is a GPU-specific GM.
        return kGPUOnly_Flag;
    }


    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        SkDevice* device = canvas->getTopDevice();
        GrRenderTarget* rt = device->accessRenderTarget();
        if (NULL == rt) {
            return;
        }
        GrContext* context = rt->getContext();
        if (NULL == context) {
            return;
        }

        struct Vertex {
            SkPoint fPosition;
            float   fKLM[4]; // The last value is ignored. The effect expects a vec4f.
        };

        static const int kNumCubics = 10;
        SkMWCRandom rand;

        int numCols = SkScalarCeilToInt(SkScalarSqrt(SkIntToScalar(kNumCubics)));
        int numRows = SkScalarCeilToInt(SkIntToScalar(kNumCubics) / numCols);
        SkScalar w = SkIntToScalar(rt->width()) / numCols;
        SkScalar h = SkIntToScalar(rt->height()) / numRows;
        int row = 0;
        int col = 0;

        for (int i = 0; i < kNumCubics; ++i) {
            SkScalar x = SkScalarMul(col, w);
            SkScalar y = SkScalarMul(row, h);
            SkPoint controlPts[] = {
                {x + rand.nextRangeF(0, w), y + rand.nextRangeF(0, h)},
                {x + rand.nextRangeF(0, w), y + rand.nextRangeF(0, h)},
                {x + rand.nextRangeF(0, w), y + rand.nextRangeF(0, h)},
                {x + rand.nextRangeF(0, w), y + rand.nextRangeF(0, h)}
            };
            SkPoint chopped[10];
            SkScalar klmEqs[9];
            SkScalar klmSigns[3];
            int cnt = GrPathUtils::chopCubicAtLoopIntersection(controlPts,
                                                               chopped,
                                                               klmEqs,
                                                               klmSigns,
                                                               controlPts);

            SkPaint ctrlPtPaint;
            ctrlPtPaint.setColor(rand.nextU() | 0xFF000000);
            for (int i = 0; i < 4; ++i) {
                canvas->drawCircle(controlPts[i].fX, controlPts[i].fY, 6.f, ctrlPtPaint);
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
                    canvas->drawCircle(pts[i].fX, pts[i].fY, 3.f, choppedPtPaint);
                }

                SkRect bounds;
                bounds.set(pts, 4);

                SkPaint boundsPaint;
                boundsPaint.setColor(0xff808080);
                boundsPaint.setStrokeWidth(0);
                boundsPaint.setStyle(SkPaint::kStroke_Style);
                canvas->drawRect(bounds, boundsPaint);

                Vertex verts[4];
                verts[0].fPosition.setRectFan(bounds.fLeft, bounds.fTop,
                                              bounds.fRight, bounds.fBottom,
                                              sizeof(Vertex));
                for (int v = 0; v < 4; ++v) {
                    verts[v].fKLM[0] = eval_line(verts[v].fPosition, klmEqs + 0, klmSigns[c]);
                    verts[v].fKLM[1] = eval_line(verts[v].fPosition, klmEqs + 3, klmSigns[c]);
                    verts[v].fKLM[2] = eval_line(verts[v].fPosition, klmEqs + 6, 1.f);
                }

                GrTestTarget tt;
                context->getTestTarget(&tt);
                if (NULL == tt.target()) {
                    continue;
                }
                GrDrawState* drawState = tt.target()->drawState();
                drawState->setVertexAttribs<kAttribs>(2);
                SkAutoTUnref<GrEffectRef> effect(HairCubicEdgeEffect::Create());
                if (!effect) {
                    continue;
                }
                drawState->addCoverageEffect(effect, 1);
                drawState->setRenderTarget(rt);
                drawState->setColor(0xff000000);

                tt.target()->setVertexSourceToArray(verts, 4);
                tt.target()->setIndexSourceToBuffer(context->getQuadIndexBuffer());
                tt.target()->drawIndexed(kTriangleFan_GrPrimitiveType, 0, 0, 4, 6);
            }
            ++col;
            if (numCols == col) {
                col = 0;
                ++row;
            }
        }
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return SkNEW(BezierEffects); )

}

#endif
