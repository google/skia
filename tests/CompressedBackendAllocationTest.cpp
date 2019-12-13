/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "include/gpu/GrContext.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkMipMap.h"
#include "src/gpu/GrContextPriv.h"
#include "src/image/SkImage_Base.h"
#include "tests/Test.h"
#include "tests/TestUtils.h"
#include "tools/ToolUtils.h"

#ifdef SK_GL
#include "src/gpu/gl/GrGLCaps.h"
#endif

// Test wrapping of compressed GrBackendTextures in SkImages (non-static since used in Mtl test)
void test_compressed_wrapping(GrContext* context, skiatest::Reporter* reporter,
                              std::function<GrBackendTexture (GrContext*, GrMipMapped)> create,
                              GrMipMapped mipMapped) {
    GrResourceCache* cache = context->priv().getResourceCache();
    const GrCaps* caps = context->priv().caps();

    const int initialCount = cache->getResourceCount();

    GrBackendTexture backendTex = create(context, mipMapped);
    if (!backendTex.isValid()) {
        ERRORF(reporter, "Couldnt create compressed backendTexture\n");
        return;
    }

    SkImage::CompressionType compression;
    SkAssertResult(caps->isFormatCompressed(backendTex.getBackendFormat(), &compression));

    // Skia proper should know nothing about the new backend object
    REPORTER_ASSERT(reporter, initialCount == cache->getResourceCount());

    {
        sk_sp<SkImage> img = SkImage::MakeFromCompressedTexture(context,
                                                                backendTex,
                                                                kTopLeft_GrSurfaceOrigin,
                                                                kPremul_SkAlphaType,
                                                                nullptr);
        if (!img) {
            ERRORF(reporter, "Couldnt make image from backendTexture for %s\n",
                   GrCompressionTypeToStr(compression));
        } else {
            SkImage_Base* ib = as_IB(img);

            GrTextureProxy* proxy = ib->peekProxy();
            REPORTER_ASSERT(reporter, proxy);

            REPORTER_ASSERT(reporter, mipMapped == proxy->proxyMipMapped());
            REPORTER_ASSERT(reporter, proxy->isInstantiated());
            REPORTER_ASSERT(reporter, mipMapped == proxy->mipMapped());

            REPORTER_ASSERT(reporter, initialCount+1 == cache->getResourceCount());
        }
    }

    REPORTER_ASSERT(reporter, initialCount == cache->getResourceCount());

    context->deleteBackendTexture(backendTex);
}

static void check_solid_pixmap(skiatest::Reporter* reporter,
                               const SkColor4f& expected, const SkPixmap& actual,
                               const char* label0, const char* label1, const char* label2) {
    // we need 0.001f across the board just for noise
    // we need 0.01f across the board for 1010102
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

// Draw the compressed backend texture (wrapped in an SkImage) into an RGBA surface, attempting to access
// all the mipMap levels.
static void check_compressed_mipmaps(GrContext* context, const GrBackendTexture& backendTex,
                                     const SkColor4f expectedColors[6],
                                     GrMipMapped mipMapped,
                                     skiatest::Reporter* reporter, const char* label) {
    const GrCaps* caps = context->priv().caps();

    SkImage::CompressionType compression;
    SkAssertResult(caps->isFormatCompressed(backendTex.getBackendFormat(), &compression));

    SkAlphaType at = kOpaque_SkAlphaType;

    sk_sp<SkImage> img = SkImage::MakeFromCompressedTexture(context,
                                                            backendTex,
                                                            kTopLeft_GrSurfaceOrigin,
                                                            at,
                                                            nullptr);
    if (!img) {
        return;
    }

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
    p.setFilterQuality(kHigh_SkFilterQuality);

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
                           GrCompressionTypeToStr(compression), label, str.c_str());
    }
}

// Test initialization of GrBackendObjects to a specific color (non-static since used in Mtl test)
void test_compressed_color_init(GrContext* context, skiatest::Reporter* reporter,
                                std::function<GrBackendTexture (GrContext*,
                                                                const SkColor4f&,
                                                                GrMipMapped)> create,
                                const SkColor4f& color, GrMipMapped mipMapped) {
    GrBackendTexture backendTex = create(context, color, mipMapped);
    if (!backendTex.isValid()) {
        // errors here should be reported by the test_wrapping test
        return;
    }

    SkColor4f expectedColors[6] = { color, color, color, color, color, color };

    check_compressed_mipmaps(context, backendTex, expectedColors, mipMapped, reporter, "colorinit");

    context->deleteBackendTexture(backendTex);
}

