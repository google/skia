/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkOpSegment.h"
#include "Test.h"

static const SkPoint cubics[][4] = {
/* 0 */    {{0, 1}, {2, 6}, {4, 2}, {5, 3}},
/* 1 */    {{10, 234}, {10, 229.581726f}, {13.5817204f, 226}, {18, 226}},
/* 2 */    {{132, 11419}, {130.89543151855469f, 11419}, {130, 11418.1044921875f}, {130, 11417}},
/* 3 */    {{130.04275512695312f, 11417.4130859375f}, {130.23307800292969f, 11418.3193359375f},
                    {131.03709411621094f, 11419}, {132, 11419}},
/* 4 */    {{0,1}, {0,5}, {4,1}, {6,4}},
/* 5 */    {{1,5}, {4,6}, {1,0}, {4,0}},
/* 6 */    {{0,1}, {0,4}, {5,1}, {6,4}},
/* 7 */    {{0,1}, {1,2}, {1,0}, {6,1}},
/* 8 */    {{0,3}, {0,1}, {2,0}, {1,0}},
/* 9 */    {{189,7}, {189,5.3431458473205566f}, {190.3431396484375f,4}, {192,4}},
/* 10 */   {{0,1}, {1,3}, {1,0}, {6,4}},
/* 11 */   {{0,1}, {2,3}, {2,1}, {4,3}},
/* 12 */   {{1,2}, {3,4}, {1,0}, {3,2}},
};

static const SkPoint quads[][3] = {
/* 0 */    {{12.3423996f, 228.342407f}, {10, 230.686295f}, {10, 234}},
/* 1 */    {{304.24319458007812f,591.75677490234375f}, {306,593.51470947265625f}, {306,596}},
};

static const SkPoint lines[][2] = {
/* 0 */    {{6, 2}, {2, 4}},
/* 1 */    {{306,617}, {306,590}},
/* 2 */    {{306,596}, {306,617}},
/* 3 */    {{6,4}, {0,1}},
/* 4 */    {{6,1}, {0,1}},
/* 5 */    {{1,0}, {0,3}},
/* 6 */    {{246,4}, {189,4}},
/* 7 */    {{192,4}, {243,4}},
/* 8 */    {{4,3}, {0,1}},
/* 9 */    {{3,2}, {1,2}},
};

struct SortSet {
    const SkPoint* ptData;
    int ptCount;
    double tStart;
    double tEnd;
};

static const SortSet set1[] = {
    {cubics[0], 4, 0.66666987081928919, 0.875},
    {lines[0], 2, 0.574070336, 0.388888889},
    {cubics[0], 4, 0.66666987081928919, 0.4050371120499307 },
    {lines[0], 2, 0.574070336, 0.9140625},
};

static const SortSet set2[] = {
    {cubics[0], 4, 0.666666667, 0.875},
    {lines[0], 2, 0.574074074, 0.388888889},
    {cubics[0], 4, 0.666666667, 0.405037112},
    {lines[0], 2, 0.574074074, 0.9140625},
};

static const SortSet set3[] = {
    {cubics[1], 4, 0, 1},
    {quads[0], 3, 1, 0},
};

static const SortSet set4[] = {
    {cubics[2], 4, 0.812114222, 1},
    {cubics[3], 4, 0.0684734759, 0},
};

static const SortSet set5[] = {
    {lines[1], 2, 0.777777778, 1},
    {quads[1], 3, 1, 4.34137342e-06},
    {lines[2], 2, 0, 1},
};

static const SortSet set6[] = {
    {lines[3], 2, 0.407407407, 0.554627832},
    {cubics[4], 4, 0.666666667, 0.548022446},
    {lines[3], 2, 0.407407407, 0},
    {cubics[4], 4, 0.666666667, 1},
};

static const SortSet set7[] = {
    {cubics[5], 4, 0.545233342, 0.545454545},
    {cubics[6], 4, 0.484938134, 0.484805744},
    {cubics[5], 4, 0.545233342, 0},
    {cubics[6], 4, 0.484938134, 0.545454545},
};

static const SortSet set8[] = {
    {cubics[7], 4, 0.5, 0.522986744 },
    {lines[4], 2, 0.75, 1},
    {cubics[7], 4, 0.5, 0},
    {lines[4], 2, 0.75, 0.737654321},
};

static const SortSet set9[] = {
    {cubics[8], 4, 0.4, 1},
    {lines[5], 2, 0.36, 0},
    {cubics[8], 4, 0.4, 0.394675838},
    {lines[5], 2, 0.36, 0.363999782},
};

static const SortSet set10[] = {
    {lines[6], 2, 0.947368421, 1},
    {cubics[9], 4, 1, 0.500000357},
    {lines[7], 2, 0, 1},
};

static const SortSet set11[] = {
    {lines[3], 2, 0.75, 1},
    {cubics[10], 4, 0.5, 0.228744269},
    {lines[3], 2, 0.75, 0.627112191},
    {cubics[10], 4, 0.5, 0.6339746},
};

