/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkFont.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypeface.h"
#include "src/core/SkBlurMask.h"
#include "src/effects/SkEmbossMaskFilter.h"
#include "tools/fonts/FontToolUtils.h"

static sk_sp<SkImage> make_bm() {
    auto surf = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(100, 100));

    SkPaint paint;
    paint.setAntiAlias(true);
    surf->getCanvas()->drawCircle(50, 50, 50, paint);
    return surf->makeImageSnapshot();
}

class EmbossGM : public skiagm::GM {
public:
    EmbossGM() {
    }

protected:
    SkString getName() const override { return SkString("emboss"); }

    SkISize getISize() override { return SkISize::Make(600, 120); }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        auto img = make_bm();
        canvas->drawImage(img, 10, 10);
        canvas->translate(img->width() + SkIntToScalar(10), 0);

        paint.setMaskFilter(SkEmbossMaskFilter::Make(
            SkBlurMask::ConvertRadiusToSigma(3),
            { { SK_Scalar1, SK_Scalar1, SK_Scalar1 }, 0, 128, 16*2 }));
        canvas->drawImage(img, 10, 10, SkSamplingOptions(), &paint);
        canvas->translate(img->width() + SkIntToScalar(10), 0);

        // this combination of emboss+colorfilter used to crash -- so we exercise it to
        // confirm that we have a fix.
        paint.setColorFilter(SkColorFilters::Blend(0xFFFF0000, SkBlendMode::kSrcATop));
        canvas->drawImage(img, 10, 10, SkSamplingOptions(), &paint);
        canvas->translate(img->width() + SkIntToScalar(10), 0);

        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(SkIntToScalar(10));
        paint.setMaskFilter(SkEmbossMaskFilter::Make(
            SkBlurMask::ConvertRadiusToSigma(4),
            { { SK_Scalar1, SK_Scalar1, SK_Scalar1 }, 0, 128, 16*2 }));
        paint.setColorFilter(nullptr);
        paint.setShader(SkShaders::Color(SK_ColorBLUE));
        paint.setDither(true);
        canvas->drawCircle(SkIntToScalar(50), SkIntToScalar(50),
                           SkIntToScalar(30), paint);
        canvas->translate(SkIntToScalar(100), 0);

        SkFont font = SkFont(ToolUtils::DefaultPortableTypeface(), 50);
        paint.setStyle(SkPaint::kFill_Style);
        canvas->drawString("Hello", 0, 50, font, paint);

        paint.setShader(nullptr);
        paint.setColor(SK_ColorGREEN);
        canvas->drawString("World", 0, 100, font, paint);
    }

private:
    using INHERITED = skiagm::GM;
};

DEF_GM(return new EmbossGM;)
