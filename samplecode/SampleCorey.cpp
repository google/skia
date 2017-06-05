
/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkAnimTimer.h"
#include "SkCanvas.h"
#include "SkDashPathEffect.h"

////////////////////////////////////////////////////////////////////////////

class CoreyView : public SampleView {
public:
    CoreyView() {}

protected:
    void onOnceBeforeDraw() override {}

    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(0xFF000000);
    }

    void onDrawContent(SkCanvas* canvas) override {
        this->drawBG(canvas);

        SkISize size = canvas->getBaseLayerSize();
        canvas->save();
        canvas->translate(size.fWidth / 2.0f - 50.0f, size.fHeight / 2.0f - 50.0f);
        SkPaint paint;
        paint.setColor(SkColorSetARGB(255, 101, 198, 225));
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(26);
        paint.setStrokeCap(SkPaint::Cap::kRound_Cap);
        const SkScalar intervals[] = { fStrokeLength, 314.0f - fStrokeLength };
        paint.setPathEffect(SkDashPathEffect::Make(intervals, 2, 0));
        SkRect rect = { 0, 0, 100.0f, 100.0f };
        canvas->drawOval(rect, paint);
    }

    bool onAnimate(const SkAnimTimer& timer) override {
        //timer.pingPong(10, 0, 314, 314)
        //timer.pingPong(10, 0, 10, 314);
        fStrokeLength = timer.pingPong(20, 0, 180, 0);
        return true;
    }

    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "CoreysBug");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

private:
    SkScalar fStrokeLength = 180.0f;

    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new CoreyView; }
static SkViewRegister reg(MyFactory);
