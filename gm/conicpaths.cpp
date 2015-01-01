/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkTArray.h"

class ConicPathsGM : public skiagm::GM {
protected:

    SkString onShortName() SK_OVERRIDE {
        return SkString("conicpaths");
    }

    SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(950, 1000);
    }

    void onOnceBeforeDraw() SK_OVERRIDE {
        {
            const SkScalar w = SkScalarSqrt(2)/2;
            SkPath* conicCirlce = &fPaths.push_back();
            conicCirlce->moveTo(0, -0);
            conicCirlce->conicTo(0, 50, 50, 50, w);
            conicCirlce->rConicTo(50, 0, 50, -50, w);
            conicCirlce->rConicTo(0, -50, -50, -50, w);
            conicCirlce->rConicTo(-50, 0, -50, 50, w);

        }
        {
            SkPath* hyperbola = &fPaths.push_back();
            hyperbola->moveTo(0, -0);
            hyperbola->conicTo(0, 100, 100, 100, 2);
        }
        {
            SkPath* thinHyperbola = &fPaths.push_back();
            thinHyperbola->moveTo(0, -0);
            thinHyperbola->conicTo(100, 100, 5, 0, 2);
        }
        {
            SkPath* veryThinHyperbola = &fPaths.push_back();
            veryThinHyperbola->moveTo(0, -0);
            veryThinHyperbola->conicTo(100, 100, 1, 0, 2);
        }
        {
            SkPath* closedHyperbola = &fPaths.push_back();
            closedHyperbola->moveTo(0, -0);
            closedHyperbola->conicTo(100, 100, 0, 0, 2);
        }
        {
            // using 1 as weight defaults to using quadTo
            SkPath* nearParabola = &fPaths.push_back();
            nearParabola->moveTo(0, -0);
            nearParabola->conicTo(0, 100, 100, 100, 0.999f);
        }
        {
            SkPath* thinEllipse = &fPaths.push_back();
            thinEllipse->moveTo(0, -0);
            thinEllipse->conicTo(100, 100, 5, 0, SK_ScalarHalf);
        }
        {
            SkPath* veryThinEllipse = &fPaths.push_back();
            veryThinEllipse->moveTo(0, -0);
            veryThinEllipse->conicTo(100, 100, 1, 0, SK_ScalarHalf);
        }
        {
            SkPath* closedEllipse = &fPaths.push_back();
            closedEllipse->moveTo(0, -0);
            closedEllipse->conicTo(100, 100, 0, 0, SK_ScalarHalf);
        }
    }

    void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        const SkAlpha kAlphaValue[] = { 0xFF, 0x40 };

        SkScalar maxH = 0;
        const SkScalar margin = 15;
        canvas->translate(margin, margin);
        canvas->save();

        SkScalar x = margin;
        int counter = 0;
        for (int p = 0; p < fPaths.count(); ++p) {
            for (size_t a = 0; a < SK_ARRAY_COUNT(kAlphaValue); ++a) {
                for (int aa = 0; aa < 2; ++aa) {
                    for (int fh = 0; fh < 2; ++fh) {

                        const SkRect& bounds = fPaths[p].getBounds();

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

                        SkScalar dx = bounds.width() + margin;
                        x += dx;
                        canvas->translate(dx, 0);

                        if (++counter == 8) {
                            counter = 0;
                            
                            canvas->restore();
                            canvas->translate(0, maxH + margin);
                            canvas->save();
                            maxH = 0;
                            x = margin;
                        }
                    }
                }
            }
        }
        canvas->restore();
    }

    uint32_t onGetFlags() const SK_OVERRIDE {
        return kSkipPDF_Flag;
    }

private:
    SkTArray<SkPath> fPaths;
    typedef skiagm::GM INHERITED;
};
DEF_GM( return SkNEW(ConicPathsGM); )

//////////////////////////////////////////////////////////////////////////////

