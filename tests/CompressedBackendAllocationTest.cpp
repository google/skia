/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkCompressedDataUtils.h"
#include "src/core/SkMipmap.h"
#include "src/core/SkPaintPriv.h"
#include "src/gpu/GrBackendUtils.h"
#include "src/gpu/GrDirectContextPriv.h"
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
sk_sp<SkImage> create_image(GrDirectContext* dContext, const GrBackendTexture& backendTex) {
    SkImage::CompressionType compression =
            GrBackendFormatToCompressionType(backendTex.getBackendFormat());

    SkAlphaType at = SkCompressionTypeIsOpaque(compression) ? kOpaque_SkAlphaType
                                                            : kPremul_SkAlphaType;

    return SkImage::MakeFromCompressedTexture(dContext,
                                              backendTex,
                                              kTopLeft_GrSurfaceOrigin,
                                              at,
                                              nullptr);
}

// Draw the compressed backend texture (wrapped in an SkImage) into an RGBA surface, attempting
// to access all the mipMap levels.
static void check_compressed_mipmaps(GrRecordingContext* rContext, sk_sp<SkImage> img,
                                     SkImage::CompressionType compressionType,
                                     const SkColor4f expectedColors[6],
                                     GrMipmapped mipMapped,
                                     skiatest::Reporter* reporter, const char* label) {

    SkImageInfo readbackSurfaceII = SkImageInfo::Make(32, 32, kRGBA_8888_SkColorType,
                                                      kPremul_SkAlphaType);

    sk_sp<SkSurface> surf = SkSurface::MakeRenderTarget(rContext,
                                                        SkBudgeted::kNo,
                                                        readbackSurfaceII, 1,
                                                        kTopLeft_GrSurfaceOrigin,
                                                        nullptr);
    if (!surf) {
        return;
    }

    SkCanvas* canvas = surf->getCanvas();

    const SkSamplingOptions sampling(SkFilterMode::kLinear,
                                     SkMipmapMode::kLinear);
    SkPaint p;
    p.setBlendMode(SkBlendMode::kSrc);

    int numMipLevels = 1;
    if (mipMapped == GrMipmapped::kYes) {
        numMipLevels = SkMipmap::ComputeLevelCount(32, 32)+1;
    }

    for (int i = 0, rectSize = 32; i < numMipLevels; ++i, rectSize /= 2) {
        SkASSERT(rectSize >= 1);

        canvas->clear(SK_ColorTRANSPARENT);

        SkRect r = SkRect::MakeWH(rectSize, rectSize);
        canvas->drawImageRect(img, r, sampling, &p);

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
static void check_readback(GrDirectContext* dContext, sk_sp<SkImage> img,
                           SkImage::CompressionType compressionType,
                           const SkColor4f& expectedColor,
                           skiatest::Reporter* reporter, const char* label) {
#ifdef SK_BUILD_FOR_IOS
    // reading back ETC2 is broken on Metal/iOS (skbug.com/9839)
    if (dContext->backend() == GrBackendApi::kMetal) {
      return;
    }
#endif

    SkAutoPixmapStorage actual;

    SkImageInfo readBackII = SkImageInfo::Make(img->width(), img->height(),
                                               kRGBA_8888_SkColorType,
                                               kUnpremul_SkAlphaType);

    SkAssertResult(actual.tryAlloc(readBackII));
    actual.erase(SkColors::kTransparent);

    bool result = img->readPixels(dContext, actual, 0, 0);
    REPORTER_ASSERT(reporter, result);

    check_solid_pixmap(reporter, expectedColor, actual,
                       GrCompressionTypeToStr(compressionType), label, "");
}

// Test initialization of compressed GrBackendTextures to a specific color
static void test_compressed_color_init(GrDirectContext* dContext,
                                       skiatest::Reporter* reporter,
                                       std::function<GrBackendTexture (GrDirectContext*,
                                                                       const SkColor4f&,
                                                                       GrMipmapped)> create,
                                       const SkColor4f& color,
                                       SkImage::CompressionType compression,
                                       GrMipmapped mipMapped) {
    GrBackendTexture backendTex = create(dContext, color, mipMapped);
    if (!backendTex.isValid()) {
        return;
    }

    sk_sp<SkImage> img = create_image(dContext, backendTex);
    if (!img) {
        return;
    }

    SkColor4f expectedColors[6] = { color, color, color, color, color, color };

    check_compressed_mipmaps(dContext, img, compression, expectedColors, mipMapped,
                             reporter, "colorinit");
    check_readback(dContext, img, compression, color, reporter, "solid readback");

    SkColor4f newColor;
    newColor.fR = color.fB;
    newColor.fG = color.fR;
    newColor.fB = color.fG;
    newColor.fA = color.fA;

    bool result = dContext->updateCompressedBackendTexture(backendTex, newColor, nullptr, nullptr);
    // Since we were able to create the compressed texture we should be able to update it.
    REPORTER_ASSERT(reporter, result);

    SkColor4f expectedNewColors[6] = {newColor, newColor, newColor, newColor, newColor, newColor};

    check_compressed_mipmaps(dContext, img, compression, expectedNewColors, mipMapped, reporter,
                             "colorinit");
    check_readback(dContext, std::move(img), compression, newColor, reporter, "solid readback");

    dContext->deleteBackendTexture(backendTex);
}

// Create compressed data pulling the color for each mipmap level from 'levelColors'.
static std::unique_ptr<const char[]> make_compressed_data(SkImage::CompressionType compression,
                                                          SkColor4f levelColors[6],
                                                          GrMipmapped mipMapped) {
    SkISize dimensions { 32, 32 };

    int numMipLevels = 1;
    if (mipMapped == GrMipmapped::kYes) {
        numMipLevels = SkMipmap::ComputeLevelCount(dimensions.width(), dimensions.height()) + 1;
    }

    SkTArray<size_t> mipMapOffsets(numMipLevels);

    size_t dataSize = SkCompressedDataSize(compression, dimensions, &mipMapOffsets,
                                           mipMapped == GrMipmapped::kYes);
    char* data = new char[dataSize];

    for (int level = 0; level < numMipLevels; ++level) {
        // We have to do this a level at a time bc we might have a different color for
        // each level
        GrFillInCompressedData(compression, dimensions,
                               GrMipmapped::kNo, &data[mipMapOffsets[level]], levelColors[level]);

        dimensions = {std::max(1, dimensions.width() /2), std::max(1, dimensions.height()/2)};
    }

    return std::unique_ptr<const char[]>(data);
}

// Verify that we can initialize a compressed backend texture with data (esp.
// the mipmap levels).
static void test_compressed_data_init(GrDirectContext* dContext,
                                      skiatest::Reporter* reporter,
                                      std::function<GrBackendTexture (GrDirectContext*,
                                                                      const char* data,
                                                                      size_t dataSize,
                                                                      GrMipmapped)> create,
                                      SkImage::CompressionType compression,
                                      GrMipmapped mipMapped) {

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
                                           mipMapped == GrMipmapped::kYes);

    GrBackendTexture backendTex = create(dContext, data.get(), dataSize, mipMapped);
    if (!backendTex.isValid()) {
        return;
    }

    sk_sp<SkImage> img = create_image(dContext, backendTex);
    if (!img) {
        return;
    }

    check_compressed_mipmaps(dContext, img, compression, expectedColors,
                             mipMapped, reporter, "pixmap");
    check_readback(dContext, img, compression, expectedColors[0], reporter, "data readback");

    SkColor4f expectedColorsNew[6] = {
        {1.0f, 1.0f, 0.0f, 1.0f},  // Y
        {1.0f, 0.0f, 0.0f, 1.0f},  // R
        {0.0f, 1.0f, 0.0f, 1.0f},  // G
        {0.0f, 0.0f, 1.0f, 1.0f},  // B
        {0.0f, 1.0f, 1.0f, 1.0f},  // C
        {1.0f, 0.0f, 1.0f, 1.0f},  // M
    };

    std::unique_ptr<const char[]> dataNew(
            make_compressed_data(compression, expectedColorsNew, mipMapped));
    size_t dataNewSize =
            SkCompressedDataSize(compression, {32, 32}, nullptr, mipMapped == GrMipMapped::kYes);

    bool result = dContext->updateCompressedBackendTexture(backendTex, dataNew.get(), dataNewSize,
                                                           nullptr, nullptr);
    // Since we were able to create the compressed texture we should be able to update it.
    REPORTER_ASSERT(reporter, result);

    check_compressed_mipmaps(dContext, img, compression, expectedColorsNew, mipMapped, reporter,
                             "pixmap");
    check_readback(dContext, std::move(img), compression, expectedColorsNew[0], reporter,
                   "data readback");

    dContext->deleteBackendTexture(backendTex);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(CompressedBackendAllocationTest, reporter, ctxInfo) {
    auto dContext = ctxInfo.directContext();
    const GrCaps* caps = dContext->priv().caps();

    struct {
        SkImage::CompressionType fCompression;
        SkColor4f                fColor;
    } combinations[] = {
        { SkImage::CompressionType::kETC2_RGB8_UNORM, SkColors::kRed },
        { SkImage::CompressionType::kBC1_RGB8_UNORM,  SkColors::kBlue },
        { SkImage::CompressionType::kBC1_RGBA8_UNORM, SkColors::kTransparent },
    };

    for (auto combo : combinations) {
        GrBackendFormat format = dContext->compressedBackendFormat(combo.fCompression);
        if (!format.isValid()) {
            continue;
        }

        if (!caps->isFormatTexturable(format, GrTextureType::k2D)) {
            continue;
        }

        for (auto mipMapped : { GrMipmapped::kNo, GrMipmapped::kYes }) {
            if (GrMipmapped::kYes == mipMapped && !caps->mipmapSupport()) {
                continue;
            }

            // color initialized
            {
                auto createWithColorMtd = [format](GrDirectContext* dContext,
                                                   const SkColor4f& color,
                                                   GrMipmapped mipMapped) {
                    return dContext->createCompressedBackendTexture(32, 32, format, color,
                                                                    mipMapped, GrProtected::kNo);
                };

                test_compressed_color_init(dContext, reporter, createWithColorMtd,
                                           combo.fColor, combo.fCompression, mipMapped);
            }

            // data initialized
            {
                auto createWithDataMtd = [format](GrDirectContext* dContext,
                                                  const char* data, size_t dataSize,
                                                  GrMipmapped mipMapped) {
                    return dContext->createCompressedBackendTexture(32, 32, format, data, dataSize,
                                                                    mipMapped, GrProtected::kNo);
                };

                test_compressed_data_init(dContext, reporter, createWithDataMtd,
                                          combo.fCompression, mipMapped);
            }

        }
    }
}
