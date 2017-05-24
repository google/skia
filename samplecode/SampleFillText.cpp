/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"

#include "SkCanvas.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkRRect.h"
#include "SkTypeface.h"

class FillTextView : public SampleView {
public:
    FillTextView() {}

    SkPaint fPaint;
    SkRandom fRandom;

protected:
    void onOnceBeforeDraw() override {
        fPaint.setTypeface(SkTypeface::MakeFromName("Verdana", SkFontStyle()));
        fPaint.setTextSize(12);
        fPaint.setColor(SK_ColorRED);
        fPaint.setAntiAlias(true);
    }

    // overrides from SkEventSink
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "FillText");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        char string[32];
        for (int di = 0; di < 1000; di++) {
            int num = fRandom.nextF() * 1000;
            sprintf(string, "%d", num);
            canvas->drawString(string,
                               SkScalarSin(di) * canvas->getBaseLayerSize().width(),
                               SkScalarCos(di) * canvas->getBaseLayerSize().height(),
                               fPaint);
        }
    }

    bool onAnimate(const SkAnimTimer&) override {
        return true;
    }

private:

    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new FillTextView; }
static SkViewRegister reg(MyFactory);
