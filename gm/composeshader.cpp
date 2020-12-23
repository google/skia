/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkTLazy.h"

#include <utility>

static sk_sp<SkShader> make_shader(SkBlendMode mode) {
    SkPoint pts[2];
    SkColor colors[2];

    pts[0].set(0, 0);
    pts[1].set(SkIntToScalar(100), 0);
    colors[0] = SK_ColorRED;
    colors[1] = SK_ColorBLUE;
    auto shaderA = SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp);

    pts[0].set(0, 0);
    pts[1].set(0, SkIntToScalar(100));
    colors[0] = SK_ColorBLACK;
    colors[1] = SkColorSetARGB(0x80, 0, 0, 0);
    auto shaderB = SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp);

    return SkShaders::Blend(mode, std::move(shaderA), std::move(shaderB));
}

class ComposeShaderGM : public skiagm::GM {
protected:
    void onOnceBeforeDraw() override {
        fShader = make_shader(SkBlendMode::kDstIn);
    }

    SkString onShortName() override {
        return SkString("composeshader");
    }

    SkISize onISize() override {
        return SkISize::Make(120, 120);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setColor(SK_ColorGREEN);
        canvas->drawRect(SkRect::MakeWH(100, 100), paint);
        paint.setShader(fShader);
        canvas->drawRect(SkRect::MakeWH(100, 100), paint);
    }

protected:
    sk_sp<SkShader> fShader;

private:
    typedef GM INHERITED ;
};
DEF_GM( return new ComposeShaderGM; )

class ComposeShaderAlphaGM : public skiagm::GM {
public:
    ComposeShaderAlphaGM() {}

protected:
    SkString onShortName() override {
        return SkString("composeshader_alpha");
    }

    SkISize onISize() override {
        return SkISize::Make(750, 220);
    }

    void onDraw(SkCanvas* canvas) override {
        sk_sp<SkShader> shaders[] = {
            make_shader(SkBlendMode::kDstIn),
            make_shader(SkBlendMode::kSrcOver),
        };

        SkPaint paint;
        paint.setColor(SK_ColorGREEN);

        const SkRect r = SkRect::MakeXYWH(5, 5, 100, 100);

        for (size_t y = 0; y < SK_ARRAY_COUNT(shaders); ++y) {
            canvas->save();
            for (int alpha = 0xFF; alpha > 0; alpha -= 0x28) {
                paint.setAlphaf(1.0f);
                paint.setShader(nullptr);
                canvas->drawRect(r, paint);

                paint.setAlpha(alpha);
                paint.setShader(shaders[y]);
                canvas->drawRect(r, paint);

                canvas->translate(r.width() + 5, 0);
            }
            canvas->restore();
            canvas->translate(0, r.height() + 5);
        }
    }

private:
    typedef GM INHERITED ;
};
DEF_GM( return new ComposeShaderAlphaGM; )

// creates a square bitmap with red background and a green circle in the center
static void draw_color_bm(SkBitmap* bm, int length) {
    SkPaint paint;
    paint.setColor(SK_ColorGREEN);

    bm->allocN32Pixels(length, length);
    bm->eraseColor(SK_ColorRED);

    SkCanvas canvas(*bm);
    canvas.drawCircle(SkIntToScalar(length/2), SkIntToScalar(length/2), SkIntToScalar(length/2),
                      paint);
}

// creates a square alpha8 bitmap with transparent background and an opaque circle in the center
static void draw_alpha8_bm(SkBitmap* bm, int length) {
    SkPaint circlePaint;
    circlePaint.setColor(SK_ColorBLACK);

    bm->allocPixels(SkImageInfo::MakeA8(length, length));
    bm->eraseColor(SK_ColorTRANSPARENT);

    SkCanvas canvas(*bm);
    canvas.drawCircle(SkIntToScalar(length/2), SkIntToScalar(length/2), SkIntToScalar(length/4),
                      circlePaint);
}

// creates a linear gradient shader
static sk_sp<SkShader> make_linear_gradient_shader(int length) {
    SkPoint pts[2];
    SkColor colors[2];
    pts[0].set(0, 0);
    pts[1].set(SkIntToScalar(length), 0);
    colors[0] = SK_ColorBLUE;
    colors[1] = SkColorSetARGB(0, 0, 0, 0xFF);
    return SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp);
}


