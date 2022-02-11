/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/effects/SkGradientShader.h"
#include "tools/Resources.h"

namespace {

sk_sp<SkShader> create_gradient_shader(SkRect r) {
    // TODO: it seems like only the x-component of sk_FragCoord is making it to the shader!
    SkPoint pts[2] = { {r.fLeft, r.fTop}, {r.fRight, r.fTop} };
    SkColor colors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE };
    float offsets[] = { 0.0f, 0.75f, 1.0f };

    return SkGradientShader::MakeLinear(pts, colors, offsets, SK_ARRAY_COUNT(colors),
                                        SkTileMode::kClamp);
}

sk_sp<SkShader> create_image_shader() {
    SkImageInfo ii = SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    SkBitmap bitmap;

    bitmap.allocPixels(ii);
    bitmap.eraseColor(SK_ColorWHITE);

    SkCanvas canvas(bitmap);

    SkPaint redPaint;
    redPaint.setColor(SK_ColorRED);
    canvas.drawCircle(50, 50, 50, redPaint);

    bitmap.setAlphaType(kOpaque_SkAlphaType);
    bitmap.setImmutable();

    sk_sp<SkImage> img = SkImage::MakeFromBitmap(bitmap);
    // TODO: we'll need a 'makeTextureImage' call here

    return img->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, SkSamplingOptions());
}

sk_sp<SkShader> create_blend_shader(SkBlendMode bm) {
    constexpr SkColor4f kTransYellow = {1.0f, 1.0f, 0.0f, 0.5f};

    sk_sp<SkShader> solid = SkShaders::Color(kTransYellow, nullptr);
    return SkShaders::Blend(bm, std::move(solid), create_image_shader());
}

} // anonymous namespace

namespace skiagm {

// This is just for bootstrapping Graphite.
class GraphiteStartGM : public GM {
public:
    GraphiteStartGM() {
        this->setBGColor(0xFFCCCCCC);
        GetResourceAsBitmap("images/color_wheel.gif", &fBitmap);
    }

protected:
    SkString onShortName() override {
        return SkString("graphitestart");
    }

    SkISize onISize() override {
        return SkISize::Make(256, 384);
    }

    void onDraw(SkCanvas* canvas) override {

        // UL corner
        {
            SkPaint p;
            p.setColor(SK_ColorRED);
            canvas->drawRect({2, 2, 127, 127}, p);
        }

        // UR corner
        {
            SkRect r{129, 2, 255, 127};
            SkPaint p;
            p.setShader(create_gradient_shader(r));
            canvas->drawRect(r, p);
        }

        // LL corner
        {
            SkPaint p;
            p.setShader(create_image_shader());
            canvas->drawRect({2, 129, 127, 255}, p);
        }

        // LR corner
        {
            SkPaint p;
            p.setShader(create_blend_shader(SkBlendMode::kDstOver));
            canvas->drawRect({129, 129, 255, 255}, p);
        }
#ifdef SK_GRAPHITE_ENABLED
        // TODO: failing serialize test on Linux, not sure what's going on
        canvas->writePixels(fBitmap, 0, 256);
#endif
    }

    SkBitmap fBitmap;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new GraphiteStartGM;)

}  // namespace skiagm