static std::unique_ptr<const char> make_etc1_data(SkColor4f levelColors[6], GrMipMapped mipMapped) {
    SkISize dimensions { 32, 32 };

    size_t dataSize = GrCompressedDataSize(SkImage::CompressionType::kETC1, dimensions, mipMapped);
    char* data = new char[dataSize];

    int numMipLevels = 1;
    if (mipMapped == GrMipMapped::kYes) {
        numMipLevels = SkMipMap::ComputeLevelCount(dimensions.width(), dimensions.height()) + 1;
    }

    size_t offset = 0;
    for (int level = 0; level < numMipLevels; ++level) {
        size_t levelSize = GrCompressedDataSize(SkImage::CompressionType::kETC1, dimensions, GrMipMapped::kNo);

        GrFillInCompressedData(SkImage::CompressionType::kETC1, dimensions,
                               GrMipMapped::kNo, &data[offset], levelColors[level]);

        offset += levelSize;
        dimensions = {SkTMax(1, dimensions.width() /2), SkTMax(1, dimensions.height()/2)};
    }

    return std::unique_ptr<const char>(data);
}

static void test_compressed_data_init(GrContext* context,
                                      skiatest::Reporter* reporter,
                                      std::function<GrBackendTexture (GrContext*,
                                                                      const char* data,
                                                                      size_t dataSize,
                                                                      GrMipMapped mipMapped)> create,
                                      GrMipMapped mipMapped) {

    // TODO: make these transparent for RGBA compressed formats
    SkColor4f expectedColors[6] = {
        { 1.0f, 0.0f, 0.0f, 1.0f }, // R
        { 0.0f, 1.0f, 0.0f, 1.0f }, // G
        { 0.0f, 0.0f, 1.0f, 1.0f }, // B
        { 0.0f, 1.0f, 1.0f, 1.0f }, // C
        { 1.0f, 0.0f, 1.0f, 1.0f }, // M
        { 1.0f, 1.0f, 0.0f, 1.0f }, // Y
    };

    std::unique_ptr<const char> data(make_etc1_data(expectedColors, mipMapped));
    size_t dataSize = GrCompressedDataSize(SkImage::CompressionType::kETC1, { 32, 32 }, mipMapped);

    GrBackendTexture backendTex = create(context, data.get(), dataSize, mipMapped);
    if (!backendTex.isValid()) {
        // errors here should have been reported by the test_wrapping test
        return;
    }

    check_compressed_mipmaps(context, backendTex, expectedColors, mipMapped, reporter, "pixmap");

    context->deleteBackendTexture(backendTex);
}

#ifdef SK_GL

DEF_GPUTEST_FOR_ALL_GL_CONTEXTS(GLCompressedBackendAllocationTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    const GrGLCaps* glCaps = static_cast<const GrGLCaps*>(context->priv().caps());

    struct {
        GrGLenum      fFormat;
        SkColor4f     fColor;
    } combinations[] = {
        { GR_GL_COMPRESSED_ETC1_RGB8, SkColors::kRed },
        { GR_GL_COMPRESSED_RGB8_ETC2, SkColors::kRed },
    };

    for (auto combo : combinations) {
        GrBackendFormat format = GrBackendFormat::MakeGL(combo.fFormat, GR_GL_TEXTURE_2D);

        if (!glCaps->isFormatTexturable(format)) {
            continue;
        }

        for (auto mipMapped : { GrMipMapped::kNo, GrMipMapped::kYes }) {
            if (GrMipMapped::kYes == mipMapped && !glCaps->mipMapSupport()) {
                continue;
            }

            // uninitialized
            {
                auto uninitCreateMtd = [format](GrContext* context, GrMipMapped mipMapped) {
                    return context->createCompressedBackendTexture(32, 32, format,
                                                                   mipMapped, GrProtected::kNo);
                };

                test_compressed_wrapping(context, reporter, uninitCreateMtd, mipMapped);
            }

            // color initialized
            {
                auto createWithColorMtd = [format](GrContext* context,
                                                   const SkColor4f& color,
                                                   GrMipMapped mipMapped) {
                    return context->createCompressedBackendTexture(32, 32, format, color,
                                                                   mipMapped, GrProtected::kNo);
                };

                test_compressed_color_init(context, reporter, createWithColorMtd, combo.fColor, mipMapped);
            }

            // data initialized
            {
                auto createWithDataMtd = [format](GrContext* context,
                                                  const char* data, size_t dataSize,
                                                  GrMipMapped mipMapped) {
                    return context->createCompressedBackendTexture(32, 32, format, data, dataSize,
                                                                   mipMapped, GrProtected::kNo);
                };

                test_compressed_data_init(context, reporter, createWithDataMtd, mipMapped);
            }

        }
    }
}

#endif

