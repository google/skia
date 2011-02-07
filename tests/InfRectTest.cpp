#include "Test.h"
#include "SkRect.h"

// Tests that hasValidCoordinates() will reject any rect with +/-inf values
// as one of its coordinates.
static void TestInfRect(skiatest::Reporter* reporter) {

    SkRect rect = SkRect::MakeXYWH(10.0f, 10.0f, 100.0f, 100.0f);
    REPORTER_ASSERT(reporter, rect.hasValidCoordinates());

    rect = SkRect::MakeXYWH(10.0f, 10.0f, 100.0f, 1.0f/0.0f); // Make 'inf' value without numeric_limits.
    REPORTER_ASSERT(reporter, !rect.hasValidCoordinates());

    rect = SkRect::MakeXYWH(10.0f, 10.0f, 1.0f/0.0f, 100.0f);
    REPORTER_ASSERT(reporter, !rect.hasValidCoordinates());

    rect = SkRect::MakeXYWH(1.0f/0.0f, 10.0f, 100.0f, 100.0f);
    REPORTER_ASSERT(reporter, !rect.hasValidCoordinates());

    rect = SkRect::MakeXYWH(10.0f, 1.0f/0.0f, 100.0f, 100.0f);
    REPORTER_ASSERT(reporter, !rect.hasValidCoordinates());

    rect = SkRect::MakeXYWH(10.0f, 10.0f, 100.0f, -1.0f/0.0f);
    REPORTER_ASSERT(reporter, !rect.hasValidCoordinates());

    rect = SkRect::MakeXYWH(10.0f, 10.0f, -1.0f/0.0f, 100.0f);
    REPORTER_ASSERT(reporter, !rect.hasValidCoordinates());

    rect = SkRect::MakeXYWH(-1.0f/0.0f, 10.0f, 100.0f, 100.0f);
    REPORTER_ASSERT(reporter, !rect.hasValidCoordinates());

    rect = SkRect::MakeXYWH(10.0f, -1.0f/0.0f, 100.0f, 100.0f);
    REPORTER_ASSERT(reporter, !rect.hasValidCoordinates());
}

// need tests for SkStrSearch

#include "TestClassDef.h"
DEFINE_TESTCLASS("InfRect", InfRectTestClass, TestInfRect)
