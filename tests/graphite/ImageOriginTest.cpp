/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkRect.h"
#include "include/core/SkTileMode.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include "src/shaders/SkImageShader.h"
#include "tools/ToolUtils.h"
#include "tools/gpu/ManagedBackendTexture.h"

namespace skgpu::graphite {

namespace {

using DrawFn = void (*)(sk_sp<SkImage>, SkCanvas*, SkRect /*srcRect*/, SkRect /*dstRect*/);

constexpr SkColor4f kTopColor = SkColors::kRed;
constexpr SkColor4f kBottomColor = SkColors::kBlue;
constexpr int32_t kHalfSize = 4;
constexpr SkISize kImageSize = {2*kHalfSize, 2*kHalfSize};

void test_draw(skiatest::Reporter* reporter,
               Context* context,
               skgpu::Origin origin,
               SkRect srcRect,
               SkRect dstRect,
               DrawFn drawImageFn) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    skgpu::Protected isProtected = skgpu::Protected(context->priv().caps()->protectedSupport());

    SkBitmap bitmap;
    bitmap.allocPixels(SkImageInfo::Make(kImageSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType),
                       0);
    bitmap.eraseColor(kTopColor);
    bitmap.erase(kBottomColor,
                 SkIRect::MakeLTRB(0, kHalfSize, kImageSize.width(), kImageSize.height()));

    auto managedTexture =
            sk_gpu_test::ManagedGraphiteTexture::MakeFromPixmap(recorder.get(),
                                                                bitmap.pixmap(),
                                                                skgpu::Mipmapped::kNo,
                                                                skgpu::Renderable::kNo,
                                                                isProtected);

    REPORTER_ASSERT(reporter, managedTexture);
    if (!managedTexture) {
        return;
    }

    sk_sp<SkImage> image = SkImages::WrapTexture(recorder.get(),
                                                 managedTexture->texture(),
                                                 kRGBA_8888_SkColorType,
                                                 kPremul_SkAlphaType,
                                                 /*colorSpace=*/nullptr,
                                                 origin);

    REPORTER_ASSERT(reporter, image);
    if (!image) {
        return;
    }

    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(
            recorder.get(),
            SkImageInfo::Make(kImageSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType));

    REPORTER_ASSERT(reporter, surface);
    if (!surface) {
        return;
    }

    SkCanvas* canvas = surface->getCanvas();

    drawImageFn(image, canvas, srcRect, dstRect);

    SkPixmap pm;

    SkBitmap result;
    result.allocPixels(SkImageInfo::Make(kImageSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    bool peekPixelsSuccess = result.peekPixels(&pm);
    REPORTER_ASSERT(reporter, peekPixelsSuccess);

    bool readPixelsSuccess = surface->readPixels(pm, 0, 0);
    REPORTER_ASSERT(reporter, readPixelsSuccess);

    bool resultTopColorOnTop = origin == skgpu::Origin::kTopLeft;

    for (int32_t y = 0; y < kImageSize.height(); ++y) {
        for (int32_t x = 0; x < kImageSize.width(); ++x) {
            SkColor4f color = pm.getColor4f(x, y);

            SkColor4f expectedColor = ((y < kHalfSize) == resultTopColorOnTop) ? kTopColor
                                                                               : kBottomColor;
            REPORTER_ASSERT(reporter,
                            color == expectedColor,
                            "At position {%d, %d}, "
                            "expected {%.1f, %.1f, %.1f, %.1f}, "
                            "found {%.1f, %.1f, %.1f, %.1f}",
                            x, y,
                            expectedColor.fR, expectedColor.fG, expectedColor.fB, expectedColor.fA,
                            color.fR, color.fG, color.fB, color.fA);
        }
    }
}

const SkRect kTestSrcRects[] = {
    // entire thing
    SkRect::MakeWH(kImageSize.width(), kImageSize.height()),
    // half rect still splitting top and bottom colors
    SkRect::MakeXYWH(2, 2, kHalfSize, kHalfSize),
};

void test_draw_fn(skiatest::Reporter* reporter,
                  Context* context,
                  DrawFn drawImageFn) {
    for (auto origin : {skgpu::Origin::kTopLeft, skgpu::Origin::kBottomLeft}) {
        for (auto srcRect: kTestSrcRects) {
            test_draw(reporter,
                      context,
                      origin,
                      srcRect,
                      SkRect::MakeWH(kImageSize.width(), kImageSize.height()),
                      drawImageFn);
        }
    }
}

void draw_image(sk_sp<SkImage> image,
                SkCanvas* canvas,
                SkRect srcRect,
                SkRect dstRect) {
    canvas->drawImageRect(image,
                          srcRect,
                          dstRect,
                          SkSamplingOptions(),
                          /*paint=*/nullptr,
                          SkCanvas::kStrict_SrcRectConstraint);
}

void draw_image_with_shader(sk_sp<SkImage> image,
                            SkCanvas* canvas,
                            SkRect srcRect,
                            SkRect dstRect) {
    SkPaint p;
    SkMatrix srcToDst = SkMatrix::RectToRect(srcRect, dstRect);
    p.setShader(SkImageShader::MakeSubset(
                std::move(image),
                srcRect,
                SkTileMode::kClamp,
                SkTileMode::kClamp,
                SkSamplingOptions(),
                &srcToDst));
    canvas->drawRect(dstRect, p);
}

}  // anonymous namespace


DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(ImageOriginTest_drawImage_Graphite, reporter, context,
                                         CtsEnforcement::kNextRelease) {
    test_draw_fn(reporter, context, draw_image);
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(ImageOriginTest_imageShader_Graphite, reporter, context,
                                         CtsEnforcement::kNextRelease) {
    test_draw_fn(reporter, context, draw_image_with_shader);
}

}  // namespace skgpu::graphite
