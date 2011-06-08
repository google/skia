#include "SkPoint.h"
#include "SkScalar.h"
#include "Test.h"

/*
   Duplicates lots of code from gpu/src/GrPathUtils.cpp
   It'd be nice not to do so, but that code's set up currently to only have a single implementation.
*/

#define MAX_COEFF_SHIFT     6
static const uint32_t MAX_POINTS_PER_CURVE = 1 << MAX_COEFF_SHIFT;

static inline int cheap_distance(SkScalar dx, SkScalar dy) {
    int idx = SkAbs32(SkScalarRound(dx));
    int idy = SkAbs32(SkScalarRound(dy));
    if (idx > idy) {
        idx += idy >> 1;
    } else {
        idx = idy + (idx >> 1);
    }
    return idx;
}

static inline int diff_to_shift(SkScalar dx, SkScalar dy) {
    int dist = cheap_distance(dx, dy);
    return (32 - SkCLZ(dist));
}

uint32_t estimatedQuadraticPointCount(const SkPoint points[], SkScalar tol) {
    int shift = diff_to_shift(points[1].fX * 2 - points[2].fX - points[0].fX,
                              points[1].fY * 2 - points[2].fY - points[0].fY);
    SkASSERT(shift >= 0);
    //SkDebugf("Quad shift %d;", shift);
    // bias to more closely approximate exact value, then clamp to zero
    shift -= 2;
    shift &= ~(shift>>31);

    if (shift > MAX_COEFF_SHIFT) {
        shift = MAX_COEFF_SHIFT;
    }
    uint32_t count = 1 << shift;
    //SkDebugf(" biased shift %d, scale %u\n", shift, count);
    return count;
}

uint32_t computedQuadraticPointCount(const SkPoint points[], SkScalar tol) {
    SkScalar d = points[1].distanceToLineSegmentBetween(points[0], points[2]);
    if (d < tol) {
       return 1;
    } else {
       int temp = SkScalarCeil(SkScalarSqrt(SkScalarDiv(d, tol)));
       uint32_t count = SkMinScalar(SkNextPow2(temp), MAX_POINTS_PER_CURVE);
       return count;
    }
}

// Curve from samplecode/SampleSlides.cpp
static const int gXY[] = {
    4, 0, 0, -4, 8, -4, 12, 0, 8, 4, 0, 4
};

static const int gSawtooth[] = {
    0, 0, 10, 10, 20, 20, 30, 10, 40, 0, 50, -10, 60, -20, 70, -10, 80, 0
};

static const int gOvalish[] = {
    0, 0, 5, 15, 20, 20, 35, 15, 40, 0
};

static const int gSharpSawtooth[] = {
    0, 0, 1, 10, 2, 0, 3, -10, 4, 0
};

// Curve crosses back over itself around 0,10
static const int gRibbon[] = {
   -4, 0, 4, 20, 0, 25, -4, 20, 4, 0
};

static bool one_d_pe(const int* array, const unsigned int count,
                     skiatest::Reporter* reporter) {
    SkPoint path [3];
    path[1] = SkPoint::Make(SkIntToScalar(array[0]), SkIntToScalar(array[1]));
    path[2] = SkPoint::Make(SkIntToScalar(array[2]), SkIntToScalar(array[3]));
    int numErrors = 0;
    for (unsigned i = 4; i < (count); i += 2) {
        path[0] = path[1];
        path[1] = path[2];
        path[2] = SkPoint::Make(SkIntToScalar(array[i]),
                                SkIntToScalar(array[i+1]));
        uint32_t computedCount =
            computedQuadraticPointCount(path, SkIntToScalar(1));
        uint32_t estimatedCount =
            estimatedQuadraticPointCount(path, SkIntToScalar(1));
        // Allow estimated to be off by a factor of two, but no more.
        if ((estimatedCount > 2 * computedCount) ||
            (computedCount > estimatedCount * 2)) {
            SkString errorDescription;
            errorDescription.printf(
                "Curve from %.2f %.2f through %.2f %.2f to %.2f %.2f "
                "computes %d, estimates %d\n",
                path[0].fX, path[0].fY, path[1].fX, path[1].fY,
                path[2].fX, path[2].fY, computedCount, estimatedCount);
            numErrors++;
            reporter->reportFailed(errorDescription);
        }
    }

    if (numErrors > 0)
        printf("%d curve segments differ\n", numErrors);
    return (numErrors == 0);
}



static void TestQuadPointCount(skiatest::Reporter* reporter) {
    one_d_pe(gXY, SK_ARRAY_COUNT(gXY), reporter);
    one_d_pe(gSawtooth, SK_ARRAY_COUNT(gSawtooth), reporter);
    one_d_pe(gOvalish, SK_ARRAY_COUNT(gOvalish), reporter);
    one_d_pe(gSharpSawtooth, SK_ARRAY_COUNT(gSharpSawtooth), reporter);
    one_d_pe(gRibbon, SK_ARRAY_COUNT(gRibbon), reporter);
}

static void TestPathCoverage(skiatest::Reporter* reporter) {
    TestQuadPointCount(reporter);

}

#include "TestClassDef.h"
DEFINE_TESTCLASS("PathCoverage", PathCoverageTestClass, TestPathCoverage)
