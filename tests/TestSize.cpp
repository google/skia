#include "Test.h"
#include "SkSize.h"

static void TestISize(skiatest::Reporter* reporter) {
    SkISize  a, b;
    
    a.set(0, 0);
    REPORTER_ASSERT(reporter, a.isEmpty());
    a.set(5, -5);
    REPORTER_ASSERT(reporter, a.isEmpty());
    a.clampNegToZero();
    REPORTER_ASSERT(reporter, a.isEmpty());
    b.set(5, 0);
    REPORTER_ASSERT(reporter, a == b);
    
    a.set(3, 5);
    REPORTER_ASSERT(reporter, !a.isEmpty());
    b = a;
    REPORTER_ASSERT(reporter, !b.isEmpty());
    REPORTER_ASSERT(reporter, a == b);
    REPORTER_ASSERT(reporter, !(a != b));
    REPORTER_ASSERT(reporter,
                    a.fWidth == b.fWidth && a.fHeight == b.fHeight);
}

static void TestSize(skiatest::Reporter* reporter) {
    TestISize(reporter);
    
    SkSize a, b;
    int ix = 5;
    int iy = 3;
    SkScalar x = SkIntToScalar(ix);
    SkScalar y = SkIntToScalar(iy);
    
    a.set(0, 0);
    REPORTER_ASSERT(reporter, a.isEmpty());
    a.set(x, -x);
    REPORTER_ASSERT(reporter, a.isEmpty());
    a.clampNegToZero();
    REPORTER_ASSERT(reporter, a.isEmpty());
    b.set(x, 0);
    REPORTER_ASSERT(reporter, a == b);
    
    a.set(y, x);
    REPORTER_ASSERT(reporter, !a.isEmpty());
    b = a;
    REPORTER_ASSERT(reporter, !b.isEmpty());
    REPORTER_ASSERT(reporter, a == b);
    REPORTER_ASSERT(reporter, !(a != b));
    REPORTER_ASSERT(reporter,
                    a.fWidth == b.fWidth && a.fHeight == b.fHeight);
    
    SkISize ia;
    ia.set(ix, iy);
    a.set(x, y);
    REPORTER_ASSERT(reporter, a.round() == ia);
};

#include "TestClassDef.h"
DEFINE_TESTCLASS("Size", TestSizeClass, TestSize)
