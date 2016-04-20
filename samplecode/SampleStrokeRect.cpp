/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkPaint.h"

class StrokeRectSample : public SampleView {
public:
    StrokeRectSample() {}

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Stroke Rects");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(SkIntToScalar(20));

        SkPaint hair;
        hair.setStyle(SkPaint::kStroke_Style);
        hair.setColor(SK_ColorRED);

        static const SkISize gSize[] = {
            {   100,   50 },
            {   100,    0 },
            {     0,   50 },
            {     0,    0 }
        };

        static const SkPaint::Join gJoin[] = {
            SkPaint::kMiter_Join,
            SkPaint::kRound_Join,
            SkPaint::kBevel_Join
        };

        canvas->translate(paint.getStrokeWidth(), paint.getStrokeWidth());
        for (size_t i = 0; i < SK_ARRAY_COUNT(gJoin); ++i) {
            paint.setStrokeJoin(gJoin[i]);

            canvas->save();
            for (size_t j = 0; j < SK_ARRAY_COUNT(gSize); ++j) {
                SkRect r = SkRect::MakeWH(SkIntToScalar(gSize[j].fWidth),
                                          SkIntToScalar(gSize[j].fHeight));
                canvas->drawRect(r, paint);
                canvas->drawRect(r, hair);
                canvas->translate(0, SkIntToScalar(100));
            }
            canvas->restore();
            canvas->translate(SkIntToScalar(150), 0);
        }
    }

private:
    typedef SampleView INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new StrokeRectSample; }
static SkViewRegister reg(MyFactory);
