
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkTypeface.h"

// effects
#include "SkGradientShader.h"
#include "SkUnitMappers.h"
#include "SkBlurDrawLooper.h"

static void makebm(SkBitmap* bm, SkBitmap::Config config, int w, int h) {
    bm->setConfig(config, w, h);
    bm->allocPixels();
    bm->eraseColor(SK_ColorTRANSPARENT);

    SkCanvas    canvas(*bm);
    SkPoint     pts[] = { { 0, 0 }, { SkIntToScalar(w), SkIntToScalar(h)} };
    SkColor     colors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE };
    SkScalar    pos[] = { 0, SK_Scalar1/2, SK_Scalar1 };
    SkPaint     paint;

    SkUnitMapper*   um = NULL;

    um = new SkCosineMapper;
//    um = new SkDiscreteMapper(12);

    SkAutoUnref au(um);

    paint.setDither(true);
    paint.setShader(SkGradientShader::CreateLinear(pts, colors, pos,
                SK_ARRAY_COUNT(colors), SkShader::kClamp_TileMode, um))->unref();
    canvas.drawPaint(paint);
}

static void setup(SkPaint* paint, const SkBitmap& bm, bool filter,
                  SkShader::TileMode tmx, SkShader::TileMode tmy) {
    SkShader* shader = SkShader::CreateBitmapShader(bm, tmx, tmy);
    paint->setShader(shader)->unref();
    paint->setFilterLevel(filter ? SkPaint::kLow_FilterLevel : SkPaint::kNone_FilterLevel);
}

static const SkBitmap::Config gConfigs[] = {
    SkBitmap::kARGB_8888_Config,
    SkBitmap::kRGB_565_Config,
};

class TilingGM : public skiagm::GM {
public:
    TilingGM(bool powerOfTwoSize)
            : fPowerOfTwoSize(powerOfTwoSize) {
    }

    SkBitmap    fTexture[SK_ARRAY_COUNT(gConfigs)];

protected:

    enum {
        kPOTSize = 32,
        kNPOTSize = 21,
    };

    SkString onShortName() {
        SkString name("tilemodes");
        if (!fPowerOfTwoSize) {
            name.append("_npot");
        }
        return name;
    }

    SkISize onISize() { return SkISize::Make(880, 560); }

