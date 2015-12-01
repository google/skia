/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

// This is a GPU-backend specific test
#if SK_SUPPORT_GPU

#include "GrContextFactory.h"

DEF_GPUTEST(GLInterfaceValidation, reporter, /*factory*/) {
    GrContextFactory testFactory;

    // Test that if we do not have NV_path_rendering -related GL extensions,
    // GrContextFactory::get(.., kEnableNVPR_GLContextOptions) always returns nullptr.
    for (int i = 0; i < GrContextFactory::kGLContextTypeCnt; ++i) {
        GrContextFactory::GLContextType glCtxType = static_cast<GrContextFactory::GLContextType>(i);
        GrContextFactory::ContextInfo* context =
                testFactory.getContextInfo(glCtxType, kNone_GrGLStandard,
                                           GrContextFactory::kNone_GLContextOptions);
        if (!context) {
            continue;
        }

        SkGLContext* glContext = context->fGLContext;
        REPORTER_ASSERT(reporter, glContext->gl()->validate());

        if (!(glContext->gl()->hasExtension("GL_NV_path_rendering") ||
              glContext->gl()->hasExtension("GL_CHROMIUM_path_rendering"))) {
                REPORTER_ASSERT(reporter,
                                nullptr == testFactory.getContextInfo(
                                    glCtxType,
                                    kNone_GrGLStandard,
                                    GrContextFactory::kEnableNVPR_GLContextOptions));
        }
    }

}

#endif
