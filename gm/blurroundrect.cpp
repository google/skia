/*
* Copyright 2013 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkDrawLooper.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkLayerDrawLooper.h"
#include "src/core/SkBlurMask.h"

// This GM mimics a blurred RR seen in the wild.
class BlurRoundRectGM : public skiagm::GM {
public:
    BlurRoundRectGM(int width, int height)
        : fName("blurroundrect"), fWidth(width), fHeight(height) {
        fName.appendf("-WH-%ix%i-unevenCorners", width,  height);
    }

    SkString onShortName() override {
        return fName;
    }

    SkISize onISize() override {
        return SkISize::Make(fWidth, fHeight);
    }

    void onOnceBeforeDraw() override {
        SkVector radii[4];
        radii[0].set(SkIntToScalar(30), SkIntToScalar(30));
        radii[1].set(SkIntToScalar(10), SkIntToScalar(10));
        radii[2].set(SkIntToScalar(30), SkIntToScalar(30));
        radii[3].set(SkIntToScalar(10), SkIntToScalar(10));
        SkRect r = SkRect::MakeWH(SkIntToScalar(fWidth), SkIntToScalar(fHeight));
        fRRect.setRectRadii(r, radii);
    }

    void onDraw(SkCanvas* canvas) override {
        SkLayerDrawLooper::Builder looperBuilder;
        {
            SkLayerDrawLooper::LayerInfo info;
            info.fPaintBits = SkLayerDrawLooper::kMaskFilter_Bit
                              | SkLayerDrawLooper::kColorFilter_Bit;
            info.fColorMode = SkBlendMode::kSrc;
            info.fOffset = SkPoint::Make(SkIntToScalar(-1), SkIntToScalar(0));
            info.fPostTranslate = false;
            SkPaint* paint = looperBuilder.addLayerOnTop(info);
            paint->setMaskFilter(SkMaskFilter::MakeBlur(
                    kNormal_SkBlurStyle,
                    SkBlurMask::ConvertRadiusToSigma(SK_ScalarHalf)));
            paint->setColorFilter(SkColorFilters::Blend(SK_ColorLTGRAY, SkBlendMode::kSrcIn));
            paint->setColor(SK_ColorGRAY);
        }
        {
            SkLayerDrawLooper::LayerInfo info;
            looperBuilder.addLayerOnTop(info);
        }
        SkPaint paint;
        canvas->drawRect(fRRect.rect(), paint);

        paint.setLooper(looperBuilder.detach());
        paint.setColor(SK_ColorCYAN);
        paint.setAntiAlias(true);

        canvas->drawRRect(fRRect, paint);
    }

private:
    SkString        fName;
    SkRRect         fRRect;
    int             fWidth, fHeight;

    typedef skiagm::GM INHERITED;
};

/*
 * Spits out a dummy gradient to test blur with shader on paint
 */
static sk_sp<SkShader> MakeRadial() {
    SkPoint pts[2] = {
        { 0, 0 },
        { SkIntToScalar(100), SkIntToScalar(100) }
    };
    SkTileMode tm = SkTileMode::kClamp;
    const SkColor colors[] = { SK_ColorRED, SK_ColorGREEN, };
    const SkScalar pos[] = { SK_Scalar1/4, SK_Scalar1*3/4 };
    SkMatrix scale;
    scale.setScale(0.5f, 0.5f);
    scale.postTranslate(5.f, 5.f);
    SkPoint center0, center1;
    center0.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center1.set(SkScalarInterp(pts[0].fX, pts[1].fX, SkIntToScalar(3)/5),
                SkScalarInterp(pts[0].fY, pts[1].fY, SkIntToScalar(1)/4));
    return SkGradientShader::MakeTwoPointConical(center1, (pts[1].fX - pts[0].fX) / 7,
                                                 center0, (pts[1].fX - pts[0].fX) / 2,
                                                 colors, pos, SK_ARRAY_COUNT(colors), tm,
                                                 0, &scale);
}

// Simpler blurred RR test cases where all the radii are the same.
class SimpleBlurRoundRectGM : public skiagm::GM {
public:
    SimpleBlurRoundRectGM()
        : fName("simpleblurroundrect") {
    }

protected:

    SkString onShortName() override {
        return fName;
    }

    SkISize onISize() override {
        return SkISize::Make(1000, 500);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->scale(1.5f, 1.5f);
        canvas->translate(50,50);

        const float blurRadii[] = { 1,5,10,20 };
        const int cornerRadii[] = { 1,5,10,20 };
        const SkRect r = SkRect::MakeWH(SkIntToScalar(25), SkIntToScalar(25));
        for (size_t i = 0; i < SK_ARRAY_COUNT(blurRadii); ++i) {
            SkAutoCanvasRestore autoRestore(canvas, true);
            canvas->translate(0, (r.height() + SkIntToScalar(50)) * i);
            for (size_t j = 0; j < SK_ARRAY_COUNT(cornerRadii); ++j) {
                for (int k = 0; k <= 1; k++) {
                    SkPaint paint;
                    paint.setColor(SK_ColorBLACK);
                    paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle,
                                   SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(blurRadii[i]))));

                    bool useRadial = SkToBool(k);
                    if (useRadial) {
                        paint.setShader(MakeRadial());
                    }

                    SkRRect rrect;
                    rrect.setRectXY(r, SkIntToScalar(cornerRadii[j]),
                                    SkIntToScalar(cornerRadii[j]));
                    canvas->drawRRect(rrect, paint);
                    canvas->translate(r.width() + SkIntToScalar(50), 0);
                }
            }
        }
    }
private:
    const SkString  fName;

    typedef         skiagm::GM INHERITED;
};

// Create one with dimensions/rounded corners based on the skp
//
// TODO(scroggo): Disabled in an attempt to rememdy
// https://code.google.com/p/skia/issues/detail?id=1801 ('Win7 Test bots all failing GenerateGMs:
// ran wrong number of tests')
//DEF_GM(return new BlurRoundRectGM(600, 5514, 6);)

// Rounded rect with two opposite corners with large radii, the other two
// small.
DEF_GM(return new BlurRoundRectGM(100, 100);)

DEF_GM(return new SimpleBlurRoundRectGM();)
