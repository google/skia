/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkOpSegment.h"
#include "Test.h"

static const SkPoint cubics[][4] = {
    {{0, 1}, {2, 6}, {4, 2}, {5, 3}}
};

static const SkPoint lines[][2] = {
    {{6, 2}, {2, 4}}
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

struct SortSetTests {
    const SortSet* set;
    size_t count;
};

static const SortSetTests tests[] = {
    { set2, SK_ARRAY_COUNT(set2) },
    { set1, SK_ARRAY_COUNT(set1) }
};

static void setup(const SortSet* set, const size_t idx, SkPoint const ** data,
        SkOpSegment* seg, int* ts) {
    SkPoint start, end;
    if (set[idx].ptCount == 2) {
        *data = set[idx].ptData;
        seg->addLine(*data, false, false);
        SkDLine dLine;
        dLine.set(set[idx].ptData);
        start = dLine.xyAtT(set[idx].tStart).asSkPoint();
        end = dLine.xyAtT(set[idx].tEnd).asSkPoint();
    } else if (set[idx].ptCount == 4) {
        *data = set[idx].ptData;
        seg->addCubic(*data, false, false);
        SkDCubic dCubic;
        dCubic.set(set[idx].ptData);
        start = dCubic.xyAtT(set[idx].tStart).asSkPoint();
        end = dCubic.xyAtT(set[idx].tEnd).asSkPoint();
    }
    seg->addT(NULL, start, set[idx].tStart);
    seg->addT(NULL, end, set[idx].tEnd);
    seg->addT(NULL, set[idx].ptData[0], 0);
    seg->addT(NULL, set[idx].ptData[set[idx].ptCount - 1], 1);
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
            first.set(lesserData, (SkPath::Verb) (set[idxL].ptCount - 1), &lesser,
                    lesserTs[0], lesserTs[1], lesser.spans());
            second.set(greaterData, (SkPath::Verb) (set[idxG].ptCount - 1), &greater,
                    greaterTs[0], greaterTs[1], greater.spans());
            bool compare = first < second;
            if (!compare) {
                SkDebugf("%s test[%d]:  lesser[%d] > greater[%d]\n", __FUNCTION__,
                        index, idxL,  idxG);
            }
            REPORTER_ASSERT(reporter, compare);
        }
    }
}

#include "TestClassDef.h"
DEFINE_TESTCLASS_SHORT(PathOpsAngleTest)
