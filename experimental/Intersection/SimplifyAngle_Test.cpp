/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Simplify.h"

namespace SimplifyAngleTest {

#include "Simplify.cpp"

} // end of SimplifyAngleTest namespace

#include "Intersection_Tests.h"

static const SkPoint lines[][2] = {
    { { 10,  10}, { 10,  20} },
    { { 10,  10}, { 20,  10} },
    { { 10,  10}, {-20,  10} },
    { { 10,  10}, { 10, -20} },
    { { 10,  10}, { 20,  20} },
    { { 10,  10}, {-20, -20} },
    { { 10,  10}, {-20,  40} },
    { { 10,  10}, { 40, -20} }
};

static const size_t lineCount = sizeof(lines) / sizeof(lines[0]);

static const SkPoint quads[][3] = {
    {{ 1,  1}, { 2,  2}, { 1,  3}}, // 0
    {{ 1,  1}, { 3,  3}, { 1,  5}}, // 1
    {{ 1,  1}, { 4,  4}, { 1,  7}}, // 2
    {{ 1,  1}, { 5,  5}, { 9,  9}}, // 3
    {{ 1,  1}, { 4,  4}, { 7,  1}}, // 4
    {{ 1,  1}, { 3,  3}, { 5,  1}}, // 5
    {{ 1,  1}, { 2,  2}, { 3,  1}}, // 6
};

static const size_t quadCount = sizeof(quads) / sizeof(quads[0]);

static const SkPoint cubics[][4] = {
    {{ 1,  1}, { 2,  2}, { 2,  3}, { 1,  4}},
    {{ 1,  1}, { 3,  3}, { 3,  5}, { 1,  7}},
    {{ 1,  1}, { 4,  4}, { 4,  7}, { 1,  10}},
    {{ 1,  1}, { 5,  5}, { 8,  8}, { 9,  9}},
    {{ 1,  1}, { 4,  4}, { 7,  4}, { 10, 1}},
    {{ 1,  1}, { 3,  3}, { 5,  3}, { 7,  1}},
    {{ 1,  1}, { 2,  2}, { 3,  2}, { 4,  1}},
};

static const size_t cubicCount = sizeof(cubics) / sizeof(cubics[0]);

struct segment {
    SkPath::Verb verb;
    SkPoint pts[4];
};

static const segment segmentTest1[] = {
    {SkPath::kLine_Verb, {{2, 1}, {1, 0}        }},
    {SkPath::kQuad_Verb, {{2, 1}, {1, 0}, {0, 0}}},
    {SkPath::kQuad_Verb, {{2, 1}, {3, 2}, {2, 3}}},
    {SkPath::kLine_Verb, {{2, 1}, {3, 2}        }},
    {SkPath::kMove_Verb                          }
};

static const segment segmentTest2[] = {
    {SkPath::kLine_Verb, {{1, 0}, {0, 0}        }},
    {SkPath::kQuad_Verb, {{1, 0}, {1.89897954f, 0.898979485f}, {2.39387703f, 1.59591794f}}},
    {SkPath::kLine_Verb, {{1, 0}, {3, 2}        }},
    {SkPath::kMove_Verb                          }
};

static const segment segmentTest3[] = {
    {SkPath::kQuad_Verb, {{0, 0}, {2, 0}, {0, 1}}},
    {SkPath::kQuad_Verb, {{0, 0}, {1, 0}, {0, 1}}},
    {SkPath::kMove_Verb                          }
};

static const segment* segmentTests[] = {
    segmentTest3,
    segmentTest2,
    segmentTest1,
};

static const size_t segmentTestCount = sizeof(segmentTests) / sizeof(segmentTests[0]);

static void testSegments(bool testFlat) {
    for (size_t testIndex = 0; testIndex < segmentTestCount; ++testIndex) {
        const segment* segPtr = segmentTests[testIndex];
        SimplifyAngleTest::Angle lesser, greater;
        int index = 0;
        do {
            int next = index + 1;
            SkTDArray<SimplifyAngleTest::Span> dummy;
            lesser.set(segPtr[index].pts, segPtr[index].verb, NULL, index, next, dummy);
            if (segPtr[next].verb == SkPath::kMove_Verb) {
                break;
            }
            greater.set(segPtr[next].pts, segPtr[next].verb, NULL, index, next, dummy);
            bool result = lesser < greater;
            SkASSERT(result);
            index = next;
        } while (true);
    }
}

