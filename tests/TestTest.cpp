/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/gpu/GrDirectContext.h"
#include "tools/gpu/gl/GLTestContext.h"

// This is an example of a normal test.
DEF_TEST(TestNormal, reporter) {
    REPORTER_ASSERT(reporter, reporter);
}

// This is an example of a conditional test with a true condition.
DEF_CONDITIONAL_TEST(TestTrueCondition, reporter, 1 == 1) {
    REPORTER_ASSERT(reporter, reporter);
}

// This is an example of a conditional test with a false condition.
DEF_CONDITIONAL_TEST(TestFalseCondition, reporter, 1 == 0) {
    ERRORF(reporter, "DEF_CONDITIONAL_TEST executed a test with a false condition");
}

// This is an example of a GPU test that uses GrContextOptions to do the test.
DEF_GPUTEST(TestGpuFactory, reporter, factory) {
    REPORTER_ASSERT(reporter, reporter);
}

// This is an example of a GPU test that tests a property that should work for all GPU contexts.
// Note: Some of the contexts might not produce a rendering output.
DEF_GPUTEST_FOR_ALL_CONTEXTS(TestGpuAllContexts, reporter, ctxInfo) {
    REPORTER_ASSERT(reporter, reporter);
    REPORTER_ASSERT(reporter, ctxInfo.directContext());
}

// This is an example of a GPU test that tests a property that should work for all GPU contexts that
// produce a rendering output.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(TestGpuRenderingContexts, reporter, ctxInfo) {
    REPORTER_ASSERT(reporter, reporter);
    REPORTER_ASSERT(reporter, ctxInfo.directContext());
}

// This is an example of a GPU test that tests a property that uses the mock context.  It should
// be used if the test tests some behavior that is mocked with the mock context.
DEF_GPUTEST_FOR_MOCK_CONTEXT(TestMockContext, reporter, ctxInfo) {
    REPORTER_ASSERT(reporter, reporter);
    REPORTER_ASSERT(reporter, ctxInfo.directContext());
}

// Conditional GPU tests will only execute if the provided condition parameter is true.
DEF_CONDITIONAL_GPUTEST_FOR_ALL_CONTEXTS(TestGpuAllContextsWithTrueCondition,
                                         reporter, ctxInfo, true) {
    REPORTER_ASSERT(reporter, reporter);
    REPORTER_ASSERT(reporter, ctxInfo.directContext());
}

DEF_CONDITIONAL_GPUTEST_FOR_ALL_CONTEXTS(TestGpuAllContextsWithFalseCondition,
                                         reporter, ctxInfo, false) {
    ERRORF(reporter, "DEF_CONDITIONAL_GPUTEST_FOR_ALL_CONTEXTS ran with a false condition");
}

DEF_CONDITIONAL_GPUTEST_FOR_RENDERING_CONTEXTS(TestGpuRenderingContextsWithTrueCondition,
                                               reporter, ctxInfo, true) {
    REPORTER_ASSERT(reporter, reporter);
    REPORTER_ASSERT(reporter, ctxInfo.directContext());
}

DEF_CONDITIONAL_GPUTEST_FOR_RENDERING_CONTEXTS(TestGpuRenderingContextsWithFalseCondition,
                                               reporter, ctxInfo, false) {
    ERRORF(reporter, "DEF_CONDITIONAL_GPUTEST_FOR_RENDERING_CONTEXTS ran with a false condition");
}

