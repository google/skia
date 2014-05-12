/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkTime.h"
#include "SkTypeface.h"
#include "SkXfermode.h"

#include "SkStream.h"
#include "SkXMLParser.h"

class PointsView : public SampleView {
public:
    PointsView() {}

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Points");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    static void fill_pts(SkPoint pts[], size_t n, SkRandom* rand) {
        for (size_t i = 0; i < n; i++)
            pts[i].set(rand->nextUScalar1() * 640, rand->nextUScalar1() * 480);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        canvas->translate(SK_Scalar1, SK_Scalar1);

        SkRandom rand;
        SkPaint  p0, p1, p2, p3;
        const size_t n = 99;

        p0.setColor(SK_ColorRED);
        p1.setColor(SK_ColorGREEN);
        p2.setColor(SK_ColorBLUE);
        p3.setColor(SK_ColorWHITE);

        p0.setStrokeWidth(SkIntToScalar(4));
        p2.setStrokeCap(SkPaint::kRound_Cap);
        p2.setStrokeWidth(SkIntToScalar(6));

        SkPoint* pts = new SkPoint[n];
        fill_pts(pts, n, &rand);

        canvas->drawPoints(SkCanvas::kPolygon_PointMode, n, pts, p0);
        canvas->drawPoints(SkCanvas::kLines_PointMode, n, pts, p1);
        canvas->drawPoints(SkCanvas::kPoints_PointMode, n, pts, p2);
        canvas->drawPoints(SkCanvas::kPoints_PointMode, n, pts, p3);

        delete[] pts;
    }

private:

    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new PointsView; }
static SkViewRegister reg(MyFactory);
