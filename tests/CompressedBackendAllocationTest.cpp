/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrContext.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkCompressedDataUtils.h"
#include "src/core/SkMipMap.h"
#include "src/gpu/GrContextPriv.h"
#include "src/image/SkImage_Base.h"
#include "tests/Test.h"
#include "tests/TestUtils.h"
#include "tools/ToolUtils.h"

// Just verify that 'actual' is entirely 'expected'
static void check_solid_pixmap(skiatest::Reporter* reporter,
                               const SkColor4f& expected, const SkPixmap& actual,
                               const char* label0, const char* label1, const char* label2) {
    const float tols[4] = { 0.01f, 0.01f, 0.01f, 0.01f };

    auto error = std::function<ComparePixmapsErrorReporter>(
        [reporter, label0, label1, label2](int x, int y, const float diffs[4]) {
            SkASSERT(x >= 0 && y >= 0);
            ERRORF(reporter, "%s %s %s - mismatch at %d, %d (%f, %f, %f %f)",
                   label0, label1, label2, x, y,
                   diffs[0], diffs[1], diffs[2], diffs[3]);
        });

    CheckSolidPixels(expected, actual, tols, error);
}

// Create an SkImage to wrap 'backendTex'
sk_sp<SkImage> create_image(GrContext* context, const GrBackendTexture& backendTex) {
    const GrCaps* caps = context->priv().caps();

    SkImage::CompressionType compression = caps->compressionType(backendTex.getBackendFormat());

    SkAlphaType at = SkCompressionTypeIsOpaque(compression) ? kOpaque_SkAlphaType
                                                            : kPremul_SkAlphaType;

    return SkImage::MakeFromCompressedTexture(context,
                                              backendTex,
                                              kTopLeft_GrSurfaceOrigin,
                                              at,
                                              nullptr);
}

// Draw the compressed backend texture (wrapped in an SkImage) into an RGBA surface, attempting
// to access all the mipMap levels.
static void check_compressed_mipmaps(GrContext* context, sk_sp<SkImage> img,
                                     SkImage::CompressionType compressionType,
                                     const SkColor4f expectedColors[6],
                                     GrMipMapped mipMapped,
                                     skiatest::Reporter* reporter, const char* label) {

    SkImageInfo readbackSurfaceII = SkImageInfo::Make(32, 32, kRGBA_8888_SkColorType,
                                                      kPremul_SkAlphaType);

    sk_sp<SkSurface> surf = SkSurface::MakeRenderTarget(context,
                                                        SkBudgeted::kNo,
                                                        readbackSurfaceII, 1,
                                                        kTopLeft_GrSurfaceOrigin,
                                                        nullptr);
    if (!surf) {
        return;
    }

    SkCanvas* canvas = surf->getCanvas();

    SkPaint p;
    p.setFilterQuality(kHigh_SkFilterQuality); // to force mipMapping
    p.setBlendMode(SkBlendMode::kSrc);

    int numMipLevels = 1;
    if (mipMapped == GrMipMapped::kYes) {
        numMipLevels = SkMipMap::ComputeLevelCount(32, 32)+1;
    }

    for (int i = 0, rectSize = 32; i < numMipLevels; ++i, rectSize /= 2) {
        SkASSERT(rectSize >= 1);

        canvas->clear(SK_ColorTRANSPARENT);

        SkRect r = SkRect::MakeWH(rectSize, rectSize);
        canvas->drawImageRect(img, r, &p);

        SkImageInfo readbackII = SkImageInfo::Make(rectSize, rectSize,
                                                   kRGBA_8888_SkColorType,
                                                   kUnpremul_SkAlphaType);
        SkAutoPixmapStorage actual2;
        SkAssertResult(actual2.tryAlloc(readbackII));
        actual2.erase(SkColors::kTransparent);

        bool result = surf->readPixels(actual2, 0, 0);
        REPORTER_ASSERT(reporter, result);

        SkString str;
        str.appendf("mip-level %d", i);

        check_solid_pixmap(reporter, expectedColors[i], actual2,
                           GrCompressionTypeToStr(compressionType), label, str.c_str());
    }
}

// Verify that we can readback from a compressed texture
static void check_readback(GrContext* context, sk_sp<SkImage> img,
                           SkImage::CompressionType compressionType,
                           const SkColor4f& expectedColor,
                           skiatest::Reporter* reporter, const char* label) {
#ifdef SK_BUILD_FOR_IOS
    // reading back ETC2 is broken on Metal/iOS (skbug.com/9839)
    if (context->backend() == GrBackendApi::kMetal) {
      return;
    }
#endif

    SkAutoPixmapStorage actual;

    SkImageInfo readBackII = SkImageInfo::Make(img->width(), img->height(),
                                               kRGBA_8888_SkColorType,
                                               kUnpremul_SkAlphaType);

    SkAssertResult(actual.tryAlloc(readBackII));
    actual.erase(SkColors::kTransparent);

    bool result = img->readPixels(actual, 0, 0);
    REPORTER_ASSERT(reporter, result);

    check_solid_pixmap(reporter, expectedColor, actual,
                       GrCompressionTypeToStr(compressionType), label, "");
}

