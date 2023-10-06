/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "tools/DecodeUtils.h"
#include "tools/GpuToolUtils.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"
#include "tools/timer/TimeUtils.h"

static sk_sp<SkImage> make_image(SkCanvas* rootCanvas) {
    static constexpr SkScalar kSize = 50;
    SkImageInfo info = SkImageInfo::MakeN32Premul(kSize, kSize);
    auto surface(SkSurfaces::Raster(info));

    SkPaint p;
    p.setAntiAlias(true);
    p.setColor(SK_ColorGREEN);

    surface->getCanvas()->drawCircle(kSize / 2, kSize / 2, kSize / 2, p);

    p.setStyle(SkPaint::kStroke_Style);
    p.setColor(SK_ColorRED);
    surface->getCanvas()->drawLine(kSize * .25f, kSize * .50f, kSize * .75f, kSize * .50f, p);
    surface->getCanvas()->drawLine(kSize * .50f, kSize * .25f, kSize * .50f, kSize * .75f, p);

    sk_sp<SkImage> img = surface->makeImageSnapshot();
    return ToolUtils::MakeTextureImage(rootCanvas, std::move(img));
}

DEF_SIMPLE_GM(localmatrixshader_nested, canvas, 450, 1200) {
    auto image = make_image(canvas);
    if (!image) {
        return;
    }

    using FactoryT = sk_sp<SkShader> (*)(const sk_sp<SkImage>&,
                                         const SkMatrix& inner,
                                         const SkMatrix& outer);
    static const FactoryT gFactories[] = {
        // SkLocalMatrixShader(SkImageShader(inner), outer)
        [](const sk_sp<SkImage>& img, const SkMatrix& inner, const SkMatrix& outer) {
            return img->makeShader(SkSamplingOptions(), inner)->makeWithLocalMatrix(outer);
        },

        // SkLocalMatrixShader(SkLocalMatrixShader(SkImageShader(I), inner), outer)
        [](const sk_sp<SkImage>& img, const SkMatrix& inner, const SkMatrix& outer) {
            return img->makeShader(SkSamplingOptions())->makeWithLocalMatrix(inner)->makeWithLocalMatrix(outer);
        },

        // SkLocalMatrixShader(SkComposeShader(SkImageShader(inner)), outer)
        [](const sk_sp<SkImage>& img, const SkMatrix& inner, const SkMatrix& outer) {
            return SkShaders::Blend(SkBlendMode::kSrcOver,
                                    SkShaders::Color(SK_ColorTRANSPARENT),
                                    img->makeShader(SkSamplingOptions(), inner))
                   ->makeWithLocalMatrix(outer);
        },

        // SkLocalMatrixShader(SkComposeShader(SkLocalMatrixShader(SkImageShader(I), inner)), outer)
        [](const sk_sp<SkImage>& img, const SkMatrix& inner, const SkMatrix& outer) {
            return SkShaders::Blend(SkBlendMode::kSrcOver,
                                    SkShaders::Color(SK_ColorTRANSPARENT),
                                    img->makeShader(SkSamplingOptions())->makeWithLocalMatrix(inner))
                   ->makeWithLocalMatrix(outer);
        },
    };

    static const auto outer = SkMatrix::Scale(2, 2),
                      inner = SkMatrix::Translate(20, 20);

    SkPaint border;
    border.setAntiAlias(true);
    border.setStyle(SkPaint::kStroke_Style);

    auto rect = SkRect::Make(image->bounds());
    SkAssertResult(SkMatrix::Concat(outer, inner).mapRect(&rect));

    const auto drawColumn = [&]() {
        SkAutoCanvasRestore acr(canvas, true);
        for (const auto& f : gFactories) {
            SkPaint p;
            p.setShader(f(image, inner, outer));

            canvas->drawRect(rect, p);
            canvas->drawRect(rect, border);

            canvas->translate(0, rect.height() * 1.5f);
        }
    };

    drawColumn();

    {
        SkAutoCanvasRestore acr(canvas, true);
        canvas->translate(0, rect.height() * std::size(gFactories) * 1.5f);
        drawColumn();
    }

    canvas->translate(rect.width() * 1.5f, 0);
    canvas->scale(2, 2);
    drawColumn();
}

