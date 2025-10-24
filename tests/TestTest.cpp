/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

struct GrContextOptions;

// This is an example of a normal test. It should not require any GPU backends, that is, it is a
// CPU test.
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

// This is an example of a GPU test that runs if any Ganesh backend is compiled in. The test itself
// is responsible for making the relevant GrDirectContext (e.g. using
// sk_gpu_test::GrContextFactory).
DEF_GANESH_TEST(TestGpuFactory, reporter, factory, CtsEnforcement::kNever) {
    REPORTER_ASSERT(reporter, reporter);
}

// This is an example of a GPU test that tests a property that should work for all Ganesh contexts.
// Note: Some of the contexts might not produce a rendering output.
DEF_GANESH_TEST_FOR_ALL_CONTEXTS(TestGpuAllContexts, reporter, ctxInfo, CtsEnforcement::kNever) {
    REPORTER_ASSERT(reporter, reporter);
    REPORTER_ASSERT(reporter, ctxInfo.directContext());
}

// This is an example of a GPU test that tests a property that should work for all Ganesh contexts
// that produce a rendering output.
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(TestGpuRenderingContexts,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    REPORTER_ASSERT(reporter, reporter);
    REPORTER_ASSERT(reporter, ctxInfo.directContext());
}

// This is an example of a GPU test that tests a property that uses the mock Ganesh context.
// It should be used if the test tests some behavior that is mocked with the mock context.
DEF_GANESH_TEST_FOR_MOCK_CONTEXT(TestMockContext, reporter, ctxInfo) {
    REPORTER_ASSERT(reporter, reporter);
    REPORTER_ASSERT(reporter, ctxInfo.directContext());
}

// Conditional variant of DEF_GANESH_TEST_FOR_ALL_CONTEXTS. It will only execute if the provided
// condition parameter is true.
DEF_CONDITIONAL_GANESH_TEST_FOR_ALL_CONTEXTS(
        TestGpuAllContextsWithTrueCondition, reporter, ctxInfo, true, CtsEnforcement::kNever) {
    REPORTER_ASSERT(reporter, reporter);
    REPORTER_ASSERT(reporter, ctxInfo.directContext());
}

DEF_CONDITIONAL_GANESH_TEST_FOR_ALL_CONTEXTS(
        TestGpuAllContextsWithFalseCondition, reporter, ctxInfo, false, CtsEnforcement::kNever) {
    ERRORF(reporter, "DEF_CONDITIONAL_GANESH_TEST_FOR_ALL_CONTEXTS ran with a false condition");
}

// Conditional variant of DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS. It will only execute if the
// provided condition parameter is true.
DEF_CONDITIONAL_GANESH_TEST_FOR_RENDERING_CONTEXTS(TestGpuRenderingContextsWithTrueCondition,
                                                   reporter,
                                                   ctxInfo,
                                                   true,
                                                   CtsEnforcement::kNever) {
    REPORTER_ASSERT(reporter, reporter);
    REPORTER_ASSERT(reporter, ctxInfo.directContext());
}

DEF_CONDITIONAL_GANESH_TEST_FOR_RENDERING_CONTEXTS(TestGpuRenderingContextsWithFalseCondition,
                                                   reporter,
                                                   ctxInfo,
                                                   false,
                                                   CtsEnforcement::kNever) {
    ERRORF(reporter,
           "DEF_CONDITIONAL_GANESH_TEST_FOR_RENDERING_CONTEXTS ran "
           "with a false condition");
}

DEF_TEST(TestCtsEnforcement, reporter) {
    auto verifyRunMode = [&](CtsEnforcement e, int apiLevel, CtsEnforcement::RunMode runMode) {
        REPORTER_ASSERT(reporter, e.eval(apiLevel) == runMode);
    };

    CtsEnforcement e1 = CtsEnforcement::kNever;
    verifyRunMode(e1, 0, CtsEnforcement::RunMode::kSkip);
    verifyRunMode(e1, CtsEnforcement::kApiLevel_T, CtsEnforcement::RunMode::kSkip);
    verifyRunMode(e1, CtsEnforcement::kNextRelease, CtsEnforcement::RunMode::kSkip);

    CtsEnforcement e2 = CtsEnforcement::kApiLevel_T;
    verifyRunMode(e2, 0, CtsEnforcement::RunMode::kSkip);
    verifyRunMode(e2, CtsEnforcement::kApiLevel_T, CtsEnforcement::RunMode::kRunStrict);
    verifyRunMode(e2, CtsEnforcement::kNextRelease, CtsEnforcement::RunMode::kRunStrict);

    CtsEnforcement e3 = CtsEnforcement::kNextRelease;
    verifyRunMode(e3, 0, CtsEnforcement::RunMode::kSkip);
    verifyRunMode(e3, CtsEnforcement::kApiLevel_T, CtsEnforcement::RunMode::kSkip);
    verifyRunMode(e3, CtsEnforcement::kNextRelease, CtsEnforcement::RunMode::kRunStrict);

    CtsEnforcement e4 = CtsEnforcement(CtsEnforcement::kNextRelease)
                                .withWorkarounds(CtsEnforcement::kApiLevel_T);
    verifyRunMode(e4, 0, CtsEnforcement::RunMode::kSkip);
    verifyRunMode(e4, CtsEnforcement::kApiLevel_T, CtsEnforcement::RunMode::kRunWithWorkarounds);
    verifyRunMode(e4, CtsEnforcement::kNextRelease, CtsEnforcement::RunMode::kRunStrict);
}
