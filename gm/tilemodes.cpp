/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "include/utils/SkTextUtils.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

#include <functional>

static void makebm(SkBitmap* bm, SkColorType ct, int w, int h) {
    bm->allocPixels(SkImageInfo::Make(w, h, ct, kPremul_SkAlphaType));
    bm->eraseColor(SK_ColorTRANSPARENT);

    SkCanvas    canvas(*bm);
    SkPoint     pts[] = { { 0, 0 }, { SkIntToScalar(w), SkIntToScalar(h)} };
    SkColor     colors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE };
    SkScalar    pos[] = { 0, SK_Scalar1/2, SK_Scalar1 };
    SkPaint     paint;

    paint.setDither(true);
    paint.setShader(SkGradientShader::MakeLinear(pts, colors, pos, std::size(colors),
                                                 SkTileMode::kClamp));
    canvas.drawPaint(paint);
}

static void setup(SkCanvas* canvas, SkPaint* paint, const SkBitmap& bm, SkFilterMode fm,
                  SkTileMode tmx, SkTileMode tmy) {
    sk_sp<SkImage> img = SkImage::MakeFromBitmap(bm);
    img = ToolUtils::MakeTextureImage(canvas, std::move(img));
    if (img) {
        // img can be null if the GPU context has been abandoned.
        paint->setShader(img->makeShader(tmx, tmy, SkSamplingOptions(fm)));
    }
}

constexpr SkColorType gColorTypes[] = {
    kN32_SkColorType,
    kRGB_565_SkColorType,
};

class TilingGM : public skiagm::GM {
public:
    TilingGM(bool powerOfTwoSize)
            : fPowerOfTwoSize(powerOfTwoSize) {
    }

    SkBitmap    fTexture[std::size(gColorTypes)];

protected:

    enum {
        kPOTSize = 32,
        kNPOTSize = 21,
    };

    SkString onShortName() override {
        SkString name("tilemodes");
        if (!fPowerOfTwoSize) {
            name.append("_npot");
        }
        return name;
    }

    SkISize onISize() override { return SkISize::Make(880, 560); }

    void onOnceBeforeDraw() override {
        int size = fPowerOfTwoSize ? kPOTSize : kNPOTSize;
        for (size_t i = 0; i < std::size(gColorTypes); i++) {
            makebm(&fTexture[i], gColorTypes[i], size, size);
        }
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint textPaint;
        SkFont  font(ToolUtils::create_portable_typeface());

        int size = fPowerOfTwoSize ? kPOTSize : kNPOTSize;

        SkRect r = { 0, 0, SkIntToScalar(size*2), SkIntToScalar(size*2) };

        const char* gConfigNames[] = { "8888", "565" };

        constexpr SkFilterMode gFilters[] = { SkFilterMode::kNearest, SkFilterMode::kLinear };
        static const char* gFilterNames[] = { "point", "bilinear" };

        constexpr SkTileMode gModes[] = {
            SkTileMode::kClamp, SkTileMode::kRepeat, SkTileMode::kMirror };
        static const char* gModeNames[] = { "C", "R", "M" };

        SkScalar y = SkIntToScalar(24);
        SkScalar x = SkIntToScalar(10);

        for (size_t kx = 0; kx < std::size(gModes); kx++) {
            for (size_t ky = 0; ky < std::size(gModes); ky++) {
                SkPaint p;
                p.setDither(true);
                SkString str;
                str.printf("[%s,%s]", gModeNames[kx], gModeNames[ky]);

                SkTextUtils::DrawString(canvas, str.c_str(), x + r.width()/2, y, font, p,
                                        SkTextUtils::kCenter_Align);

                x += r.width() * 4 / 3;
            }
        }

        y += SkIntToScalar(16);

        for (size_t i = 0; i < std::size(gColorTypes); i++) {
            for (size_t j = 0; j < std::size(gFilters); j++) {
                x = SkIntToScalar(10);
                for (size_t kx = 0; kx < std::size(gModes); kx++) {
                    for (size_t ky = 0; ky < std::size(gModes); ky++) {
                        SkPaint paint;
#if 1 // Temporary change to regen bitmap before each draw. This may help tracking down an issue
      // on SGX where resizing NPOT textures to POT textures exhibits a driver bug.
                        if (!fPowerOfTwoSize) {
                            makebm(&fTexture[i], gColorTypes[i], size, size);
                        }
#endif
                        setup(canvas, &paint, fTexture[i], gFilters[j], gModes[kx], gModes[ky]);
                        paint.setDither(true);

                        canvas->save();
                        canvas->translate(x, y);
                        canvas->drawRect(r, paint);
                        canvas->restore();

                        x += r.width() * 4 / 3;
                    }
                }
                canvas->drawString(SkStringPrintf("%s, %s", gConfigNames[i], gFilterNames[j]),
                                   x, y + r.height() * 2 / 3, font, textPaint);

                y += r.height() * 4 / 3;
            }
        }
    }

private:
    bool fPowerOfTwoSize;
    using INHERITED = skiagm::GM;
};
DEF_GM( return new TilingGM(true); )
DEF_GM( return new TilingGM(false); )

