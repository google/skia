/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkGradientShader.h"
#include "SkPath.h"
#include "SkUtils.h"
#include "SkView.h"

static void draw_rect(SkCanvas* canvas, const SkRect& r, const SkPaint& p) {
    canvas->drawRect(r, p);

    SkPaint frame(p);
    frame.setShader(nullptr);
    frame.setStyle(SkPaint::kStroke_Style);
    canvas->drawRect(r, frame);
}

static void draw_gradient(SkCanvas* canvas) {
    SkRect r = { 0, 0, SkIntToScalar(256), SkIntToScalar(32) };
    SkPoint pts[] = { { r.fLeft, r.fTop }, { r.fRight, r.fTop } };
    SkColor colors[] = { 0xFF000000, 0xFFFF0000 };
    SkPaint p;
    p.setShader(SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkShader::kClamp_TileMode));
    draw_rect(canvas, r, p);

    canvas->translate(0, SkIntToScalar(40));
    p.setDither(true);
    draw_rect(canvas, r, p);
}

static bool test_pathregion() {
    SkPath path;
    SkRegion region;
    path.moveTo(25071800.f, -141823808.f);
    path.lineTo(25075500.f, -141824000.f);
    path.lineTo(25075400.f, -141827712.f);
    path.lineTo(25071810.f, -141827600.f);
    path.close();

    SkIRect bounds;
    path.getBounds().round(&bounds);
    SkRegion clip(bounds);
    return region.setPath(path, clip); // <-- !! DOWN !!
}

static SkBitmap make_bitmap() {
    SkPMColor c[256];
    for (int i = 0; i < 256; i++) {
        c[i] = SkPackARGB32(0xFF, i, 0, 0);
    }
    SkColorTable* ctable = new SkColorTable(c, 256);

    SkBitmap bm;
    bm.allocPixels(SkImageInfo::Make(256, 32, kIndex_8_SkColorType, kPremul_SkAlphaType),
                   nullptr, ctable);
    ctable->unref();

    bm.lockPixels();
    for (int y = 0; y < bm.height(); y++) {
        uint8_t* p = bm.getAddr8(0, y);
        for (int x = 0; x < 256; x++) {
            p[x] = x;
        }
    }
    bm.unlockPixels();
    return bm;
}

class DitherBitmapView : public SampleView {
    SkBitmap    fBM8;
    SkBitmap    fBM32;
    bool        fResult;
public:
    DitherBitmapView() {
        fResult = test_pathregion();
        fBM8 = make_bitmap();
        fBM8.copyTo(&fBM32, kN32_SkColorType);

        this->setBGColor(0xFFDDDDDD);
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "DitherBitmap");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    static void setBitmapOpaque(SkBitmap* bm, bool isOpaque) {
        SkAutoLockPixels alp(*bm);  // needed for ctable
        bm->setAlphaType(isOpaque ? kOpaque_SkAlphaType : kPremul_SkAlphaType);
    }

    static void draw2(SkCanvas* canvas, const SkBitmap& bm) {
        SkPaint paint;
        SkBitmap bitmap(bm);

        setBitmapOpaque(&bitmap, false);
        paint.setDither(false);
        canvas->drawBitmap(bitmap, 0, 0, &paint);
        paint.setDither(true);
        canvas->drawBitmap(bitmap, 0, SkIntToScalar(bm.height() + 10), &paint);

        setBitmapOpaque(&bitmap, true);
        SkScalar x = SkIntToScalar(bm.width() + 10);
        paint.setDither(false);
        canvas->drawBitmap(bitmap, x, 0, &paint);
        paint.setDither(true);
        canvas->drawBitmap(bitmap, x, SkIntToScalar(bm.height() + 10), &paint);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        canvas->translate(SkIntToScalar(20), SkIntToScalar(20));

        draw2(canvas, fBM8);
        canvas->translate(0, SkIntToScalar(fBM8.height() *3));
        draw2(canvas, fBM32);

        canvas->translate(0, SkIntToScalar(fBM8.height() *3));
        draw_gradient(canvas);

        char resultTrue[] = "SkRegion::setPath returned true";
        char resultFalse[] = "SkRegion::setPath returned false";
        SkPaint p;
        if (fResult)
            canvas->drawText(resultTrue, sizeof(resultTrue) - 1, 0, 50, p);
        else
            canvas->drawText(resultFalse, sizeof(resultFalse) - 1, 0, 50, p);
    }

private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new DitherBitmapView; }
static SkViewRegister reg(MyFactory);
