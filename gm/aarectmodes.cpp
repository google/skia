/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkPath.h"
#include "SkShader.h"

static void test4(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    SkPoint pts[] = {
        {10, 160}, {610, 160},
        {610, 160}, {10, 160},

        {610, 160}, {610, 160},
        {610, 199}, {610, 199},

        {10, 198}, {610, 198},
        {610, 199}, {10, 199},

        {10, 160}, {10, 160},
        {10, 199}, {10, 199}
    };
    char verbs[] = {
        0, 1, 1, 1, 4,
        0, 1, 1, 1, 4,
        0, 1, 1, 1, 4,
        0, 1, 1, 1, 4
    };
    SkPath path;
    SkPoint* ptPtr = pts;
    for (size_t i = 0; i < sizeof(verbs); ++i) {
        switch ((SkPath::Verb) verbs[i]) {
            case SkPath::kMove_Verb:
                path.moveTo(ptPtr->fX, ptPtr->fY);
                ++ptPtr;
                break;
            case SkPath::kLine_Verb:
                path.lineTo(ptPtr->fX, ptPtr->fY);
                ++ptPtr;
                break;
            case SkPath::kClose_Verb:
                path.close();
                break;
            default:
                SkASSERT(false);
                break;
        }
    }
    SkRect clip = {0, 130, 772, 531};
    canvas->clipRect(clip);
    canvas->drawPath(path, paint);
}

static const struct {
    SkXfermode::Mode  fMode;
    const char*         fLabel;
} gModes[] = {
    { SkXfermode::kClear_Mode,    "Clear"     },
    { SkXfermode::kSrc_Mode,      "Src"       },
    { SkXfermode::kDst_Mode,      "Dst"       },
    { SkXfermode::kSrcOver_Mode,  "SrcOver"   },
    { SkXfermode::kDstOver_Mode,  "DstOver"   },
    { SkXfermode::kSrcIn_Mode,    "SrcIn"     },
    { SkXfermode::kDstIn_Mode,    "DstIn"     },
    { SkXfermode::kSrcOut_Mode,   "SrcOut"    },
    { SkXfermode::kDstOut_Mode,   "DstOut"    },
    { SkXfermode::kSrcATop_Mode,  "SrcATop"   },
    { SkXfermode::kDstATop_Mode,  "DstATop"   },
    { SkXfermode::kXor_Mode,      "Xor"       },
};

const int gWidth = 64;
const int gHeight = 64;
const SkScalar W = SkIntToScalar(gWidth);
const SkScalar H = SkIntToScalar(gHeight);

static SkScalar drawCell(SkCanvas* canvas, SkXfermode* mode,
                         SkAlpha a0, SkAlpha a1) {

    SkPaint paint;
    paint.setAntiAlias(true);

    SkRect r = SkRect::MakeWH(W, H);
    r.inset(W/10, H/10);

    paint.setColor(SK_ColorBLUE);
    paint.setAlpha(a0);
    canvas->drawOval(r, paint);

    paint.setColor(SK_ColorRED);
    paint.setAlpha(a1);
    paint.setXfermode(mode);

    SkScalar offset = SK_Scalar1 / 3;
    SkRect rect = SkRect::MakeXYWH(W / 4 + offset,
                                   H / 4 + offset,
                                   W / 2, H / 2);
    canvas->drawRect(rect, paint);

    return H;
}

static SkShader* make_bg_shader() {
    SkBitmap bm;
    bm.allocN32Pixels(2, 2);
    *bm.getAddr32(0, 0) = *bm.getAddr32(1, 1) = 0xFFFFFFFF;
    *bm.getAddr32(1, 0) = *bm.getAddr32(0, 1) = SkPackARGB32(0xFF, 0xCE,
                                                             0xCF, 0xCE);

    const SkMatrix m = SkMatrix::MakeScale(SkIntToScalar(6), SkIntToScalar(6));
    SkShader* s = SkShader::CreateBitmapShader(bm,
                                               SkShader::kRepeat_TileMode,
                                               SkShader::kRepeat_TileMode,
                                               &m);

    return s;
}

namespace skiagm {

    class AARectModesGM : public GM {
        SkPaint fBGPaint;
    public:
        AARectModesGM () {
            fBGPaint.setShader(make_bg_shader())->unref();
        }

    protected:

        SkString onShortName() override {
            return SkString("aarectmodes");
        }

        SkISize onISize() override { return SkISize::Make(640, 480); }

        void onDraw(SkCanvas* canvas) override {
            if (false) { // avoid bit rot, suppress warning
                test4(canvas);
            }
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
                    SkXfermode* mode = SkXfermode::Create(gModes[i].fMode);

                    canvas->drawRect(bounds, fBGPaint);
                    canvas->saveLayer(&bounds, NULL);
                    SkScalar dy = drawCell(canvas, mode,
                                           gAlphaValue[alpha & 1],
                                           gAlphaValue[alpha & 2]);
                    canvas->restore();

                    canvas->translate(0, dy * 5 / 4);
                    SkSafeUnref(mode);
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

    static GM* MyFactory(void*) { return new AARectModesGM; }
    static GMRegistry reg(MyFactory);

}