constexpr int gWidth = 32;
constexpr int gHeight = 32;

static sk_sp<SkShader> make_bm(SkTileMode tx, SkTileMode ty) {
    SkBitmap bm;
    makebm(&bm, kN32_SkColorType, gWidth, gHeight);
    return bm.makeShader(tx, ty, SkSamplingOptions());
}

static sk_sp<SkShader> make_grad(SkTileMode tx, SkTileMode ty) {
    SkPoint pts[] = { { 0, 0 }, { SkIntToScalar(gWidth), SkIntToScalar(gHeight)} };
    SkPoint center = { SkIntToScalar(gWidth)/2, SkIntToScalar(gHeight)/2 };
    SkScalar rad = SkIntToScalar(gWidth)/2;
    SkColor  colors[] = {0xFFFF0000, ToolUtils::color_to_565(0xFF0044FF)};

    int index = (int)ty;
    switch (index % 3) {
        case 0:
            return SkGradientShader::MakeLinear(pts, colors, nullptr, std::size(colors), tx);
        case 1:
            return SkGradientShader::MakeRadial(center, rad, colors, nullptr, std::size(colors), tx);
        case 2:
            return SkGradientShader::MakeSweep(center.fX, center.fY, colors, nullptr,
                                               std::size(colors), tx, 135, 225, 0, nullptr);
    }
    return nullptr;
}

typedef sk_sp<SkShader> (*ShaderProc)(SkTileMode, SkTileMode);

class Tiling2GM : public skiagm::GM {
    ShaderProc fProc;
    const char* fName;

public:
    Tiling2GM(ShaderProc proc, const char name[]) : fProc(proc), fName(name) {}

private:
    SkString onShortName() override { return SkString(fName); }

    SkISize onISize() override { return SkISize::Make(650, 610); }

