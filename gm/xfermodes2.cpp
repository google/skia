
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "SkBitmap.h"
#include "SkShader.h"
#include "SkXfermode.h"
#include "SkColorPriv.h"

namespace skiagm {

class Xfermodes2GM : public GM {
public:
    Xfermodes2GM() {}

protected:
    virtual SkString onShortName() SK_OVERRIDE {
        return SkString("xfermodes2");
    }

    virtual SkISize onISize() SK_OVERRIDE {
        return make_isize(455, 475);
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        canvas->translate(SkIntToScalar(10), SkIntToScalar(20));

        const SkScalar w = SkIntToScalar(kSize);
        const SkScalar h = SkIntToScalar(kSize);

        SkPaint labelP;
        labelP.setAntiAlias(true);
        labelP.setTextAlign(SkPaint::kCenter_Align);

        const int W = 6;

        SkScalar x = 0, y = 0;
        for (size_t m = 0; m <= SkXfermode::kLastMode; m++) {
            SkXfermode::Mode mode = static_cast<SkXfermode::Mode>(m);
            SkXfermode* xm = SkXfermode::Create(mode);
            SkAutoUnref aur(xm);

            canvas->save();

            canvas->translate(x, y);
            SkPaint p;
            p.setAntiAlias(false);
            p.setStyle(SkPaint::kFill_Style);
            p.setShader(fBG);
            SkRect r = SkRect::MakeWH(w, h);
            canvas->drawRect(r, p);

            canvas->saveLayer(&r, NULL, SkCanvas::kARGB_ClipLayer_SaveFlag);

            p.setShader(fDst);
            canvas->drawRect(r, p);
            p.setShader(fSrc);
            p.setXfermode(xm);
            canvas->drawRect(r, p);

            canvas->restore();

            r.inset(-SK_ScalarHalf, -SK_ScalarHalf);
            p.setStyle(SkPaint::kStroke_Style);
            p.setShader(NULL);
            p.setXfermode(NULL);
            canvas->drawRect(r, p);

            canvas->restore();

#if 1
            canvas->drawText(SkXfermode::ModeName(mode), strlen(SkXfermode::ModeName(mode)),
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
    virtual void onOnceBeforeDraw() SK_OVERRIDE {
        static const uint32_t kCheckData[] = {
            SkPackARGB32(0xFF, 0x40, 0x40, 0x40),
            SkPackARGB32(0xFF, 0xD0, 0xD0, 0xD0),
            SkPackARGB32(0xFF, 0xD0, 0xD0, 0xD0),
            SkPackARGB32(0xFF, 0x40, 0x40, 0x40)
        };
        SkBitmap bg;
        bg.setConfig(SkBitmap::kARGB_8888_Config, 2, 2, 0, kOpaque_SkAlphaType);
        bg.allocPixels();
        SkAutoLockPixels bgAlp(bg);
        memcpy(bg.getPixels(), kCheckData, sizeof(kCheckData));

        fBG.reset(SkShader::CreateBitmapShader(bg,
                                               SkShader::kRepeat_TileMode,
                                               SkShader::kRepeat_TileMode));
        SkMatrix lm;
        lm.setScale(SkIntToScalar(16), SkIntToScalar(16));
        fBG->setLocalMatrix(lm);

        SkBitmap dstBmp;
        dstBmp.setConfig(SkBitmap::kARGB_8888_Config, kSize, kSize);
        dstBmp.allocPixels();
        SkAutoLockPixels dstAlp(dstBmp);
        SkPMColor* pixels = reinterpret_cast<SkPMColor*>(dstBmp.getPixels());

        for (int y = 0; y < kSize; ++y) {
            int c = y * (1 << kShift);
            SkPMColor rowColor = SkPackARGB32(c, c, 0, c/2);
            for (int x = 0; x < kSize; ++x) {
                pixels[kSize * y + x] = rowColor;
            }
        }
        fSrc.reset(SkShader::CreateBitmapShader(dstBmp,
                                                SkShader::kClamp_TileMode,
                                                SkShader::kClamp_TileMode));
        SkBitmap srcBmp;
        srcBmp.setConfig(SkBitmap::kARGB_8888_Config, kSize, kSize);
        srcBmp.allocPixels();
        SkAutoLockPixels srcAlp(srcBmp);
        pixels = reinterpret_cast<SkPMColor*>(srcBmp.getPixels());

        for (int x = 0; x < kSize; ++x) {
            int c = x * (1 << kShift);
            SkPMColor colColor = SkPackARGB32(c, 0, c, c/2);
            for (int y = 0; y < kSize; ++y) {
                pixels[kSize * y + x] = colColor;
            }
        }
        fDst.reset(SkShader::CreateBitmapShader(srcBmp,
                                                SkShader::kClamp_TileMode,
                                                SkShader::kClamp_TileMode));
    }

    enum {
        kShift = 2,
        kSize = 256 >> kShift,
    };

    SkAutoTUnref<SkShader> fBG;
    SkAutoTUnref<SkShader> fSrc;
    SkAutoTUnref<SkShader> fDst;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new Xfermodes2GM; }
static GMRegistry reg(MyFactory);

}
