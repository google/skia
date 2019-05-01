/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkGeometry.h"

#include <math.h>

namespace skiagm {

// Slices paths into sliver-size contours shaped like ice cream cones.
class MandolineSlicer {
public:
    static constexpr int kDefaultSubdivisions = 10;

    MandolineSlicer(SkPoint anchorPt) {
        fPath.setFillType(SkPath::kEvenOdd_FillType);
        fPath.setIsVolatile(true);
        this->reset(anchorPt);
    }

    void reset(SkPoint anchorPt) {
        fPath.reset();
        fLastPt = fAnchorPt = anchorPt;
    }

    void sliceLine(SkPoint pt, int numSubdivisions = kDefaultSubdivisions) {
        if (numSubdivisions <= 0) {
            fPath.moveTo(fAnchorPt);
            fPath.lineTo(fLastPt);
            fPath.lineTo(pt);
            fPath.close();
            fLastPt = pt;
            return;
        }
        float T = this->chooseChopT(numSubdivisions);
        if (0 == T) {
            return;
        }
        SkPoint midpt = fLastPt * (1 - T) + pt * T;
        this->sliceLine(midpt, numSubdivisions - 1);
        this->sliceLine(pt, numSubdivisions - 1);
    }

    void sliceQuadratic(SkPoint p1, SkPoint p2, int numSubdivisions = kDefaultSubdivisions) {
        if (numSubdivisions <= 0) {
            fPath.moveTo(fAnchorPt);
            fPath.lineTo(fLastPt);
            fPath.quadTo(p1, p2);
            fPath.close();
            fLastPt = p2;
            return;
        }
        float T = this->chooseChopT(numSubdivisions);
        if (0 == T) {
            return;
        }
        SkPoint P[3] = {fLastPt, p1, p2}, PP[5];
        SkChopQuadAt(P, PP, T);
        this->sliceQuadratic(PP[1], PP[2], numSubdivisions - 1);
        this->sliceQuadratic(PP[3], PP[4], numSubdivisions - 1);
    }

    void sliceCubic(SkPoint p1, SkPoint p2, SkPoint p3,
                    int numSubdivisions = kDefaultSubdivisions) {
        if (numSubdivisions <= 0) {
            fPath.moveTo(fAnchorPt);
            fPath.lineTo(fLastPt);
            fPath.cubicTo(p1, p2, p3);
            fPath.close();
            fLastPt = p3;
            return;
        }
        float T = this->chooseChopT(numSubdivisions);
        if (0 == T) {
            return;
        }
        SkPoint P[4] = {fLastPt, p1, p2, p3}, PP[7];
        SkChopCubicAt(P, PP, T);
        this->sliceCubic(PP[1], PP[2], PP[3], numSubdivisions - 1);
        this->sliceCubic(PP[4], PP[5], PP[6], numSubdivisions - 1);
    }

    void sliceConic(SkPoint p1, SkPoint p2, float w, int numSubdivisions = kDefaultSubdivisions) {
        if (numSubdivisions <= 0) {
            fPath.moveTo(fAnchorPt);
            fPath.lineTo(fLastPt);
            fPath.conicTo(p1, p2, w);
            fPath.close();
            fLastPt = p2;
            return;
        }
        float T = this->chooseChopT(numSubdivisions);
        if (0 == T) {
            return;
        }
        SkConic conic(fLastPt, p1, p2, w), halves[2];
        if (!conic.chopAt(T, halves)) {
            SK_ABORT("SkConic::chopAt failed");
        }
        this->sliceConic(halves[0].fPts[1], halves[0].fPts[2], halves[0].fW, numSubdivisions - 1);
        this->sliceConic(halves[1].fPts[1], halves[1].fPts[2], halves[1].fW, numSubdivisions - 1);
    }

    const SkPath& path() const { return fPath; }

private:
    float chooseChopT(int numSubdivisions) {
        SkASSERT(numSubdivisions > 0);
        if (numSubdivisions > 1) {
            return .5f;
        }
        float T = (0 == fRand.nextU() % 10) ? 0 : scalbnf(1, -(int)fRand.nextRangeU(10, 149));
        SkASSERT(T >= 0 && T < 1);
        return T;
    }

    SkRandom fRand;
    SkPath fPath;
    SkPoint fAnchorPt;
    SkPoint fLastPt;
};

class SliverPathsGM : public GM {
public:
    SliverPathsGM() {
        this->setBGColor(SK_ColorBLACK);
    }

protected:
    SkString onShortName() override {
        return SkString("mandoline");
    }

    SkISize onISize() override {
        return SkISize::Make(560, 475);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setColor(SK_ColorWHITE);
        paint.setAntiAlias(true);

        MandolineSlicer mandoline({41, 43});
        mandoline.sliceCubic({5, 277}, {381, -74}, {243, 162});
        mandoline.sliceLine({41, 43});
        canvas->drawPath(mandoline.path(), paint);

        mandoline.reset({357.049988f, 446.049988f});
        mandoline.sliceCubic({472.750000f, -71.950012f}, {639.750000f, 531.950012f},
                             {309.049988f, 347.950012f});
        mandoline.sliceLine({309.049988f, 419});
        mandoline.sliceLine({357.049988f, 446.049988f});
        canvas->drawPath(mandoline.path(), paint);

        canvas->save();
        canvas->translate(421, 105);
        canvas->scale(100, 81);
        mandoline.reset({-cosf(SkDegreesToRadians(-60)), sinf(SkDegreesToRadians(-60))});
        mandoline.sliceConic({-2, 0},
                             {-cosf(SkDegreesToRadians(60)), sinf(SkDegreesToRadians(60))}, .5f);
        mandoline.sliceConic({-cosf(SkDegreesToRadians(120))*2, sinf(SkDegreesToRadians(120))*2},
                             {1, 0}, .5f);
        mandoline.sliceLine({0, 0});
        mandoline.sliceLine({-cosf(SkDegreesToRadians(-60)), sinf(SkDegreesToRadians(-60))});
        canvas->drawPath(mandoline.path(), paint);
        canvas->restore();

        canvas->save();
        canvas->translate(150, 300);
        canvas->scale(75, 75);
        mandoline.reset({1, 0});
        constexpr int nquads = 5;
        for (int i = 0; i < nquads; ++i) {
            float theta1 = 2*SK_ScalarPI/nquads * (i + .5f);
            float theta2 = 2*SK_ScalarPI/nquads * (i + 1);
            mandoline.sliceQuadratic({cosf(theta1)*2, sinf(theta1)*2},
                                     {cosf(theta2), sinf(theta2)});
        }
        canvas->drawPath(mandoline.path(), paint);
        canvas->restore();
    }
};

DEF_GM(return new SliverPathsGM;)

}