    void onDraw(SkCanvas* canvas) override {
        canvas->scale(SkIntToScalar(3)/2, SkIntToScalar(3)/2);

        const SkScalar w = SkIntToScalar(gWidth);
        const SkScalar h = SkIntToScalar(gHeight);
        SkRect r = { -w, -h, w*2, h*2 };

        constexpr SkTileMode gModes[] = {
            SkTileMode::kClamp, SkTileMode::kRepeat, SkTileMode::kMirror
        };
        const char* gModeNames[] = {
            "Clamp", "Repeat", "Mirror"
        };

        SkScalar y = SkIntToScalar(24);
        SkScalar x = SkIntToScalar(66);

        SkFont font(ToolUtils::create_portable_typeface());

        for (size_t kx = 0; kx < std::size(gModes); kx++) {
            SkString str(gModeNames[kx]);
            SkTextUtils::DrawString(canvas, str.c_str(), x + r.width()/2, y, font, SkPaint(),
                                    SkTextUtils::kCenter_Align);
            x += r.width() * 4 / 3;
        }

        y += SkIntToScalar(16) + h;

        for (size_t ky = 0; ky < std::size(gModes); ky++) {
            x = SkIntToScalar(16) + w;

            SkString str(gModeNames[ky]);
            SkTextUtils::DrawString(canvas, str.c_str(), x, y + h/2, font, SkPaint(),
                                    SkTextUtils::kRight_Align);

            x += SkIntToScalar(50);
            for (size_t kx = 0; kx < std::size(gModes); kx++) {
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
};

DEF_GM( return new Tiling2GM(make_bm,   "tilemode_bitmap"); )
DEF_GM( return new Tiling2GM(make_grad, "tilemode_gradient"); )

////////////////////

DEF_SIMPLE_GM(tilemode_decal, canvas, 720, 1100) {
    auto img = GetResourceAsImage("images/mandrill_128.png");
    SkPaint bgpaint;
    bgpaint.setColor(SK_ColorYELLOW);

    SkRect r = { -20, -20, img->width() + 20.0f, img->height() + 20.0f };
    canvas->translate(45, 45);

    std::function<void(SkPaint*, SkTileMode, SkTileMode)> shader_procs[] = {
        [img](SkPaint* paint, SkTileMode tx, SkTileMode ty) {
            // Test no filtering with decal mode
            paint->setShader(img->makeShader(tx, ty, SkSamplingOptions(SkFilterMode::kNearest)));
        },
        [img](SkPaint* paint, SkTileMode tx, SkTileMode ty) {
            // Test bilerp approximation for decal mode (or clamp to border HW)
            paint->setShader(img->makeShader(tx, ty, SkSamplingOptions(SkFilterMode::kLinear)));
        },
        [img](SkPaint* paint, SkTileMode tx, SkTileMode ty) {
            // Test bicubic filter with decal mode
            paint->setShader(img->makeShader(tx, ty, SkSamplingOptions(SkCubicResampler::Mitchell())));
        },
        [img](SkPaint* paint, SkTileMode tx, SkTileMode ty) {
            SkColor colors[] = { SK_ColorRED, SK_ColorBLUE };
            const SkPoint pts[] = {{ 0, 0 }, {img->width()*1.0f, img->height()*1.0f }};
            const SkScalar* pos = nullptr;
            const int count = std::size(colors);
            paint->setShader(SkGradientShader::MakeLinear(pts, colors, pos, count, tx));
        },
        [img](SkPaint* paint, SkTileMode tx, SkTileMode ty) {
            SkColor colors[] = { SK_ColorRED, SK_ColorBLUE };
            const SkScalar* pos = nullptr;
            const int count = std::size(colors);
            paint->setShader(SkGradientShader::MakeRadial({ img->width()*0.5f, img->width()*0.5f },
                                                      img->width()*0.5f, colors, pos, count, tx));
        },
    };

    const struct XY {
        SkTileMode  fX;
        SkTileMode  fY;
    } pairs[] = {
        { SkTileMode::kClamp,    SkTileMode::kClamp },
        { SkTileMode::kClamp,    SkTileMode::kDecal },
        { SkTileMode::kDecal,    SkTileMode::kClamp },
        { SkTileMode::kDecal,    SkTileMode::kDecal },
    };
    for (const auto& p : pairs) {
        SkPaint paint;
        canvas->save();
        for (const auto& proc : shader_procs) {
            canvas->save();
            // Apply a slight rotation to highlight the differences between filtered and unfiltered
            // decal edges
            canvas->rotate(4);
            canvas->drawRect(r, bgpaint);
            proc(&paint, p.fX, p.fY);
            canvas->drawRect(r, paint);
            canvas->restore();
            canvas->translate(0, r.height() + 20);
        }
        canvas->restore();
        canvas->translate(r.width() + 10, 0);
    }
}
