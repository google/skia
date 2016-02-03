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
    // Test that if NVPR is possible, caps are in sync.
    for (int i = 0; i < GrContextFactory::kGLContextTypeCnt; ++i) {
        GrContextFactory::GLContextType glCtxType = static_cast<GrContextFactory::GLContextType>(i);
        GrContext* context = testFactory.get(glCtxType,
                                             GrContextFactory::kEnableNVPR_GLContextOptions);
        if (!context) {
            continue;
        }
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

DEF_GPUTEST(GrContextFactory_abandon, reporter, /*factory*/) {
    GrContextFactory testFactory;
    for (int i = 0; i < GrContextFactory::kGLContextTypeCnt; ++i) {
        GrContextFactory::GLContextType glCtxType = (GrContextFactory::GLContextType) i;
        GrContextFactory::ContextInfo info1 =
                testFactory.getContextInfo(glCtxType);
        if (!info1.fGrContext) {
            continue;
        }
        REPORTER_ASSERT(reporter, info1.fGLContext);
         // Ref for comparison. The API does not explicitly say that this stays alive.
        info1.fGrContext->ref();
        testFactory.abandonContexts();

        // Test that we get different context after abandon.
        GrContextFactory::ContextInfo info2 =
                testFactory.getContextInfo(glCtxType);
        REPORTER_ASSERT(reporter, info2.fGrContext);
        REPORTER_ASSERT(reporter, info2.fGLContext);
        REPORTER_ASSERT(reporter, info1.fGrContext != info2.fGrContext);
        // fGLContext should also change, but it also could get the same address.

        info1.fGrContext->unref();
    }
}

#endif
