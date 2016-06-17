/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkColorFilter.h"
#include "SkGradientShader.h"

#define WIDTH 512
#define HEIGHT 1024

namespace skiagm {

// Using gradients because GPU doesn't currently have an implementation of SkColorShader (duh!)
static sk_sp<SkShader> make_color_shader(SkColor color) {
    static const SkPoint kPts[] = {{0, 0}, {1, 1}};
    SkColor colors[] = {color, color};

    return SkGradientShader::MakeLinear(kPts, colors, nullptr, 2, SkShader::kClamp_TileMode);
}

static sk_sp<SkShader> make_solid_shader() {
    return make_color_shader(SkColorSetARGB(0xFF, 0x42, 0x82, 0x21));
}

static sk_sp<SkShader> make_transparent_shader() {
    return make_color_shader(SkColorSetARGB(0x80, 0x10, 0x70, 0x20));
}

static sk_sp<SkShader> make_trans_black_shader() {
    return make_color_shader(0x0);
}

// draws a background behind each test rect to see transparency
static sk_sp<SkShader> make_bg_shader(int checkSize) {
    SkBitmap bmp;
    bmp.allocN32Pixels(2 * checkSize, 2 * checkSize);
    SkCanvas canvas(bmp);
    canvas.clear(sk_tool_utils::color_to_565(0xFF800000));
    SkPaint paint;
    paint.setColor(sk_tool_utils::color_to_565(0xFF000080));
    SkRect rect0 = SkRect::MakeXYWH(0, 0,
                                    SkIntToScalar(checkSize), SkIntToScalar(checkSize));
    SkRect rect1 = SkRect::MakeXYWH(SkIntToScalar(checkSize), SkIntToScalar(checkSize),
                                    SkIntToScalar(checkSize), SkIntToScalar(checkSize));
    canvas.drawRect(rect1, paint);
    canvas.drawRect(rect0, paint);
    return SkShader::MakeBitmapShader(bmp, SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode);
}

class ModeColorFilterGM : public GM {
public:
    ModeColorFilterGM() {
        this->setBGColor(sk_tool_utils::color_to_565(0xFF303030));
    }

protected:
    SkString onShortName() override  {
        return SkString("modecolorfilters");
    }

    SkISize onISize() override  {
        return SkISize::Make(WIDTH, HEIGHT);
    }

    void onDraw(SkCanvas* canvas) override {
        // size of rect for each test case
        static const int kRectWidth  = 20;
        static const int kRectHeight = 20;

        static const int kCheckSize  = 10;

        if (!fBmpShader) {
            fBmpShader = make_bg_shader(kCheckSize);
        }
        SkPaint bgPaint;
        bgPaint.setShader(fBmpShader);
        bgPaint.setXfermodeMode(SkXfermode::kSrc_Mode);

        sk_sp<SkShader> shaders[] = {
            nullptr,                                   // use a paint color instead of a shader
            make_solid_shader(),
            make_transparent_shader(),
            make_trans_black_shader(),
        };

        // used without shader
        SkColor colors[] = {
            SkColorSetARGB(0xFF, 0xFF, 0xFF, 0xFF),
            SkColorSetARGB(0xFF, 0x00, 0x00, 0x00),
            SkColorSetARGB(0x00, 0x00, 0x00, 0x00),
            SkColorSetARGB(0xFF, 0x10, 0x20, 0x42),
            SkColorSetARGB(0xA0, 0x20, 0x30, 0x90),
        };

        // used with shaders
        SkColor alphas[] = {0xFFFFFFFF, 0x80808080};

        SkXfermode::Mode modes[]  = { // currently just doing the Modes expressible as Coeffs
            SkXfermode::kClear_Mode,
            SkXfermode::kSrc_Mode,
            SkXfermode::kDst_Mode,
            SkXfermode::kSrcOver_Mode,
            SkXfermode::kDstOver_Mode,
            SkXfermode::kSrcIn_Mode,
            SkXfermode::kDstIn_Mode,
            SkXfermode::kSrcOut_Mode,
            SkXfermode::kDstOut_Mode,
            SkXfermode::kSrcATop_Mode,
            SkXfermode::kDstATop_Mode,
            SkXfermode::kXor_Mode,
            SkXfermode::kPlus_Mode,
            SkXfermode::kModulate_Mode,
        };

        SkPaint paint;
        int idx = 0;
        static const int kRectsPerRow = SkMax32(this->getISize().fWidth / kRectWidth, 1);
        for (size_t cfm = 0; cfm < SK_ARRAY_COUNT(modes); ++cfm) {
            for (size_t cfc = 0; cfc < SK_ARRAY_COUNT(colors); ++cfc) {
                paint.setColorFilter(SkColorFilter::MakeModeFilter(colors[cfc], modes[cfm]));
                for (size_t s = 0; s < SK_ARRAY_COUNT(shaders); ++s) {
                    paint.setShader(shaders[s]);
                    bool hasShader = nullptr == paint.getShader();
                    int paintColorCnt = hasShader ? SK_ARRAY_COUNT(alphas) : SK_ARRAY_COUNT(colors);
                    SkColor* paintColors = hasShader ? alphas : colors;
                    for (int pc = 0; pc < paintColorCnt; ++pc) {
                        paint.setColor(paintColors[pc]);
                        SkScalar x = SkIntToScalar(idx % kRectsPerRow);
                        SkScalar y = SkIntToScalar(idx / kRectsPerRow);
                        SkRect rect = SkRect::MakeXYWH(x * kRectWidth, y * kRectHeight,
                                                       SkIntToScalar(kRectWidth),
                                                       SkIntToScalar(kRectHeight));
                        canvas->saveLayer(&rect, nullptr);
                        canvas->drawRect(rect, bgPaint);
                        canvas->drawRect(rect, paint);
                        canvas->restore();
                        ++idx;
                    }
                }
            }
        }
    }

private:
    sk_sp<SkShader> fBmpShader;
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new ModeColorFilterGM; }
static GMRegistry reg(MyFactory);

}
