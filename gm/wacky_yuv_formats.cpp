/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkPath.h"

static SkPoint intersect(const SkPoint& o1, const SkVector& v1,
                         const SkPoint& o2, const SkVector& v2) {
    SkScalar t = ((o2.fX - o1.fX) * v2.fY - (o2.fY - o1.fY) * v2.fX) / (v1.fX * v2.fY - v1.fY * v2.fX);

    SkPoint o = { o1.fX + t * v1.fX, o1.fY + t * v1.fY };
    return o;
#if 0
    x1 = o1.fX;
        y1 = o1.fY;
        (x2 - x1) = v1.fX;
        (y2 - y1) = v1.fY;
        x3 = o2.fX;
        y3 = o2.fY;
        (x4 - x3) = v2.fX;
        (y4 - y3) = v2.fY;
#endif

}

static SkPath foo(SkScalar innerRadius, SkScalar outerRadius, int numLobes) {
    if (numLobes <= 1) {
        return SkPath();
    }

    SkPath p;

    int numDivisions = 2 * numLobes;
    SkScalar lobeRadians = SK_ScalarPI / numLobes;
    SkMatrix m, invM;
    m.setRotate(SkRadiansToDegrees(lobeRadians));
    invM.setRotate(-SkRadiansToDegrees(lobeRadians));
    SkVector curV = SkVector::Make(0.0f, 1.0f);
    SkVector prevV = invM.mapVector(curV.fX, curV.fY);
    SkVector nextV = m.mapVector(curV.fX, curV.fY);

    p.moveTo(innerRadius * curV.fX, innerRadius * curV.fY);

    for (int i = 0; i < numDivisions; ++i) {

        if (0 == (i % 2)) {
            SkPoint top = SkPoint::Make(outerRadius * curV.fX, outerRadius * curV.fY);
            SkPoint nextTop = SkPoint::Make(outerRadius * nextV.fX, outerRadius * nextV.fY);

            SkPoint topMid = intersect(top, { -curV.fY, curV.fX},
                                       nextTop, { nextV.fY, -nextV.fX });


            p.lineTo(top);
            p.lineTo(topMid);
            p.lineTo(nextTop);
        } else {
            SkPoint bot = SkPoint::Make(innerRadius * curV.fX, innerRadius * curV.fY);
            SkPoint nextBot = SkPoint::Make(innerRadius * nextV.fX, innerRadius * nextV.fY);

            SkPoint botMid = intersect(bot, { -curV.fY, curV.fX},
                                       nextBot, { nextV.fY, -nextV.fX });
            p.lineTo(bot);
            p.lineTo(botMid);
            p.lineTo(nextBot);
        }

        prevV = curV;
        curV = nextV;
        nextV = m.mapVector(curV.fX, curV.fY);
    }

    p.close();

    return p;
}

namespace skiagm {

class WackyYUVFormatsGM : public GM {
public:
    WackyYUVFormatsGM() {
        this->setBGColor(0xFFCCCCCC);
    }

protected:

    SkString onShortName() override {
        return SkString("wacky_yuv_formats");
    }

    SkISize onISize() override {
        return SkISize::Make(64, 64);
    }

    void onOnceBeforeDraw() override {
        fPath = foo(10.0f, 20.0f, 3);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint p;

        //p.setStyle(SkPaint::kStroke_Style);

        canvas->translate(20.0f, 20.0f);
        canvas->drawPath(fPath, p);
    }

private:
    SkPath fPath;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new WackyYUVFormatsGM;)
}
