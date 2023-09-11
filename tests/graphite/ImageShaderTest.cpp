/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkTileMode.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include "src/shaders/SkImageShader.h"
#include "tools/ToolUtils.h"

namespace skgpu::graphite {

namespace {

constexpr SkColor4f kRectColor = SkColors::kRed;
constexpr SkColor4f kBgColor = SkColors::kTransparent;

struct Expectation {
    SkPoint pos;
    SkColor4f color;
};

void test_draw(skiatest::Reporter* reporter,
               Context* context,
               SkISize canvasSize,
               SkISize imageSize,
               SkRect srcRect,
               SkRect dstRect,
               SkTileMode tileMode,
               SkSamplingOptions samplingOptions,
               std::vector<Expectation> expectations) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(
            recorder.get(),
            SkImageInfo::Make(canvasSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    SkCanvas* canvas = surface->getCanvas();

    SkBitmap bitmap;
    bitmap.allocPixels(SkImageInfo::Make(imageSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType),
                       0);
    bitmap.eraseColor(kRectColor);
    bitmap.setImmutable();
    sk_sp<SkImage> image = ToolUtils::MakeTextureImage(canvas, bitmap.asImage());

    SkPaint p;
    SkMatrix srcToDst = SkMatrix::RectToRect(srcRect, dstRect);
    p.setShader(SkImageShader::MakeSubset(
            std::move(image), srcRect, tileMode, tileMode, samplingOptions, &srcToDst));
    canvas->drawRect(dstRect, p);

    SkPixmap pm;

    SkBitmap result;
    result.allocPixels(SkImageInfo::Make(canvasSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    bool peekPixelsSuccess = result.peekPixels(&pm);
    REPORTER_ASSERT(reporter, peekPixelsSuccess);

    bool readPixelsSuccess = surface->readPixels(pm, 0, 0);
    REPORTER_ASSERT(reporter, readPixelsSuccess);

    for (const Expectation& e : expectations) {
        SkColor4f a = e.color;
        SkColor4f b = pm.getColor4f(e.pos.fX, e.pos.fY);
        REPORTER_ASSERT(reporter,
                        a == b,
                        "At position {%.1f, %.1f}, "
                        "expected {%.1f, %.1f, %.1f, %.1f}, "
                        "found {%.1f, %.1f, %.1f, %.1f}",
                        e.pos.fX, e.pos.fY,
                        a.fR, a.fG, a.fB, a.fA,
                        b.fR, b.fG, b.fB, b.fA);
    }
}

}  // anonymous namespace

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(ImageShaderTest, reporter, context,
                                         CtsEnforcement::kNextRelease) {
    // Test that a subset bound covering less than half of a pixel causes that pixel not to be
    // drawn when using decal tiling and nearest-neighbor filtering. In this case we have a subset
    // that covers 3/4 the pixel column at y=1, all of the y=2 column, and 1/4 the y=3 column.
    test_draw(reporter,
              context,
              /*canvasSize=*/SkISize::Make(100, 100),
              /*imageSize=*/SkISize::Make(4, 4),
              /*srcRect=*/SkRect::MakeLTRB(1.25, 0.0f, 3.25f, 2.0f),
              /*dstRect=*/SkRect::MakeLTRB(0, 0, 80, 80),
              SkTileMode::kDecal,
              SkSamplingOptions(),

              // Pixel that should sample the image at y=1, since that's where the subset starts.
              {{{0, 40}, kRectColor},
               // Pixel that would sample the image at y=3, but the subset bound at y=3.25 prevents
               // us from sampling the image.
               {{75, 40}, kBgColor}});
}

}  // namespace skgpu::graphite
