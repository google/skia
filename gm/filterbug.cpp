/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"

static sk_sp<SkImage> make_image(int firstBlackRow, int lastBlackRow) {
    static const int kWidth = 25;
    static const int kHeight = 27;

    SkBitmap bm;
    bm.allocN32Pixels(kWidth, kHeight);
    bm.eraseColor(SK_ColorWHITE);
    for (int y = firstBlackRow; y < lastBlackRow; ++y) {
        for (int x = 0; x < kWidth; ++x) {
            *bm.getAddr32(x, y) = SkPackARGB32(0xFF, 0x0, 0x0, 0x0);
        }
    }

    bm.setAlphaType(SkAlphaType::kOpaque_SkAlphaType);
    bm.setImmutable();

    return bm.asImage();
}

// GM to reproduce crbug.com/673261.
class FilterBugGM : public skiagm::GM {
public:
    FilterBugGM() { this->setBGColor(SK_ColorRED); }

protected:
    SkString onShortName() override { return SkString("filterbug"); }

    SkISize onISize() override { return SkISize::Make(150, 150); }

    void onOnceBeforeDraw() override {
        // The top texture has 5 black rows on top and then 22 white rows on the bottom
        fTop = make_image(0, 5);
        // The bottom texture has 5 black rows on the bottom and then 22 white rows on the top
        fBot = make_image(22, 27);
    }

    void onDraw(SkCanvas* canvas) override {
        static const SkSamplingOptions kSampling(SkCubicResampler::Mitchell());
        static const bool kDoAA = true;

        {
            SkRect r1 = SkRect::MakeXYWH(50.0f, 0.0f, 50.0f, 50.0f);
            SkPaint p1;
            p1.setAntiAlias(kDoAA);
            SkMatrix localMat;
            localMat.setScaleTranslate(2.0f, 2.0f, 50.0f, 0.0f);
            p1.setShader(fTop->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat,
                                          kSampling, &localMat));

            canvas->drawRect(r1, p1);
        }

        {
            SkRect r2 = SkRect::MakeXYWH(50.0f, 50.0f, 50.0f, 36.0f);

            SkPaint p2;
            p2.setColor(SK_ColorWHITE);
            p2.setAntiAlias(kDoAA);

            canvas->drawRect(r2, p2);
        }

        {
            SkRect r3 = SkRect::MakeXYWH(50.0f, 86.0f, 50.0f, 50.0f);

            SkPaint p3;
            p3.setAntiAlias(kDoAA);
            SkMatrix localMat;
            localMat.setScaleTranslate(2.0f, 2.0f, 50.0f, 86.0f);
            p3.setShader(fBot->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat,
                                          kSampling, &localMat));

            canvas->drawRect(r3, p3);
        }
    }

private:
    sk_sp<SkImage> fTop;
    sk_sp<SkImage> fBot;

    using INHERITED = skiagm::GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new FilterBugGM;)