static void testLines(bool testFlat) {
    // create angles in a circle
    SkTDArray<SimplifyAngleTest::Angle> angles;
    SkTDArray<SimplifyAngleTest::Angle* > angleList;
    SkTDArray<double> arcTans;
    size_t x;
    for (x = 0; x < lineCount; ++x) {
        SimplifyAngleTest::Angle* angle = angles.append();
        SkTDArray<SimplifyAngleTest::Span> dummy;
        angle->set(lines[x], SkPath::kLine_Verb, 0, x, x + 1, dummy);
        double arcTan = atan2(lines[x][0].fX - lines[x][1].fX,
                lines[x][0].fY - lines[x][1].fY);
        arcTans.push(arcTan);
    }
    for (x = 0; x < lineCount; ++x) {
        angleList.push(&angles[x]);
    }
    QSort<SimplifyAngleTest::Angle>(angleList.begin(), angleList.end() - 1);
    bool first = true;
    bool wrap = false;
    double base, last;
    for (size_t x = 0; x < lineCount; ++x) {
        const SimplifyAngleTest::Angle* angle = angleList[x];
        int span = angle->start();
//        SkDebugf("%s [%d] %1.9g (%1.9g,%1.9g %1.9g,%1.9g)\n", __FUNCTION__,
//                span, arcTans[span], lines[span][0].fX, lines[span][0].fY,
//                lines[span][1].fX, lines[span][1].fY);
        if (first) {
            base = last = arcTans[span];
            first = false;
            continue;
        }
        if (last < arcTans[span]) {
            last = arcTans[span];
            continue;
        }
        if (!wrap) {
            if (base < arcTans[span]) {
                SkDebugf("%s !wrap [%d] %g\n", __FUNCTION__, span, arcTans[span]);
                SkASSERT(0);
            }
            last = arcTans[span];
            wrap = true;
            continue;
        }
        SkDebugf("%s wrap [%d] %g\n", __FUNCTION__, span, arcTans[span]);
        SkASSERT(0);
    }
}

static void testQuads(bool testFlat) {
    SkTDArray<SimplifyAngleTest::Angle> angles;
    SkTDArray<SimplifyAngleTest::Angle* > angleList;
    size_t x;
    for (x = 0; x < quadCount; ++x) {
        SimplifyAngleTest::Angle* angle = angles.append();
        SkTDArray<SimplifyAngleTest::Span> dummy;
        angle->set(quads[x], SkPath::kQuad_Verb, 0, x, x + 1, dummy);
   }
    for (x = 0; x < quadCount; ++x) {
        angleList.push(&angles[x]);
    }
    QSort<SimplifyAngleTest::Angle>(angleList.begin(), angleList.end() - 1);
    for (size_t x = 0; x < quadCount; ++x) {
        *angleList[x] < *angleList[x + 1];
        SkASSERT(x == quadCount - 1 || *angleList[x] < *angleList[x + 1]);
        const SimplifyAngleTest::Angle* angle = angleList[x];
        if ((int) x != angle->start()) {
            SkDebugf("%s [%d] [%d]\n", __FUNCTION__, x, angle->start());
            SkASSERT(0);
        }
    }
}

static void testCubics(bool testFlat) {
    SkTDArray<SimplifyAngleTest::Angle> angles;
    SkTDArray<SimplifyAngleTest::Angle* > angleList;
    for (size_t x = 0; x < cubicCount; ++x) {
        SimplifyAngleTest::Angle* angle = angles.append();
            SkTDArray<SimplifyAngleTest::Span> dummy;
        angle->set(cubics[x], SkPath::kCubic_Verb, 0, x, x + 1, dummy);
        angleList.push(angle);
    }
    QSort<SimplifyAngleTest::Angle>(angleList.begin(), angleList.end() - 1);
    for (size_t x = 0; x < cubicCount; ++x) {
        const SimplifyAngleTest::Angle* angle = angleList[x];
        if ((int) x != angle->start()) {
            SkDebugf("%s [%d] [%d]\n", __FUNCTION__, x, angle->start());
            SkASSERT(0);
        }
    }
}

struct segmentWithT {
    SkPath::Verb verb;
    SkPoint pts[4];
    double ts[2];
};


static const segmentWithT oneOffTest1[] = {
    {SkPath::kQuad_Verb, {{391.653534f, 183.286819f}, {391.653534f, 157.724487f}, {405.469604f, 141.372879f}},
        {0.62346946335026932, 0.62344389027237135}},
    {SkPath::kQuad_Verb, {{399.365234f, 171.695801f}, {399.365234f, 140.337967f}, {375.976227f, 140.337967f}},
        {0.31638302676995866, 0.31637992418411398}},
    {SkPath::kMove_Verb }
};

static const segmentWithT oneOffTest2[] = {
    {SkPath::kQuad_Verb, {{399.070374f, 151.722f}, {391.101532f, 163.002533f}, {391.101532f, 182.665863f}},
        {0.13793711854916513, 0.13790171160614006}},
    {SkPath::kQuad_Verb, {{391.653534f, 183.286819f}, {391.653534f, 157.724487f}, {405.469604f, 141.372879f}},
        {0.62344389027237135, 0.62346946335026932}},
    {SkPath::kMove_Verb }
};

static const segmentWithT oneOffTest3[] = {
    {SkPath::kQuad_Verb, {{399.365234f, 171.695801f}, {399.365234f, 140.337967f}, {375.976227f, 140.337967f}},
        {0.31637992418411398, 0.31638302676995866, }},
    {SkPath::kQuad_Verb, {{399.070374f, 151.722f}, {391.101532f, 163.002533f}, {391.101532f, 182.665863f}},
        {0.13790171160614006, 0.13793711854916513}},
    {SkPath::kMove_Verb }
};

