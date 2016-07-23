/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkShader.h"
#include "SkString.h"
#include "SkXfermode.h"

static sk_sp<SkShader> make_shader(SkXfermode::Mode mode) {
    SkPoint pts[2];
    SkColor colors[2];

    pts[0].set(0, 0);
    pts[1].set(SkIntToScalar(100), 0);
    colors[0] = SK_ColorRED;
    colors[1] = SK_ColorBLUE;
    auto shaderA = SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkShader::kClamp_TileMode);

    pts[0].set(0, 0);
    pts[1].set(0, SkIntToScalar(100));
    colors[0] = SK_ColorBLACK;
    colors[1] = SkColorSetARGB(0x80, 0, 0, 0);
    auto shaderB = SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkShader::kClamp_TileMode);

    return SkShader::MakeComposeShader(std::move(shaderA), std::move(shaderB),
                                       SkXfermode::Make(mode));
}

class ComposeShaderGM : public skiagm::GM {
public:
    ComposeShaderGM() {
        fShader = make_shader(SkXfermode::kDstIn_Mode);
    }

protected:
    SkString onShortName() override {
        return SkString("composeshader");
    }

    SkISize onISize() override {
        return SkISize::Make(120, 120);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setColor(SK_ColorGREEN);
        canvas->drawRectCoords(0, 0, SkIntToScalar(100), SkIntToScalar(100), paint);
        paint.setShader(fShader);
        canvas->drawRectCoords(0, 0, SkIntToScalar(100), SkIntToScalar(100), paint);
    }

protected:
    sk_sp<SkShader> fShader;

private:
    typedef GM INHERITED ;
};

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
            make_shader(SkXfermode::kDstIn_Mode),
            make_shader(SkXfermode::kSrcOver_Mode),
        };

        SkPaint paint;
        paint.setColor(SK_ColorGREEN);

        const SkRect r = SkRect::MakeXYWH(5, 5, 100, 100);

        for (size_t y = 0; y < SK_ARRAY_COUNT(shaders); ++y) {
            canvas->save();
            for (int alpha = 0xFF; alpha > 0; alpha -= 0x28) {
                paint.setAlpha(0xFF);
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
    return SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkShader::kClamp_TileMode);
}


class ComposeShaderBitmapGM : public skiagm::GM {
public:
    ComposeShaderBitmapGM() {}

protected:
    void onOnceBeforeDraw() override {
        draw_color_bm(&fColorBitmap, squareLength);
        draw_alpha8_bm(&fAlpha8Bitmap, squareLength);
        SkMatrix s;
        s.reset();
        fColorBitmapShader = SkShader::MakeBitmapShader(fColorBitmap, SkShader::kRepeat_TileMode,
                                                        SkShader::kRepeat_TileMode, &s);
        fAlpha8BitmapShader = SkShader::MakeBitmapShader(fAlpha8Bitmap, SkShader::kRepeat_TileMode,
                                                         SkShader::kRepeat_TileMode, &s);
        fLinearGradientShader = make_linear_gradient_shader(squareLength);
    }

    SkString onShortName() override {
        return SkString("composeshader_bitmap");
    }

    SkISize onISize() override {
        return SkISize::Make(7 * (squareLength + 5), 2 * (squareLength + 5));
    }

    void onDraw(SkCanvas* canvas) override {
        auto xfer(SkXfermode::Make(SkXfermode::kDstOver_Mode));

        sk_sp<SkShader> shaders[] = {
            // gradient should appear over color bitmap
            SkShader::MakeComposeShader(fLinearGradientShader, fColorBitmapShader, xfer),
            // gradient should appear over alpha8 bitmap colorized by the paint color
            SkShader::MakeComposeShader(fLinearGradientShader, fAlpha8BitmapShader, xfer),
        };

        SkPaint paint;
        paint.setColor(SK_ColorYELLOW);

        const SkRect r = SkRect::MakeXYWH(0, 0, SkIntToScalar(squareLength),
                                          SkIntToScalar(squareLength));

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
    static const int squareLength = 20;

    SkBitmap fColorBitmap;
    SkBitmap fAlpha8Bitmap;
    sk_sp<SkShader> fColorBitmapShader;
    sk_sp<SkShader> fAlpha8BitmapShader;
    sk_sp<SkShader> fLinearGradientShader;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new ComposeShaderGM; )
DEF_GM( return new ComposeShaderAlphaGM; )
DEF_GM( return new ComposeShaderBitmapGM; )
