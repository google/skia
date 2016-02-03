/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "Resources.h"
#include "SkBitmapScaler.h"
#include "SkGradientShader.h"
#include "SkTypeface.h"
#include "SkImageDecoder.h"
#include "SkStream.h"
#include "SkPaint.h"
#include "SkMipMap.h"
#include "Resources.h"
#include "sk_tool_utils.h"

static SkBitmap make_bitmap(int size) {
    SkBitmap bm;
    bm.allocN32Pixels(size, size);
    SkCanvas canvas(bm);
    canvas.clear(0xFFFFFFFF);
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(size / 16.0f);
    canvas.drawCircle(size/2.0f, size/2.0f, size/3.0f, paint);
    return bm;
}

static SkBitmap make_bitmap2(int size) {
    SkBitmap bm;
    bm.allocN32Pixels(size, size);
    SkCanvas canvas(bm);
    canvas.clear(0xFFFFFFFF);
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);

    SkScalar inset = 2;
    SkRect r = SkRect::MakeIWH(size, size).makeInset(0.5f, 0.5f);
    while (r.width() > 4) {
        canvas.drawRect(r, paint);
        r.inset(inset, inset);
        inset += 1;
    }
    return bm;
}

#include "SkNx.h"
static SkBitmap make_bitmap3(int size) {
    SkBitmap bm;
    bm.allocN32Pixels(size, size);
    SkCanvas canvas(bm);
    canvas.clear(0xFFFFFFFF);
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(2.1f);

    SkScalar s = SkIntToScalar(size);
    Sk4f p(s, -s, -s, s);
    Sk4f d(5);
    while (p.kth<1>() < s) {
        canvas.drawLine(p.kth<0>(),p.kth<1>(), p.kth<2>(), p.kth<3>(), paint);
        p = p + d;
    }
    return bm;
}

class ShowMipLevels : public skiagm::GM {
    const int fN;
    SkBitmap  fBM[4];

public:
    static unsigned gamma(unsigned n) {
        float x = n / 255.0f;
#if 0
        x = sqrtf(x);
#else
        if (x > 0.0031308f) {
            x = 1.055f * (powf(x, (1.0f / 2.4f))) - 0.055f;
        } else {
            x = 12.92f * x;
        }
#endif
        return (int)(x * 255);
    }

    static void apply_gamma(const SkBitmap& bm) {
        return; // below is our experiment for sRGB correction
        bm.lockPixels();
        for (int y = 0; y < bm.height(); ++y) {
            for (int x = 0; x < bm.width(); ++x) {
                SkPMColor c = *bm.getAddr32(x, y);
                unsigned r = gamma(SkGetPackedR32(c));
                unsigned g = gamma(SkGetPackedG32(c));
                unsigned b = gamma(SkGetPackedB32(c));
                *bm.getAddr32(x, y) = SkPackARGB32(0xFF, r, g, b);
            }
        }
    }

    ShowMipLevels(int N) : fN(N) {
        fBM[0] = sk_tool_utils::create_checkerboard_bitmap(N, N, SK_ColorBLACK, SK_ColorWHITE, 2);
        fBM[1] = make_bitmap(N);
        fBM[2] = make_bitmap2(N);
        fBM[3] = make_bitmap3(N);
    }

protected:

    SkString onShortName() override {
        SkString str;
        str.printf("showmiplevels_%d", fN);
        return str;
    }

    SkISize onISize() override {
        return { 824, 862 };
    }

    static void DrawAndFrame(SkCanvas* canvas, const SkBitmap& orig, SkScalar x, SkScalar y) {
        SkBitmap bm;
        orig.copyTo(&bm);
        apply_gamma(bm);

        canvas->drawBitmap(bm, x, y, nullptr);
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(0xFFFFCCCC);
        canvas->drawRect(SkRect::MakeIWH(bm.width(), bm.height()).makeOffset(x, y).makeOutset(0.5f, 0.5f), paint);
    }

    template <typename F> void drawLevels(SkCanvas* canvas, const SkBitmap& baseBM, F func) {
        SkScalar x = 4;
        SkScalar y = 4;

        SkPixmap prevPM;
        baseBM.lockPixels();
        baseBM.peekPixels(&prevPM);

        SkAutoTUnref<SkMipMap> mm(SkMipMap::Build(baseBM, nullptr));

        int index = 0;
        SkMipMap::Level level;
        SkScalar scale = 0.5f;
        while (mm->extractLevel(scale, &level)) {
            SkImageInfo info = SkImageInfo::MakeN32Premul(level.fWidth, level.fHeight);
            SkPixmap levelPM{ info, level.fPixels, level.fRowBytes };

            SkBitmap bm = func(prevPM, levelPM);
            DrawAndFrame(canvas, bm, x, y);

            if (info.width() <= 2 || info.height() <= 2) {
                break;
            }
            if (index & 1) {
                x += info.width() + 4;
            } else {
                y += info.height() + 4;
            }
            scale /= 2;
            prevPM = levelPM;
            index += 1;
        }
    }

    void drawSet(SkCanvas* canvas, const SkBitmap& orig) {
        SkAutoCanvasRestore acr(canvas, true);

        drawLevels(canvas, orig, [](const SkPixmap& prev, const SkPixmap& curr) {
            SkBitmap bm;
            bm.installPixels(curr);
            return bm;
        });

        const SkBitmapScaler::ResizeMethod methods[] = {
            SkBitmapScaler::RESIZE_BOX,
            SkBitmapScaler::RESIZE_TRIANGLE,
            SkBitmapScaler::RESIZE_LANCZOS3,
            SkBitmapScaler::RESIZE_HAMMING,
            SkBitmapScaler::RESIZE_MITCHELL,
        };

        SkPixmap basePM;
        orig.lockPixels();
        orig.peekPixels(&basePM);
        for (auto method : methods) {
            canvas->translate(orig.width()/2 + 8.0f, 0);
            drawLevels(canvas, orig, [basePM, method](const SkPixmap& prev, const SkPixmap& curr) {
                SkBitmap bm;
                SkBitmapScaler::Resize(&bm, prev, method, curr.width(), curr.height());
                return bm;
            });
        }
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(4, 4);
        for (const auto& bm : fBM) {
            this->drawSet(canvas, bm);
            canvas->translate(0, bm.height() * 0.85f);
        }
    }
    
private:
    typedef skiagm::GM INHERITED;
};
DEF_GM( return new ShowMipLevels(255); )
DEF_GM( return new ShowMipLevels(256); )