static const segmentWithT oneOffTest4[] = {
    {SkPath::kCubic_Verb, {{1,2}, {1,3}, {1,0}, {5,3}}, {0.134792, 0}},
    {SkPath::kCubic_Verb, {{0,1}, {3,5}, {2,1}, {3,1}}, {0.134792094, 0}},
    {SkPath::kCubic_Verb, {{0,1}, {3,5}, {2,1}, {3,1}}, {0.134792094, 0.551812363}},
    {SkPath::kCubic_Verb, {{1,2}, {1,3}, {1,0}, {5,3}}, {0.134792, 0.333333333}},
    {SkPath::kMove_Verb }
};

static const segmentWithT* oneOffTests[] = {
    oneOffTest1,
    oneOffTest2,
    oneOffTest3,
    oneOffTest4,
};

static const size_t oneOffTestCount = sizeof(oneOffTests) / sizeof(oneOffTests[0]);

static void oneOffTest(bool testFlat) {
    for (size_t testIndex = 0; testIndex < oneOffTestCount; ++testIndex) {
        const segmentWithT* segPtr = oneOffTests[testIndex];
        SimplifyAngleTest::Angle lesser, greater;
        int index = 0;
        do {
            int next = index + 1;
            SkTDArray<SimplifyAngleTest::Span> dummy; // FIXME
            lesser.set(segPtr[index].pts, segPtr[index].verb, 0, index, next, dummy); // FIXME: segPtr[index].ts[0], segPtr[index].ts[1]);
            if (segPtr[next].verb == SkPath::kMove_Verb) {
                break;
            }
            greater.set(segPtr[next].pts, segPtr[next].verb, 0, index, next, dummy); // FIXME: segPtr[next].ts[0], segPtr[next].ts[1]);
            bool result = lesser < greater;
            SkASSERT(result);
            index = next;
        } while (true);
    }
    SkDebugf("%s finished\n", __FUNCTION__);
}

#if 0 // seems too complicated for this to be useful (didn't finish writing/debugging this)
// this (trys to) take a pair of curves, construct segments/spans, and verify that they sort correctly
static void oneOffTestNew() {
    const segmentWithT* segPtr = oneOffTest4;
    SimplifyAngleTest::Segment segOne, segTwo;
    segOne.init(segPtr[0].pts, segPtr[0].verb, false, false);
    segTwo.init(segPtr[1].pts, segPtr[1].verb, false, false);
    int index = 0;
    do {
        int next = index + 1;
        if (segPtr[index].verb == SkPath::kMove_Verb) {
            break;
        }
        SkPoint sub[4];
        (*SegmentSubDivide[segPtr[index].verb])(segPtr[index].pts, segPtr[index].ts[0],
                segPtr[index].ts[1], sub);
        if (memcmp(segPtr[index].pts, segPtr[0].pts, sizeof(SkPoint) * (segPtr[index].verb + 1) == 0) {
            segOne.addT(&segTwo, sub[0], segPtr[index].ts[0]);
            segOne.addT(&segTwo, sub[segPtr[index].verb], segPtr[index].ts[1]);
        } else {
            segTwo.addT(&segOne, sub[0], segPtr[index].ts[0]);
            segTwo.addT(&v, sub[segPtr[index].verb], segPtr[index].ts[1]);
        }
    } while (true);
    SimplifyAngleTest::Angle lesser, greater;
    do {
        int next = index + 1;
        if (segPtr[next].verb == SkPath::kMove_Verb) {
            break;
        }
        SkPoint one[4], two[4];
        bool use1 = memcmp(segPtr[index].pts, segPtr[0].pts, sizeof(SkPoint) * (segPtr[index].verb + 1) == 0;
        lesser.set(segPtr[index].pts, segPtr[index].verb, use1 ? &segOne : &segTwo, index, next, dummy);
        use1 = memcmp(segPtr[next].pts, segPtr[0].pts, sizeof(SkPoint) * (segPtr[next].verb + 1) == 0;
        greater.set(segPtr[next].pts, segPtr[next].verb, use1 ? &segOne : &segTwo, index, next, dummy);
        bool result = lesser < greater;
        SkASSERT(result);
        index = next;
    } while (true);
    SkDebugf("%s finished\n", __FUNCTION__);
}
#endif

static void (*tests[])(bool) = {
    oneOffTest,
    testSegments,
    testLines,
    testQuads,
    testCubics
};

static const size_t testCount = sizeof(tests) / sizeof(tests[0]);

static void (*firstTest)(bool) = 0;
static bool skipAll = false;

void SimplifyAngle_Test() {
    if (skipAll) {
        return;
    }
    size_t index = 0;
    if (firstTest) {
        while (index < testCount && tests[index] != firstTest) {
            ++index;
        }
    }
    bool firstTestComplete = false;
    for ( ; index < testCount; ++index) {
        (*tests[index])(false); // set to true to exercise setFlat
        firstTestComplete = true;
    }
}
