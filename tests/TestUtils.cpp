/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/TestUtils.h"

#include "include/encode/SkPngEncoder.h"
#include "include/private/GrSurfaceProxy.h"
#include "include/private/GrTextureProxy.h"
#include "include/utils/SkBase64.h"
#include "src/core/SkUtils.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrSurfaceContext.h"
#include "src/gpu/GrSurfaceContextPriv.h"
#include "src/gpu/SkGr.h"
#include "tools/gpu/ProxyUtils.h"

void test_read_pixels(skiatest::Reporter* reporter,
                      GrSurfaceContext* srcContext, uint32_t expectedPixelValues[],
                      const char* testName) {
    int pixelCnt = srcContext->width() * srcContext->height();
    SkAutoTMalloc<uint32_t> pixels(pixelCnt);
    memset(pixels.get(), 0, sizeof(uint32_t)*pixelCnt);

    SkImageInfo ii = SkImageInfo::Make(srcContext->width(), srcContext->height(),
                                       kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    bool read = srcContext->readPixels(ii, pixels.get(), 0, 0, 0);
    if (!read) {
        ERRORF(reporter, "%s: Error reading from texture.", testName);
    }

    for (int i = 0; i < pixelCnt; ++i) {
        if (pixels.get()[i] != expectedPixelValues[i]) {
            ERRORF(reporter, "%s: Error, pixel value %d should be 0x%08x, got 0x%08x.",
                   testName, i, expectedPixelValues[i], pixels.get()[i]);
            break;
        }
    }
}

void test_write_pixels(skiatest::Reporter* reporter,
                       GrSurfaceContext* dstContext, bool expectedToWork,
                       const char* testName) {
    int pixelCnt = dstContext->width() * dstContext->height();
    SkAutoTMalloc<uint32_t> pixels(pixelCnt);
    for (int y = 0; y < dstContext->width(); ++y) {
        for (int x = 0; x < dstContext->height(); ++x) {
            pixels.get()[y * dstContext->width() + x] =
                SkColorToPremulGrColor(SkColorSetARGB(2*y, x, y, x + y));
        }
    }

    SkImageInfo ii = SkImageInfo::Make(dstContext->width(), dstContext->height(),
                                       kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    bool write = dstContext->writePixels(ii, pixels.get(), 0, 0, 0);
    if (!write) {
        if (expectedToWork) {
            ERRORF(reporter, "%s: Error writing to texture.", testName);
        }
        return;
    }

    if (write && !expectedToWork) {
        ERRORF(reporter, "%s: writePixels succeeded when it wasn't supposed to.", testName);
        return;
    }

    test_read_pixels(reporter, dstContext, pixels.get(), testName);
}

void test_copy_from_surface(skiatest::Reporter* reporter, GrContext* context,
                            GrSurfaceProxy* proxy, uint32_t expectedPixelValues[],
                            bool onlyTestRTConfig, const char* testName) {
    GrSurfaceDesc copyDstDesc;
    copyDstDesc.fWidth = proxy->width();
    copyDstDesc.fHeight = proxy->height();
    copyDstDesc.fConfig = kRGBA_8888_GrPixelConfig;

    for (auto flags : { kNone_GrSurfaceFlags, kRenderTarget_GrSurfaceFlag }) {
        if (kNone_GrSurfaceFlags == flags && onlyTestRTConfig) {
            continue;
        }

        copyDstDesc.fFlags = flags;
        auto origin = (kNone_GrSurfaceFlags == flags) ? kTopLeft_GrSurfaceOrigin
                                                      : kBottomLeft_GrSurfaceOrigin;

        sk_sp<GrSurfaceContext> dstContext(
                GrSurfaceProxy::TestCopy(context, copyDstDesc, origin, proxy));

        test_read_pixels(reporter, dstContext.get(), expectedPixelValues, testName);
    }
}

void test_copy_to_surface(skiatest::Reporter* reporter,
                          GrContext* context,
                          GrSurfaceContext* dstContext,
                          const char* testName) {

    int pixelCnt = dstContext->width() * dstContext->height();
    SkAutoTMalloc<uint32_t> pixels(pixelCnt);
    for (int y = 0; y < dstContext->width(); ++y) {
        for (int x = 0; x < dstContext->height(); ++x) {
            pixels.get()[y * dstContext->width() + x] =
                SkColorToPremulGrColor(SkColorSetARGB(2*y, y, x, x * y));
        }
    }

    for (auto renderable : {GrRenderable::kNo, GrRenderable::kYes}) {
        for (auto origin : {kTopLeft_GrSurfaceOrigin, kBottomLeft_GrSurfaceOrigin}) {
            auto src = sk_gpu_test::MakeTextureProxyFromData(
                    context, renderable, dstContext->width(),
                    dstContext->height(), kRGBA_8888_SkColorType, origin, pixels.get(), 0);
            dstContext->copy(src.get());
            test_read_pixels(reporter, dstContext, pixels.get(), testName);
        }
    }
}

void fill_pixel_data(int width, int height, GrColor* data) {
    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            unsigned int red = (unsigned int)(256.f * (i / (float)width));
            unsigned int green = (unsigned int)(256.f * (j / (float)height));
            data[i + j * width] = GrColorPackRGBA(red - (red >> 8), green - (green >> 8),
                                                  0xff, 0xff);
        }
    }
}

