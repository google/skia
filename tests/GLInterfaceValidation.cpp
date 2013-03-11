
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

static void GLInterfaceValidationTest(skiatest::Reporter* reporter, GrContextFactory* factory) {
    for (int i = 0; i <= GrContextFactory::kLastGLContextType; ++i) {
        GrContextFactory::GLContextType glCtxType = (GrContextFactory::GLContextType)i;
        // this forces the factory to make the context if it hasn't yet
        factory->get(glCtxType);
        SkGLContextHelper* glCtxHelper = factory->getGLContext(glCtxType);
        REPORTER_ASSERT(reporter, NULL != glCtxHelper);
        if (NULL != glCtxHelper) {
            const GrGLInterface* interface = glCtxHelper->gl();
            for (GrGLBinding binding = kFirstGrGLBinding;
                 binding <= kLastGrGLBinding;
                 binding = static_cast<GrGLBinding>(binding << 1)) {
                if (interface->fBindingsExported & binding) {
                    REPORTER_ASSERT(reporter, interface->validate(binding));
                }
            }
        }
    }
}


#include "TestClassDef.h"
DEFINE_GPUTESTCLASS("GLInterfaceValidation",
                    GLInterfaceValidationTestClass,
                    GLInterfaceValidationTest)

#endif
