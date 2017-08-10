/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "sk_tool_utils.h"
#include "SkColorFilter.h"
#include "SkMaskFilter.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"

// effects
#include "SkGradientShader.h"
#include "SkBlurDrawLooper.h"

static void makebm(SkBitmap* bm, SkColorType ct, int w, int h) {
    bm->allocPixels(SkImageInfo::Make(w, h, ct, kPremul_SkAlphaType));
    bm->eraseColor(SK_ColorTRANSPARENT);

    SkCanvas    canvas(*bm);
    SkPoint     pts[] = { { 0, 0 }, { SkIntToScalar(w), SkIntToScalar(h)} };
    SkColor     colors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE };
    SkScalar    pos[] = { 0, SK_Scalar1/2, SK_Scalar1 };
    SkPaint     paint;

    paint.setDither(true);
    paint.setShader(SkGradientShader::MakeLinear(pts, colors, pos,
                SK_ARRAY_COUNT(colors), SkShader::kClamp_TileMode));
    canvas.drawPaint(paint);
}

static void setup(SkPaint* paint, const SkBitmap& bm, SkFilterQuality filter_level,
                  SkShader::TileMode tmx, SkShader::TileMode tmy) {
    paint->setShader(SkShader::MakeBitmapShader(bm, tmx, tmy));
    paint->setFilterQuality(filter_level);
}

constexpr SkColorType gColorTypes[] = {
    kN32_SkColorType,
    kRGB_565_SkColorType,
};

class ScaledTilingGM : public skiagm::GM {
public:
    ScaledTilingGM(bool powerOfTwoSize)
            : fPowerOfTwoSize(powerOfTwoSize) {
    }

    SkBitmap    fTexture[SK_ARRAY_COUNT(gColorTypes)];

protected:
    enum {
        kPOTSize = 4,
        kNPOTSize = 3,
    };

    SkString onShortName() override {
        SkString name("scaled_tilemodes");
        if (!fPowerOfTwoSize) {
            name.append("_npot");
        }
        return name;
    }

    SkISize onISize() override { return SkISize::Make(880, 760); }

    void onOnceBeforeDraw() override {
        int size = fPowerOfTwoSize ? kPOTSize : kNPOTSize;
        for (size_t i = 0; i < SK_ARRAY_COUNT(gColorTypes); i++) {
            makebm(&fTexture[i], gColorTypes[i], size, size);
        }
    }