bool create_backend_texture(GrContext* context, GrBackendTexture* backendTex,
                            const SkImageInfo& ii, GrMipMapped mipMapped, SkColor color,
                            GrRenderable renderable) {
    GrGpu* gpu = context->priv().getGpu();
    if (!gpu) {
        return false;
    }

    SkBitmap bm;
    bm.allocPixels(ii);
    // TODO: a SkBitmap::eraseColor would be better here
    sk_memset32(bm.getAddr32(0, 0), color, ii.width() * ii.height());

    *backendTex = gpu->createTestingOnlyBackendTexture(ii.width(), ii.height(), ii.colorType(),
                                                       mipMapped, renderable,
                                                       bm.getPixels(), bm.rowBytes());
    if (!backendTex->isValid() || !gpu->isTestingOnlyBackendTexture(*backendTex)) {
        return false;
    }

    return true;
}

void delete_backend_texture(GrContext* context, const GrBackendTexture& backendTex) {
    context->priv().deleteBackendTexture(backendTex);
}

bool does_full_buffer_contain_correct_color(GrColor* srcBuffer,
                                            GrColor* dstBuffer,
                                            int width,
                                            int height) {
    GrColor* srcPtr = srcBuffer;
    GrColor* dstPtr = dstBuffer;
    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            if (srcPtr[i] != dstPtr[i]) {
                return false;
            }
        }
        srcPtr += width;
        dstPtr += width;
    }
    return true;
}

bool bitmap_to_base64_data_uri(const SkBitmap& bitmap, SkString* dst) {
    SkPixmap pm;
    if (!bitmap.peekPixels(&pm)) {
        dst->set("peekPixels failed");
        return false;
    }

    // We're going to embed this PNG in a data URI, so make it as small as possible
    SkPngEncoder::Options options;
    options.fFilterFlags = SkPngEncoder::FilterFlag::kAll;
    options.fZLibLevel = 9;

    SkDynamicMemoryWStream wStream;
    if (!SkPngEncoder::Encode(&wStream, pm, options)) {
        dst->set("SkPngEncoder::Encode failed");
        return false;
    }

    sk_sp<SkData> pngData = wStream.detachAsData();
    size_t len = SkBase64::Encode(pngData->data(), pngData->size(), nullptr);

    // The PNG can be almost arbitrarily large. We don't want to fill our logs with enormous URLs.
    // Infra says these can be pretty big, as long as we're only outputting them on failure.
    static const size_t kMaxBase64Length = 1024 * 1024;
    if (len > kMaxBase64Length) {
        dst->printf("Encoded image too large (%u bytes)", static_cast<uint32_t>(len));
        return false;
    }

    dst->resize(len);
    SkBase64::Encode(pngData->data(), pngData->size(), dst->writable_str());
    dst->prepend("data:image/png;base64,");
    return true;
}

#include "src/utils/SkCharToGlyphCache.h"

static SkGlyphID hash_to_glyph(uint32_t value) {
    return SkToU16(((value >> 16) ^ value) & 0xFFFF);
}

namespace {
class UnicharGen {
    SkUnichar fU;
    const int fStep;
public:
    UnicharGen(int step) : fU(0), fStep(step) {}

    SkUnichar next() {
        fU += fStep;
        return fU;
    }
};
}

DEF_TEST(chartoglyph_cache, reporter) {
    SkCharToGlyphCache cache;
    const int step = 3;

    UnicharGen gen(step);
    for (int i = 0; i < 500; ++i) {
        SkUnichar c = gen.next();
        SkGlyphID glyph = hash_to_glyph(c);

        int index = cache.findGlyphIndex(c);
        if (index >= 0) {
            index = cache.findGlyphIndex(c);
        }
        REPORTER_ASSERT(reporter, index < 0);
        cache.insertCharAndGlyph(~index, c, glyph);

        UnicharGen gen2(step);
        for (int j = 0; j <= i; ++j) {
            c = gen2.next();
            glyph = hash_to_glyph(c);
            index = cache.findGlyphIndex(c);
            if ((unsigned)index != glyph) {
                index = cache.findGlyphIndex(c);
            }
            REPORTER_ASSERT(reporter, (unsigned)index == glyph);
        }
    }
}
