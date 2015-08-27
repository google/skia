
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

static void draw_sweep(SkCanvas* c, int width, int height, SkScalar angle) {
    SkRect  r;
    SkPaint p;

    p.setAntiAlias(true);
//    p.setDither(true);
    p.setStrokeWidth(SkIntToScalar(width/10));
    p.setStyle(SkPaint::kStroke_Style);

    r.set(0, 0, SkIntToScalar(width), SkIntToScalar(height));

    //    SkColor colors[] = { SK_ColorRED, SK_ColorBLUE, SK_ColorGREEN, SK_ColorCYAN };
    SkColor colors[] = { 0x4c737373, 0x4c737373, 0xffffd300 };
    SkShader* s = SkGradientShader::CreateSweep(r.centerX(), r.centerY(),
                                                colors, nullptr, SK_ARRAY_COUNT(colors));
    p.setShader(s)->unref();

    SkAutoCanvasRestore acr(c, true);

    c->translate(r.centerX(), r.centerY());
    c->rotate(angle);
    c->translate(-r.centerX(), -r.centerY());

    SkRect bounds = r;
    r.inset(p.getStrokeWidth(), p.getStrokeWidth());
    SkRect innerBounds = r;

    if (true) {
        c->drawOval(r, p);
    } else {
        SkScalar x = r.centerX();
        SkScalar y = r.centerY();
        SkScalar radius = r.width() / 2;
        SkScalar thickness = p.getStrokeWidth();
        SkScalar sweep = 360.0f;
        SkPath path;

        path.moveTo(x + radius, y);
        // outer top
        path.lineTo(x + radius + thickness, y);
        // outer arc
        path.arcTo(bounds, 0, sweep, false);
        // inner arc
        path.arcTo(innerBounds, sweep, -sweep, false);
        path.close();
    }
}

static void make_bm(SkBitmap* bm) {
    bm->allocN32Pixels(100, 100);
    bm->eraseColor(SK_ColorTRANSPARENT);

    SkCanvas c(*bm);
    draw_sweep(&c, bm->width(), bm->height(), 0);
}

static void pre_dither(const SkBitmap& bm) {
    SkAutoLockPixels alp(bm);

    for (int y = 0; y < bm.height(); y++) {
        DITHER_4444_SCAN(y);

        SkPMColor* p = bm.getAddr32(0, y);
        for (int x = 0; x < bm.width(); x++) {
            SkPMColor c = *p;

            unsigned a = SkGetPackedA32(c);
            unsigned r = SkGetPackedR32(c);
            unsigned g = SkGetPackedG32(c);
            unsigned b = SkGetPackedB32(c);

            unsigned d = DITHER_VALUE(x);

            a = SkDITHER_A32To4444(a, d);
            r = SkDITHER_R32To4444(r, d);
            g = SkDITHER_G32To4444(g, d);
            b = SkDITHER_B32To4444(b, d);

            a = SkA4444ToA32(a);
            r = SkR4444ToR32(r);
            g = SkG4444ToG32(g);
            b = SkB4444ToB32(b);

            *p++ = SkPackARGB32(a, r, g, b);
        }
    }
}

class DitherView : public SampleView {
public:
    SkBitmap    fBM, fBMPreDither, fBM16;
    SkScalar fAngle;

    DitherView() {
        make_bm(&fBM);
        make_bm(&fBMPreDither);
        pre_dither(fBMPreDither);
        fBM.copyTo(&fBM16, kARGB_4444_SkColorType);

        fAngle = 0;

        this->setBGColor(0xFF181818);
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Dither");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        SkPaint paint;
        SkScalar x = SkIntToScalar(10);
        SkScalar y = SkIntToScalar(10);
        const SkScalar DX = SkIntToScalar(fBM.width() + 10);

        paint.setAntiAlias(true);

        if (true) {
            canvas->drawBitmap(fBM, x, y, &paint);
            x += DX;
            paint.setDither(true);
            canvas->drawBitmap(fBM, x, y, &paint);

            x += DX;
            paint.setDither(false);
            canvas->drawBitmap(fBMPreDither, x, y, &paint);

            x += DX;
            canvas->drawBitmap(fBM16, x, y, &paint);
        }

        canvas->translate(DX, DX*2);
        draw_sweep(canvas, fBM.width(), fBM.height(), fAngle);
        canvas->translate(DX, 0);
        draw_sweep(canvas, fBM.width()>>1, fBM.height()>>1, fAngle);
        canvas->translate(DX, 0);
        draw_sweep(canvas, fBM.width()>>2, fBM.height()>>2, fAngle);

        fAngle += SK_Scalar1/2;
        this->inval(nullptr);
    }

private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new DitherView; }
static SkViewRegister reg(MyFactory);
