/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/codec/SkEncodedOrigin.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkYUVAInfo.h"
#include "include/core/SkYUVAPixmaps.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include "src/shaders/SkImageShader.h"
#include "tools/ToolUtils.h"

#include <algorithm>

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
    REPORTER_ASSERT(reporter, recorder);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(
            recorder.get(),
            SkImageInfo::Make(canvasSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    SkCanvas* canvas = surface->getCanvas();

    SkBitmap bitmap;
    bitmap.allocPixels(SkImageInfo::Make(imageSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType),
                       0);
    bitmap.eraseColor(kRectColor);
    bitmap.setImmutable();
    sk_sp<SkImage> image = SkImages::TextureFromImage(recorder.get(), bitmap.asImage(),
                                                      {/*fMipmapped=*/false});

    SkPaint p;
    SkMatrix srcToDst = SkMatrix::RectToRectOrIdentity(srcRect, dstRect);
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
                                         CtsEnforcement::kApiLevel_202404) {
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

#if defined(SK_ENABLE_YUVA_PACKED_422)
static bool supports_packed_yuyv(Recorder* recorder) {
    const Caps* caps = recorder->priv().caps();
    TextureInfo info = caps->getDefaultSampledTextureInfo(kR16G16B16A16_unorm_SkColorType,
                                                          Mipmapped::kNo,
                                                          Protected::kNo,
                                                          Renderable::kNo);
    return info.isValid() && caps->isTexturable(info);
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(ImageShaderPackedYUYVDecalTest, reporter, context,
                                         CtsEnforcement::kNever) {
    // Test that decal tiling on a packed 4:2:2 image is resolved per pixel, not per pixel pair,
    // when the subset bounds fall on odd pixels. The plane holds one texel per 2x1 block of pixels,
    // so pixels 2 and 3 (and 6 and 7) share a texel even though the subset splits them.
    constexpr int kWidth = 8;
    constexpr int kHeight = 2;

    // Channels 0..3 hold (Y0, U, Y1, V). Even pixels are white (Y0 = 1) and odd pixels are black
    // (Y1 = 0); U and V are neutral.
    SkBitmap plane;
    plane.allocPixels(SkImageInfo::Make(kWidth / 2,
                                        kHeight,
                                        kR16G16B16A16_unorm_SkColorType,
                                        kUnpremul_SkAlphaType));
    for (int y = 0; y < kHeight; ++y) {
        for (int x = 0; x < kWidth / 2; ++x) {
            uint16_t* texel = static_cast<uint16_t*>(plane.getAddr(x, y));
            texel[0] = 0xFFFF;
            texel[1] = 0x8000;
            texel[2] = 0x0000;
            texel[3] = 0x8000;
        }
    }
    plane.setImmutable();

    SkYUVAInfo yuvaInfo({kWidth, kHeight},
                        SkYUVAInfo::PlaneConfig::kYUYV,
                        SkYUVAInfo::Subsampling::k422,
                        kJPEG_Full_SkYUVColorSpace);
    SkPixmap pixmaps[SkYUVAInfo::kMaxPlanes];
    pixmaps[0] = plane.pixmap();
    SkYUVAPixmaps yuvaPixmaps = SkYUVAPixmaps::FromExternalPixmaps(yuvaInfo, pixmaps);
    REPORTER_ASSERT(reporter, yuvaPixmaps.isValid());

    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    REPORTER_ASSERT(reporter, recorder);
    if (!recorder || !supports_packed_yuyv(recorder.get())) {
        return;
    }
    sk_sp<SkImage> image = SkImages::TextureFromYUVAPixmaps(recorder.get(), yuvaPixmaps);
    REPORTER_ASSERT(reporter, image);
    if (!image) {
        return;
    }

    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(
            recorder.get(),
            SkImageInfo::Make(kWidth, kHeight, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    REPORTER_ASSERT(reporter, surface);
    if (!surface) {
        return;
    }
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorTRANSPARENT);

    SkPaint p;
    p.setShader(SkImageShader::MakeSubset(std::move(image),
                                          SkRect::MakeLTRB(3, 0, 7, kHeight),
                                          SkTileMode::kDecal,
                                          SkTileMode::kDecal,
                                          SkSamplingOptions(),
                                          /*localMatrix=*/nullptr));
    canvas->drawRect(SkRect::MakeWH(kWidth, kHeight), p);

    SkBitmap result;
    result.allocPixels(SkImageInfo::Make(kWidth, kHeight, kRGBA_8888_SkColorType,
                                         kPremul_SkAlphaType));
    SkPixmap pm;
    REPORTER_ASSERT(reporter, result.peekPixels(&pm));
    REPORTER_ASSERT(reporter, surface->readPixels(pm, 0, 0));

    // Pixels 3..6 are inside the subset: odd pixels take Y1 (black), even pixels take Y0 (white).
    // Pixels 2 and 7 are outside of it and must be transparent, even though they share a texel with
    // pixels inside the subset.
    struct Expectation {
        int x;
        float alpha;
        float luma;
    };
    const Expectation kExpectations[] = {{2, 0.f, 0.f},
                                         {3, 1.f, 0.f},
                                         {4, 1.f, 1.f},
                                         {5, 1.f, 0.f},
                                         {6, 1.f, 1.f},
                                         {7, 0.f, 0.f}};
    for (int y = 0; y < kHeight; ++y) {
        for (const Expectation& e : kExpectations) {
            SkColor4f c = pm.getColor4f(e.x, y);
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(c.fA, e.alpha, 0.02f) &&
                            SkScalarNearlyEqual(c.fR, e.luma * e.alpha, 0.02f),
                            "At position {%d, %d}, expected alpha %.1f and luma %.1f, "
                            "found {%.2f, %.2f, %.2f, %.2f}",
                            e.x, y, e.alpha, e.luma, c.fR, c.fG, c.fB, c.fA);
        }
    }
}
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(ImageShaderPackedYUYVTileOriginTest, reporter, context,
                                         CtsEnforcement::kNever) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    REPORTER_ASSERT(reporter, recorder);
    if (!recorder || !supports_packed_yuyv(recorder.get())) {
        return;
    }

    // Distinct rows, pairs, and luma lanes detect wrong coordinates/parity. Non-neutral, unequal
    // U and V detect swapped chroma channels, which a black-and-white pattern cannot catch.
    constexpr int kPlaneWidth = 4;
    constexpr int kPlaneHeight = 6;
    SkBitmap plane;
    plane.allocPixels(SkImageInfo::Make(kPlaneWidth, kPlaneHeight,
                                        kR16G16B16A16_unorm_SkColorType, kUnpremul_SkAlphaType));
    for (int y = 0; y < kPlaneHeight; ++y) {
        for (int x = 0; x < kPlaneWidth; ++x) {
            auto* t = static_cast<uint16_t*>(plane.getAddr(x, y));
            t[0] = 0x3000 + x * 0x1600 + y * 0x0700;
            t[1] = 0x5000 + x * 0x1000 + y * 0x0300;
            t[2] = 0xB000 - x * 0x1000 - y * 0x0500;
            t[3] = 0xB000 - x * 0x0900 - y * 0x0600;
        }
    }
    plane.setImmutable();

    constexpr SkTileMode kTileModes[] = {SkTileMode::kClamp, SkTileMode::kRepeat,
                                         SkTileMode::kMirror, SkTileMode::kDecal};
    for (int o = kTopLeft_SkEncodedOrigin; o <= kLast_SkEncodedOrigin; ++o) {
        auto origin = static_cast<SkEncodedOrigin>(o);
        SkISize size = SkEncodedOriginSwapsWidthHeight(origin)
                              ? SkISize::Make(kPlaneHeight, 2 * kPlaneWidth)
                              : SkISize::Make(2 * kPlaneWidth, kPlaneHeight);
        SkYUVAInfo info(size, SkYUVAInfo::PlaneConfig::kYUYV,
                        SkYUVAInfo::Subsampling::k422, kJPEG_Full_SkYUVColorSpace, origin);
        SkPixmap planes[SkYUVAInfo::kMaxPlanes];
        planes[0] = plane.pixmap();
        auto pixmaps = SkYUVAPixmaps::FromExternalPixmaps(info, planes);
        REPORTER_ASSERT(reporter, pixmaps.isValid());
        if (!pixmaps.isValid()) {
            return;
        }
        auto image = SkImages::TextureFromYUVAPixmaps(recorder.get(), pixmaps);
        REPORTER_ASSERT(reporter, image, "origin %d: image creation failed", o);
        if (!image) {
            return;
        }

        // Decode JPEG full-range YUV into an independently sampled CPU RGBA reference, already
        // in display orientation. Raster tiling must agree with the packed Graphite shader.
        SkBitmap reference;
        reference.allocPixels(SkImageInfo::Make(size, kRGBA_8888_SkColorType,
                                                kPremul_SkAlphaType));
        const SkMatrix toPlane = info.inverseOriginMatrix();
        for (int y = 0; y < size.height(); ++y) {
            for (int x = 0; x < size.width(); ++x) {
                SkPoint p = toPlane.mapPoint({x + 0.5f, y + 0.5f});
                int px = static_cast<int>(p.fX);
                int py = static_cast<int>(p.fY);
                const auto* t = static_cast<const uint16_t*>(plane.getAddr(px / 2, py));
                float luma = t[(px & 1) ? 2 : 0] / 65535.f;
                float u = t[1] / 65535.f - 0.5f;
                float v = t[3] / 65535.f - 0.5f;
                SkColor4f rgb = {std::clamp(luma + 1.402f * v, 0.f, 1.f),
                                 std::clamp(luma - 0.344136f * u - 0.714136f * v, 0.f, 1.f),
                                 std::clamp(luma + 1.772f * u, 0.f, 1.f), 1.f};
                reference.erase(rgb, SkIRect::MakeXYWH(x, y, 1, 1));
            }
        }
        reference.setImmutable();

        // Padding samples negative coordinates and multiple repeat/mirror periods on both axes.
        constexpr int kPad = 9;
        SkImageInfo dstInfo = SkImageInfo::Make(size.width() + 2 * kPad,
                                                size.height() + 2 * kPad,
                                                kRGBA_8888_SkColorType, kPremul_SkAlphaType);
        auto surface = SkSurfaces::RenderTarget(recorder.get(), dstInfo);
        auto referenceSurface = SkSurfaces::Raster(dstInfo);
        REPORTER_ASSERT(reporter, surface && referenceSurface);
        if (!surface || !referenceSurface) {
            return;
        }
        SkBitmap actual, expected;
        actual.allocPixels(dstInfo);
        expected.allocPixels(dstInfo);
        const SkRect subsets[] = {
                SkRect::Make(size),
                // Odd boundaries split pairs; odd width changes parity on successive repeats.
                SkRect::MakeLTRB(1, 1, size.width() - 1, size.height() - 1),
                SkRect::MakeLTRB(1, 1, size.width() - 2, size.height() - 2)};
        const SkMatrix localMatrix = SkMatrix::Translate(kPad, kPad);
        for (const SkRect& subset : subsets) {
            // Raster image shaders do not support MakeSubset. Crop the reference bitmap instead
            // and translate its new origin to the subset's position in the original image.
            SkBitmap croppedReference;
            bool extracted = reference.extractSubset(&croppedReference, subset.roundOut());
            REPORTER_ASSERT(reporter, extracted);
            if (!extracted) {
                return;
            }
            auto referenceImage = croppedReference.asImage();
            REPORTER_ASSERT(reporter, referenceImage);
            if (!referenceImage) {
                return;
            }
            const SkMatrix referenceMatrix = SkMatrix::Translate(kPad + subset.left(),
                                                                  kPad + subset.top());
            for (SkTileMode tileX : kTileModes) {
                for (SkTileMode tileY : kTileModes) {
                    auto shader = SkImageShader::MakeSubset(image, subset, tileX, tileY,
                                                             SkSamplingOptions(), &localMatrix);
                    auto referenceShader = referenceImage->makeShader(
                            tileX, tileY, SkSamplingOptions(), &referenceMatrix);
                    REPORTER_ASSERT(reporter, shader && referenceShader);
                    if (!shader || !referenceShader) {
                        return;
                    }
                    auto draw = [](SkSurface* dst, sk_sp<SkShader> shader) {
                        SkPaint paint;
                        paint.setShader(std::move(shader));
                        dst->getCanvas()->clear(SK_ColorTRANSPARENT);
                        dst->getCanvas()->drawPaint(paint);
                    };
                    draw(surface.get(), std::move(shader));
                    draw(referenceSurface.get(), std::move(referenceShader));
                    bool readActual = surface->readPixels(actual.pixmap(), 0, 0);
                    bool readExpected = referenceSurface->readPixels(expected.pixmap(), 0, 0);
                    REPORTER_ASSERT(reporter, readActual && readExpected);
                    if (!readActual || !readExpected) {
                        return;
                    }
                    for (int y = 0; y < dstInfo.height(); ++y) {
                        for (int x = 0; x < dstInfo.width(); ++x) {
                            SkColor4f a = actual.getColor4f(x, y);
                            SkColor4f e = expected.getColor4f(x, y);
                            bool matches = SkScalarNearlyEqual(a.fR, e.fR, 0.015f) &&
                                           SkScalarNearlyEqual(a.fG, e.fG, 0.015f) &&
                                           SkScalarNearlyEqual(a.fB, e.fB, 0.015f) &&
                                           SkScalarNearlyEqual(a.fA, e.fA, 0.015f);
                            REPORTER_ASSERT(reporter, matches,
                                            "origin %d tile (%d,%d) subset [%.0f,%.0f,%.0f,%.0f] "
                                            "pixel (%d,%d): expected {%g,%g,%g,%g}, "
                                            "found {%g,%g,%g,%g}",
                                            o, static_cast<int>(tileX), static_cast<int>(tileY),
                                            subset.left(), subset.top(), subset.right(),
                                            subset.bottom(), x, y,
                                            e.fR, e.fG, e.fB, e.fA, a.fR, a.fG, a.fB, a.fA);
                            if (!matches) {
                                return;
                            }
                        }
                    }
                }
            }
        }
    }
    INFOF(reporter, "Verified 384 packed YUYV origin/subset/tile combinations.\n");
}
#endif  // SK_ENABLE_YUVA_PACKED_422

}  // namespace skgpu::graphite
