/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextureCompressionType.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/base/SkTArray.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkCompressedDataUtils.h"
#include "src/core/SkMipmap.h"
#include "src/gpu/ganesh/GrBackendUtils.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDataUtils.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tests/TestUtils.h"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <memory>
#include <utility>

using namespace skia_private;

class GrRecordingContext;
class SkPixmap;
struct GrContextOptions;

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
    SkTextureCompressionType compression =
            GrBackendFormatToCompressionType(backendTex.getBackendFormat());

    SkAlphaType at = SkTextureCompressionTypeIsOpaque(compression) ? kOpaque_SkAlphaType
                                                            : kPremul_SkAlphaType;

    return SkImages::TextureFromCompressedTexture(
            dContext, backendTex, kTopLeft_GrSurfaceOrigin, at, nullptr);
}

// Draw the compressed backend texture (wrapped in an SkImage) into an RGBA surface, attempting
// to access all the mipMap levels.
static void check_compressed_mipmaps(GrRecordingContext* rContext,
                                     sk_sp<SkImage> img,
                                     SkTextureCompressionType compressionType,
                                     const SkColor4f expectedColors[6],
                                     skgpu::Mipmapped mipmapped,
                                     skiatest::Reporter* reporter,
                                     const char* label) {
    SkImageInfo readbackSurfaceII = SkImageInfo::Make(32, 32, kRGBA_8888_SkColorType,
                                                      kPremul_SkAlphaType);

    sk_sp<SkSurface> surf = SkSurfaces::RenderTarget(rContext,
                                                     skgpu::Budgeted::kNo,
                                                     readbackSurfaceII,
                                                     1,
                                                     kTopLeft_GrSurfaceOrigin,
                                                     nullptr);
    if (!surf) {
        return;
    }

    SkCanvas* canvas = surf->getCanvas();

    // Given that we bias LOD selection with MIP maps, hitting a level exactly using
    // SkMipmap::kLinear is difficult so we use kNearest.
    const SkSamplingOptions sampling(SkFilterMode::kLinear,
                                     SkMipmapMode::kNearest);
    SkPaint p;
    p.setBlendMode(SkBlendMode::kSrc);

    int numMipLevels = 1;
    if (mipmapped == skgpu::Mipmapped::kYes) {
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
                           SkTextureCompressionType compressionType,
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
static void test_compressed_color_init(
        GrDirectContext* dContext,
        skiatest::Reporter* reporter,
        std::function<GrBackendTexture(GrDirectContext*, const SkColor4f&, skgpu::Mipmapped)>
                create,
        const SkColor4f& color,
        SkTextureCompressionType compression,
        skgpu::Mipmapped mipmapped) {
    GrBackendTexture backendTex = create(dContext, color, mipmapped);
    if (!backendTex.isValid()) {
        return;
    }

    sk_sp<SkImage> img = create_image(dContext, backendTex);
    if (!img) {
        return;
    }

    SkColor4f expectedColors[6] = { color, color, color, color, color, color };

    check_compressed_mipmaps(dContext, img, compression, expectedColors, mipmapped,
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

    check_compressed_mipmaps(dContext, img, compression, expectedNewColors, mipmapped, reporter,
                             "colorinit");
    check_readback(dContext, std::move(img), compression, newColor, reporter, "solid readback");

    dContext->deleteBackendTexture(backendTex);
}

// Create compressed data pulling the color for each mipmap level from 'levelColors'.
static std::unique_ptr<const char[]> make_compressed_data(SkTextureCompressionType compression,
                                                          SkColor4f levelColors[6],
                                                          skgpu::Mipmapped mipmapped) {
    SkISize dimensions { 32, 32 };

    int numMipLevels = 1;
    if (mipmapped == skgpu::Mipmapped::kYes) {
        numMipLevels = SkMipmap::ComputeLevelCount(dimensions.width(), dimensions.height()) + 1;
    }

    TArray<size_t> mipMapOffsets(numMipLevels);

    size_t dataSize = SkCompressedDataSize(
            compression, dimensions, &mipMapOffsets, mipmapped == skgpu::Mipmapped::kYes);
    char* data = new char[dataSize];

    for (int level = 0; level < numMipLevels; ++level) {
        // We have to do this a level at a time bc we might have a different color for
        // each level
        GrFillInCompressedData(compression,
                               dimensions,
                               skgpu::Mipmapped::kNo,
                               &data[mipMapOffsets[level]],
                               levelColors[level]);

        dimensions = {std::max(1, dimensions.width() /2), std::max(1, dimensions.height()/2)};
    }

    return std::unique_ptr<const char[]>(data);
}

// Verify that we can initialize a compressed backend texture with data (esp.
// the mipmap levels).
static void test_compressed_data_init(
        GrDirectContext* dContext,
        skiatest::Reporter* reporter,
        std::function<GrBackendTexture(
                GrDirectContext*, const char* data, size_t dataSize, skgpu::Mipmapped)> create,
        SkTextureCompressionType compression,
        skgpu::Mipmapped mipmapped) {
    SkColor4f expectedColors[6] = {
        { 1.0f, 0.0f, 0.0f, 1.0f }, // R
        { 0.0f, 1.0f, 0.0f, 1.0f }, // G
        { 0.0f, 0.0f, 1.0f, 1.0f }, // B
        { 0.0f, 1.0f, 1.0f, 1.0f }, // C
        { 1.0f, 0.0f, 1.0f, 1.0f }, // M
        { 1.0f, 1.0f, 0.0f, 1.0f }, // Y
    };

    std::unique_ptr<const char[]> data(make_compressed_data(compression, expectedColors,
                                                            mipmapped));
    size_t dataSize = SkCompressedDataSize(
            compression, {32, 32}, nullptr, mipmapped == skgpu::Mipmapped::kYes);

    GrBackendTexture backendTex = create(dContext, data.get(), dataSize, mipmapped);
    if (!backendTex.isValid()) {
        return;
    }

    sk_sp<SkImage> img = create_image(dContext, backendTex);
    if (!img) {
        return;
    }

    check_compressed_mipmaps(dContext, img, compression, expectedColors,
                             mipmapped, reporter, "pixmap");
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
            make_compressed_data(compression, expectedColorsNew, mipmapped));
    size_t dataNewSize = SkCompressedDataSize(
            compression, {32, 32}, nullptr, mipmapped == skgpu::Mipmapped::kYes);

    bool result = dContext->updateCompressedBackendTexture(backendTex, dataNew.get(), dataNewSize,
                                                           nullptr, nullptr);
    // Since we were able to create the compressed texture we should be able to update it.
    REPORTER_ASSERT(reporter, result);

    check_compressed_mipmaps(dContext, img, compression, expectedColorsNew, mipmapped, reporter,
                             "pixmap");
    check_readback(dContext, std::move(img), compression, expectedColorsNew[0], reporter,
                   "data readback");

    dContext->deleteBackendTexture(backendTex);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(CompressedBackendAllocationTest,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();
    const GrCaps* caps = dContext->priv().caps();

    struct {
        SkTextureCompressionType fCompression;
        SkColor4f                fColor;
    } combinations[] = {
        { SkTextureCompressionType::kETC2_RGB8_UNORM, SkColors::kRed },
        { SkTextureCompressionType::kBC1_RGB8_UNORM,  SkColors::kBlue },
        { SkTextureCompressionType::kBC1_RGBA8_UNORM, SkColors::kTransparent },
    };

    for (auto combo : combinations) {
        GrBackendFormat format = dContext->compressedBackendFormat(combo.fCompression);
        if (!format.isValid()) {
            continue;
        }

        if (!caps->isFormatTexturable(format, GrTextureType::k2D)) {
            continue;
        }

        for (auto mipmapped : {skgpu::Mipmapped::kNo, skgpu::Mipmapped::kYes}) {
            if (skgpu::Mipmapped::kYes == mipmapped && !caps->mipmapSupport()) {
                continue;
            }

            // color initialized
            {
                auto createWithColorMtd = [format](GrDirectContext* dContext,
                                                   const SkColor4f& color,
                                                   skgpu::Mipmapped mipmapped) {
                    return dContext->createCompressedBackendTexture(32, 32, format, color,
                                                                    mipmapped, GrProtected::kNo);
                };

                test_compressed_color_init(dContext, reporter, createWithColorMtd,
                                           combo.fColor, combo.fCompression, mipmapped);
            }

            // data initialized
            {
                auto createWithDataMtd = [format](GrDirectContext* dContext,
                                                  const char* data,
                                                  size_t dataSize,
                                                  skgpu::Mipmapped mipmapped) {
                    return dContext->createCompressedBackendTexture(32, 32, format, data, dataSize,
                                                                    mipmapped, GrProtected::kNo);
                };

                test_compressed_data_init(dContext, reporter, createWithDataMtd,
                                          combo.fCompression, mipmapped);
            }
        }
    }
}
