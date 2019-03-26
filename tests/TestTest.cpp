/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#include "GrContext.h"
#include "gl/GLTestContext.h"

// This is an example of a normal test.
DEF_TEST(TestNormal, reporter) {
    REPORTER_ASSERT(reporter, reporter);
}

// This is an example of a GPU test that uses GrContextOptions to do the test.
DEF_GPUTEST(TestGpuFactory, reporter, factory) {
    REPORTER_ASSERT(reporter, reporter);
}

// This is an example of a GPU test that tests a property that should work for all GPU contexts.
// Note: Some of the contexts might not produce a rendering output.
DEF_GPUTEST_FOR_ALL_CONTEXTS(TestGpuAllContexts, reporter, ctxInfo) {
    REPORTER_ASSERT(reporter, reporter);
    REPORTER_ASSERT(reporter, ctxInfo.grContext());
}

// This is an example of a GPU test that tests a property that should work for all GPU contexts that
// produce a rendering output.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(TestGpuRenderingContexts, reporter, ctxInfo) {
    REPORTER_ASSERT(reporter, reporter);
    REPORTER_ASSERT(reporter, ctxInfo.grContext());
}

// This is an example of a GPU test that tests a property that uses the mock context.  It should
// be used if the test tests some behavior that is mocked with the mock context.
DEF_GPUTEST_FOR_MOCK_CONTEXT(TestMockContext, reporter, ctxInfo) {
    REPORTER_ASSERT(reporter, reporter);
    REPORTER_ASSERT(reporter, ctxInfo.grContext());
}
