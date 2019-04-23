/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkImage.h"
#include "include/core/SkPath.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkGradientShader.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

static sk_sp<SkImage> make_image1() { return GetResourceAsImage("images/mandrill_128.png"); }

static sk_sp<SkImage> make_image2() {
    return GetResourceAsImage("images/brickwork-texture.jpg")->makeSubset({0, 0, 128, 128});
}

namespace skiagm {

class PerspImages : public GM {
public:
    PerspImages() = default;

protected:
    SkString onShortName() override { return SkString("persp_images"); }

    SkISize onISize() override { return SkISize::Make(1150, 1280); }

    void onOnceBeforeDraw() override {
        fImages.push_back(make_image1());
        fImages.push_back(make_image2());
    }

    void onDraw(SkCanvas* canvas) override {
        SkTDArray<SkMatrix> matrices;
        matrices.push()->setAll(1.f, 0.f,    0.f,
                                0.f, 1.f,    0.f,
                                0.f, 0.005f, 1.f);
        matrices.push()->setAll(1.f,     0.f,    0.f,
                                0.f,     1.f,    0.f,
                                0.007f, -0.005f, 1.f);
        matrices[1].preSkew(0.2f, -0.1f);
        matrices[1].preRotate(-65.f);
        matrices[1].preScale(1.2f, .8f);
        matrices[1].postTranslate(0.f, 60.f);
        SkPaint paint;
        int n = 0;
        SkRect bounds = SkRect::MakeEmpty();
        for (const auto& img : fImages) {
            SkRect imgB = SkRect::MakeWH(img->width(), img->height());
            for (const auto& m : matrices) {
                SkRect temp;
                m.mapRect(&temp, imgB);
                bounds.join(temp);
            }
        }
        canvas->translate(-bounds.fLeft + 10.f, -bounds.fTop + 10.f);
        canvas->save();
        enum class DrawType {
            kDrawImage,
            kDrawImageRectStrict,
            kDrawImageRectFast,
        };
        for (auto type :
             {DrawType::kDrawImage, DrawType::kDrawImageRectStrict, DrawType::kDrawImageRectFast}) {
            for (const auto& m : matrices) {
                for (auto aa : {false, true}) {
                    paint.setAntiAlias(aa);
                    for (auto filter : {kNone_SkFilterQuality, kLow_SkFilterQuality,
                                        kMedium_SkFilterQuality, kHigh_SkFilterQuality}) {
                        for (const auto& img : fImages) {
                            paint.setFilterQuality(filter);
                            canvas->save();
                            canvas->concat(m);
                            SkRect src = {img->width() / 4.f, img->height() / 4.f,
                                          3.f * img->width() / 4.f, 3.f * img->height() / 4};
                            SkRect dst = {0, 0,
                                          3.f / 4.f * img->width(), 3.f / 4.f * img->height()};
                            switch (type) {
                                case DrawType::kDrawImage:
                                    canvas->drawImage(img, 0, 0, &paint);
                                    break;
                                case DrawType::kDrawImageRectStrict:
                                    canvas->drawImageRect(img, src, dst, &paint,
                                                          SkCanvas::kStrict_SrcRectConstraint);
                                    break;
                                case DrawType::kDrawImageRectFast:
                                    canvas->drawImageRect(img, src, dst, &paint,
                                                          SkCanvas::kFast_SrcRectConstraint);
                                    break;
                            }
                            canvas->restore();
                            ++n;
                            if (n < 8) {
                                canvas->translate(bounds.width() + 10.f, 0);
                            } else {
                                canvas->restore();
                                canvas->translate(0, bounds.height() + 10.f);
                                canvas->save();
                                n = 0;
                            }
                        }
                    }
                }
            }
        }
        canvas->restore();
    }

private:
    static constexpr int kNumImages = 4;
    SkTArray<sk_sp<SkImage>> fImages;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new PerspImages();)

}  // namespace skiagm
