#include "Test.h"
#include "../../src/core/SkConcaveToTriangles.h"
#include "SkGeometry.h"

static int GetIndexFromPoint(const SkPoint &pt,
                             int numPts, const SkPoint *pts) {
    for (int i = 0; i < numPts; ++i)
        if (pt.fX == pts[i].fX && pt.fY == pts[i].fY)
            return i;
    return -1;
}


bool gPrintTriangles = false;   // Can we set this on the command line?

static void PrintTriangles(const SkTDArray<SkPoint> &triangles,
                           int numPts, const SkPoint *pts,
                           skiatest::Reporter* reporter) {
    if (gPrintTriangles) {
        SkPoint *p = triangles.begin();
        int n = triangles.count();
        REPORTER_ASSERT(reporter, n % 3 == 0);
        n /= 3;
        printf("%d Triangles:\n{\n", n);
        for (; n-- != 0; p += 3)
            printf("    { {%.7g, %.7g}, {%.7g, %.7g}, {%.7g, %.7g} },  "
                   "// { %2d, %2d, %2d }\n",
                p[0].fX, p[0].fY,
                p[1].fX, p[1].fY,
                p[2].fX, p[2].fY,
                GetIndexFromPoint(p[0], numPts, pts),
                GetIndexFromPoint(p[1], numPts, pts),
                GetIndexFromPoint(p[2], numPts, pts));
        printf("}\n");
    }
}


static bool CompareTriangleList(size_t numTriangles,
                                const float refTriangles[][3][2],
                                const SkTDArray<SkPoint> &triangles) {
    if (triangles.count() != numTriangles * 3) {
        printf("Expected %d triangles, not %d\n",
               numTriangles, triangles.count() / 3);
        return false;
    }
    size_t numErrors = 0;
    for (size_t i = 0; i < numTriangles; ++i) {
        const float *r = &refTriangles[i][0][0];
        const SkScalar *t = &triangles[i * 3].fX;
        bool equalTriangle = true;
        for (int j = 6; j-- != 0; r++, t++)
            if (SkFloatToScalar(*r) != *t)
                equalTriangle = false;
        if (equalTriangle == false) {
            ++numErrors;
            printf("Triangle %d differs\n", i);
        }
    }
    if (numErrors > 0)
        printf("%d triangles differ\n", numErrors);
    return numErrors == 0;
}


