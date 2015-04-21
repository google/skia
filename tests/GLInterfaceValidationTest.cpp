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

DEF_GPUTEST(GLInterfaceValidation, reporter, factory) {
    for (int i = 0; i <= GrContextFactory::kLastGLContextType; ++i) {
        GrContextFactory::GLContextType glCtxType = (GrContextFactory::GLContextType)i;
        // this forces the factory to make the context if it hasn't yet
        factory->get(glCtxType);
        SkGLContext* glCtx = factory->getGLContext(glCtxType);

        // We're supposed to fail the NVPR context type when we the native context that does not
        // support the NVPR extension.
        if (GrContextFactory::kNVPR_GLContextType == glCtxType &&
            factory->getGLContext(GrContextFactory::kNative_GLContextType) &&
            !factory->getGLContext(GrContextFactory::kNative_GLContextType)->gl()->hasExtension("GL_NV_path_rendering")) {
            REPORTER_ASSERT(reporter, NULL == glCtx);
            continue;
        }

        REPORTER_ASSERT(reporter, glCtx);
        if (glCtx) {
            const GrGLInterface* interface = glCtx->gl();
            REPORTER_ASSERT(reporter, interface->validate());
        }
    }
}

#endif
