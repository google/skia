/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/TestUtils.h"

#include "include/encode/SkPngEncoder.h"
#include "include/utils/SkBase64.h"
#include "src/core/SkUtils.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrSurfaceContext.h"
#include "src/gpu/GrSurfaceProxy.h"
#include "src/gpu/GrTextureContext.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/SkGr.h"

void test_read_pixels(skiatest::Reporter* reporter,
                      GrSurfaceContext* srcContext, uint32_t expectedPixelValues[],
                      const char* testName) {
    int pixelCnt = srcContext->width() * srcContext->height();
    SkAutoTMalloc<uint32_t> pixels(pixelCnt);
    memset(pixels.get(), 0, sizeof(uint32_t)*pixelCnt);

    SkImageInfo ii = SkImageInfo::Make(srcContext->width(), srcContext->height(),
                                       kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    bool read = srcContext->readPixels(ii, pixels.get(), 0, {0, 0});
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
    bool write = dstContext->writePixels(ii, pixels.get(), 0, {0, 0});
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

void test_copy_from_surface(skiatest::Reporter* reporter, GrContext* context, GrSurfaceProxy* proxy,
                            GrColorType colorType, uint32_t expectedPixelValues[],
                            const char* testName) {
    sk_sp<GrTextureProxy> dstProxy = GrSurfaceProxy::Copy(context, proxy, GrMipMapped::kNo,
                                                          SkBackingFit::kExact, SkBudgeted::kYes);
    SkASSERT(dstProxy);

    auto dstContext = context->priv().makeWrappedSurfaceContext(std::move(dstProxy), colorType,
                                                                kPremul_SkAlphaType);
    SkASSERT(dstContext);

    test_read_pixels(reporter, dstContext.get(), expectedPixelValues, testName);
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
                            const SkImageInfo& ii, const SkColor4f& color,
                            GrMipMapped mipMapped, GrRenderable renderable) {
    // TODO: use the color-init version of createBackendTexture once Metal supports it.
#if 0
    *backendTex = context->createBackendTexture(ii.width(), ii.height(), ii.colorType(),
                                                color, mipMapped, renderable);
#else
    SkBitmap bm;
    bm.allocPixels(ii);
    sk_memset32(bm.getAddr32(0, 0), color.toSkColor(), ii.width() * ii.height());

    SkASSERT(GrMipMapped::kNo == mipMapped);
    *backendTex = context->priv().createBackendTexture(&bm.pixmap(), 1, renderable,
                                                       GrProtected::kNo);
#endif

    return backendTex->isValid();
}

void delete_backend_texture(GrContext* context, const GrBackendTexture& backendTex) {
    GrFlushInfo flushInfo;
    flushInfo.fFlags = kSyncCpu_GrFlushFlag;
    context->flush(flushInfo);
    context->deleteBackendTexture(backendTex);
}

bool does_full_buffer_contain_correct_color(const GrColor* srcBuffer,
                                            const GrColor* dstBuffer,
                                            int width,
                                            int height) {
    const GrColor* srcPtr = srcBuffer;
    const GrColor* dstPtr = dstBuffer;
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

using AccessPixelFn = const float*(const char* floatBuffer, int x, int y);

bool compare_pixels(int width, int height,
                    const char* floatA, std::function<AccessPixelFn>& atA,
                    const char* floatB, std::function<AccessPixelFn>& atB,
                    const float tolRGBA[4], std::function<ComparePixmapsErrorReporter>& error) {

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const float* rgbaA = atA(floatA, x, y);
            const float* rgbaB = atB(floatB, x, y);
            float diffs[4];
            bool bad = false;
            for (int i = 0; i < 4; ++i) {
                diffs[i] = rgbaB[i] - rgbaA[i];
                if (std::abs(diffs[i]) > std::abs(tolRGBA[i])) {
                    bad = true;
                }
            }
            if (bad) {
                error(x, y, diffs);
                return false;
            }
        }
    }
    return true;
}

bool compare_pixels(const GrPixelInfo& infoA, const char* a, size_t rowBytesA,
                    const GrPixelInfo& infoB, const char* b, size_t rowBytesB,
                    const float tolRGBA[4], std::function<ComparePixmapsErrorReporter>& error) {
    if (infoA.width() != infoB.width() || infoA.height() != infoB.height()) {
        static constexpr float kDummyDiffs[4] = {};
        error(-1, -1, kDummyDiffs);
        return false;
    }

    SkAlphaType floatAlphaType = infoA.alphaType();
    // If one is premul and the other is unpremul we do the comparison in premul space.
    if ((infoA.alphaType() == kPremul_SkAlphaType ||
         infoB.alphaType() == kPremul_SkAlphaType) &&
        (infoA.alphaType() == kUnpremul_SkAlphaType ||
         infoB.alphaType() == kUnpremul_SkAlphaType)) {
        floatAlphaType = kPremul_SkAlphaType;
    }
    sk_sp<SkColorSpace> floatCS;
    if (SkColorSpace::Equals(infoA.colorSpace(), infoB.colorSpace())) {
        floatCS = infoA.refColorSpace();
    } else {
        floatCS = SkColorSpace::MakeSRGBLinear();
    }
    GrPixelInfo floatInfo(GrColorType::kRGBA_F32, floatAlphaType, std::move(floatCS),
                          infoA.width(), infoA.height());

    size_t floatBpp = GrColorTypeBytesPerPixel(GrColorType::kRGBA_F32);
    size_t floatRowBytes = floatBpp * infoA.width();
    std::unique_ptr<char[]> floatA(new char[floatRowBytes * infoA.height()]);
    std::unique_ptr<char[]> floatB(new char[floatRowBytes * infoA.height()]);
    SkAssertResult(GrConvertPixels(floatInfo, floatA.get(), floatRowBytes, infoA, a, rowBytesA));
    SkAssertResult(GrConvertPixels(floatInfo, floatB.get(), floatRowBytes, infoB, b, rowBytesB));

    auto at = std::function<AccessPixelFn>(
        [floatBpp, floatRowBytes](const char* floatBuffer, int x, int y) {
            return reinterpret_cast<const float*>(floatBuffer + y * floatRowBytes + x * floatBpp);
        });

    return compare_pixels(infoA.width(), infoA.height(),
                          floatA.get(), at, floatB.get(), at,
                          tolRGBA, error);
}

bool compare_pixels(const SkPixmap& a, const SkPixmap& b, const float tolRGBA[4],
                    std::function<ComparePixmapsErrorReporter>& error) {
    return compare_pixels(a.info(), static_cast<const char*>(a.addr()), a.rowBytes(),
                          b.info(), static_cast<const char*>(b.addr()), b.rowBytes(),
                          tolRGBA, error);
}

bool check_solid_pixels(const SkColor4f& col, const SkPixmap& pixmap,
                        const float tolRGBA[4], std::function<ComparePixmapsErrorReporter>& error) {

    size_t floatBpp = GrColorTypeBytesPerPixel(GrColorType::kRGBA_F32);

    std::unique_ptr<char[]> floatA(new char[floatBpp]);
    // First convert 'col' to be compatible with 'pixmap'
    {
        sk_sp<SkColorSpace> srcCS = SkColorSpace::MakeSRGBLinear();
        GrPixelInfo srcInfo(GrColorType::kRGBA_F32, kUnpremul_SkAlphaType, std::move(srcCS), 1, 1);
        GrPixelInfo dstInfo(GrColorType::kRGBA_F32, pixmap.alphaType(), pixmap.refColorSpace(), 1, 1);

        SkAssertResult(GrConvertPixels(dstInfo, floatA.get(), floatBpp, srcInfo,
                                       col.vec(), floatBpp));
    }

    size_t floatRowBytes = floatBpp * pixmap.width();
    std::unique_ptr<char[]> floatB(new char[floatRowBytes * pixmap.height()]);
    // Then convert 'pixmap' to RGBA_F32
    {
        GrPixelInfo dstInfo(GrColorType::kRGBA_F32, pixmap.alphaType(), pixmap.refColorSpace(),
                            pixmap.width(), pixmap.height());

        SkAssertResult(GrConvertPixels(dstInfo, floatB.get(), floatRowBytes, pixmap.info(),
                                       pixmap.addr(), pixmap.rowBytes()));
    }

    auto atA = std::function<AccessPixelFn>(
        [](const char* floatBuffer, int /* x */, int /* y */) {
            return reinterpret_cast<const float*>(floatBuffer);
        });

    auto atB = std::function<AccessPixelFn>(
        [floatBpp, floatRowBytes](const char* floatBuffer, int x, int y) {
            return reinterpret_cast<const float*>(floatBuffer + y * floatRowBytes + x * floatBpp);
        });

    return compare_pixels(pixmap.width(), pixmap.height(), floatA.get(), atA, floatB.get(), atB,
                          tolRGBA, error);
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