DEF_SIMPLE_GM(localmatrixshader_persp, canvas, 542, 266) {
    auto image = ToolUtils::GetResourceAsImage("images/yellow_rose.png");

    SkBitmap downsized;
    downsized.allocPixels(image->imageInfo().makeWH(128, 128));
    image->scalePixels(downsized.pixmap(), SkSamplingOptions(SkFilterMode::kLinear));
    image = downsized.asImage();
    SkRect imgRect = SkRect::MakeIWH(image->width(), image->height());

    // scale matrix
    SkMatrix scale = SkMatrix::Scale(1.f / 5.f, 1.f / 5.f);

    // perspective matrix
    SkPoint src[4];
    imgRect.toQuad(src);
    SkPoint dst[4] = {{0, 10.f},
                      {image->width() + 28.f, -100.f},
                      {image->width() - 28.f, image->height() + 100.f},
                      {0.f, image->height() - 10.f}};
    SkMatrix persp;
    SkAssertResult(persp.setPolyToPoly(src, dst, 4));

    // combined persp * scale
    SkMatrix perspScale = SkMatrix::Concat(persp, scale);

    auto draw = [&](sk_sp<SkShader> shader, bool applyPerspToCTM) {
        canvas->save();
        canvas->clipRect(imgRect);
        if (applyPerspToCTM) {
            canvas->concat(persp);
        }
        SkPaint imgShaderPaint;
        imgShaderPaint.setShader(std::move(shader));
        canvas->drawPaint(imgShaderPaint);
        canvas->restore();

        canvas->translate(10.f + image->width(), 0.f); // advance
    };

    // SkImageShader
    canvas->save();
    // 4 variants that all attempt to apply sample at persp * scale w/ an image shader
    // 1. scale provided to SkImage::makeShader(...) but drawn with persp
    auto s1 = image->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat,
                                SkSamplingOptions(), &scale);
    draw(s1, true);

    // 2. scale provided to SkImage::makeShader, then wrapped in persp makeWithLocalMatrix
    // These post-concat, so it ends up as persp * scale.
    auto s2 = image->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat,
                                SkSamplingOptions(), &scale)
                   ->makeWithLocalMatrix(persp);
    draw(s2, false);

    // 3. Providing pre-computed persp*scale to SkImage::makeShader()
    auto s3 = image->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat,
                                SkSamplingOptions(), &perspScale);
    draw(s3, false);

    // 4. Providing pre-computed persp*scale to makeWithLocalMatrix
    auto s4 = image->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, SkSamplingOptions())
                   ->makeWithLocalMatrix(perspScale);
    draw(s4, false);
    canvas->restore();

    canvas->translate(0.f, 10.f + image->height()); // advance to next row

    // SkGradientShader
    const SkColor kGradColors[] = { SK_ColorBLACK, SK_ColorTRANSPARENT };
    canvas->save();
    // 1. scale provided to Make, drawn with persp
    auto g1 = SkGradientShader::MakeRadial({imgRect.centerX(), imgRect.centerY()},
                                           imgRect.width() / 2.f, kGradColors, nullptr, 2,
                                           SkTileMode::kRepeat, 0, &scale);
    draw(g1, true);

    // 2. scale provided to Make, then wrapped with makeWithLocalMatrix (post-concat as before).
    auto g2 = SkGradientShader::MakeRadial({imgRect.centerX(), imgRect.centerY()},
                                           imgRect.width() / 2.f, kGradColors, nullptr, 2,
                                           SkTileMode::kRepeat, 0, &scale)
                              ->makeWithLocalMatrix(persp);
    draw(g2, false);

    // 3. Provide per-computed persp*scale to Make
    auto g3 = SkGradientShader::MakeRadial({imgRect.centerX(), imgRect.centerY()},
                                           imgRect.width() / 2.f, kGradColors, nullptr, 2,
                                           SkTileMode::kRepeat, 0, &perspScale);
    draw(g3, false);

    // 4.  Providing pre-computed persp*scale to makeWithLocalMatrix
    auto g4 = SkGradientShader::MakeRadial({imgRect.centerX(), imgRect.centerY()},
                                           imgRect.width() / 2.f, kGradColors, nullptr, 2,
                                           SkTileMode::kRepeat)
                              ->makeWithLocalMatrix(perspScale);
    draw(g4, false);
    canvas->restore();
}

namespace skiagm {
class LocalMatrixOrder : public GM {
public:
    LocalMatrixOrder() {}

protected:
    SkString getName() const override { return SkString("localmatrix_order"); }

    SkISize getISize() override { return SkISize::Make(500, 500); }

    void onOnceBeforeDraw() override {
        auto mandrill = ToolUtils::GetResourceAsImage("images/mandrill_256.png");  // 256x256
        auto example5 = ToolUtils::GetResourceAsImage("images/example_5.png");     // 128x128

        auto mshader = mandrill->makeShader(
                SkTileMode::kRepeat,
                SkTileMode::kRepeat,
                SkFilterMode::kNearest,
                SkMatrix::RotateDeg(45, {128, 128})); // rotate about center
        auto eshader = example5->makeShader(
                SkTileMode::kRepeat,
                SkTileMode::kRepeat,
                SkFilterMode::kNearest,
                SkMatrix::Scale(2, 2)); // make same size as mandrill and...
        // ... rotate about center
        eshader = eshader->makeWithLocalMatrix(SkMatrix::RotateDeg(45, {128, 128}));

        // blend the two rotated and aligned images.
        fShader = SkShaders::Blend(SkBlendMode::kModulate, mshader, eshader);
    }

    void onDraw(SkCanvas* canvas) override {
        // Rotate fShader about the canvas center
        auto center = SkRect::Make(canvas->imageInfo().bounds()).center();

        // viewer can insert a dpi scaling matrix. Make the animation always rotate about the device
        // center.
        if (auto ictm = canvas->getTotalMatrix(); ictm.invert(&ictm)) {
            center = ictm.mapPoint(center);
        }

        auto shader = fShader->makeWithLocalMatrix(SkMatrix::RotateDeg(fAngle, center));

        SkPaint paint;
        paint.setShader(shader);
        canvas->drawPaint(paint);
    }

    bool onAnimate(double nanos) override {
        fAngle = TimeUtils::NanosToSeconds(nanos) * 5.f;
        return true;
    }

    sk_sp<SkShader> fShader;
    float fAngle = 0.f;
};

DEF_GM(return new LocalMatrixOrder;)
}  // namespace skiagm
