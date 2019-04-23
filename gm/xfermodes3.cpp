/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkGradientShader.h"
#include "include/gpu/GrContext.h"
#include "include/utils/SkTextUtils.h"
#include "src/core/SkBlendModePriv.h"
#include "tools/ToolUtils.h"

namespace skiagm {

/**
 * This tests drawing device-covering rects with solid colors and bitmap shaders over a
 * checkerboard background using different xfermodes.
 */
class Xfermodes3GM : public GM {
public:
    Xfermodes3GM() { this->setBGColor(ToolUtils::color_to_565(0xFF70D0E0)); }

protected:
    SkString onShortName() override {
        return SkString("xfermodes3");
    }

    SkISize onISize() override {
        return SkISize::Make(630, 1215);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(SkIntToScalar(10), SkIntToScalar(20));

        SkFont  font(ToolUtils::create_portable_typeface());
        SkPaint labelP;

        constexpr SkColor kSolidColors[] = {
            SK_ColorTRANSPARENT,
            SK_ColorBLUE,
            0x80808000
        };

        constexpr SkColor kBmpAlphas[] = {
            0xff,
            0x80,
        };

        auto tempSurface(this->makeTempSurface(canvas, kSize, kSize));

        int test = 0;
        int x = 0, y = 0;
        constexpr struct { SkPaint::Style fStyle; SkScalar fWidth; } kStrokes[] = {
            {SkPaint::kFill_Style, 0},
            {SkPaint::kStroke_Style, SkIntToScalar(kSize) / 2},
        };
        for (size_t s = 0; s < SK_ARRAY_COUNT(kStrokes); ++s) {
            for (size_t m = 0; m <= (size_t)SkBlendMode::kLastMode; ++m) {
                SkBlendMode mode = static_cast<SkBlendMode>(m);
                canvas->drawString(SkBlendMode_Name(mode),
                                   SkIntToScalar(x),
                                   SkIntToScalar(y + kSize + 3) + font.getSize(),
                                   font, labelP);
                for (size_t c = 0; c < SK_ARRAY_COUNT(kSolidColors); ++c) {
                    SkPaint modePaint;
                    modePaint.setBlendMode(mode);
                    modePaint.setColor(kSolidColors[c]);
                    modePaint.setStyle(kStrokes[s].fStyle);
                    modePaint.setStrokeWidth(kStrokes[s].fWidth);

                    this->drawMode(canvas, x, y, kSize, kSize, modePaint, tempSurface.get());

                    ++test;
                    x += kSize + 10;
                    if (!(test % kTestsPerRow)) {
                        x = 0;
                        y += kSize + 30;
                    }
                }
                for (size_t a = 0; a < SK_ARRAY_COUNT(kBmpAlphas); ++a) {
                    SkPaint modePaint;
                    modePaint.setBlendMode(mode);
                    modePaint.setAlpha(kBmpAlphas[a]);
                    modePaint.setShader(fBmpShader);
                    modePaint.setStyle(kStrokes[s].fStyle);
                    modePaint.setStrokeWidth(kStrokes[s].fWidth);

                    this->drawMode(canvas, x, y, kSize, kSize, modePaint, tempSurface.get());

                    ++test;
                    x += kSize + 10;
                    if (!(test % kTestsPerRow)) {
                        x = 0;
                        y += kSize + 30;
                    }
                }
            }
        }
    }

private:
    /**
     * GrContext has optimizations around full rendertarget draws that can be replaced with clears.
     * We are trying to test those. We could use saveLayer() to create small SkGpuDevices but
     * saveLayer() uses the texture cache. This means that the actual render target may be larger
     * than the layer. Because the clip will contain the layer's bounds, no draws will be full-RT.
     * So explicitly create a temporary canvas with dimensions exactly the layer size.
     */
    sk_sp<SkSurface> makeTempSurface(SkCanvas* baseCanvas, int w, int h) {
        SkImageInfo baseInfo = baseCanvas->imageInfo();
        SkImageInfo info = SkImageInfo::Make(w, h, baseInfo.colorType(), baseInfo.alphaType(),
                                             baseInfo.refColorSpace());
        return baseCanvas->makeSurface(info);
    }

    void drawMode(SkCanvas* canvas,
                  int x, int y, int w, int h,
                  const SkPaint& modePaint, SkSurface* surface) {
        canvas->save();
        canvas->translate(SkIntToScalar(x), SkIntToScalar(y));

        SkRect r = SkRect::MakeWH(SkIntToScalar(w), SkIntToScalar(h));

        SkCanvas* modeCanvas;
        if (nullptr == surface) {
            canvas->saveLayer(&r, nullptr);
            canvas->clipRect(r);
            modeCanvas = canvas;
        } else {
            modeCanvas = surface->getCanvas();
        }

        SkPaint bgPaint;
        bgPaint.setAntiAlias(false);
        bgPaint.setShader(fBGShader);
        modeCanvas->drawRect(r, bgPaint);
        modeCanvas->drawRect(r, modePaint);
        modeCanvas = nullptr;

        if (nullptr == surface) {
            canvas->restore();
        } else {
            surface->draw(canvas, 0, 0, nullptr);
        }

        r.inset(-SK_ScalarHalf, -SK_ScalarHalf);
        SkPaint borderPaint;
        borderPaint.setStyle(SkPaint::kStroke_Style);
        canvas->drawRect(r, borderPaint);

        canvas->restore();
    }

    void onOnceBeforeDraw() override {
        const uint32_t kCheckData[] = {
            SkPackARGB32(0xFF, 0x42, 0x41, 0x42),
            SkPackARGB32(0xFF, 0xD6, 0xD3, 0xD6),
            SkPackARGB32(0xFF, 0xD6, 0xD3, 0xD6),
            SkPackARGB32(0xFF, 0x42, 0x41, 0x42)
        };
        SkBitmap bg;
        bg.allocN32Pixels(2, 2, true);
        memcpy(bg.getPixels(), kCheckData, sizeof(kCheckData));

        SkMatrix lm;
        lm.setScale(SkIntToScalar(kCheckSize), SkIntToScalar(kCheckSize));
        fBGShader = bg.makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, &lm);

        SkPaint bmpPaint;
        const SkPoint kCenter = { SkIntToScalar(kSize) / 2, SkIntToScalar(kSize) / 2 };
        const SkColor kColors[] = {
            SK_ColorTRANSPARENT, 0x80800000, 0xF020F060, SK_ColorWHITE
        };
        bmpPaint.setShader(SkGradientShader::MakeRadial(kCenter, 3 * SkIntToScalar(kSize) / 4,
                                                        kColors, nullptr, SK_ARRAY_COUNT(kColors),
                                                        SkTileMode::kRepeat));

        SkBitmap bmp;
        bmp.allocN32Pixels(kSize, kSize);
        SkCanvas bmpCanvas(bmp);

        bmpCanvas.clear(SK_ColorTRANSPARENT);
        SkRect rect = { SkIntToScalar(kSize) / 8, SkIntToScalar(kSize) / 8,
                        7 * SkIntToScalar(kSize) / 8, 7 * SkIntToScalar(kSize) / 8};
        bmpCanvas.drawRect(rect, bmpPaint);

        fBmpShader = bmp.makeShader();
    }

    enum {
        kCheckSize = 8,
        kSize = 30,
        kTestsPerRow = 15,
    };

    sk_sp<SkShader> fBGShader;
    sk_sp<SkShader> fBmpShader;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new Xfermodes3GM;)

}
