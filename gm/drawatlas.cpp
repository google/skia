/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkRSXform.h"
#include "SkSurface.h"

class DrawAtlasGM : public skiagm::GM {
    static sk_sp<SkImage> MakeAtlas(SkCanvas* caller, const SkRect& target) {
        SkImageInfo info = SkImageInfo::MakeN32Premul(100, 100);
        auto surface(caller->makeSurface(info));
        if (nullptr == surface) {
            surface = SkSurface::MakeRaster(info);
        }
        SkCanvas* canvas = surface->getCanvas();
        // draw red everywhere, but we don't expect to see it in the draw, testing the notion
        // that drawAtlas draws a subset-region of the atlas.
        canvas->clear(SK_ColorRED);

        SkPaint paint;
        paint.setXfermodeMode(SkXfermode::kClear_Mode);
        SkRect r(target);
        r.inset(-1, -1);
        // zero out a place (with a 1-pixel border) to land our drawing.
        canvas->drawRect(r, paint);
        paint.setXfermode(nullptr);
        paint.setColor(SK_ColorBLUE);
        paint.setAntiAlias(true);
        canvas->drawOval(target, paint);
        return surface->makeImageSnapshot();
    }

    sk_sp<SkImage> fAtlas;

public:
    DrawAtlasGM() {}

protected:

    SkString onShortName() override {
        return SkString("draw-atlas");
    }

    SkISize onISize() override {
        return SkISize::Make(640, 480);
    }

    void onDraw(SkCanvas* canvas) override {
        const SkRect target = { 50, 50, 80, 90 };
        if (nullptr == fAtlas) {
            fAtlas = MakeAtlas(canvas, target);
        }

        const struct {
            SkScalar fScale;
            SkScalar fDegrees;
            SkScalar fTx;
            SkScalar fTy;

            void apply(SkRSXform* xform) const {
                const SkScalar rad = SkDegreesToRadians(fDegrees);
                xform->fSCos = fScale * SkScalarCos(rad);
                xform->fSSin = fScale * SkScalarSin(rad);
                xform->fTx   = fTx;
                xform->fTy   = fTy;
            }
        } rec[] = {
            { 1, 0, 10, 10 },       // just translate
            { 2, 0, 110, 10 },      // scale + translate
            { 1, 30, 210, 10 },     // rotate + translate
            { 2, -30, 310, 30 },    // scale + rotate + translate
        };

        const int N = SK_ARRAY_COUNT(rec);
        SkRSXform xform[N];
        SkRect tex[N];
        SkColor colors[N];

        for (int i = 0; i < N; ++i) {
            rec[i].apply(&xform[i]);
            tex[i] = target;
            colors[i] = 0x80FF0000 + (i * 40 * 256);
        }

        SkPaint paint;
        paint.setFilterQuality(kLow_SkFilterQuality);
        paint.setAntiAlias(true);

        canvas->drawAtlas(fAtlas.get(), xform, tex, N, nullptr, &paint);
        canvas->translate(0, 100);
        canvas->drawAtlas(fAtlas.get(), xform, tex, colors, N, SkXfermode::kSrcIn_Mode, nullptr, &paint);
    }

private:
    typedef GM INHERITED;
};
DEF_GM( return new DrawAtlasGM; )

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "SkPath.h"
#include "SkPathMeasure.h"

static void draw_text_on_path_rigid(SkCanvas* canvas, const void* text, size_t length,
                                    const SkPoint xy[], const SkPath& path, const SkPaint& paint) {
    SkPathMeasure meas(path, false);

    int count = paint.countText(text, length);
    SkAutoSTArray<100, SkRSXform> xform(count);

    for (int i = 0; i < count; ++i) {
        SkPoint pos;
        SkVector tan;
        if (!meas.getPosTan(xy[i].x(), &pos, &tan)) {
            pos = xy[i];
            tan.set(1, 0);
        }
        xform[i].fSCos = tan.x();
        xform[i].fSSin = tan.y();
        xform[i].fTx   = pos.x();
        xform[i].fTy   = pos.y();
    }

    canvas->drawTextRSXform(text, length, &xform[0], nullptr, paint);
}

DEF_SIMPLE_GM(drawTextRSXform, canvas, 510, 310) {
    const char text[] = "ABCDFGHJKLMNOPQRSTUVWXYZ";
    const int N = sizeof(text) - 1;
    SkPoint pos[N];
    SkRSXform xform[N];

    canvas->translate(0, 30);

    SkScalar x = 20;
    SkScalar dx = 20;
    SkScalar rad = 0;
    SkScalar drad = 2 * SK_ScalarPI / (N - 1);
    for (int i = 0; i < N; ++i) {
        xform[i].fSCos = SkScalarCos(rad);
        xform[i].fSSin = SkScalarSin(rad);
        xform[i].fTx = x;
        xform[i].fTy = 0;
        pos[i].set(x, 0);
        x += dx;
        rad += drad;
    }

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(20);
    canvas->drawTextRSXform(text, N, xform, nullptr, paint);

    SkPath path;
    path.addOval(SkRect::MakeXYWH(150, 50, 200, 200));

    draw_text_on_path_rigid(canvas, text, N, pos, path, paint);

    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawPath(path, paint);
}


