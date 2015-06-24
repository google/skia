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
    static SkImage* MakeAtlas(SkCanvas* caller, const SkRect& target) {
        SkImageInfo info = SkImageInfo::MakeN32Premul(100, 100);
        SkAutoTUnref<SkSurface> surface(caller->newSurface(info));
        if (NULL == surface) {
            surface.reset(SkSurface::NewRaster(info));
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
        paint.setXfermode(NULL);
        paint.setColor(SK_ColorBLUE);
        paint.setAntiAlias(true);
        canvas->drawOval(target, paint);
        return surface->newImageSnapshot();
    }

    SkAutoTUnref<SkImage> fAtlas;

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
        if (NULL == fAtlas) {
            fAtlas.reset(MakeAtlas(canvas, target));
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

        canvas->drawAtlas(fAtlas, xform, tex, N, NULL, &paint);
        canvas->translate(0, 100);
        canvas->drawAtlas(fAtlas, xform, tex, colors, N, SkXfermode::kSrcIn_Mode, NULL, &paint);
    }
    
private:
    typedef GM INHERITED;
};
DEF_GM( return new DrawAtlasGM; )