#ifndef LEFT_HANDED_POLYGONS
static const SkPoint star[] = {
    // Outer contour is clockwise if Y is down, counterclockwise if Y is up.
    { SkFloatToScalar(110), SkFloatToScalar( 20)  },
    { SkFloatToScalar(100), SkFloatToScalar( 50)  },
    { SkFloatToScalar(130), SkFloatToScalar( 80)  },
    { SkFloatToScalar( 90), SkFloatToScalar( 80)  },
    { SkFloatToScalar( 70), SkFloatToScalar(120)  },
    { SkFloatToScalar( 50), SkFloatToScalar( 80)  },
    { SkFloatToScalar( 10), SkFloatToScalar( 80)  },
    { SkFloatToScalar( 40), SkFloatToScalar( 50)  },
    { SkFloatToScalar( 30), SkFloatToScalar( 20)  },
    { SkFloatToScalar( 70), SkFloatToScalar( 40)  },
    // Inner contour is counterclockwise if Y is down, clockwise if Y is up.
    { SkFloatToScalar(110), SkFloatToScalar( 20)  },
    { SkFloatToScalar( 70), SkFloatToScalar( 50)  },
    { SkFloatToScalar( 60), SkFloatToScalar( 60)  },
    { SkFloatToScalar( 60), SkFloatToScalar( 70)  },
    { SkFloatToScalar( 80), SkFloatToScalar( 70)  },
    { SkFloatToScalar( 80), SkFloatToScalar( 60)  },
    { SkFloatToScalar( 70), SkFloatToScalar( 50)  }
};
static const SkPoint plus[] = {
    { SkFloatToScalar( 70), SkFloatToScalar( 10)  },
    { SkFloatToScalar( 70), SkFloatToScalar( 50)  },
    { SkFloatToScalar(110), SkFloatToScalar( 50)  },
    { SkFloatToScalar(110), SkFloatToScalar( 70)  },
    { SkFloatToScalar( 70), SkFloatToScalar( 70)  },
    { SkFloatToScalar( 70), SkFloatToScalar(110)  },
    { SkFloatToScalar( 50), SkFloatToScalar(110)  },
    { SkFloatToScalar( 50), SkFloatToScalar( 70)  },
    { SkFloatToScalar( 10), SkFloatToScalar( 70)  },
    { SkFloatToScalar( 10), SkFloatToScalar( 50)  },
    { SkFloatToScalar( 50), SkFloatToScalar( 50)  },
    { SkFloatToScalar( 50), SkFloatToScalar( 10)  }
};
static const SkPoint zipper[] = {
    { SkFloatToScalar( 10), SkFloatToScalar( 60)  },
    { SkFloatToScalar( 10), SkFloatToScalar( 10)  },
    { SkFloatToScalar( 20), SkFloatToScalar( 10)  },
    { SkFloatToScalar( 20), SkFloatToScalar( 50)  },
    { SkFloatToScalar( 30), SkFloatToScalar( 50)  },
    { SkFloatToScalar( 30), SkFloatToScalar( 10)  },
    { SkFloatToScalar( 60), SkFloatToScalar( 10)  },
    { SkFloatToScalar( 60), SkFloatToScalar( 50)  },
    { SkFloatToScalar( 70), SkFloatToScalar( 50)  },
    { SkFloatToScalar( 70), SkFloatToScalar( 10)  },
    { SkFloatToScalar(100), SkFloatToScalar( 10)  },
    { SkFloatToScalar(100), SkFloatToScalar( 50)  },
    { SkFloatToScalar(110), SkFloatToScalar( 50)  },
    { SkFloatToScalar(110), SkFloatToScalar( 10)  },
    { SkFloatToScalar(140), SkFloatToScalar( 10)  },
    { SkFloatToScalar(140), SkFloatToScalar( 60)  },
    { SkFloatToScalar(130), SkFloatToScalar( 60)  },
    { SkFloatToScalar(130), SkFloatToScalar( 20)  },
    { SkFloatToScalar(120), SkFloatToScalar( 20)  },
    { SkFloatToScalar(120), SkFloatToScalar( 60)  },
    { SkFloatToScalar( 90), SkFloatToScalar( 60)  },
    { SkFloatToScalar( 90), SkFloatToScalar( 20)  },
    { SkFloatToScalar( 80), SkFloatToScalar( 20)  },
    { SkFloatToScalar( 80), SkFloatToScalar( 60)  },
    { SkFloatToScalar( 50), SkFloatToScalar( 60)  },
    { SkFloatToScalar( 50), SkFloatToScalar( 20)  },
    { SkFloatToScalar( 40), SkFloatToScalar( 20)  },
    { SkFloatToScalar( 40), SkFloatToScalar( 60)  }
};
#else  // LEFT_HANDED_POLYGONS
static const SkPoint star[] = {
    // Outer contour is counterclockwise if Y is down, clockwise if Y is up.
    { SkFloatToScalar(110), SkFloatToScalar( 20)  },
    { SkFloatToScalar( 70), SkFloatToScalar( 40)  },
    { SkFloatToScalar( 30), SkFloatToScalar( 20)  },
    { SkFloatToScalar( 40), SkFloatToScalar( 50)  },
    { SkFloatToScalar( 10), SkFloatToScalar( 80)  },
    { SkFloatToScalar( 50), SkFloatToScalar( 80)  },
    { SkFloatToScalar( 70), SkFloatToScalar(120)  },
    { SkFloatToScalar( 90), SkFloatToScalar( 80)  },
    { SkFloatToScalar(130), SkFloatToScalar( 80)  },
    { SkFloatToScalar(100), SkFloatToScalar( 50)  },
    // Inner contour is clockwise if Y is down, counterclockwise if Y is up.
    { SkFloatToScalar(110), SkFloatToScalar( 20)  },
    { SkFloatToScalar( 70), SkFloatToScalar( 50)  },
    { SkFloatToScalar( 80), SkFloatToScalar( 60)  },
    { SkFloatToScalar( 80), SkFloatToScalar( 70)  },
    { SkFloatToScalar( 60), SkFloatToScalar( 70)  },
    { SkFloatToScalar( 60), SkFloatToScalar( 60)  },
    { SkFloatToScalar( 70), SkFloatToScalar( 50)  }
};
#endif  // LEFT_HANDED_POLYGONS
#define kNumStarVertices 10
#define kNumStarHoleVertices (sizeof(star) / sizeof(star[0]))


