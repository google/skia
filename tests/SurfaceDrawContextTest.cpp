/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test.

#include "tests/Test.h"

#include "include/gpu/GrDirectContext.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/GrTextureProxy.h"

static const int kSize = 64;

static std::unique_ptr<GrSurfaceDrawContext> get_sdc(GrRecordingContext* rContext) {
    return GrSurfaceDrawContext::Make(rContext, GrColorType::kRGBA_8888, nullptr,
                                      SkBackingFit::kExact, {kSize, kSize}, SkSurfaceProps());
}

static void check_instantiation_status(skiatest::Reporter* reporter,
                                       GrSurfaceDrawContext* sdCtx,
                                       bool wrappedExpectation) {
    REPORTER_ASSERT(reporter, sdCtx->asRenderTargetProxy()->isInstantiated() == wrappedExpectation);

    GrTextureProxy* tProxy = sdCtx->asTextureProxy();
    REPORTER_ASSERT(reporter, tProxy);

    REPORTER_ASSERT(reporter, tProxy->isInstantiated() == wrappedExpectation);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SurfaceDrawContextTest, reporter, ctxInfo) {
    auto dContext = ctxInfo.directContext();

    // Calling instantiate on a GrSurfaceDrawContext's textureProxy also instantiates the
    // GrSurfaceDrawContext
    {
        auto sdCtx = get_sdc(dContext);

        check_instantiation_status(reporter, sdCtx.get(), false);

        GrTextureProxy* tProxy = sdCtx->asTextureProxy();
        REPORTER_ASSERT(reporter, tProxy);

        REPORTER_ASSERT(reporter, tProxy->instantiate(dContext->priv().resourceProvider()));

        check_instantiation_status(reporter, sdCtx.get(), true);
    }

    // readPixels switches a deferred sdCtx to wrapped
    {
        auto sdCtx = get_sdc(dContext);

        check_instantiation_status(reporter, sdCtx.get(), false);

        SkImageInfo dstInfo = SkImageInfo::MakeN32Premul(kSize, kSize);
        GrPixmap dstPM = GrPixmap::Allocate(dstInfo);

        bool result = sdCtx->readPixels(dContext, dstPM, {0, 0});
        REPORTER_ASSERT(reporter, result);

        check_instantiation_status(reporter, sdCtx.get(), true);
    }

    // TODO: in a future world we should be able to add a test that the majority of
    // GrSurfaceDrawContext calls do not force the instantiation of a deferred
    // GrSurfaceDrawContext
}