    void onDraw(SkCanvas* canvas) override {
        float scale = 32.f/kPOTSize;

        int size = fPowerOfTwoSize ? kPOTSize : kNPOTSize;

        SkRect r = { 0, 0, SkIntToScalar(size*2), SkIntToScalar(size*2) };

        const char* gColorTypeNames[] = { "8888" , "565", "4444" };

        constexpr SkFilterQuality gFilterQualitys[] =
            { kNone_SkFilterQuality,
              kLow_SkFilterQuality,
              kMedium_SkFilterQuality,
              kHigh_SkFilterQuality };
        const char* gFilterNames[] = { "None", "Low", "Medium", "High" };

        constexpr SkShader::TileMode gModes[] = {
            SkShader::kClamp_TileMode, SkShader::kRepeat_TileMode, SkShader::kMirror_TileMode };
        const char* gModeNames[] = { "C", "R", "M" };

        SkScalar y = SkIntToScalar(24);
        SkScalar x = SkIntToScalar(10)/scale;

        for (size_t kx = 0; kx < SK_ARRAY_COUNT(gModes); kx++) {
            for (size_t ky = 0; ky < SK_ARRAY_COUNT(gModes); ky++) {
                SkPaint p;
                SkString str;
                p.setAntiAlias(true);
                sk_tool_utils::set_portable_typeface(&p);
                str.printf("[%s,%s]", gModeNames[kx], gModeNames[ky]);

                p.setTextAlign(SkPaint::kCenter_Align);
                canvas->drawString(str, scale*(x + r.width()/2), y, p);

                x += r.width() * 4 / 3;
            }
        }

        y = SkIntToScalar(40) / scale;

        for (size_t i = 0; i < SK_ARRAY_COUNT(gColorTypes); i++) {
            for (size_t j = 0; j < SK_ARRAY_COUNT(gFilterQualitys); j++) {
                x = SkIntToScalar(10)/scale;
                for (size_t kx = 0; kx < SK_ARRAY_COUNT(gModes); kx++) {
                    for (size_t ky = 0; ky < SK_ARRAY_COUNT(gModes); ky++) {
                        SkPaint paint;
#if 1 // Temporary change to regen bitmap before each draw. This may help tracking down an issue
      // on SGX where resizing NPOT textures to POT textures exhibits a driver bug.
                        if (!fPowerOfTwoSize) {
                            makebm(&fTexture[i], gColorTypes[i], size, size);
                        }
#endif
                        setup(&paint, fTexture[i], gFilterQualitys[j], gModes[kx], gModes[ky]);
                        paint.setDither(true);

                        canvas->save();
                        canvas->scale(scale,scale);
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
                    sk_tool_utils::set_portable_typeface(&p);
                    str.printf("%s, %s", gColorTypeNames[i], gFilterNames[j]);
                    canvas->drawString(str, scale*x, scale*(y + r.height() * 2 / 3), p);
                }

                y += r.height() * 4 / 3;
            }
        }
    }

private:
    bool fPowerOfTwoSize;
    typedef skiagm::GM INHERITED;
};

constexpr int gWidth = 32;
constexpr int gHeight = 32;

static sk_sp<SkShader> make_bm(SkShader::TileMode tx, SkShader::TileMode ty) {
    SkBitmap bm;
    makebm(&bm, kN32_SkColorType, gWidth, gHeight);
    return SkShader::MakeBitmapShader(bm, tx, ty);
}

static sk_sp<SkShader> make_grad(SkShader::TileMode tx, SkShader::TileMode ty) {
    SkPoint pts[] = { { 0, 0 }, { SkIntToScalar(gWidth), SkIntToScalar(gHeight)} };
    SkPoint center = { SkIntToScalar(gWidth)/2, SkIntToScalar(gHeight)/2 };
    SkScalar rad = SkIntToScalar(gWidth)/2;
    SkColor colors[] = { 0xFFFF0000, sk_tool_utils::color_to_565(0xFF0044FF) };

    int index = (int)ty;
    switch (index % 3) {
        case 0:
            return SkGradientShader::MakeLinear(pts, colors, nullptr, SK_ARRAY_COUNT(colors), tx);
        case 1:
            return SkGradientShader::MakeRadial(center, rad, colors, nullptr, SK_ARRAY_COUNT(colors), tx);
        case 2:
            return SkGradientShader::MakeSweep(center.fX, center.fY, colors, nullptr, SK_ARRAY_COUNT(colors));
    }

    return nullptr;
}

typedef sk_sp<SkShader> (*ShaderProc)(SkShader::TileMode, SkShader::TileMode);

class ScaledTiling2GM : public skiagm::GM {
    ShaderProc fProc;
    SkString   fName;
public:
    ScaledTiling2GM(ShaderProc proc, const char name[]) : fProc(proc) {
        fName.printf("scaled_tilemode_%s", name);
    }

protected:

    SkString onShortName() override {
        return fName;
    }

    SkISize onISize() override { return SkISize::Make(650, 610); }

    void onDraw(SkCanvas* canvas) override {
        canvas->scale(SkIntToScalar(3)/2, SkIntToScalar(3)/2);

        const SkScalar w = SkIntToScalar(gWidth);
        const SkScalar h = SkIntToScalar(gHeight);
        SkRect r = { -w, -h, w*2, h*2 };

        constexpr SkShader::TileMode gModes[] = {
            SkShader::kClamp_TileMode, SkShader::kRepeat_TileMode, SkShader::kMirror_TileMode
        };
        const char* gModeNames[] = {
            "Clamp", "Repeat", "Mirror"
        };

        SkScalar y = SkIntToScalar(24);
        SkScalar x = SkIntToScalar(66);

        SkPaint p;
        p.setAntiAlias(true);
        sk_tool_utils::set_portable_typeface(&p);
        p.setTextAlign(SkPaint::kCenter_Align);

        for (size_t kx = 0; kx < SK_ARRAY_COUNT(gModes); kx++) {
            SkString str(gModeNames[kx]);
            canvas->drawString(str, x + r.width()/2, y, p);
            x += r.width() * 4 / 3;
        }

        y += SkIntToScalar(16) + h;
        p.setTextAlign(SkPaint::kRight_Align);

        for (size_t ky = 0; ky < SK_ARRAY_COUNT(gModes); ky++) {
            x = SkIntToScalar(16) + w;

            SkString str(gModeNames[ky]);
            canvas->drawString(str, x, y + h/2, p);

            x += SkIntToScalar(50);
            for (size_t kx = 0; kx < SK_ARRAY_COUNT(gModes); kx++) {
                SkPaint paint;
                paint.setShader(fProc(gModes[kx], gModes[ky]));

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

DEF_GM( return new ScaledTilingGM(true); )
DEF_GM( return new ScaledTilingGM(false); )
DEF_GM( return new ScaledTiling2GM(make_bm, "bitmap"); )
DEF_GM( return new ScaledTiling2GM(make_grad, "gradient"); )
