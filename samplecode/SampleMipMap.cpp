
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkPaint.h"
#include "SkShader.h"

static SkBitmap createBitmap(int n) {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(n, n);
    bitmap.eraseColor(SK_ColorTRANSPARENT);

    SkCanvas canvas(bitmap);
    SkRect r;
    r.set(0, 0, SkIntToScalar(n), SkIntToScalar(n));
    SkPaint paint;
    paint.setAntiAlias(true);

    paint.setColor(SK_ColorRED);
    canvas.drawOval(r, paint);
    paint.setColor(SK_ColorBLUE);
    paint.setStrokeWidth(SkIntToScalar(n)/15);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas.drawLine(0, 0, r.fRight, r.fBottom, paint);
    canvas.drawLine(0, r.fBottom, r.fRight, 0, paint);

    return bitmap;
}

class MipMapView : public SampleView {
    SkBitmap fBitmap;
    enum {
        N = 64
    };
    bool fOnce;
public:
    MipMapView() {
        fOnce = false;
    }

    void init() {
        if (fOnce) {
            return;
        }
        fOnce = true;

        fBitmap = createBitmap(N);

        fWidth = N;
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "MipMaps");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        this->init();

        static const SkPaint::FilterLevel gLevel[] = {
            SkPaint::kNone_FilterLevel,
            SkPaint::kLow_FilterLevel,
            SkPaint::kMedium_FilterLevel,
            SkPaint::kHigh_FilterLevel,
        };

        SkPaint paint;

        for (size_t i = 0; i < SK_ARRAY_COUNT(gLevel); ++i) {
            SkScalar x = 10.0f + i * 100;
            SkScalar y = 10.0f;

            paint.setFilterLevel(gLevel[i]);

            canvas->drawBitmap(fBitmap, x, y, &paint);
        }
        this->inval(NULL);
    }

private:
    int fWidth;

    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new MipMapView; }
static SkViewRegister reg(MyFactory);
