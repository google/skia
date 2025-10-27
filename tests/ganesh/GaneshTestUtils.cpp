/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/GaneshTestUtils.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkSize.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrRecordingContext.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDataUtils.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrImageInfo.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/SurfaceContext.h"
#include "tests/Test.h"

#include <cmath>
#include <cstdlib>
#include <utility>

void TestReadPixels(skiatest::Reporter* reporter,
                    GrDirectContext* dContext,
                    skgpu::ganesh::SurfaceContext* srcContext,
                    uint32_t expectedPixelValues[],
                    const char* testName) {
    int pixelCnt = srcContext->width() * srcContext->height();
    SkImageInfo ii = SkImageInfo::Make(srcContext->dimensions(),
                                       kRGBA_8888_SkColorType,
                                       kPremul_SkAlphaType);
    SkAutoPixmapStorage pm;
    pm.alloc(ii);
    pm.erase(SK_ColorTRANSPARENT);

    bool read = srcContext->readPixels(dContext, pm, {0, 0});
    if (!read) {
        ERRORF(reporter, "%s: Error reading from texture.", testName);
    }

    for (int i = 0; i < pixelCnt; ++i) {
        if (pm.addr32()[i] != expectedPixelValues[i]) {
            ERRORF(reporter, "%s: Error, pixel value %d should be 0x%08x, got 0x%08x.",
                   testName, i, expectedPixelValues[i], pm.addr32()[i]);
            break;
        }
    }
}

void TestWritePixels(skiatest::Reporter* reporter,
                     GrDirectContext* dContext,
                     skgpu::ganesh::SurfaceContext* dstContext,
                     bool expectedToWork,
                     const char* testName) {
    SkImageInfo ii = SkImageInfo::Make(dstContext->dimensions(),
                                       kRGBA_8888_SkColorType,
                                       kPremul_SkAlphaType);
    SkAutoPixmapStorage pm;
    pm.alloc(ii);
    for (int y = 0; y < dstContext->height(); ++y) {
        for (int x = 0; x < dstContext->width(); ++x) {
            *pm.writable_addr32(x, y) = SkColorToPremulGrColor(SkColorSetARGB(2*y, x, y, x + y));
        }
    }

    bool write = dstContext->writePixels(dContext, pm, {0, 0});
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

    TestReadPixels(reporter, dContext, dstContext, pm.writable_addr32(0, 0), testName);
}

void TestCopyFromSurface(skiatest::Reporter* reporter,
                         GrDirectContext* dContext,
                         sk_sp<GrSurfaceProxy> proxy,
                         GrSurfaceOrigin origin,
                         GrColorType colorType,
                         uint32_t expectedPixelValues[],
                         const char* testName) {
    auto copy = GrSurfaceProxy::Copy(dContext,
                                     std::move(proxy),
                                     origin,
                                     skgpu::Mipmapped::kNo,
                                     SkBackingFit::kExact,
                                     skgpu::Budgeted::kYes,
                                     /*label=*/"CopyFromSurface_Test");
    SkASSERT(copy && copy->asTextureProxy());
    auto swizzle = dContext->priv().caps()->getReadSwizzle(copy->backendFormat(), colorType);
    GrSurfaceProxyView view(std::move(copy), origin, swizzle);
    auto dstContext = dContext->priv().makeSC(std::move(view),
                                              {colorType, kPremul_SkAlphaType, nullptr});
    SkASSERT(dstContext);

    TestReadPixels(reporter, dContext, dstContext.get(), expectedPixelValues, testName);
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

bool CompareGaneshPixels(const GrCPixmap& a,
                         const GrCPixmap& b,
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
    GrImageInfo floatInfo(GrColorType::kRGBA_F32,
                          floatAlphaType,
                          std::move(floatCS),
                          a.dimensions());

    GrPixmap floatA = GrPixmap::Allocate(floatInfo);
    GrPixmap floatB = GrPixmap::Allocate(floatInfo);
    SkAssertResult(GrConvertPixels(floatA, a));
    SkAssertResult(GrConvertPixels(floatB, b));

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

void CheckSingleThreadedProxyRefs(skiatest::Reporter* reporter,
                                  GrSurfaceProxy* proxy,
                                  int32_t expectedProxyRefs,
                                  int32_t expectedBackingRefs) {
    int32_t actualBackingRefs = proxy->testingOnly_getBackingRefCnt();

    REPORTER_ASSERT(reporter, proxy->refCntGreaterThan(expectedProxyRefs - 1) &&
                              !proxy->refCntGreaterThan(expectedProxyRefs));
    REPORTER_ASSERT(reporter, actualBackingRefs == expectedBackingRefs);
}

std::unique_ptr<skgpu::ganesh::SurfaceContext> CreateSurfaceContext(GrRecordingContext* rContext,
                                                                    const GrImageInfo& info,
                                                                    SkBackingFit fit,
                                                                    GrSurfaceOrigin origin,
                                                                    GrRenderable renderable,
                                                                    int sampleCount,
                                                                    skgpu::Mipmapped mipmapped,
                                                                    GrProtected isProtected,
                                                                    skgpu::Budgeted budgeted) {
    GrBackendFormat format = rContext->priv().caps()->getDefaultBackendFormat(info.colorType(),
                                                                              renderable);
    return rContext->priv().makeSC(info,
                                   format,
                                   /*label=*/{},
                                   fit,
                                   origin,
                                   renderable,
                                   sampleCount,
                                   mipmapped,
                                   isProtected,
                                   budgeted);
}
