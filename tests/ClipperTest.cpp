#include "Test.h"
#include "SkPath.h"
#include "SkLineClipper.h"
#include "SkEdgeClipper.h"

static void test_intersectline(skiatest::Reporter* reporter) {
    static const SkScalar L = 0;
    static const SkScalar T = 0;
    static const SkScalar R = SkIntToScalar(100);
    static const SkScalar B = SkIntToScalar(100);
    static const SkScalar CX = SkScalarHalf(L + R);
    static const SkScalar CY = SkScalarHalf(T + B);
    static const SkRect gR = { L, T, R, B };

    size_t i;
    SkPoint dst[2];

    static const SkPoint gEmpty[] = {
        // sides
        { L, CY }, { L - 10, CY },
        { R, CY }, { R + 10, CY },
        { CX, T }, { CX, T - 10 },
        { CX, B }, { CX, B + 10 },
        // corners
        { L, T }, { L - 10, T - 10 },
        { L, B }, { L - 10, B + 10 },
        { R, T }, { R + 10, T - 10 },
        { R, B }, { R + 10, B + 10 },
    };
    for (i = 0; i < SK_ARRAY_COUNT(gEmpty); i += 2) {
        bool valid = SkLineClipper::IntersectLine(&gEmpty[i], gR, dst);
        if (valid) {
            SkDebugf("----- [%d] %g %g -> %g %g\n", i/2, dst[0].fX, dst[0].fY, dst[1].fX, dst[1].fY);
        }
        REPORTER_ASSERT(reporter, !valid);
    }
    
    static const SkPoint gFull[] = {
        // diagonals, chords
        { L, T }, { R, B },
        { L, B }, { R, T },
        { CX, T }, { CX, B },
        { L, CY }, { R, CY },
        { CX, T }, { R, CY },
        { CX, T }, { L, CY },
        { L, CY }, { CX, B },
        { R, CY }, { CX, B },
        // edges
        { L, T }, { L, B },
        { R, T }, { R, B },
        { L, T }, { R, T },
        { L, B }, { R, B },
    };
    for (i = 0; i < SK_ARRAY_COUNT(gFull); i += 2) {
        bool valid = SkLineClipper::IntersectLine(&gFull[i], gR, dst);
        if (!valid || memcmp(&gFull[i], dst, sizeof(dst))) {
            SkDebugf("++++ [%d] %g %g -> %g %g\n", i/2, dst[0].fX, dst[0].fY, dst[1].fX, dst[1].fY);
        }
        REPORTER_ASSERT(reporter, valid && !memcmp(&gFull[i], dst, sizeof(dst)));
    }
    
    static const SkPoint gPartial[] = {
        { L - 10, CY }, { CX, CY }, { L, CY }, { CX, CY },
        { CX, T - 10 }, { CX, CY }, { CX, T }, { CX, CY },
        { R + 10, CY }, { CX, CY }, { R, CY }, { CX, CY },
        { CX, B + 10 }, { CX, CY }, { CX, B }, { CX, CY },
        // extended edges
        { L, T - 10 }, { L, B + 10 }, { L, T }, { L, B },
        { R, T - 10 }, { R, B + 10 }, { R, T }, { R, B },
        { L - 10, T }, { R + 10, T }, { L, T }, { R, T },
        { L - 10, B }, { R + 10, B }, { L, B }, { R, B },
    };
    for (i = 0; i < SK_ARRAY_COUNT(gPartial); i += 4) {
        bool valid = SkLineClipper::IntersectLine(&gPartial[i], gR, dst);
        if (!valid || memcmp(&gPartial[i+2], dst, sizeof(dst))) {
            SkDebugf("++++ [%d] %g %g -> %g %g\n", i/2, dst[0].fX, dst[0].fY, dst[1].fX, dst[1].fY);
        }
        REPORTER_ASSERT(reporter, valid &&
                                  !memcmp(&gPartial[i+2], dst, sizeof(dst)));
    }
    
}

void TestClipper(skiatest::Reporter* reporter) {
    test_intersectline(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Clipper", TestClipperClass, TestClipper)
