/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#if 1 //SK_SUPPORT_GPU

#include "GrSurfaceProxy.h"
#include "GrTextureProducer.h"
#include "GrTextureProxy.h"

class GrTextureProducer_TestAccess {
public:
    typedef GrTextureProducer::DomainMode DomainMode;

    static DomainMode DetermineDomainMode(
                                const SkRect& constraintRect,
                                GrTextureProducer::FilterConstraint filterConstraint,
                                bool coordsLimitedToConstraintRect,
                                int texW, int texH,
                                const SkIRect* textureContentArea,
                                const GrSamplerParams::FilterMode* filterModeOrNullForBicubic,
                                SkRect* domainRect) {
        return GrTextureProducer::DetermineDomainMode(constraintRect,
                                                      filterConstraint,
                                                      coordsLimitedToConstraintRect,
                                                      texW, texH,
                                                      textureContentArea,
                                                      filterModeOrNullForBicubic,
                                                      domainRect);
    }

    static DomainMode DetermineDomainMode(
                                const SkRect& constraintRect,
                                GrTextureProducer::FilterConstraint filterConstraint,
                                bool coordsLimitedToConstraintRect,
                                GrTextureProxy* proxy,
                                const SkIRect* textureContentArea,
                                const GrSamplerParams::FilterMode* filterModeOrNullForBicubic,
                                SkRect* domainRect) {
        return GrTextureProducer::DetermineDomainMode(constraintRect,
                                                      filterConstraint,
                                                      coordsLimitedToConstraintRect,
                                                      proxy,
                                                      textureContentArea,
                                                      filterModeOrNullForBicubic,
                                                      domainRect);    
    }
};



static void texture_test(skiatest::Reporter* reporter, GrResourceProvider* resourceProvider) {
}

static void proxy_test(skiatest::Reporter* reporter, GrResourceProvider* resourceProvider) {
    GrSurfaceDesc desc;

    sk_sp<GrTextureProxy> proxy = GrSurfaceProxy::MakeDeferred(resourceProvider,
                                                               desc, SkBackingFit::kExact,
                                                               SkBudgeted::kYes);

    GrTextureProducer_TestAccess::DomainMode actual;

    SkRect constraintRect;
    GrTextureProducer::FilterConstraint filterConstraint;
    bool coordsLimitedToConstraintRect;
    SkIRect textureContextArea;
    GrSamplerParams::FilterMode filterModeOrNullForBicubic;
    SkRect domainRect;

    actual = GrTextureProducer_TestAccess::DetermineDomainMode(constraintRect,
                                                               filterConstraint,
                                                               coordsLimitedToConstraintRect,
                                                               proxy.get(),
                                                               &textureContextArea,
                                                               &filterModeOrNullForBicubic,
                                                               &domainRect);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(DetermineDomainMode, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    texture_test(reporter, context->resourceProvider());
    proxy_test(reporter, context->resourceProvider());
}

#endif
