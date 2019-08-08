/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkImage.h"
#include "SkRect.h"
#include "SkSurface.h"
#include "SkBlurImageFilter.h"
#include "GrContext.h"

namespace skiagm {

// This is an attempt to repro a CastOS memory regression when repeated blurring
// an image
class BlurReproGM : public GM {
public:
    BlurReproGM() {
        this->setBGColor(0xFFCCCCCC);
    }

protected:

    SkString onShortName() override {
        return SkString("blurrepro");
    }

    SkISize onISize() override {
        return SkISize::Make(1024, 600);
    }

    void onDraw(SkCanvas* canvas) override {
        GrContext* context = canvas->getGrContext();
        if (!context) {
            return;
        }

        sk_sp<SkImage> bigImg;

        {
            SkImageInfo srcImageII = SkImageInfo::Make(1280, 1280, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
            SkBitmap bm;
            bm.allocPixels(srcImageII);
            bm.eraseColor(SK_ColorRED);
            bm.eraseArea(SkIRect::MakeXYWH(1, 2, 1277, 1274), SK_ColorGREEN);

            sk_sp<SkImage> rasterImg = SkImage::MakeFromBitmap(bm);
            bigImg = rasterImg->makeTextureImage(context, nullptr);
        }

        sk_sp<SkImage> smImg;

        {
            SkImageInfo screenII = SkImageInfo::Make(1024, 600, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

            sk_sp<SkSurface> s = SkSurface::MakeRenderTarget(context, SkBudgeted::kYes,
                                                             screenII, 1, kTopLeft_GrSurfaceOrigin,
                                                             nullptr);
            SkCanvas* c = s->getCanvas();

            c->drawImageRect(bigImg, SkRect::MakeWH(1024, 600), nullptr);

            smImg = s->makeImageSnapshot();
        }

        context->flush();
        SkDebugf("Post Flush --------------------------------------------------------------------\n");

        const SkIRect subset = SkIRect::MakeWH(1024, 600);

        for (int i = 0; i < 30; ++i) {
#if 0
            sk_sp<SkImageFilter> blur = SkImageFilters::Blur(20, 20, nullptr);
#else
            sk_sp<SkImageFilter> blur = SkBlurImageFilter::Make(20, 20, nullptr);
#endif

            SkIRect outSubset;
            SkIPoint offset;
            sk_sp<SkImage> filteredImg = smImg->makeWithFilter(context, blur.get(), subset, subset,
                                                               &outSubset, &offset);

            SkRect dstRect = SkRect::MakeXYWH(offset.fX, offset.fY,
                                              outSubset.width(), outSubset.height());
            canvas->drawImageRect(filteredImg, outSubset, dstRect, nullptr);

            context->flush();
        }

        bigImg = nullptr;
        smImg = nullptr;
        SkDebugf("Exiting -----------------------------------------------------------------------\n");
        context->purgeUnlockedResources(false);
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////
DEF_GM(return new BlurReproGM;)
}
