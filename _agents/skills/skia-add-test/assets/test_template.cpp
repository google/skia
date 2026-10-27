/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

// Basic CPU test
DEF_TEST(MyNewCPUTest, reporter) {
    REPORTER_ASSERT(reporter, true);

    // skiatest::ReporterContext helps identify which subtest failed by pushing a message
    // onto the reporter's context stack.
    {
        skiatest::ReporterContext subtest(reporter, SkString("Subtest Name"));
        REPORTER_ASSERT(reporter, 1 + 1 == 2);
    }
}

#if defined(SK_GANESH)
// Ganesh GPU test.
// CtsEnforcement helps track which tests must pass for Android's conformance suite (CTS).
// kNever means this test is not part of CTS.
DEF_GANESH_TEST(MyNewGaneshTest, reporter, options, CtsEnforcement::kNever) {
    REPORTER_ASSERT(reporter, true);
}
#endif

#if defined(SK_GRAPHITE)
// Graphite GPU test
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(MyNewGraphiteTest, reporter, context, CtsEnforcement::kNever) {
    REPORTER_ASSERT(reporter, context);
}
#endif
