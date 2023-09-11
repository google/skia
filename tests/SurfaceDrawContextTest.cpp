/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test.

#include "include/core/SkColorSpace.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrDirectContext.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrPixmap.h"
#include "src/gpu/ganesh/GrRenderTargetProxy.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

#include <memory>

class GrRecordingContext;
struct GrContextOptions;

static const int kSize = 64;

static std::unique_ptr<skgpu::ganesh::SurfaceDrawContext> get_sdc(GrRecordingContext* rContext) {
    return skgpu::ganesh::SurfaceDrawContext::Make(rContext,
                                                   GrColorType::kRGBA_8888,
                                                   nullptr,
                                                   SkBackingFit::kExact,
                                                   {kSize, kSize},
                                                   SkSurfaceProps(),
                                                   /*label=*/{});
}

static void check_instantiation_status(skiatest::Reporter* reporter,
                                       skgpu::ganesh::SurfaceDrawContext* sdc,
                                       bool wrappedExpectation) {
    REPORTER_ASSERT(reporter, sdc->asRenderTargetProxy()->isInstantiated() == wrappedExpectation);

    GrTextureProxy* tProxy = sdc->asTextureProxy();
    REPORTER_ASSERT(reporter, tProxy);

    REPORTER_ASSERT(reporter, tProxy->isInstantiated() == wrappedExpectation);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SurfaceDrawContextTest,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();

    // Calling instantiate on a SurfaceDrawContext's textureProxy also instantiates the
    // SurfaceDrawContext
    {
        auto sdc = get_sdc(dContext);

        check_instantiation_status(reporter, sdc.get(), false);

        GrTextureProxy* tProxy = sdc->asTextureProxy();
        REPORTER_ASSERT(reporter, tProxy);

        REPORTER_ASSERT(reporter, tProxy->instantiate(dContext->priv().resourceProvider()));

        check_instantiation_status(reporter, sdc.get(), true);
    }

    // readPixels switches a deferred sdCtx to wrapped
    {
        auto sdc = get_sdc(dContext);

        check_instantiation_status(reporter, sdc.get(), false);

        SkImageInfo dstInfo = SkImageInfo::MakeN32Premul(kSize, kSize);
        GrPixmap dstPM = GrPixmap::Allocate(dstInfo);

        bool result = sdc->readPixels(dContext, dstPM, {0, 0});
        REPORTER_ASSERT(reporter, result);

        check_instantiation_status(reporter, sdc.get(), true);
    }

    // TODO: in a future world we should be able to add a test that the majority of
    // SurfaceDrawContext calls do not force the instantiation of a deferred
    // SurfaceDrawContext
}
