/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrDirectContext.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrSurfaceContext.h"
#include "tests/Test.h"
#include "tests/TestUtils.h"

static void testing_only_texture_test(skiatest::Reporter* reporter, GrDirectContext* dContext,
                                      SkColorType ct, GrRenderable renderable, bool doDataUpload,
                                      GrMipmapped mipMapped) {
    const int kWidth = 16;
    const int kHeight = 16;

    SkImageInfo ii = SkImageInfo::Make(kWidth, kHeight, ct, kPremul_SkAlphaType);

    SkAutoPixmapStorage expectedPixels, actualPixels;
    expectedPixels.alloc(ii);
    actualPixels.alloc(ii);

    const GrCaps* caps = dContext->priv().caps();

    GrColorType grCT = SkColorTypeToGrColorType(ct);

    GrBackendFormat backendFormat = dContext->defaultBackendFormat(ct, renderable);
    if (!backendFormat.isValid()) {
        return;
    }

    GrBackendTexture backendTex;

    if (doDataUpload) {
        SkASSERT(GrMipmapped::kNo == mipMapped);

        FillPixelData(kWidth, kHeight, expectedPixels.writable_addr32(0, 0));

        backendTex = dContext->createBackendTexture(&expectedPixels, 1,
                                                    renderable, GrProtected::kNo);
    } else {
        backendTex = dContext->createBackendTexture(kWidth, kHeight, ct, SkColors::kTransparent,
                                                    mipMapped, renderable, GrProtected::kNo);

        size_t allocSize = SkAutoPixmapStorage::AllocSize(ii, nullptr);
        // createBackendTexture will fill the texture with 0's if no data is provided, so
        // we set the expected result likewise.
        memset(expectedPixels.writable_addr32(0, 0), 0, allocSize);
    }
    if (!backendTex.isValid()) {
        return;
    }
    // skbug.com/9165
    auto supportedRead =
            caps->supportedReadPixelsColorType(grCT, backendTex.getBackendFormat(), grCT);
    if (supportedRead.fColorType != grCT) {
        return;
    }

    sk_sp<GrTextureProxy> wrappedProxy;
    if (GrRenderable::kYes == renderable) {
        wrappedProxy = dContext->priv().proxyProvider()->wrapRenderableBackendTexture(
                backendTex, 1, kAdopt_GrWrapOwnership, GrWrapCacheable::kNo, nullptr);
    } else {
        wrappedProxy = dContext->priv().proxyProvider()->wrapBackendTexture(
                backendTex, kAdopt_GrWrapOwnership, GrWrapCacheable::kNo, GrIOType::kRW_GrIOType);
    }
    REPORTER_ASSERT(reporter, wrappedProxy);

    GrSwizzle swizzle = dContext->priv().caps()->getReadSwizzle(wrappedProxy->backendFormat(),
                                                                grCT);
    GrSurfaceProxyView view(std::move(wrappedProxy), kTopLeft_GrSurfaceOrigin, swizzle);
    auto surfaceContext = GrSurfaceContext::Make(dContext, std::move(view), grCT,
                                                 kPremul_SkAlphaType, nullptr);
    REPORTER_ASSERT(reporter, surfaceContext);

    bool result = surfaceContext->readPixels(dContext,
                                             {grCT, kPremul_SkAlphaType, nullptr, kWidth, kHeight},
                                             actualPixels.writable_addr(), actualPixels.rowBytes(),
                                             {0, 0});

    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter,
                    DoesFullBufferContainCorrectColor(expectedPixels.addr32(),
                                                      actualPixels.addr32(), kWidth, kHeight));
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrTestingBackendTextureUploadTest, reporter, ctxInfo) {
    for (auto colorType: { kRGBA_8888_SkColorType, kBGRA_8888_SkColorType }) {
        for (auto renderable: { GrRenderable::kYes, GrRenderable::kNo }) {
            for (bool doDataUpload: {true, false}) {
                testing_only_texture_test(reporter, ctxInfo.directContext(), colorType,
                                          renderable, doDataUpload, GrMipmapped::kNo);

                if (!doDataUpload) {
                    testing_only_texture_test(reporter, ctxInfo.directContext(), colorType,
                                              renderable, doDataUpload, GrMipmapped::kYes);
                }
            }
        }
    }
}

