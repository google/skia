#include "Test.h"
#include "SkRegion.h"
#include "SkRandom.h"

static void rand_rect(SkIRect* rect, SkRandom& rand) {
    int bits = 6;
    int shift = 32 - bits;
    rect->set(rand.nextU() >> shift, rand.nextU() >> shift,
              rand.nextU() >> shift, rand.nextU() >> shift);
    rect->sort();
}

static bool test_rects(const SkIRect rect[], int count) {
    SkRegion rgn0, rgn1;

    for (int i = 0; i < count; i++) {
        rgn0.op(rect[i], SkRegion::kUnion_Op);
    }
    rgn1.setRects(rect, count);

    if (rgn0 != rgn1) {
        SkDebugf("\n");
        for (int i = 0; i < count; i++) {
            SkDebugf(" { %d, %d, %d, %d },\n",
                     rect[i].fLeft, rect[i].fTop,
                     rect[i].fRight, rect[i].fBottom);
        }
        SkDebugf("\n");
        return false;
    }
    return true;
}

static void TestRegion(skiatest::Reporter* reporter) {
    const SkIRect r2[] = {
        { 0, 0, 1, 1 },
        { 2, 2, 3, 3 },
    };
    REPORTER_ASSERT(reporter, test_rects(r2, SK_ARRAY_COUNT(r2)));
    
    const SkIRect rects[] = {
        { 0, 0, 1, 2 },
        { 2, 1, 3, 3 },
        { 4, 0, 5, 1 },
        { 6, 0, 7, 4 },
    };
    REPORTER_ASSERT(reporter, test_rects(rects, SK_ARRAY_COUNT(rects)));

    SkRandom rand;
    for (int i = 0; i < 1000; i++) {
        SkRegion rgn0, rgn1;

        const int N = 8;
        SkIRect rect[N];
        for (int j = 0; j < N; j++) {
            rand_rect(&rect[j], rand);
        }
        REPORTER_ASSERT(reporter, test_rects(rect, N));
    }
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Region", RegionTestClass, TestRegion)
