/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "Resources.h"
#include "SampleCode.h"
#include "SkCanvas.h"
#include "SkInterpolator.h"
#include "SkSurface.h"
#include "SkRandom.h"
#include "SkTime.h"

#define N   128

static SkImage* make_image() {
    SkImageInfo info = SkImageInfo::MakeN32Premul(N, N);
    SkAutoTUnref<SkSurface> surface(SkSurface::NewRaster(info));
    SkCanvas* canvas = surface->getCanvas();

    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);

    path.addRect(SkRect::MakeWH(N/2, N));
    path.addRect(SkRect::MakeWH(N, N/2));
    path.moveTo(0, 0); path.lineTo(N, 0); path.lineTo(0, N); path.close();

    canvas->drawPath(path, SkPaint());
    return surface->newImageSnapshot();
}

struct AnimValue {
    SkScalar fValue;
    SkScalar fMin;
    SkScalar fMax;
    SkScalar fMod;

    operator SkScalar() const { return fValue; }

    void set(SkScalar value, SkScalar min, SkScalar max) {
        fValue = value;
        fMin = min;
        fMax = max;
        fMod = 0;
    }

    void setMod(SkScalar value, SkScalar mod) {
        fValue = value;
        fMin = 0;
        fMax = 0;
        fMod = mod;
    }

    SkScalar inc(SkScalar delta) {
        fValue += delta;
        return this->fixUp();
    }

    SkScalar fixUp() {
        if (fMod) {
            fValue = SkScalarMod(fValue, fMod);
        } else {
            if (fValue > fMax) {
                fValue = fMax;
            } else if (fValue < fMin) {
                fValue = fMin;
            }
        }
        return fValue;
    }
};

#define ANGLE_DELTA 3
#define SCALE_DELTA (SK_Scalar1 / 32)

class FilterQualityView : public SampleView {
    SkAutoTUnref<SkImage> fImage;
    AnimValue             fScale, fAngle;

    SkInterpolator      fTrans;

public:
    FilterQualityView() : fImage(make_image()), fTrans(2, 2) {
        fScale.set(1, SK_Scalar1 / 8, 1);
        fAngle.setMod(0, 360);

        SkScalar values[2];
        fTrans.setMirror(true);
        fTrans.setReset(true);

        fTrans.setRepeatCount(999);
        values[0] = values[1] = 0;
        fTrans.setKeyFrame(0, SkTime::GetMSecs(), values);
        values[0] = values[1] = 1;
        fTrans.setKeyFrame(1, SkTime::GetMSecs() + 2000, values);
    }

protected:

    // overrides from SkEventSink
    bool onQuery(SkEvent* evt) SK_OVERRIDE {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "FilterQuality");
            return true;
        }
        SkUnichar uni;
        if (SampleCode::CharQ(*evt, &uni)) {
            switch (uni) {
                case '1': fAngle.inc(-ANGLE_DELTA); this->inval(NULL); return true;
                case '2': fAngle.inc( ANGLE_DELTA); this->inval(NULL); return true;
                case '3': fScale.inc(-SCALE_DELTA); this->inval(NULL); return true;
                case '4': fScale.inc( SCALE_DELTA); this->inval(NULL); return true;
                default: break;
            }
        }
        return this->INHERITED::onQuery(evt);
    }

    void drawHere(SkCanvas* canvas, SkScalar x, SkScalar y, SkPaint::FilterLevel filter) {
        SkAutoCanvasRestore acr(canvas, true);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setFilterLevel(filter);

        canvas->translate(x, y);
        canvas->scale(fScale, fScale);
        canvas->rotate(fAngle);
        canvas->drawImage(fImage, -SkScalarHalf(fImage->width()), -SkScalarHalf(fImage->height()),
                          &paint);
    }

    void onDrawContent(SkCanvas* canvas) SK_OVERRIDE {
        SkScalar trans[2];
        fTrans.timeToValues(SkTime::GetMSecs(), trans);
        canvas->translate(trans[0], trans[1]);
        this->inval(NULL);

        const struct {
            SkScalar                fX;
            SkScalar                fY;
            SkPaint::FilterLevel    fFilter;
        } rec[] = {
            { 100, 100, SkPaint::kNone_FilterLevel },
            { 300, 100, SkPaint::kLow_FilterLevel },
            { 100, 300, SkPaint::kMedium_FilterLevel },
            { 300, 300, SkPaint::kHigh_FilterLevel },
        };

        for (size_t i = 0; i < SK_ARRAY_COUNT(rec); ++i) {
            this->drawHere(canvas, rec[i].fX, rec[i].fY, rec[i].fFilter);
        }

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setTextSize(40);
        SkString str;
        str.appendScalar(fScale);
        canvas->drawText(str.c_str(), str.size(), 450, 100, paint);
        str.reset(); str.appendScalar(fAngle);
        canvas->drawText(str.c_str(), str.size(), 450, 150, paint);

        str.reset(); str.appendScalar(trans[0]);
        canvas->drawText(str.c_str(), str.size(), 450, 200, paint);
        str.reset(); str.appendScalar(trans[1]);
        canvas->drawText(str.c_str(), str.size(), 450, 250, paint);
}

    virtual bool handleKey(SkKey key) {
        this->inval(NULL);
        return true;
    }

private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new FilterQualityView; }
static SkViewRegister reg(MyFactory);
