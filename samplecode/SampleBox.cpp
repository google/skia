
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"

class SimpleView : public SampleView {
public:
    SimpleView() {
        this->setBGColor(0xFFDDDDDD);
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt)  {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Box Gradient");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(SkScalarHalf(SkIntToScalar(3)));
        paint.setStyle(SkPaint::kFill_Style);

        SkRect  r;
        SkScalar x,y;
        x = 10;
        y = 10;

        r.set(x, y, x + SkIntToScalar(100), y + SkIntToScalar(100));
        for (int i = 0; i < 256; ++i) {
            canvas->translate(1, 1);
            paint.setColor(0xFF000000 + i * 0x00010000);
            canvas->drawRect(r, paint);
        }
    }

private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new SimpleView; }
static SkViewRegister reg(MyFactory);
