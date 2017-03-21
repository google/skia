/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "Resources.h"

#include "SkBlurImageFilter.h"
#include "SkImageSource.h"
#include "SkSurface.h"

// This GM attempts to replicate CC's usage of SkImage::makeWithFilter
class MakeWithFilterGM : public skiagm::GM {
public:
    MakeWithFilterGM() {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFCCCCCC));
    }

protected:

    SkString onShortName() override {
        return SkString("makewithfilter");
    }

    SkISize onISize() override {
        return SkISize::Make(300, 300);
    }

    void onOnceBeforeDraw() override {
    }

    void onDraw(SkCanvas* canvas) override {
        SkImageInfo info = SkImageInfo::MakeN32(250, 250, kOpaque_SkAlphaType);
        sk_sp<SkSurface> surface = canvas->makeSurface(info);
        if (!surface) {
            surface = SkSurface::MakeRaster(info);
        }
        sk_tool_utils::draw_checkerboard(surface->getCanvas());
        sk_sp<SkImage> source = surface->makeImageSnapshot();

        sk_sp<SkImageFilter> blur(SkBlurImageFilter::Make(2.0f, 2.0f, nullptr));

        SkIRect srcRect = SkIRect::MakeWH(200, 200);
        SkIRect clip = srcRect;

        SkIRect outSubset;
        SkIPoint outOffset;
        sk_sp<SkImage> result = source->makeWithFilter(blur.get(), srcRect, clip,
                                                       &outSubset, &outOffset);

        SkDebugf("****************************************************************\n");
        SkDebugf("src %x %d %d\n", source, source->width(), source->height());
        SkDebugf("srcRect %d %d %d %d\n",
                 srcRect.fLeft, srcRect.fTop, srcRect.fRight, srcRect.fBottom);
        SkDebugf("clip: %d %d %d %d\n", clip.fLeft, clip.fTop, clip.fRight, clip.fBottom);
        SkDebugf("result: %x %d %d\n", result, result->width(), result->height());
        SkDebugf("out_subset: %d %d %d %d\n",
                 outSubset.fLeft, outSubset.fTop, outSubset.fRight, outSubset.fBottom);
        SkDebugf("offset: %d %d\n",
                 outOffset.fX, outOffset.fY);

        SkRect dstRect = SkRect::MakeXYWH(srcRect.fLeft + outOffset.fX,
                                          srcRect.fTop + outOffset.fY,
                                          outSubset.width(), outSubset.height());

        SkPaint paint;
        canvas->drawImageRect(result.get(), srcRect, dstRect, &paint, SkCanvas::kFast_SrcRectConstraint);
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new MakeWithFilterGM;)
