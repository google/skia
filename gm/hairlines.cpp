/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkTArray.h"

namespace skiagm {

class HairlinesGM : public GM {
protected:

    virtual SkString onShortName() SK_OVERRIDE {
        return SkString("hairlines");
    }

    virtual SkISize onISize() { return make_isize(800, 600); }

    virtual void onOnceBeforeDraw() SK_OVERRIDE {
        {
            SkPath* lineAnglesPath = &fPaths.push_back();
            enum {
                kNumAngles = 15,
                kRadius = 40,
            };
            for (int i = 0; i < kNumAngles; ++i) {
                SkScalar angle = SK_ScalarPI * SkIntToScalar(i) / kNumAngles;
                SkScalar x = kRadius * SkScalarCos(angle);
                SkScalar y = kRadius * SkScalarSin(angle);
                lineAnglesPath->moveTo(x, y);
                lineAnglesPath->lineTo(-x, -y);
            }
        }

        {
            SkPath* kindaTightQuad = &fPaths.push_back();
            kindaTightQuad->moveTo(0, -10 * SK_Scalar1);
            kindaTightQuad->quadTo(SkIntToScalar(100), SkIntToScalar(100), -10 * SK_Scalar1, 0);
        }

        {
            SkPath* tightQuad = &fPaths.push_back();
            tightQuad->moveTo(0, -5 * SK_Scalar1);
            tightQuad->quadTo(SkIntToScalar(100), SkIntToScalar(100), -5 * SK_Scalar1, 0);
        }

        {
            SkPath* tighterQuad = &fPaths.push_back();
            tighterQuad->moveTo(0, -2 * SK_Scalar1);
            tighterQuad->quadTo(SkIntToScalar(100), SkIntToScalar(100), -2 * SK_Scalar1, 0);
        }

        {
            SkPath* unevenTighterQuad = &fPaths.push_back();
            unevenTighterQuad->moveTo(0, -1 * SK_Scalar1);
            SkPoint p;
            p.set(-2 * SK_Scalar1 + 3 * SkIntToScalar(102) / 4, SkIntToScalar(75));
            unevenTighterQuad->quadTo(SkIntToScalar(100), SkIntToScalar(100), p.fX, p.fY);
        }

        {
            SkPath* reallyTightQuad = &fPaths.push_back();
            reallyTightQuad->moveTo(0, -1 * SK_Scalar1);
            reallyTightQuad->quadTo(SkIntToScalar(100), SkIntToScalar(100), -1 * SK_Scalar1, 0);
        }

        {
            SkPath* closedQuad = &fPaths.push_back();
            closedQuad->moveTo(0, -0);
            closedQuad->quadTo(SkIntToScalar(100), SkIntToScalar(100), 0, 0);
        }

        {
            SkPath* unevenClosedQuad = &fPaths.push_back();
            unevenClosedQuad->moveTo(0, -0);
            unevenClosedQuad->quadTo(SkIntToScalar(100), SkIntToScalar(100),
                                     SkIntToScalar(75), SkIntToScalar(75));
        }
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        static const SkAlpha kAlphaValue[] = { 0xFF, 0x40 };

        enum {
            kMargin = 5,
        };
        int wrapX = canvas->getDeviceSize().fWidth - kMargin;

        SkScalar maxH = 0;
        canvas->translate(SkIntToScalar(kMargin), SkIntToScalar(kMargin));
        canvas->save();

        SkScalar x = SkIntToScalar(kMargin);
        for (int p = 0; p < fPaths.count(); ++p) {
            for (size_t a = 0; a < SK_ARRAY_COUNT(kAlphaValue); ++a) {
                for (int aa = 0; aa < 2; ++aa) {
                    const SkRect& bounds = fPaths[p].getBounds();

                    if (x + bounds.width() > wrapX) {
                        canvas->restore();
                        canvas->translate(0, maxH + SkIntToScalar(kMargin));
                        canvas->save();
                        maxH = 0;
                        x = SkIntToScalar(kMargin);
                    }

                    SkPaint paint;
                    paint.setARGB(kAlphaValue[a], 0, 0, 0);
                    paint.setAntiAlias(SkToBool(aa));
                    paint.setStyle(SkPaint::kStroke_Style);
                    paint.setStrokeWidth(0);

                    canvas->save();
                    canvas->translate(-bounds.fLeft, -bounds.fTop);
                    canvas->drawPath(fPaths[p], paint);
                    canvas->restore();

                    maxH = SkMaxScalar(maxH, bounds.height());

                    SkScalar dx = bounds.width() + SkIntToScalar(kMargin);
                    x += dx;
                    canvas->translate(dx, 0);
                }
            }
        }
        canvas->restore();
    }

private:
    SkTArray<SkPath> fPaths;
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new HairlinesGM; }
static GMRegistry reg(MyFactory);

}
