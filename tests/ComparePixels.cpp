/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/ComparePixels.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkPixmap.h"
#include "include/private/base/SkTemplates.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkConvertPixels.h"

#include <cmath>
#include <utility>

[[nodiscard]] static bool convert_pixels(const SkPixmap& dst, const SkPixmap& src) {
    return SkConvertPixels(dst.info(), dst.writable_addr(), dst.rowBytes(),
                           src.info(), src.         addr(), src.rowBytes());
}

static bool compare_colors(int x, int y,
                           const float rgbaA[],
                           const float rgbaB[],
                           const float tolRGBA[4],
                           std::function<ComparePixmapsErrorReporter>& error) {
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
    return true;
}

bool ComparePixels(const SkPixmap& a,
                   const SkPixmap& b,
                   const float tolRGBA[4],
                   std::function<ComparePixmapsErrorReporter>& error) {
    if (a.dimensions() != b.dimensions()) {
        static constexpr float kEmptyDiffs[4] = {};
        error(-1, -1, kEmptyDiffs);
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
    auto at = [rb = floatA.rowBytes()](const void* base, int x, int y) {
        return SkTAddOffset<const float>(base, y*rb + x*sizeof(float)*4);
    };

    for (int y = 0; y < floatA.height(); ++y) {
        for (int x = 0; x < floatA.width(); ++x) {
            const float* rgbaA = at(floatA.addr(), x, y);
            const float* rgbaB = at(floatB.addr(), x, y);
            if (!compare_colors(x, y, rgbaA, rgbaB, tolRGBA, error)) {
                return false;
            }
        }
    }
    return true;
}

bool CheckSolidPixels(const SkColor4f& col,
                      const SkPixmap& pixmap,
                      const float tolRGBA[4],
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

    for (int y = 0; y < f32Pixmap.height(); ++y) {
        for (int x = 0; x < f32Pixmap.width(); ++x) {
            auto rgbaA = SkTAddOffset<const float>(f32Pixmap.addr(),
                                                   f32Pixmap.rowBytes()*y + floatBpp*x);
            auto rgbaB = static_cast<const float*>(colorPixmap.addr());
            if (!compare_colors(x, y, rgbaA, rgbaB, tolRGBA, error)) {
                return false;
            }
        }
    }
    return true;
}