class ComposeShaderBitmapGM : public skiagm::GM {
public:
    ComposeShaderBitmapGM(bool use_lm) : fUseLocalMatrix(use_lm) {}

protected:
    void onOnceBeforeDraw() override {
        draw_color_bm(&fColorBitmap, squareLength);
        draw_alpha8_bm(&fAlpha8Bitmap, squareLength);
        SkMatrix s;
        s.reset();
        fColorBitmapShader = fColorBitmap.makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat,
                                                     SkSamplingOptions(), s);
        fAlpha8BitmapShader = fAlpha8Bitmap.makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat,
                                                       SkSamplingOptions(), s);
        fLinearGradientShader = make_linear_gradient_shader(squareLength);
    }

    SkString onShortName() override {
        return SkStringPrintf("composeshader_bitmap%s", fUseLocalMatrix ? "_lm" : "");
    }

    SkISize onISize() override {
        return SkISize::Make(7 * (squareLength + 5), 2 * (squareLength + 5));
    }

    void onDraw(SkCanvas* canvas) override {
        SkBlendMode mode = SkBlendMode::kDstOver;

        SkMatrix lm = SkMatrix::Translate(0, squareLength * 0.5f);

        sk_sp<SkShader> shaders[] = {
            // gradient should appear over color bitmap
            SkShaders::Blend(mode, fLinearGradientShader, fColorBitmapShader),
            // gradient should appear over alpha8 bitmap colorized by the paint color
            SkShaders::Blend(mode, fLinearGradientShader, fAlpha8BitmapShader),
        };
        if (fUseLocalMatrix) {
            for (unsigned i = 0; i < SK_ARRAY_COUNT(shaders); ++i) {
                shaders[i] = shaders[i]->makeWithLocalMatrix(lm);
            }
        }

        SkPaint paint;
        paint.setColor(SK_ColorYELLOW);

        const SkRect r = SkRect::MakeIWH(squareLength, squareLength);

        for (size_t y = 0; y < SK_ARRAY_COUNT(shaders); ++y) {
            canvas->save();
            for (int alpha = 0xFF; alpha > 0; alpha -= 0x28) {
                paint.setAlpha(alpha);
                paint.setShader(shaders[y]);
                canvas->drawRect(r, paint);

                canvas->translate(r.width() + 5, 0);
            }
            canvas->restore();
            canvas->translate(0, r.height() + 5);
        }
    }

private:
    /** This determines the length and width of the bitmaps used in the ComposeShaders.  Values
     *  above 20 may cause an SkASSERT to fail in SkSmallAllocator. However, larger values will
     *  work in a release build.  You can change this parameter and then compile a release build
     *  to have this GM draw larger bitmaps for easier visual inspection.
     */
    static constexpr int squareLength = 20;

    const bool fUseLocalMatrix;

    SkBitmap fColorBitmap;
    SkBitmap fAlpha8Bitmap;
    sk_sp<SkShader> fColorBitmapShader;
    sk_sp<SkShader> fAlpha8BitmapShader;
    sk_sp<SkShader> fLinearGradientShader;

    using INHERITED = GM;
};
DEF_GM( return new ComposeShaderBitmapGM(false); )
DEF_GM( return new ComposeShaderBitmapGM(true); )

