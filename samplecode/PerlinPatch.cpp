/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkAnimTimer.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkPatchUtils.h"
#include "SkPerlinNoiseShader2/SkPerlinNoiseShader2.h"
#include "SkComposeShader.h"

static void draw_control_points(SkCanvas* canvas, const SkPoint cubics[12]) {
    //draw control points
    SkPaint paint;
    SkPoint bottom[SkPatchUtils::kNumPtsCubic];
    SkPatchUtils::getBottomCubic(cubics, bottom);
    SkPoint top[SkPatchUtils::kNumPtsCubic];
    SkPatchUtils::getTopCubic(cubics, top);
    SkPoint left[SkPatchUtils::kNumPtsCubic];
    SkPatchUtils::getLeftCubic(cubics, left);
    SkPoint right[SkPatchUtils::kNumPtsCubic];
    SkPatchUtils::getRightCubic(cubics, right);

    paint.setColor(SK_ColorBLACK);
    paint.setStrokeWidth(0.5f);
    SkPoint corners[4] = { bottom[0], bottom[3], top[0], top[3] };
    canvas->drawPoints(SkCanvas::kLines_PointMode, 4, bottom, paint);
    canvas->drawPoints(SkCanvas::kLines_PointMode, 2, bottom + 1, paint);
    canvas->drawPoints(SkCanvas::kLines_PointMode, 4, top, paint);
    canvas->drawPoints(SkCanvas::kLines_PointMode, 4, left, paint);
    canvas->drawPoints(SkCanvas::kLines_PointMode, 4, right, paint);

    canvas->drawPoints(SkCanvas::kLines_PointMode, 2, top + 1, paint);
    canvas->drawPoints(SkCanvas::kLines_PointMode, 2, left + 1, paint);
    canvas->drawPoints(SkCanvas::kLines_PointMode, 2, right + 1, paint);

    paint.setStrokeWidth(2);

    paint.setColor(SK_ColorRED);
    canvas->drawPoints(SkCanvas::kPoints_PointMode, 4, corners, paint);

    paint.setColor(SK_ColorBLUE);
    canvas->drawPoints(SkCanvas::kPoints_PointMode, 2, bottom + 1, paint);

    paint.setColor(SK_ColorCYAN);
    canvas->drawPoints(SkCanvas::kPoints_PointMode, 2, top + 1, paint);

    paint.setColor(SK_ColorYELLOW);
    canvas->drawPoints(SkCanvas::kPoints_PointMode, 2, left + 1, paint);

    paint.setColor(SK_ColorGREEN);
    canvas->drawPoints(SkCanvas::kPoints_PointMode, 2, right + 1, paint);
}

// These are actually half the total width and hieghts
const SkScalar TexWidth = 100.0f;
const SkScalar TexHeight = 100.0f;

class PerlinPatchView : public SampleView {
    SkShader* fShader0;    
    SkShader* fShader1;    
    SkShader* fShaderCompose;    
    SkScalar fXFreq;
    SkScalar fYFreq;
    SkScalar fSeed;
    SkPoint  fPts[SkPatchUtils::kNumCtrlPts];
    SkScalar fTexX;
    SkScalar fTexY;
    SkScalar fTexScale;
    SkMatrix fInvMatrix;
public:
    PerlinPatchView() : fXFreq(0.025f), fYFreq(0.025f), fSeed(0.0f),
                        fTexX(100.0), fTexY(50.0), fTexScale(1.0f) {
        const SkScalar s = 2;
        // The order of the colors and points is clockwise starting at upper-left corner.
        //top points
        fPts[0].set(100 * s, 100 * s);
        fPts[1].set(150 * s, 50 * s);
        fPts[2].set(250 * s, 150 * s);
        fPts[3].set(300 * s, 100 * s);
        //right points
        fPts[4].set(275 * s, 150 * s);
        fPts[5].set(350 * s, 250 * s);
        //bottom points
        fPts[6].set(300 * s, 300 * s);
        fPts[7].set(250 * s, 250 * s);
        //left points
        fPts[8].set(150 * s, 350 * s);
        fPts[9].set(100 * s, 300 * s);
        fPts[10].set(50 * s, 250 * s);
        fPts[11].set(150 * s, 150 * s);

        const SkColor colors[SkPatchUtils::kNumCorners] = {
            0xFF5555FF, 0xFF8888FF, 0xFFCCCCFF
        };
        const SkPoint points[2] = { SkPoint::Make(0.0f, 0.0f),
                                    SkPoint::Make(100.0f, 100.0f) };
        fShader0 = SkGradientShader::CreateLinear(points,
                                                  colors,
                                                  NULL,
                                                  3,
                                                  SkShader::kMirror_TileMode,
                                                  0,
                                                  NULL);
    }

