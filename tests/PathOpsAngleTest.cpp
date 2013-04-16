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
    {lines[0], 2, 0.54070336, 0.388888889},
    {cubics[0], 4, 0.666669871, 0.405037112},
    {lines[0], 2, 0.54070336, 0.9140625},
    {cubics[0], 4, 0.666669871, 0.875},
};

static const SortSet set2[] = {
    {lines[0], 2, 0.574074074, 0.388888889},
    {cubics[0], 4, 0.666666667, 0.405037112},
    {lines[0], 2, 0.574074074, 0.9140625},
    {cubics[0], 4, 0.666666667, 0.875},
};

struct SortSetTests {
    const SortSet* set;
    size_t count;
};

static const SortSetTests tests[] = {
    { set1, SK_ARRAY_COUNT(set1) },
    { set2, SK_ARRAY_COUNT(set2) }
};

static void setup(const SortSet* set, const size_t idx, SkPoint const ** data,
        SkPoint* reverse, SkOpSegment* seg) {
    SkPoint start, end;
    if (set[idx].ptCount == 2) {
        if (set[idx].tStart < set[idx].tEnd) {
            *data = set[idx].ptData;
        } else {
            reverse[0] = set[idx].ptData[1];
            reverse[1] = set[idx].ptData[0];
            *data = reverse;
        }
        seg->addLine(*data, false, false);
        SkDLine dLine;
        dLine.set(set[idx].ptData);
        start = dLine.xyAtT(set[idx].tStart).asSkPoint();
        end = dLine.xyAtT(set[idx].tEnd).asSkPoint();
    } else if (set[idx].ptCount == 4) {
        if (set[idx].tStart < set[idx].tEnd) {
            *data = set[idx].ptData;
        } else {
            reverse[0] = set[idx].ptData[3];
            reverse[1] = set[idx].ptData[2];
            reverse[2] = set[idx].ptData[1];
            reverse[3] = set[idx].ptData[0];
            *data = reverse;
        }
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
}

static void PathOpsAngleTest(skiatest::Reporter* reporter) {
    for (size_t index = 0; index < SK_ARRAY_COUNT(tests); ++index) {
        const SortSetTests& test = tests[index];
        for (size_t idxL = 0; idxL < test.count - 1; ++idxL) {
            SkOpSegment lesser, greater;
            SkPoint lesserReverse[4], greaterReverse[4];
            const SkPoint* lesserData, * greaterData;
            const SortSet* set = test.set;
            setup(set, idxL, &lesserData, lesserReverse, &lesser);
            size_t idxG = idxL + 1;
            setup(set, idxG, &greaterData, greaterReverse, &greater);
            SkOpAngle first, second;
            first.set(lesserData, (SkPath::Verb) (set[idxL].ptCount - 1), &lesser,
                    0, 1, lesser.spans());
            first.setSpans();
            second.set(greaterData, (SkPath::Verb) (set[idxG].ptCount - 1), &greater,
                    0, 1, greater.spans());
            second.setSpans();
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