// Star test
static void TestStarTriangulation(skiatest::Reporter* reporter) {
    static const float refTriangles[][3][2] = {
        { { 30,  20}, { 70,  40}, { 40,  50} },  // { 8, 9, 7 }
        { {100,  50}, { 10,  80}, { 40,  50} },  // { 1, 6, 7 }
        { {100,  50}, { 40,  50}, { 70,  40} },  // { 1, 7, 9 }
        { {100,  50}, { 70,  40}, {110,  20} },  // { 1, 9, 0 }
        { { 90,  80}, { 70, 120}, { 50,  80} },  // { 3, 4, 5 }
        { {130,  80}, { 90,  80}, { 50,  80} },  // { 2, 3, 5 } degen
        { {130,  80}, { 50,  80}, { 10,  80} },  // { 2, 5, 6 } degen
        { {130,  80}, { 10,  80}, {100,  50} }   // { 2, 6, 1 }
    };
    const size_t numRefTriangles = sizeof(refTriangles)
                                 / (6 * sizeof(refTriangles[0][0][0]));
    SkTDArray<SkPoint> triangles;
    bool success = SkConcaveToTriangles(kNumStarVertices, star, &triangles);
    PrintTriangles(triangles, kNumStarVertices, star, reporter);
    success = CompareTriangleList(numRefTriangles, refTriangles, triangles)
            && success;
    reporter->report("Triangulate Star", success ? reporter->kPassed
                                                 : reporter->kFailed);
}


// Star with hole test
static void TestStarHoleTriangulation(skiatest::Reporter* reporter) {
    static const float refTriangles[][3][2] = {
        { {100,  50}, { 80,  60}, { 70,  50} },  // {  1, 15, 16 }
        { {100,  50}, { 70,  50}, {110,  20} },  // {  1, 16,  0 }
        { { 30,  20}, { 70,  40}, { 40,  50} },  // {  8,  9,  7 }
        { { 60,  70}, { 80,  70}, { 10,  80} },  // { 13, 14,  6 }
        { { 60,  60}, { 60,  70}, { 10,  80} },  // { 12, 13,  6 }
        { { 70,  50}, { 60,  60}, { 10,  80} },  // { 11, 12,  6 }
        { { 70,  50}, { 10,  80}, { 40,  50} },  // { 11,  6,  7 }
        { { 70,  50}, { 40,  50}, { 70,  40} },  // { 11,  7,  9 }
        { { 70,  50}, { 70,  40}, {110,  20} },  // { 11,  9, 10 }
        { { 90,  80}, { 70, 120}, { 50,  80} },  // {  3,  4,  5 }
        { {130,  80}, { 90,  80}, { 50,  80} },  // {  2,  3,  5 } degen
        { {130,  80}, { 50,  80}, { 10,  80} },  // {  2,  5,  6 } degen
        { {130,  80}, { 10,  80}, { 80,  70} },  // {  2,  6, 14 }
        { {130,  80}, { 80,  70}, { 80,  60} },  // {  2, 14, 15 }
        { {130,  80}, { 80,  60}, {100,  50} }   // {  2, 15,  1 }
    };
    const size_t numRefTriangles = sizeof(refTriangles)
                                 / (6 * sizeof(refTriangles[0][0][0]));
    SkTDArray<SkPoint> triangles;
    bool success = SkConcaveToTriangles(kNumStarHoleVertices, star, &triangles);
    PrintTriangles(triangles, kNumStarHoleVertices, star, reporter);
    success = CompareTriangleList(numRefTriangles, refTriangles, triangles)
            && success;
    reporter->report("Triangulate Star With Hole", success ? reporter->kPassed
                                                           : reporter->kFailed);
}


// Plus test
static void TestPlusTriangulation(skiatest::Reporter* reporter) {
    static const float refTriangles[][3][2] = {
        { { 50,  10}, { 70,  10}, { 50, 50} },  // { 11,  0, 10 }
        { { 70,  50}, {110,  50}, { 10, 70} },  // {  1,  2,  8 }
        { { 70,  50}, { 10,  70}, { 10, 50} },  // {  1,  8,  9 }
        { { 70,  50}, { 10,  50}, { 50, 50} },  // {  1,  9, 10 }
        { { 70,  50}, { 50,  50}, { 70, 10} },  // {  1, 10,  0 }
        { { 70,  70}, { 50, 110}, { 50, 70} },  // {  4,  6,  7 }
        { {110,  70}, { 70,  70}, { 50, 70} },  // {  3,  4,  7 }
        { {110,  70}, { 50,  70}, { 10, 70} },  // {  3,  7,  8 }
        { {110,  70}, { 10,  70}, {110, 50} },  // {  3,  8,  2 }
        { { 70, 110}, { 50, 110}, { 70, 70} },  // {  5,  6,  4 }
    };
    const size_t numRefTriangles = sizeof(refTriangles)
                                 / (6 * sizeof(refTriangles[0][0][0]));
    SkTDArray<SkPoint> triangles;
    const size_t numVertices = sizeof(plus) / sizeof(SkPoint);
    bool success = SkConcaveToTriangles(numVertices, plus, &triangles);
    PrintTriangles(triangles, numVertices, plus, reporter);
    success = CompareTriangleList(numRefTriangles, refTriangles, triangles)
            && success;
    reporter->report("Triangulate Plus", success ? reporter->kPassed
                                                 : reporter->kFailed);
}