    virtual ~PerlinPatchView() {
        SkSafeUnref(fShader0);
    }
protected:
    // overrides from SkEventSink
    bool onQuery(SkEvent* evt)  override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "PerlinPatch");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    bool onAnimate(const SkAnimTimer& timer) override {
        fSeed += 0.005f;
        return true;
    }


    void onDrawContent(SkCanvas* canvas) override {
        if (!canvas->getTotalMatrix().invert(&fInvMatrix)) {
            return;
        }

        SkPaint paint;
        
        SkScalar texWidth = fTexScale * TexWidth;
        SkScalar texHeight = fTexScale * TexHeight;
        const SkPoint texCoords[SkPatchUtils::kNumCorners] = {
            { fTexX - texWidth, fTexY - texHeight},
            { fTexX + texWidth, fTexY - texHeight},
            { fTexX + texWidth, fTexY + texHeight},
            { fTexX - texWidth, fTexY + texHeight}}
        ;
        
        SkAutoTUnref<SkXfermode> xfer(SkXfermode::Create(SkXfermode::kSrc_Mode));

        SkScalar scaleFreq = 2.0;
        fShader1 = SkPerlinNoiseShader2::CreateImprovedNoise(fXFreq/scaleFreq, fYFreq/scaleFreq, 4,
                                                             fSeed);
        fShaderCompose = new SkComposeShader(fShader0, fShader1);

        paint.setShader(fShaderCompose);
        canvas->drawPatch(fPts, nullptr, texCoords, xfer, paint);

        draw_control_points(canvas, fPts);
        SkSafeUnref(fShader1);
        SkSafeUnref(fShaderCompose);
    }

    class PtClick : public Click {
    public:
        int fIndex;
        PtClick(SkView* view, int index) : Click(view), fIndex(index) {}
    };

    static bool hittest(const SkPoint& pt, SkScalar x, SkScalar y) {
        return SkPoint::Length(pt.fX - x, pt.fY - y) < SkIntToScalar(5);
    }

    SkView::Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned modi) override {
        // holding down shift
        if (1 == modi) {
            return new PtClick(this, -1);
        }
        // holding down ctrl
        if (2 == modi) {
            return new PtClick(this, -2);
        }
        SkPoint clickPoint = {x, y};
        fInvMatrix.mapPoints(&clickPoint, 1);
        for (size_t i = 0; i < SK_ARRAY_COUNT(fPts); i++) {
            if (hittest(fPts[i], clickPoint.fX, clickPoint.fY)) {
                return new PtClick(this, (int)i);
            }
        }
        return this->INHERITED::onFindClickHandler(x, y, modi);
    }

    bool onClick(Click* click) override {
        PtClick* ptClick = (PtClick*)click;
        if (ptClick->fIndex >= 0) {
            fPts[ptClick->fIndex].set(click->fCurr.fX , click->fCurr.fY );
        } else if (-1 == ptClick->fIndex) {
            SkScalar xDiff = click->fPrev.fX - click->fCurr.fX;
            SkScalar yDiff = click->fPrev.fY - click->fCurr.fY;
            fTexX += xDiff * fTexScale;
            fTexY += yDiff * fTexScale;
        } else if (-2 == ptClick->fIndex) {
            SkScalar yDiff = click->fCurr.fY - click->fPrev.fY;
            fTexScale += yDiff / 10.0f;
            fTexScale = SkTMax(0.1f, SkTMin(20.f, fTexScale));
        }
        this->inval(nullptr);
        return true;
    }

private:
    typedef SampleView INHERITED;
};

DEF_SAMPLE( return new PerlinPatchView(); )

