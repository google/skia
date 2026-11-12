/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/ComparePixels.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkSpan.h"
#include "include/private/SkAttributes.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkConvertPixels.h"

#include <array>
#include <cmath>
#include <utility>

[[nodiscard]] static bool convert_pixels(const SkPixmap& dst, const SkPixmap& src) {
    return SkConvertPixels(dst.info(), dst.writable_addr(), dst.rowBytes(),
                           src.info(), src.         addr(), src.rowBytes());
}

static bool compare_colors(int x,
                           int y,
                           SkSpan<const float, 4> rgbaA,
                           SkSpan<const float, 4> rgbaB,
                           SkSpan<const float, 4> tolRGBA,
                           std::function<ComparePixmapsErrorReporter>& error) {
    std::array<float, 4> diffs;
    bool bad = false;
    for (int i = 0; i < 4; ++i) {
        diffs[i] = rgbaB[i] - rgbaA[i];
        if (std::abs(diffs[i]) > std::abs(tolRGBA[i])) {
            bad = true;
        }
    }
    if (bad) {
        error(x, y, diffs.data());
        return false;
    }
    return true;
}

static SkSpan<const float> f32_span(const SkPixmap& pm) {
    SkASSERT(pm.colorType() == kRGBA_F32_SkColorType);
    SkASSERT(pm.rowBytes() % sizeof(float) == 0);
    // TODO(crbug.com/582862856): Use a span accessor once SkPixmap provides one.
    return SK_UNSAFE_TODO(SkSpan<const float>(static_cast<const float*>(pm.addr()),
                                              pm.computeByteSize() / sizeof(float)));
}

static SkSpan<const float, 4> f32_pixel(SkSpan<const float> pixels, size_t rowBytes, int x, int y) {
    size_t offset = y * (rowBytes / sizeof(float)) + x * 4;
    return pixels.subspan(offset).first<4>();
}

bool ComparePixels(const SkPixmap& a,
                   const SkPixmap& b,
                   SkSpan<const float, 4> tolRGBA,
                   std::function<ComparePixmapsErrorReporter>& error) {
    if (a.dimensions() != b.dimensions()) {
        static constexpr std::array<float, 4> kEmptyDiffs = {};
        error(-1, -1, kEmptyDiffs.data());
        return false;
    }

    SkAlphaType floatAlphaType = a.alphaType();
    // If one is premul and the other is unpremul we do the comparison in premul space.
    if ((a.alphaType() == kPremul_SkAlphaType   || b.alphaType() == kPremul_SkAlphaType) &&
        (a.alphaType() == kUnpremul_SkAlphaType || b.alphaType() == kUnpremul_SkAlphaType)) {
        floatAlphaType = kPremul_SkAlphaType;
    }
    sk_sp<SkColorSpace> floatCS;
    if (SkColorSpace::Equals(a.colorSpace(), b.colorSpace())) {
        floatCS = a.refColorSpace();
    } else {
        floatCS = SkColorSpace::MakeSRGBLinear();
    }
    SkImageInfo floatInfo = SkImageInfo::Make(a.dimensions(),
                                              kRGBA_F32_SkColorType,
                                              floatAlphaType,
                                              std::move(floatCS));

    SkAutoPixmapStorage floatA;
    SkAutoPixmapStorage floatB;
    floatA.alloc(floatInfo);
    floatB.alloc(floatInfo);
    SkAssertResult(convert_pixels(floatA, a));
    SkAssertResult(convert_pixels(floatB, b));

    SkASSERT(floatA.rowBytes() == floatB.rowBytes());
    const size_t rb = floatA.rowBytes();
    const SkSpan<const float> pixelsA = f32_span(floatA);
    const SkSpan<const float> pixelsB = f32_span(floatB);

    for (int y = 0; y < floatA.height(); ++y) {
        for (int x = 0; x < floatA.width(); ++x) {
            if (!compare_colors(x,
                                y,
                                f32_pixel(pixelsA, rb, x, y),
                                f32_pixel(pixelsB, rb, x, y),
                                tolRGBA,
                                error)) {
                return false;
            }
        }
    }
    return true;
}

bool CheckSolidPixels(const SkColor4f& col,
                      const SkPixmap& pixmap,
                      SkSpan<const float, 4> tolRGBA,
                      std::function<ComparePixmapsErrorReporter>& error) {
    size_t floatBpp = SkColorTypeBytesPerPixel(kRGBA_F32_SkColorType);

    // First convert 'col' to be compatible with 'pixmap'
    SkAutoPixmapStorage colorPixmap;
    {
        sk_sp<SkColorSpace> srcCS = SkColorSpace::MakeSRGBLinear();
        auto srcInfo = SkImageInfo::Make({1, 1},
                                         kRGBA_F32_SkColorType,
                                         kUnpremul_SkAlphaType,
                                         std::move(srcCS));
        SkPixmap srcPixmap(srcInfo, col.vec(), floatBpp);
        SkImageInfo dstInfo =
                srcInfo.makeAlphaType(pixmap.alphaType()).makeColorSpace(pixmap.refColorSpace());
        colorPixmap.alloc(dstInfo);
        SkAssertResult(convert_pixels(colorPixmap, srcPixmap));
    }

    size_t floatRowBytes = floatBpp * pixmap.width();
    std::unique_ptr<char[]> floatB(new char[floatRowBytes * pixmap.height()]);
    // Then convert 'pixmap' to F32_RGBA
    SkAutoPixmapStorage f32Pixmap;
    f32Pixmap.alloc(pixmap.info().makeColorType(kRGBA_F32_SkColorType));
    SkAssertResult(convert_pixels(f32Pixmap, pixmap));

    const SkSpan<const float> pixelsA = f32_span(f32Pixmap);
    const SkSpan<const float, 4> rgbaB = f32_span(colorPixmap).first<4>();

    for (int y = 0; y < f32Pixmap.height(); ++y) {
        for (int x = 0; x < f32Pixmap.width(); ++x) {
            if (!compare_colors(x,
                                y,
                                f32_pixel(pixelsA, f32Pixmap.rowBytes(), x, y),
                                rgbaB,
                                tolRGBA,
                                error)) {
                return false;
            }
        }
    }
    return true;
}
