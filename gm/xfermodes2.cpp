/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "sk_tool_utils.h"
#include "SkBitmap.h"
#include "SkShader.h"
#include "SkBlendModePriv.h"
#include "SkColorPriv.h"

namespace skiagm {

class Xfermodes2GM : public GM {
public:
    Xfermodes2GM() {}

protected:
    SkString onShortName() override {
        return SkString("xfermodes2");
    }

    SkISize onISize() override {
        return SkISize::Make(455, 475);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(SkIntToScalar(10), SkIntToScalar(20));

        const SkScalar w = SkIntToScalar(kSize);
        const SkScalar h = SkIntToScalar(kSize);

        SkPaint labelP;
        labelP.setAntiAlias(true);
        sk_tool_utils::set_portable_typeface(&labelP);
        labelP.setTextAlign(SkPaint::kCenter_Align);

        const int W = 6;

        SkScalar x = 0, y = 0;
        for (size_t m = 0; m <= (size_t)SkBlendMode::kLastMode; m++) {
            SkBlendMode mode = static_cast<SkBlendMode>(m);

            canvas->save();

            canvas->translate(x, y);
            SkPaint p;
            p.setAntiAlias(false);
            p.setStyle(SkPaint::kFill_Style);
            p.setShader(fBG);
            SkRect r = SkRect::MakeWH(w, h);
            canvas->drawRect(r, p);

            canvas->saveLayer(&r, nullptr);

            p.setShader(fDst);
            canvas->drawRect(r, p);
            p.setShader(fSrc);
            p.setBlendMode(mode);
            canvas->drawRect(r, p);

            canvas->restore();

            r.inset(-SK_ScalarHalf, -SK_ScalarHalf);
            p.setStyle(SkPaint::kStroke_Style);
            p.setShader(nullptr);
            p.setBlendMode(SkBlendMode::kSrcOver);
            canvas->drawRect(r, p);

            canvas->restore();

#if 1
            canvas->drawString(SkBlendMode_Name(mode),
                               x + w/2, y - labelP.getTextSize()/2, labelP);
#endif
            x += w + SkIntToScalar(10);
            if ((m % W) == W - 1) {
                x = 0;
                y += h + SkIntToScalar(30);
            }
        }
    }

private:
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
        lm.setScale(SkIntToScalar(16), SkIntToScalar(16));
        fBG = SkShader::MakeBitmapShader(bg, SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode,
                                         &lm);

        SkBitmap srcBmp;
        srcBmp.allocN32Pixels(kSize, kSize);
        SkPMColor* pixels = reinterpret_cast<SkPMColor*>(srcBmp.getPixels());

        for (int y = 0; y < kSize; ++y) {
            int c = y * (1 << kShift);
            SkPMColor rowColor = SkPackARGB32(c, c, 0, c/2);
            for (int x = 0; x < kSize; ++x) {
                pixels[kSize * y + x] = rowColor;
            }
        }
        fSrc = SkShader::MakeBitmapShader(srcBmp, SkShader::kClamp_TileMode,
                                          SkShader::kClamp_TileMode);
        SkBitmap dstBmp;
        dstBmp.allocN32Pixels(kSize, kSize);
        pixels = reinterpret_cast<SkPMColor*>(dstBmp.getPixels());

        for (int x = 0; x < kSize; ++x) {
            int c = x * (1 << kShift);
            SkPMColor colColor = SkPackARGB32(c, 0, c, c/2);
            for (int y = 0; y < kSize; ++y) {
                pixels[kSize * y + x] = colColor;
            }
        }
        fDst = SkShader::MakeBitmapShader(dstBmp, SkShader::kClamp_TileMode,
                                          SkShader::kClamp_TileMode);
    }

    enum {
        kShift = 2,
        kSize = 256 >> kShift,
    };

    sk_sp<SkShader> fBG;
    sk_sp<SkShader> fSrc;
    sk_sp<SkShader> fDst;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new Xfermodes2GM; }
static GMRegistry reg(MyFactory);

}
