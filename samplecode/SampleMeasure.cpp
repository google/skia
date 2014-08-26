
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
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "Sk1DPathEffect.h"
#include "SkCornerPathEffect.h"
#include "SkPathMeasure.h"
#include "SkRandom.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkDither.h"

// exercise scale/linear/devkern
struct Setting {
    bool        fLinearText;
    bool        fDevKernText;
};

static const Setting gSettings[] = {
    { false,  false   },
    { false,  true    },
    { true,   false   },
    { true,   true    },
};

static void doMeasure(SkCanvas* canvas, const SkPaint& paint, const char text[]) {
    SkScalar    dy = paint.getFontMetrics(NULL);

    size_t      len = strlen(text);
    SkAutoTMalloc<SkScalar> autoWidths(len);
    SkScalar*   widths = autoWidths.get();
    SkAutoTMalloc<SkRect> autoRects(len);
    SkRect*     rects = autoRects.get();
    SkRect      bounds;

    SkPaint p(paint);
    for (size_t i = 0; i < SK_ARRAY_COUNT(gSettings); i++) {
        p.setLinearText(gSettings[i].fLinearText);
        p.setDevKernText(gSettings[i].fDevKernText);

        int n = p.getTextWidths(text, len, widths, rects);
        SkScalar w = p.measureText(text, len, &bounds);

        p.setStyle(SkPaint::kFill_Style);
        p.setColor(0x8888FF88);
        canvas->drawRect(bounds, p);
        p.setColor(0xFF000000);
        canvas->drawText(text, len, 0, 0, p);

        p.setStyle(SkPaint::kStroke_Style);
        p.setStrokeWidth(0);
        p.setColor(0xFFFF0000);
        SkScalar x = 0;
        for (int j = 0; j < n; j++) {
            SkRect r = rects[j];
            r.offset(x, 0);
            canvas->drawRect(r, p);
            x += widths[j];
        }

        p.setColor(0xFF0000FF);
        canvas->drawLine(0, 0, w, 0, p);
        p.setStrokeWidth(SkIntToScalar(4));
        canvas->drawPoint(x, 0, p);

        canvas->translate(0, dy);
    }
}

class MeasureView : public SampleView {
public:
    SkPaint fPaint;

    MeasureView() {
        fPaint.setAntiAlias(true);
        fPaint.setTextSize(SkIntToScalar(64));
        this->setBGColor(0xFFDDDDDD);
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Measure");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        canvas->translate(fPaint.getTextSize(), fPaint.getTextSize());
        doMeasure(canvas, fPaint, "Hamburgefons");
    }

private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new MeasureView; }
static SkViewRegister reg(MyFactory);
