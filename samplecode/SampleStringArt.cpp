/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkCanvas.h"
#include "SkPath.h"

// Reproduces https://code.google.com/p/chromium/issues/detail?id=279014

// Renders a string art shape.
// The particular shape rendered can be controlled by clicking horizontally, thereby
// generating an angle from 0 to 1.

class StringArtView : public SampleView {
public:
    StringArtView() : fAngle(0.305f) {}

protected:
    // overrides from SkEventSink
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "StringArt");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        SkScalar angle = fAngle*SK_ScalarPI + SkScalarHalf(SK_ScalarPI);

        SkPoint center = SkPoint::Make(SkScalarHalf(this->width()), SkScalarHalf(this->height()));
        SkScalar length = 5;
        SkScalar step = angle;

        SkPath path;
        path.moveTo(center);

        while (length < (SkScalarHalf(SkMinScalar(this->width(), this->height())) - 10.f))
        {
            SkPoint rp = SkPoint::Make(length*SkScalarCos(step) + center.fX,
                                       length*SkScalarSin(step) + center.fY);
            path.lineTo(rp);
            length += angle / SkScalarHalf(SK_ScalarPI);
            step += angle;
        }
        path.close();

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(0xFF007700);

        canvas->drawPath(path, paint);
    }

    SkView::Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned) override {
        fAngle = x/width();
        this->inval(nullptr);
        return nullptr;
    }
private:

    SkScalar fAngle;
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new StringArtView; }
static SkViewRegister reg(MyFactory);