    virtual void onOnceBeforeDraw() SK_OVERRIDE {
        int size = fPowerOfTwoSize ? kPOTSize : kNPOTSize;
        for (size_t i = 0; i < SK_ARRAY_COUNT(gConfigs); i++) {
            makebm(&fTexture[i], gConfigs[i], size, size);
        }
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {

        int size = fPowerOfTwoSize ? kPOTSize : kNPOTSize;

        SkRect r = { 0, 0, SkIntToScalar(size*2), SkIntToScalar(size*2) };

        static const char* gConfigNames[] = { "8888", "565", "4444" };

        static const bool           gFilters[] = { false, true };
        static const char*          gFilterNames[] = {     "point",                     "bilinear" };

        static const SkShader::TileMode gModes[] = { SkShader::kClamp_TileMode, SkShader::kRepeat_TileMode, SkShader::kMirror_TileMode };
        static const char*          gModeNames[] = {    "C",                    "R",                   "M" };

        SkScalar y = SkIntToScalar(24);
        SkScalar x = SkIntToScalar(10);

        for (size_t kx = 0; kx < SK_ARRAY_COUNT(gModes); kx++) {
            for (size_t ky = 0; ky < SK_ARRAY_COUNT(gModes); ky++) {
                SkPaint p;
                SkString str;
                p.setAntiAlias(true);
                p.setDither(true);
                str.printf("[%s,%s]", gModeNames[kx], gModeNames[ky]);

                p.setTextAlign(SkPaint::kCenter_Align);
                canvas->drawText(str.c_str(), str.size(), x + r.width()/2, y, p);

                x += r.width() * 4 / 3;
            }
        }

        y += SkIntToScalar(16);

        for (size_t i = 0; i < SK_ARRAY_COUNT(gConfigs); i++) {
            for (size_t j = 0; j < SK_ARRAY_COUNT(gFilters); j++) {
                x = SkIntToScalar(10);
                for (size_t kx = 0; kx < SK_ARRAY_COUNT(gModes); kx++) {
                    for (size_t ky = 0; ky < SK_ARRAY_COUNT(gModes); ky++) {
                        SkPaint paint;
#if 1 // Temporary change to regen bitmap before each draw. This may help tracking down an issue
      // on SGX where resizing NPOT textures to POT textures exhibits a driver bug.
                        if (!fPowerOfTwoSize) {
                            makebm(&fTexture[i], gConfigs[i], size, size);
                        }
#endif
                        setup(&paint, fTexture[i], gFilters[j], gModes[kx], gModes[ky]);
                        paint.setDither(true);

                        canvas->save();
                        canvas->translate(x, y);
                        canvas->drawRect(r, paint);
                        canvas->restore();

                        x += r.width() * 4 / 3;
                    }
                }
                {
                    SkPaint p;
                    SkString str;
                    p.setAntiAlias(true);
                    str.printf("%s, %s", gConfigNames[i], gFilterNames[j]);
                    canvas->drawText(str.c_str(), str.size(), x, y + r.height() * 2 / 3, p);
                }

                y += r.height() * 4 / 3;
            }
        }
    }

private:
    bool fPowerOfTwoSize;
    typedef skiagm::GM INHERITED;
};

static const int gWidth = 32;
static const int gHeight = 32;

static SkShader* make_bm(SkShader::TileMode tx, SkShader::TileMode ty) {
    SkBitmap bm;
    makebm(&bm, SkBitmap::kARGB_8888_Config, gWidth, gHeight);
    return SkShader::CreateBitmapShader(bm, tx, ty);
}

static SkShader* make_grad(SkShader::TileMode tx, SkShader::TileMode ty) {
    SkPoint pts[] = { { 0, 0 }, { SkIntToScalar(gWidth), SkIntToScalar(gHeight)} };
    SkPoint center = { SkIntToScalar(gWidth)/2, SkIntToScalar(gHeight)/2 };
    SkScalar rad = SkIntToScalar(gWidth)/2;
    SkColor colors[] = { 0xFFFF0000, 0xFF0044FF };

    int index = (int)ty;
    switch (index % 3) {
        case 0:
            return SkGradientShader::CreateLinear(pts, colors, NULL, SK_ARRAY_COUNT(colors), tx);
        case 1:
            return SkGradientShader::CreateRadial(center, rad, colors, NULL, SK_ARRAY_COUNT(colors), tx);
        case 2:
            return SkGradientShader::CreateSweep(center.fX, center.fY, colors, NULL, SK_ARRAY_COUNT(colors));
    }

    return NULL;
}

typedef SkShader* (*ShaderProc)(SkShader::TileMode, SkShader::TileMode);

class Tiling2GM : public skiagm::GM {
    ShaderProc fProc;
    SkString   fName;
public:
    Tiling2GM(ShaderProc proc, const char name[]) : fProc(proc) {
        fName.printf("tilemode_%s", name);
    }

protected:
    SkString onShortName() {
        return fName;
    }

    SkISize onISize() { return SkISize::Make(880, 560); }

    virtual void onDraw(SkCanvas* canvas) {
        canvas->scale(SkIntToScalar(3)/2, SkIntToScalar(3)/2);

        const SkScalar w = SkIntToScalar(gWidth);
        const SkScalar h = SkIntToScalar(gHeight);
        SkRect r = { -w, -h, w*2, h*2 };

        static const SkShader::TileMode gModes[] = {
            SkShader::kClamp_TileMode, SkShader::kRepeat_TileMode, SkShader::kMirror_TileMode
        };
        static const char* gModeNames[] = {
            "Clamp", "Repeat", "Mirror"
        };

        SkScalar y = SkIntToScalar(24);
        SkScalar x = SkIntToScalar(66);

        SkPaint p;
        p.setAntiAlias(true);
        p.setTextAlign(SkPaint::kCenter_Align);

        for (size_t kx = 0; kx < SK_ARRAY_COUNT(gModes); kx++) {
            SkString str(gModeNames[kx]);
            canvas->drawText(str.c_str(), str.size(), x + r.width()/2, y, p);
            x += r.width() * 4 / 3;
        }

        y += SkIntToScalar(16) + h;
        p.setTextAlign(SkPaint::kRight_Align);

        for (size_t ky = 0; ky < SK_ARRAY_COUNT(gModes); ky++) {
            x = SkIntToScalar(16) + w;

            SkString str(gModeNames[ky]);
            canvas->drawText(str.c_str(), str.size(), x, y + h/2, p);

            x += SkIntToScalar(50);
            for (size_t kx = 0; kx < SK_ARRAY_COUNT(gModes); kx++) {
                SkPaint paint;
                paint.setShader(fProc(gModes[kx], gModes[ky]))->unref();

                canvas->save();
                canvas->translate(x, y);
                canvas->drawRect(r, paint);
                canvas->restore();

                x += r.width() * 4 / 3;
            }
            y += r.height() * 4 / 3;
        }
    }

private:
    typedef skiagm::GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new TilingGM(true); )
DEF_GM( return new TilingGM(false); )
DEF_GM( return new Tiling2GM(make_bm, "bitmap"); )
DEF_GM( return new Tiling2GM(make_grad, "gradient"); )
