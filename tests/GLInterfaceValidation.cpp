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
        SkGLContextHelper* glCtxHelper = factory->getGLContext(glCtxType);
        REPORTER_ASSERT(reporter, NULL != glCtxHelper);
        if (NULL != glCtxHelper) {
            const GrGLInterface* interface = glCtxHelper->gl();
            REPORTER_ASSERT(reporter, interface->validate());
        }
    }
}

#endif
