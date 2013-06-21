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

class ConicPathsGM : public GM {
protected:

    virtual SkString onShortName() SK_OVERRIDE {
        return SkString("conicpaths");
    }

    virtual SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(1000, 1000);
    }

    virtual void onOnceBeforeDraw() SK_OVERRIDE {
        {
            SkPath* conicCirlce = &fPaths.push_back();
            conicCirlce->moveTo(0, -0);
            conicCirlce->conicTo(SkIntToScalar(0), SkIntToScalar(50),
                                        SkIntToScalar(50), SkIntToScalar(50),
                                        SkScalarHalf(SkScalarSqrt(2)));
            conicCirlce->rConicTo(SkIntToScalar(50), SkIntToScalar(0),
                                         SkIntToScalar(50), SkIntToScalar(-50),
                                         SkScalarHalf(SkScalarSqrt(2)));
            conicCirlce->rConicTo(SkIntToScalar(0), SkIntToScalar(-50),
                                         SkIntToScalar(-50), SkIntToScalar(-50),
                                         SkScalarHalf(SkScalarSqrt(2)));
            conicCirlce->rConicTo(SkIntToScalar(-50), SkIntToScalar(0),
                                         SkIntToScalar(-50), SkIntToScalar(50),
                                         SkScalarHalf(SkScalarSqrt(2)));

        }
        {
            SkPath* hyperbola = &fPaths.push_back();
            hyperbola->moveTo(0, -0);
            hyperbola->conicTo(SkIntToScalar(0), SkIntToScalar(100),
                                        SkIntToScalar(100), SkIntToScalar(100),
                                        SkIntToScalar(2));
        }
        {
            SkPath* thinHyperbola = &fPaths.push_back();
            thinHyperbola->moveTo(0, -0);
            thinHyperbola->conicTo(SkIntToScalar(100), SkIntToScalar(100),
                                        SkIntToScalar(5), SkIntToScalar(0),
                                        SkIntToScalar(2));
        }
        {
            SkPath* veryThinHyperbola = &fPaths.push_back();
            veryThinHyperbola->moveTo(0, -0);
            veryThinHyperbola->conicTo(SkIntToScalar(100), SkIntToScalar(100),
                                        SkIntToScalar(1), SkIntToScalar(0),
                                        SkIntToScalar(2));
        }
        {
            SkPath* closedHyperbola = &fPaths.push_back();
            closedHyperbola->moveTo(0, -0);
            closedHyperbola->conicTo(SkIntToScalar(100), SkIntToScalar(100),
                                        SkIntToScalar(0), SkIntToScalar(0),
                                        SkIntToScalar(2));
        }
        {
            // using 1 as weight defaults to using quadTo
            SkPath* nearParabola = &fPaths.push_back();
            nearParabola->moveTo(0, -0);
            nearParabola->conicTo(SkIntToScalar(0), SkIntToScalar(100),
                                        SkIntToScalar(100), SkIntToScalar(100),
                                        0.999f);
        }
        {
            SkPath* thinEllipse = &fPaths.push_back();
            thinEllipse->moveTo(0, -0);
            thinEllipse->conicTo(SkIntToScalar(100), SkIntToScalar(100),
                                        SkIntToScalar(5), SkIntToScalar(0),
                                        SK_ScalarHalf);
        }
        {
            SkPath* veryThinEllipse = &fPaths.push_back();
            veryThinEllipse->moveTo(0, -0);
            veryThinEllipse->conicTo(SkIntToScalar(100), SkIntToScalar(100),
                                        SkIntToScalar(1), SkIntToScalar(0),
                                        SK_ScalarHalf);
        }
        {
            SkPath* closedEllipse = &fPaths.push_back();
            closedEllipse->moveTo(0, -0);
            closedEllipse->conicTo(SkIntToScalar(100), SkIntToScalar(100),
                                        SkIntToScalar(0), SkIntToScalar(0),
                                        SK_ScalarHalf);
        }
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        static const SkAlpha kAlphaValue[] = { 0xFF, 0x40 };

        enum {
            kMargin = 15,
        };
        int wrapX = canvas->getDeviceSize().fWidth - kMargin;

        SkScalar maxH = 0;
        canvas->translate(SkIntToScalar(kMargin), SkIntToScalar(kMargin));
        canvas->save();

        SkScalar x = SkIntToScalar(kMargin);
        for (int p = 0; p < fPaths.count(); ++p) {
            for (size_t a = 0; a < SK_ARRAY_COUNT(kAlphaValue); ++a) {
                for (int aa = 0; aa < 2; ++aa) {
                    for (int fh = 0; fh < 2; ++fh) {

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
                        if (fh == 1) {
                            paint.setStyle(SkPaint::kStroke_Style);
                            paint.setStrokeWidth(0);
                        } else if (fh == 0) {
                            paint.setStyle(SkPaint::kFill_Style);
                        }
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
        }
        canvas->restore();
    }

    virtual uint32_t onGetFlags() const SK_OVERRIDE {
        return  kSkipPDF_Flag;
    }

private:
    SkTArray<SkPath> fPaths;
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return SkNEW(ConicPathsGM); )
}
