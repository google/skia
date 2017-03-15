/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkShader.h"

static const SkBlendMode gModes[] = {
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

    SkScalar offset = SK_Scalar1 / 3;
    SkRect rect = SkRect::MakeXYWH(W / 4 + offset,
                                   H / 4 + offset,
                                   W / 2, H / 2);
    canvas->drawRect(rect, paint);

    return H;
}

static sk_sp<SkShader> make_bg_shader() {
    SkBitmap bm;
    bm.allocN32Pixels(2, 2);
    *bm.getAddr32(0, 0) = *bm.getAddr32(1, 1) = 0xFFFFFFFF;
    *bm.getAddr32(1, 0) = *bm.getAddr32(0, 1) = SkPackARGB32(0xFF, 0xCC,
                                                             0xCC, 0xCC);

    SkMatrix m;
    m.setScale(SkIntToScalar(6), SkIntToScalar(6));

    return SkShader::MakeBitmapShader(bm,
                                      SkShader::kRepeat_TileMode,
                                      SkShader::kRepeat_TileMode,
                                      &m);
}

class AARectsModesView : public SampleView {
    SkPaint fBGPaint;
public:
    AARectsModesView () {
        fBGPaint.setShader(make_bg_shader());
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "AARectsModes");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        const SkRect bounds = SkRect::MakeWH(W, H);
        static const SkAlpha gAlphaValue[] = { 0xFF, 0x88, 0x88 };

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
                SkScalar dy = drawCell(canvas, gModes[i], gAlphaValue[alpha & 1],
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
    typedef SampleView INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new AARectsModesView; }
static SkViewRegister reg(MyFactory);
