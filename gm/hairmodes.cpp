/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkShader.h"

constexpr SkBlendMode gModes[] = {
    SkBlendMode::kClear,
    SkBlendMode::kSrc,
    SkBlendMode::kDst,
    SkBlendMode::kSrcOver,
    SkBlendMode::kDstOver,
    SkBlendMode::kSrcIn,
    SkBlendMode::kDstIn,
    SkBlendMode::kSrcOut,
    SkBlendMode::kDstOut,
    SkBlendMode::kSrcATop,
    SkBlendMode::kDstATop,
    SkBlendMode::kXor,
};

const int gWidth = 64;
const int gHeight = 64;
const SkScalar W = SkIntToScalar(gWidth);
const SkScalar H = SkIntToScalar(gHeight);

static SkScalar drawCell(SkCanvas* canvas, SkBlendMode mode, SkAlpha a0, SkAlpha a1) {

    SkPaint paint;
    paint.setAntiAlias(true);

    SkRect r = SkRect::MakeWH(W, H);
    r.inset(W/10, H/10);

    paint.setColor(SK_ColorBLUE);
    paint.setAlpha(a0);
    canvas->drawOval(r, paint);

    paint.setColor(SK_ColorRED);
    paint.setAlpha(a1);
    paint.setBlendMode(mode);
    for (int angle = 0; angle < 24; ++angle) {
        SkScalar x = SkScalarCos(SkIntToScalar(angle) * (SK_ScalarPI * 2) / 24) * gWidth;
        SkScalar y = SkScalarSin(SkIntToScalar(angle) * (SK_ScalarPI * 2) / 24) * gHeight;
        paint.setStrokeWidth(SK_Scalar1 * angle * 2 / 24);
        canvas->drawLine(W/2, H/2, W/2 + x, H/2 + y, paint);
    }

    return H;
}

static sk_sp<SkShader> make_bg_shader() {
    SkBitmap bm;
    bm.allocN32Pixels(2, 2);
    *bm.getAddr32(0, 0) = *bm.getAddr32(1, 1) = 0xFFFFFFFF;
    *bm.getAddr32(1, 0) = *bm.getAddr32(0, 1) = SkPackARGB32(0xFF, 0xCE, 0xCF, 0xCE);

    SkMatrix m;
    m.setScale(SkIntToScalar(6), SkIntToScalar(6));
    return bm.makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, &m);
}

namespace skiagm {

    class HairModesGM : public GM {
        SkPaint fBGPaint;

    protected:
        SkString onShortName() override {
            return SkString("hairmodes");
        }

        virtual SkISize onISize() override { return SkISize::Make(640, 480); }

        void onOnceBeforeDraw() override {
            fBGPaint.setShader(make_bg_shader());
        }

        void onDraw(SkCanvas* canvas) override {
            const SkRect bounds = SkRect::MakeWH(W, H);
            constexpr SkAlpha gAlphaValue[] = { 0xFF, 0x88, 0x88 };

            canvas->translate(SkIntToScalar(4), SkIntToScalar(4));

            for (int alpha = 0; alpha < 4; ++alpha) {
                canvas->save();
                canvas->save();
                for (size_t i = 0; i < SK_ARRAY_COUNT(gModes); ++i) {
                    if (6 == i) {
                        canvas->restore();
                        canvas->translate(W * 5, 0);
                        canvas->save();
                    }

                    canvas->drawRect(bounds, fBGPaint);
                    canvas->saveLayer(&bounds, nullptr);
                    SkScalar dy = drawCell(canvas, gModes[i],
                                           gAlphaValue[alpha & 1],
                                           gAlphaValue[alpha & 2]);
                    canvas->restore();

                    canvas->translate(0, dy * 5 / 4);
                }
                canvas->restore();
                canvas->restore();
                canvas->translate(W * 5 / 4, 0);
            }
        }

    private:
        typedef GM INHERITED;
    };

    //////////////////////////////////////////////////////////////////////////////

    DEF_GM( return new HairModesGM; )
}
