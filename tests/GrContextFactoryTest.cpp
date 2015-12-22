/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#if SK_SUPPORT_GPU

#include "GrContextFactory.h"
#include "GrCaps.h"
#include "Test.h"

DEF_GPUTEST(GrContextFactory_NVPRContextOptionHasPathRenderingSupport, reporter, /*factory*/) {
    // Test that if NVPR is requested, the context always has path rendering
    // or the context creation fails.
    GrContextFactory testFactory;
    GrContext* context = testFactory.get(GrContextFactory::kNative_GLContextType,
                                         kNone_GrGLStandard,
                                         GrContextFactory::kEnableNVPR_GLContextOptions);
    if (context) {
        REPORTER_ASSERT(
            reporter,
            context->caps()->shaderCaps()->pathRenderingSupport());
    }
}

DEF_GPUTEST(GrContextFactory_NoPathRenderingUnlessNVPRRequested, reporter, /*factory*/) {
    // Test that if NVPR is not requested, the context never has path rendering support.

    GrContextFactory testFactory;
    for (int i = 0; i <= GrContextFactory::kLastGLContextType; ++i) {
        GrContextFactory::GLContextType glCtxType = (GrContextFactory::GLContextType)i;
        GrContext* context = testFactory.get(glCtxType);
        if (context) {
            REPORTER_ASSERT(
                reporter,
                !context->caps()->shaderCaps()->pathRenderingSupport());
        }
    }
}

#endif
