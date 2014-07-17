/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#if SK_SUPPORT_GPU

#include "GrContextFactory.h"
#include "Test.h"

DEF_GPUTEST(GrContextFactory, reporter, factory) {
    // Reset in case some other test has been using it first.
    factory->destroyContexts();

    // Before we ask for a context, we expect the GL context to not be there.
    REPORTER_ASSERT(reporter,
                    NULL == factory->getGLContext(GrContextFactory::kNull_GLContextType));

    // After we ask for a context, we expect that the GL context to be there.
    factory->get(GrContextFactory::kNull_GLContextType);
    REPORTER_ASSERT(reporter,
                    factory->getGLContext(GrContextFactory::kNull_GLContextType) != NULL);

    // If we did not ask for a context with the particular GL context, we would
    // expect the particular GL context to not be there.
    REPORTER_ASSERT(reporter,
                    NULL == factory->getGLContext(GrContextFactory::kDebug_GLContextType));
}

#endif