DEF_SIMPLE_GM(composeshader_bitmap2, canvas, 200, 200) {
    int width = 255;
    int height = 255;
    SkTDArray<uint8_t> dst8Storage;
    dst8Storage.setCount(width * height);
    SkTDArray<uint32_t> dst32Storage;
    dst32Storage.setCount(width * height * sizeof(int32_t));
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            dst8Storage[y * width + x] = (y + x) / 2;
            dst32Storage[y * width + x] = SkPackARGB32(0xFF, x, y, 0);
        }
    }
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorBLUE);
    SkRect r = {0, 0, SkIntToScalar(width), SkIntToScalar(height)};
    canvas->drawRect(r, paint);
    SkBitmap skBitmap, skMask;
    SkImageInfo imageInfo = SkImageInfo::Make(width, height,
            SkColorType::kN32_SkColorType, kPremul_SkAlphaType);
    skBitmap.installPixels(imageInfo, dst32Storage.begin(), width * sizeof(int32_t),
                           nullptr, nullptr);
    imageInfo = SkImageInfo::Make(width, height,
            SkColorType::kAlpha_8_SkColorType, kPremul_SkAlphaType);
    skMask.installPixels(imageInfo, dst8Storage.begin(), width, nullptr, nullptr);
    sk_sp<SkImage> skSrc = skBitmap.asImage();
    sk_sp<SkImage> skMaskImage = skMask.asImage();
    paint.setShader(
        SkShaders::Blend(SkBlendMode::kSrcIn,
                         skMaskImage->makeShader(SkSamplingOptions()),
                         skSrc->makeShader(SkSamplingOptions())));
    canvas->drawRect(r, paint);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static sk_sp<SkShader> make_src_shader(SkScalar size) {
    const SkPoint pts[] = { { 0, 0 }, { 0, size } };
    const SkColor colors[] = { 0xFF0000FF, 0x000000FF };
    return SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp);
}

static sk_sp<SkShader> make_dst_shader(SkScalar size) {
    const SkPoint pts[] = { { 0, 0 }, { size, 0 } };
    const SkColor colors[] = { SK_ColorRED, 0x00FF0000 };
    return SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp);
}

const SkScalar gCellSize = 100;

static void draw_cell(SkCanvas* canvas, sk_sp<SkShader> src, sk_sp<SkShader> dst,
                      SkBlendMode mode, SkAlpha alpha) {
    const SkRect r = SkRect::MakeWH(gCellSize, gCellSize);
    SkPaint p;
    p.setAlpha(alpha);

    SkAutoCanvasRestore acr(canvas, false);
    canvas->saveLayer(&r, &p);
    p.setAlpha(0xFF);

    p.setShader(dst);
    p.setBlendMode(SkBlendMode::kSrc);
    canvas->drawRect(r, p);

    p.setShader(src);
    p.setBlendMode(mode);
    canvas->drawRect(r, p);
}

static void draw_composed(SkCanvas* canvas, sk_sp<SkShader> src, sk_sp<SkShader> dst,
                          SkBlendMode mode, SkAlpha alpha) {
    SkPaint p;
    p.setAlpha(alpha);
    p.setShader(SkShaders::Blend(mode, dst, src));
    canvas->drawRect(SkRect::MakeWH(gCellSize, gCellSize), p);
}

static void draw_pair(SkCanvas* canvas, sk_sp<SkShader> src, sk_sp<SkShader> dst,
                      SkBlendMode mode) {
    SkAutoCanvasRestore acr(canvas, true);

    const SkScalar gap = 4;
    SkRect r = SkRect::MakeWH(2 * gCellSize + gap, 2 * gCellSize + gap);
    r.outset(gap + 1.5f, gap + 1.5f);
    SkPaint p;
    p.setStyle(SkPaint::kStroke_Style);
    canvas->drawRect(r, p); // border

    SkAlpha alpha = 0xFF;
    for (int y = 0; y < 2; ++y) {
        draw_cell(canvas, src, dst, mode, alpha);
        canvas->save();
        canvas->translate(gCellSize + gap, 0);
        draw_composed(canvas, src, dst, mode, alpha);
        canvas->restore();

        canvas->translate(0, gCellSize + gap);
        alpha = 0x80;
    }
}

DEF_SIMPLE_GM(composeshader_grid, canvas, 882, 882) {
    auto src = make_src_shader(gCellSize);
    auto dst = make_dst_shader(gCellSize);

    const SkScalar margin = 15;
    const SkScalar dx = 2*gCellSize + margin;
    const SkScalar dy = 2*gCellSize + margin;

    canvas->translate(margin, margin);
    canvas->save();
    for (int m = 0; m < 16; ++m) {
        SkBlendMode mode = static_cast<SkBlendMode>(m);
        draw_pair(canvas, src, dst, mode);
        if ((m % 4) == 3) {
            canvas->restore();
            canvas->translate(0, dy);
            canvas->save();
        } else {
            canvas->translate(dx, 0);
        }
    }
    canvas->restore();
}