// Test initialization of compressed GrBackendTextures to a specific color
static void test_compressed_color_init(GrContext* context,
                                       skiatest::Reporter* reporter,
                                       std::function<GrBackendTexture (GrContext*,
                                                                       const SkColor4f&,
                                                                       GrMipMapped)> create,
                                       const SkColor4f& color,
                                       SkImage::CompressionType compression,
                                       GrMipMapped mipMapped) {
    GrBackendTexture backendTex = create(context, color, mipMapped);
    if (!backendTex.isValid()) {
        return;
    }

    sk_sp<SkImage> img = create_image(context, backendTex);
    if (!img) {
        return;
    }

    SkColor4f expectedColors[6] = { color, color, color, color, color, color };

    check_compressed_mipmaps(context, img, compression, expectedColors, mipMapped,
                             reporter, "colorinit");
    check_readback(context, std::move(img), compression, color, reporter,
                   "solid readback");

    context->deleteBackendTexture(backendTex);
}

// Create compressed data pulling the color for each mipmap level from 'levelColors'.
static std::unique_ptr<const char[]> make_compressed_data(SkImage::CompressionType compression,
                                                          SkColor4f levelColors[6],
                                                          GrMipMapped mipMapped) {
    SkISize dimensions { 32, 32 };

    int numMipLevels = 1;
    if (mipMapped == GrMipMapped::kYes) {
        numMipLevels = SkMipMap::ComputeLevelCount(dimensions.width(), dimensions.height()) + 1;
    }

    SkTArray<size_t> mipMapOffsets(numMipLevels);

    size_t dataSize = SkCompressedDataSize(compression, dimensions, &mipMapOffsets,
                                           mipMapped == GrMipMapped::kYes);
    char* data = new char[dataSize];

    for (int level = 0; level < numMipLevels; ++level) {
        // We have to do this a level at a time bc we might have a different color for
        // each level
        GrFillInCompressedData(compression, dimensions,
                               GrMipMapped::kNo, &data[mipMapOffsets[level]], levelColors[level]);

        dimensions = {std::max(1, dimensions.width() /2), std::max(1, dimensions.height()/2)};
    }

    return std::unique_ptr<const char[]>(data);
}

// Verify that we can initialize a compressed backend texture with data (esp.
// the mipmap levels).
static void test_compressed_data_init(GrContext* context,
                                      skiatest::Reporter* reporter,
                                      std::function<GrBackendTexture (GrContext*,
                                                                      const char* data,
                                                                      size_t dataSize,
                                                                      GrMipMapped)> create,
                                      SkImage::CompressionType compression,
                                      GrMipMapped mipMapped) {

    SkColor4f expectedColors[6] = {
        { 1.0f, 0.0f, 0.0f, 1.0f }, // R
        { 0.0f, 1.0f, 0.0f, 1.0f }, // G
        { 0.0f, 0.0f, 1.0f, 1.0f }, // B
        { 0.0f, 1.0f, 1.0f, 1.0f }, // C
        { 1.0f, 0.0f, 1.0f, 1.0f }, // M
        { 1.0f, 1.0f, 0.0f, 1.0f }, // Y
    };

    std::unique_ptr<const char[]> data(make_compressed_data(compression, expectedColors,
                                                            mipMapped));
    size_t dataSize = SkCompressedDataSize(compression, { 32, 32 }, nullptr,
                                           mipMapped == GrMipMapped::kYes);

    GrBackendTexture backendTex = create(context, data.get(), dataSize, mipMapped);
    if (!backendTex.isValid()) {
        return;
    }

    sk_sp<SkImage> img = create_image(context, backendTex);
    if (!img) {
        return;
    }

    check_compressed_mipmaps(context, img, compression, expectedColors,
                             mipMapped, reporter, "pixmap");
    check_readback(context, std::move(img), compression, expectedColors[0], reporter,
                   "data readback");

    context->deleteBackendTexture(backendTex);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(CompressedBackendAllocationTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    const GrCaps* caps = context->priv().caps();

    struct {
        SkImage::CompressionType fCompression;
        SkColor4f                fColor;
    } combinations[] = {
        { SkImage::CompressionType::kETC2_RGB8_UNORM, SkColors::kRed },
        { SkImage::CompressionType::kBC1_RGB8_UNORM,  SkColors::kBlue },
        { SkImage::CompressionType::kBC1_RGBA8_UNORM, SkColors::kTransparent },
    };

    for (auto combo : combinations) {
        GrBackendFormat format = context->compressedBackendFormat(combo.fCompression);
        if (!format.isValid()) {
            continue;
        }

        if (!caps->isFormatTexturable(format)) {
            continue;
        }

        for (auto mipMapped : { GrMipMapped::kNo, GrMipMapped::kYes }) {
            if (GrMipMapped::kYes == mipMapped && !caps->mipMapSupport()) {
                continue;
            }

            // color initialized
            {
                auto createWithColorMtd = [format](GrContext* context,
                                                   const SkColor4f& color,
                                                   GrMipMapped mipMapped) {
                    return context->createCompressedBackendTexture(32, 32, format, color,
                                                                   mipMapped, GrProtected::kNo);
                };

                test_compressed_color_init(context, reporter, createWithColorMtd,
                                           combo.fColor, combo.fCompression, mipMapped);
            }

            // data initialized
            {
                auto createWithDataMtd = [format](GrContext* context,
                                                  const char* data, size_t dataSize,
                                                  GrMipMapped mipMapped) {
                    return context->createCompressedBackendTexture(32, 32, format, data, dataSize,
                                                                   mipMapped, GrProtected::kNo);
                };

                test_compressed_data_init(context, reporter, createWithDataMtd,
                                          combo.fCompression, mipMapped);
            }

        }
    }
}