static const SortSet set12[] = {
    {cubics[12], 4, 0.5, 1},
    {lines[8], 2, 0.5, 1},
    {cubics[11], 4, 0.5, 0},
    {lines[9], 2, 0.5, 1},
    {cubics[12], 4, 0.5, 0},
    {lines[8], 2, 0.5, 0},
    {cubics[11], 4, 0.5, 1},
    {lines[9], 2, 0.5, 0},
};

struct SortSetTests {
    const SortSet* set;
    size_t count;
};

static const SortSetTests tests[] = {
    { set12, SK_ARRAY_COUNT(set12) },
    { set11, SK_ARRAY_COUNT(set11) },
    { set10, SK_ARRAY_COUNT(set10) },
    { set9, SK_ARRAY_COUNT(set9) },
    { set8, SK_ARRAY_COUNT(set8) },
    { set7, SK_ARRAY_COUNT(set7) },
    { set6, SK_ARRAY_COUNT(set6) },
    { set2, SK_ARRAY_COUNT(set2) },
    { set5, SK_ARRAY_COUNT(set5) },
    { set4, SK_ARRAY_COUNT(set4) },
    { set3, SK_ARRAY_COUNT(set3) },
    { set1, SK_ARRAY_COUNT(set1) },
};

static void setup(const SortSet* set, const size_t idx, SkPoint const ** data,
        SkOpSegment* seg, int* ts) {
    SkPoint start, end;
    *data = set[idx].ptData;
    switch(set[idx].ptCount) {
        case 2: {
            seg->addLine(*data, false, false);
            SkDLine dLine;
            dLine.set(set[idx].ptData);
            start = dLine.xyAtT(set[idx].tStart).asSkPoint();
            end = dLine.xyAtT(set[idx].tEnd).asSkPoint();
            } break;
        case 3: {
            seg->addQuad(*data, false, false);
            SkDQuad dQuad;
            dQuad.set(set[idx].ptData);
            start = dQuad.xyAtT(set[idx].tStart).asSkPoint();
            end = dQuad.xyAtT(set[idx].tEnd).asSkPoint();
            } break;
        case 4: {
            seg->addCubic(*data, false, false);
            SkDCubic dCubic;
            dCubic.set(set[idx].ptData);
            start = dCubic.xyAtT(set[idx].tStart).asSkPoint();
            end = dCubic.xyAtT(set[idx].tEnd).asSkPoint();
            } break;
    }
    double tStart = set[idx].tStart;
    double tEnd = set[idx].tEnd;
    seg->addT(NULL, start, tStart);
    seg->addT(NULL, end, tEnd);
    double tLeft = tStart < tEnd ? 0 : 1;
    if (tStart != tLeft && tEnd != tLeft) {
        seg->addT(NULL, set[idx].ptData[0], tLeft);
    }
    double tRight = tStart < tEnd ? 1 : 0;
    if (tStart != tRight && tEnd != tRight) {
        seg->addT(NULL, set[idx].ptData[set[idx].ptCount - 1], tRight);
    }
    int tIndex = 0;
    do {
        if (seg->t(tIndex) == set[idx].tStart) {
            ts[0] = tIndex;
        }
        if (seg->t(tIndex) == set[idx].tEnd) {
            ts[1] = tIndex;
        }
        if (seg->t(tIndex) >= 1) {
            break;
        }
    } while (++tIndex);
}

static void PathOpsAngleTest(skiatest::Reporter* reporter) {
    for (size_t index = 0; index < SK_ARRAY_COUNT(tests); ++index) {
        const SortSetTests& test = tests[index];
        for (size_t idxL = 0; idxL < test.count - 1; ++idxL) {
            SkOpSegment lesser, greater;
            int lesserTs[2], greaterTs[2];
            const SkPoint* lesserData, * greaterData;
            const SortSet* set = test.set;
            setup(set, idxL, &lesserData, &lesser, lesserTs);
            size_t idxG = idxL + 1;
            setup(set, idxG, &greaterData, &greater, greaterTs);
            SkOpAngle first, second;
            first.set(lesserData, SkPathOpsPointsToVerb(set[idxL].ptCount - 1), &lesser,
                    lesserTs[0], lesserTs[1], lesser.spans());
            second.set(greaterData, SkPathOpsPointsToVerb(set[idxG].ptCount - 1), &greater,
                    greaterTs[0], greaterTs[1], greater.spans());
            bool compare = first < second;
            if (!compare) {
                SkDebugf("%s test[%d]:  lesser[%d] > greater[%d]\n", __FUNCTION__,
                        index, idxL,  idxG);
                compare = first < second;
            }
            REPORTER_ASSERT(reporter, compare);
            reporter->bumpTestCount();
        }
    }
}

#include "TestClassDef.h"
DEFINE_TESTCLASS_SHORT(PathOpsAngleTest)
