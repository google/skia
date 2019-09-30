/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkAutoPixmapStorage.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrSurfaceContext.h"
#include "src/gpu/SkGr.h"
#include "tests/Test.h"
#include "tests/TestUtils.h"

void testing_only_texture_test(skiatest::Reporter* reporter, GrContext* context, SkColorType ct,
                               GrRenderable renderable, bool doDataUpload, GrMipMapped mipMapped) {

    const int kWidth = 16;
    const int kHeight = 16;

    SkImageInfo ii = SkImageInfo::Make(kWidth, kHeight, ct, kPremul_SkAlphaType);

    SkAutoPixmapStorage expectedPixels, actualPixels;
    expectedPixels.alloc(ii);
    actualPixels.alloc(ii);

    const GrCaps* caps = context->priv().caps();

    GrColorType grCT = SkColorTypeToGrColorType(ct);

    GrBackendFormat backendFormat = context->defaultBackendFormat(ct, renderable);
    if (!backendFormat.isValid()) {
        return;
    }

    GrBackendTexture backendTex;

    if (doDataUpload) {
        SkASSERT(GrMipMapped::kNo == mipMapped);

        fill_pixel_data(kWidth, kHeight, expectedPixels.writable_addr32(0, 0));

        backendTex = context->priv().createBackendTexture(&expectedPixels, 1,
                                                          renderable, GrProtected::kNo);
    } else {
        backendTex = context->createBackendTexture(kWidth, kHeight, ct, SkColors::kTransparent,
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
        wrappedProxy = context->priv().proxyProvider()->wrapRenderableBackendTexture(
                backendTex, kTopLeft_GrSurfaceOrigin, 1, grCT, kAdopt_GrWrapOwnership,
                GrWrapCacheable::kNo);
    } else {
        wrappedProxy = context->priv().proxyProvider()->wrapBackendTexture(
                backendTex, grCT, kTopLeft_GrSurfaceOrigin, kAdopt_GrWrapOwnership,
                GrWrapCacheable::kNo, GrIOType::kRW_GrIOType);
    }
    REPORTER_ASSERT(reporter, wrappedProxy);

    auto surfaceContext = context->priv().makeWrappedSurfaceContext(std::move(wrappedProxy), grCT,
                                                                    kPremul_SkAlphaType);
    REPORTER_ASSERT(reporter, surfaceContext);

    bool result = surfaceContext->readPixels({grCT, kPremul_SkAlphaType, nullptr, kWidth, kHeight},
                                             actualPixels.writable_addr(), actualPixels.rowBytes(),
                                             {0, 0}, context);

    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, does_full_buffer_contain_correct_color(expectedPixels.addr32(),
                                                                     actualPixels.addr32(),
                                                                     kWidth, kHeight));
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrTestingBackendTextureUploadTest, reporter, ctxInfo) {
    for (auto colorType: { kRGBA_8888_SkColorType, kBGRA_8888_SkColorType }) {
        for (auto renderable: { GrRenderable::kYes, GrRenderable::kNo }) {
            for (bool doDataUpload: {true, false}) {
                testing_only_texture_test(reporter, ctxInfo.grContext(), colorType,
                                          renderable, doDataUpload, GrMipMapped::kNo);

                if (!doDataUpload) {
                    testing_only_texture_test(reporter, ctxInfo.grContext(), colorType,
                                              renderable, doDataUpload, GrMipMapped::kYes);
                }
            }
        }
    }
}

