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

DEF_GPUTEST(GrContextFactoryNVPRContextOptions, reporter, /*factory*/) {
    GrContextFactory testFactory;
    // Test that if NVPR is possible, caps are in sync.
    for (int i = 0; i < GrContextFactory::kGLContextTypeCnt; ++i) {
        GrContextFactory::GLContextType glCtxType = static_cast<GrContextFactory::GLContextType>(i);
        GrContext* context = testFactory.get(glCtxType,
                                             kNone_GrGLStandard,
                                             GrContextFactory::kEnableNVPR_GLContextOptions);
        if (!context) {
            continue;
        }
        REPORTER_ASSERT(
                    reporter,
                    context->caps()->shaderCaps()->pathRenderingSupport());
    }
}

#endif
