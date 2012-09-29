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

static void test_context_factory(skiatest::Reporter* reporter) {
    GrContextFactory contextFactory;

    // Before we ask for a context, we expect the GL context to not be there.
    REPORTER_ASSERT(reporter,
                    NULL == contextFactory.getGLContext(GrContextFactory::kNative_GLContextType));

    // After we ask for a context, we expect that the GL context to be there.
    contextFactory.get(GrContextFactory::kNative_GLContextType);
    REPORTER_ASSERT(reporter,
                    contextFactory.getGLContext(GrContextFactory::kNative_GLContextType) != NULL);

    // If we did not ask for a context with the particular GL context, we would
    // expect the particular GL context to not be there.
    REPORTER_ASSERT(reporter,
                    NULL == contextFactory.getGLContext(GrContextFactory::kNull_GLContextType));
}


#include "TestClassDef.h"
DEFINE_TESTCLASS("GrContextFactory", GrContextFactoryClass, test_context_factory);

#endif