// Zipper test
static void TestZipperTriangulation(skiatest::Reporter* reporter) {
    static const float refTriangles[][3][2] = {
        { { 10, 10}, { 20, 10}, { 20, 50} },  // {  1,  2,  3 }
        { { 20, 50}, { 30, 50}, { 10, 60} },  // {  3,  4,  0 }
        { { 10, 10}, { 20, 50}, { 10, 60} },  // {  1,  3,  0 }
        { { 30, 10}, { 60, 10}, { 40, 20} },  // {  5,  6, 26 }
        { { 30, 10}, { 40, 20}, { 30, 50} },  // {  5, 26,  4 }
        { { 40, 60}, { 10, 60}, { 30, 50} },  // { 27,  0,  4 }
        { { 40, 60}, { 30, 50}, { 40, 20} },  // { 27,  4, 26 }
        { { 60, 50}, { 70, 50}, { 50, 60} },  // {  7,  8, 24 }
        { { 50, 20}, { 60, 50}, { 50, 60} },  // { 25,  7, 24 }
        { { 50, 20}, { 40, 20}, { 60, 10} },  // { 25, 26,  6 }
        { { 60, 50}, { 50, 20}, { 60, 10} },  // {  7, 25,  6 }
        { { 70, 10}, {100, 10}, { 80, 20} },  // {  9, 10, 22 }
        { { 70, 10}, { 80, 20}, { 70, 50} },  // {  9, 22,  8 }
        { { 80, 60}, { 50, 60}, { 70, 50} },  // { 23, 24,  8 }
        { { 80, 60}, { 70, 50}, { 80, 20} },  // { 23,  8, 22 }
        { {100, 50}, {110, 50}, { 90, 60} },  // { 11, 12, 20 }
        { { 90, 20}, {100, 50}, { 90, 60} },  // { 21, 11, 20 }
        { { 90, 20}, { 80, 20}, {100, 10} },  // { 21, 22, 10 }
        { {100, 50}, { 90, 20}, {100, 10} },  // { 11, 21, 10 }
        { {110, 10}, {140, 10}, {120, 20} },  // { 13, 14, 18 }
        { {110, 10}, {120, 20}, {110, 50} },  // { 13, 18, 12 }
        { {120, 60}, { 90, 60}, {110, 50} },  // { 19, 20, 12 }
        { {120, 60}, {110, 50}, {120, 20} },  // { 19, 12, 18 }
        { {140, 60}, {130, 60}, {130, 20} },  // { 15, 16, 17 }
        { {130, 20}, {120, 20}, {140, 10} },  // { 17, 18, 14 }
        { {140, 60}, {130, 20}, {140, 10} },  // { 15, 17, 14 }
    };
    const size_t numRefTriangles = sizeof(refTriangles)
                                 / (6 * sizeof(refTriangles[0][0][0]));
    SkTDArray<SkPoint> triangles;
    const size_t numVertices = sizeof(zipper) / sizeof(SkPoint);
    bool success = SkConcaveToTriangles(numVertices, zipper, &triangles);
    PrintTriangles(triangles, numVertices, zipper, reporter);
    success = CompareTriangleList(numRefTriangles, refTriangles, triangles)
            && success;
    reporter->report("Triangulate Zipper", success ? reporter->kPassed
                                                   : reporter->kFailed);
}


static void TestTriangulation(skiatest::Reporter* reporter) {
    TestStarTriangulation(reporter);
    TestStarHoleTriangulation(reporter);
    TestPlusTriangulation(reporter);
    TestZipperTriangulation(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Triangulation", TriangulationTestClass, TestTriangulation)
