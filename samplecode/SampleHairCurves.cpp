/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"
#include "include/utils/SkRandom.h"
#include "samplecode/Sample.h"

class HairCurvesView : public Sample {
public:
    HairCurvesView() {
    }

protected:
    virtual bool onQuery(Sample::Event* evt) {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "HairCurves");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }


    virtual void onDrawContent(SkCanvas* canvas) {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(0);
        canvas->save();
        canvas->scale(1000 * SK_Scalar1, 1000 * SK_Scalar1);
        SkRandom rand;
        SkRandom randW;
        SkPath curves;
        SkPath hulls;
        SkPath ctrlPts;
        for (int i = 0; i < 100; ++i) {
            SkScalar pts[] = {
                rand.nextUScalar1(), rand.nextUScalar1(),
                rand.nextUScalar1(), rand.nextUScalar1(),
                rand.nextUScalar1(), rand.nextUScalar1(),
                rand.nextUScalar1(), rand.nextUScalar1()
            };
            curves.moveTo(pts[0], pts[1]);
            curves.cubicTo(pts[2], pts[3],
                         pts[4], pts[5],
                         pts[6], pts[7]);

            hulls.moveTo(pts[0], pts[1]);
            hulls.lineTo(pts[2], pts[3]);
            hulls.lineTo(pts[4], pts[5]);
            hulls.lineTo(pts[6], pts[7]);

            ctrlPts.addCircle(pts[0], pts[1], SK_Scalar1 / 200);
            ctrlPts.addCircle(pts[2], pts[3], SK_Scalar1 / 200);
            ctrlPts.addCircle(pts[4], pts[5], SK_Scalar1 / 200);
            ctrlPts.addCircle(pts[6], pts[7], SK_Scalar1 / 200);
        }
        for (int i = 0; i < 100; ++i) {
            SkScalar pts[] = {
                rand.nextUScalar1(), rand.nextUScalar1(),
                rand.nextUScalar1(), rand.nextUScalar1(),
                rand.nextUScalar1(), rand.nextUScalar1(),
            };
            curves.moveTo(pts[0], pts[1]);
            curves.quadTo(pts[2], pts[3],
                          pts[4], pts[5]);

            hulls.moveTo(pts[0], pts[1]);
            hulls.lineTo(pts[2], pts[3]);
            hulls.lineTo(pts[4], pts[5]);

            ctrlPts.addCircle(pts[0], pts[1], SK_Scalar1 / 200);
            ctrlPts.addCircle(pts[2], pts[3], SK_Scalar1 / 200);
            ctrlPts.addCircle(pts[4], pts[5], SK_Scalar1 / 200);
        }
        for (int i = 0; i < 100; ++i) {
            SkScalar pts[] = {
                rand.nextUScalar1(), rand.nextUScalar1(),
                rand.nextUScalar1(), rand.nextUScalar1(),
                rand.nextUScalar1(), rand.nextUScalar1(),
            };
            SkScalar weight = randW.nextUScalar1() * 2.0f;

            curves.moveTo(pts[0], pts[1]);
            curves.conicTo(pts[2], pts[3],
                          pts[4], pts[5],
                          weight);

            hulls.moveTo(pts[0], pts[1]);
            hulls.lineTo(pts[2], pts[3]);
            hulls.lineTo(pts[4], pts[5]);

            ctrlPts.addCircle(pts[0], pts[1], SK_Scalar1 / 200);
            ctrlPts.addCircle(pts[2], pts[3], SK_Scalar1 / 200);
            ctrlPts.addCircle(pts[4], pts[5], SK_Scalar1 / 200);
        }
        for (int i = 0; i < 100; ++i) {
            SkScalar pts[] = {
                rand.nextUScalar1(), rand.nextUScalar1(),
                rand.nextUScalar1(), rand.nextUScalar1(),
            };
            curves.moveTo(pts[0], pts[1]);
            curves.lineTo(pts[2], pts[3]);

            ctrlPts.addCircle(pts[0], pts[1], SK_Scalar1 / 200);
            ctrlPts.addCircle(pts[2], pts[3], SK_Scalar1 / 200);
        }

        paint.setColor(SK_ColorBLACK);
        canvas->drawPath(curves, paint);
        paint.setColor(SK_ColorRED);
        //canvas->drawPath(hulls, paint);
        paint.setStyle(SkPaint::kFill_Style);
        paint.setColor(SK_ColorBLUE);
        //canvas->drawPath(ctrlPts, paint);

        canvas->restore();
    }

private:
    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new HairCurvesView(); )
