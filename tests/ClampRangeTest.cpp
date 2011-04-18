#include "Test.h"
#include "SkClampRange.h"
#include "SkRandom.h"

static skiatest::Reporter* gReporter;

static int classify_value(SkFixed fx, int v0, int v1) {
    if (fx <= 0) {
        return v0;
    }
    if (fx >= 0xFFFF) {
        return v1;
    }
    REPORTER_ASSERT(gReporter, false);
    return 0;
}

#define V0  -42
#define V1  1024

static void slow_check(const SkClampRange& range,
                       SkFixed fx, SkFixed dx, int count) {
    SkASSERT(range.fCount0 + range.fCount1 + range.fCount2 == count);
    
    int i;
    for (i = 0; i < range.fCount0; i++) {
        int v = classify_value(fx, V0, V1);
        REPORTER_ASSERT(gReporter, v == range.fV0);
        fx += dx;
    }
    REPORTER_ASSERT(gReporter, range.fCount1 == 0 || fx == range.fFx1);
    for (i = 0; i < range.fCount1; i++) {
        REPORTER_ASSERT(gReporter, fx >= 0 && fx <= 0xFFFF);
        fx += dx;
    }
    for (i = 0; i < range.fCount2; i++) {
        int v = classify_value(fx, V0, V1);
        REPORTER_ASSERT(gReporter, v == range.fV1);
        fx += dx;
    }
}

static void test_range(SkFixed fx, SkFixed dx, int count) {
    SkClampRange range;
    range.init(fx, dx, count, V0, V1);
    slow_check(range, fx, dx, count);
}

#define ff(x)   SkIntToFixed(x)

static void TestClampRange(skiatest::Reporter* reporter) {
    gReporter = reporter;

    test_range(0, 0, 20);
    test_range(0xFFFF, 0, 20);
    test_range(-ff(2), 0, 20);
    test_range( ff(2), 0, 20);
    
    test_range(-10, 1, 20);
    test_range(10, -1, 20);
    test_range(-10, 3, 20);
    test_range(10, -3, 20);
    
    SkRandom rand;
    
    // test non-overflow cases
    for (int i = 0; i < 1000000; i++) {
        SkFixed fx = rand.nextS() >> 1;
        SkFixed sx = rand.nextS() >> 1;
        int count = rand.nextU() % 1000 + 1;
        SkFixed dx = (sx - fx) / count;
        test_range(fx, dx, count);
    }
    
    // test overflow cases
    for (int i = 0; i < 0*1000000; i++) {
        SkFixed fx = rand.nextS();
        SkFixed sx = rand.nextS();
        int count = rand.nextU() % 1000 + 1;
        SkFixed dx = (sx - fx) / count;
        test_range(fx, dx, count);
    }
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("ClampRange", ClampRangeClass, TestClampRange)
