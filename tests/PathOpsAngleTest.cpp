/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsTestCommon.h"
#include "SkOpSegment.h"
#include "SkTArray.h"
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
/* 13 */   {{0,1}, {4,6}, {4,3}, {5,4}},
/* 14 */   {{806,11419}, {806.962890625f,11419}, {807.76690673828125f,11418.3193359375f}, {807.957275390625f,11417.4130859375f}},
/* 15 */   {{808,11417}, {808,11418.1044921875f}, {807.10455322265625f,11419}, {806,11419}},
/* 16 */   {{132,11419}, {130.89543151855469f,11419}, {130,11418.1044921875f}, {130,11417}},
/* 17 */   {{130.04275512695312f,11417.4130859375f}, {130.23312377929687f,11418.3193359375f}, {131.03707885742187f,11419}, {132,11419}},
/* 18 */   {{1006.6951293945312f,291}, {1023.263671875f,291}, {1033.8402099609375f,304.43145751953125f}, {1030.318359375f,321}},
};

static const SkPoint quads[][3] = {
/* 0 */    {{12.3423996f, 228.342407f}, {10, 230.686295f}, {10, 234}},
/* 1 */    {{304.24319458007812f,591.75677490234375f}, {306,593.51470947265625f}, {306,596}},
/* 2 */    {{0,0}, {3,1}, {0,3}},
/* 3 */    {{0,1}, {3,1}, {0,2}},
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
/* 10 */   {{6,4}, {3,4}},
/* 11 */   {{979.30487060546875f,561}, {1036.695068359375f,291}},
};

struct SortSet {
    const SkPoint* ptData;
    int ptCount;
    double tStart;
    double tEnd;
    SkPoint endPt;
};

/*static const SortSet set1[] = {
    {cubics[0], 4, 0.66666987081928919, 0.875, {0, 0}},
    {lines[0], 2, 0.574070336, 0.388888889, {0, 0}},
    {cubics[0], 4, 0.66666987081928919, 0.4050371120499307, {0, 0}},
    {lines[0], 2, 0.574070336, 0.9140625, {0, 0}},
};

static const SortSet set1a[] = {
    {cubics[0], 4, 0.666666667, 0.405037112, {4.58007812f,2.83203125f}},
    {lines[0], 2, 0.574074074, 0.9140625, {4.44444466f,2.77777767f}},
};*/

static const SortSet set2[] = {
    {cubics[0], 4, 0.666666667, 0.875, {0, 0}},
    {lines[0], 2, 0.574074074, 0.388888889, {0, 0}},
    {cubics[0], 4, 0.666666667, 0.405037112, {0, 0}},
    {lines[0], 2, 0.574074074, 0.9140625, {0, 0}},
};

static const SortSet set3[] = {
    {cubics[1], 4, 0, 1, {0, 0}},
    {quads[0], 3, 1, 0, {0, 0}},
};

/*static const SortSet set4[] = {
    {cubics[2], 4, 0.812114222, 1, {0, 0}},
    {cubics[3], 4, 0.0684734759, 0, {0, 0}},
};*/

static const SortSet set5[] = {
    {lines[1], 2, 0.777777778, 1, {0, 0}},
    {quads[1], 3, 1, 4.34137342e-06, {0, 0}},
    {lines[2], 2, 0, 1, {0, 0}},
};

static const SortSet set5a[] = {
    {lines[1], 2, 0.777777778, 1, {306,590}},
    {quads[1], 3, 1, 4.34137342e-06, {304.243195f,591.756775f}},
    {lines[2], 2, 0, 1, {306,617}},
};

static const SortSet set6[] = {
    {lines[3], 2, 0.407407407, 0.554627832, {0, 0}},
    {cubics[4], 4, 0.666666667, 0.548022446, {0, 0}},
    {lines[3], 2, 0.407407407, 0, {0, 0}},
    {cubics[4], 4, 0.666666667, 1, {0, 0}},
};

static const SortSet set6a[] = {
    {lines[3], 2, 0.407407407, 0.554627832, {2.6722331f,2.33611655f}},
    {cubics[4], 4, 0.666666667, 0.548022446, {2.61642241f,2.83718514f}},
    {lines[3], 2, 0.407407407, 0, {6,4}},
    {cubics[4], 4, 0.666666667, 1, {6,4}},
};

static const SortSet set7[] = {
    {cubics[5], 4, 0.545233342, 0.545454545, {0, 0}},
    {cubics[6], 4, 0.484938134, 0.484805744, {0, 0}},
    {cubics[5], 4, 0.545233342, 0, {0, 0}},
    {cubics[6], 4, 0.484938134, 0.545454545, {0, 0}},
};

static const SortSet set8[] = {
    {cubics[7], 4, 0.5, 0.522986744, {0, 0}},
    {lines[4], 2, 0.75, 1, {0, 0}},
    {cubics[7], 4, 0.5, 0, {0, 0}},
    {lines[4], 2, 0.75, 0.737654321, {0, 0}},
};

static const SortSet set8a[] = {
    {cubics[7], 4, 0.5, 0.522986744, {1.60668361f,0.965592742f}},
    {lines[4], 2, 0.75, 1, {0,1}},
    {cubics[7], 4, 0.5, 0, {0,1}},
    {lines[4], 2, 0.75, 0.737654321, {1.57407403f,1}},
};

static const SortSet set9[] = {
    {cubics[8], 4, 0.4, 1, {0, 0}},
    {lines[5], 2, 0.36, 0, {0, 0}},
    {cubics[8], 4, 0.4, 0.394675838, {0, 0}},
    {lines[5], 2, 0.36, 0.363999782, {0, 0}},
};

static const SortSet set10[] = {
    {lines[6], 2, 0.947368421, 1, {0, 0}},
    {cubics[9], 4, 1, 0.500000357, {0, 0}},
    {lines[7], 2, 0, 1, {0, 0}},
};

static const SortSet set11[] = {
    {lines[3], 2, 0.75, 1, {0, 0}},
    {cubics[10], 4, 0.5, 0.228744269, {0, 0}},
    {lines[3], 2, 0.75, 0.627112191, {0, 0}},
    {cubics[10], 4, 0.5, 0.6339746, {0, 0}},
};

static const SortSet set12[] = {
    {cubics[12], 4, 0.5, 1, {0, 0}},
    {lines[8], 2, 0.5, 1, {0, 0}},
    {cubics[11], 4, 0.5, 0, {0, 0}},
    {lines[9], 2, 0.5, 1, {0, 0}},
    {cubics[12], 4, 0.5, 0, {0, 0}},
    {lines[8], 2, 0.5, 0, {0, 0}},
    {cubics[11], 4, 0.5, 1, {0, 0}},
    {lines[9], 2, 0.5, 0, {0, 0}},
};

/*static const SortSet set13[] = {
    {cubics[13], 4, 0.5, 0.400631046, {0, 0}},
    {lines[10], 2, 0.791666667, 0.928, {0, 0}},
    {lines[10], 2, 0.791666667, 0.333333333, {0, 0}},
    {cubics[13], 4, 0.5, 0.866666667, {0, 0}},
};*/

static const SortSet set14[] = {
    {quads[2], 3, 0.5, 0.310102051, {0, 0}},
    {quads[3], 3, 0.5, 0.2, {0, 0}},
    {quads[3], 3, 0.5, 0.770156212, {0, 0}},
    {quads[2], 3, 0.5, 0.7, {0, 0}},
};

/*static const SortSet set15[] = {
    {cubics[14], 4, 0.93081374, 1, {0, 0}},
    {cubics[15], 4, 0.188518131, 0, {0, 0}},
    {cubics[14], 4, 0.93081374, 0, {0, 0}},
};*/

static const SortSet set16[] = {
    {cubics[17], 4, 0.0682619216, 0, {130.042755f,11417.4131f}},
    {cubics[16], 4, 0.812302088, 1, {130,11417}},
    {cubics[17], 4, 0.0682619216, 1, {132,11419}},
};

static const SortSet set17[] = {
    {lines[11], 2, 0.888889581, 1, {0, 0}},
    {cubics[18], 4, 0.999996241, 0, {0, 0}},
    {lines[11], 2, 0.888889581, 0, {0, 0}},
    {cubics[18], 4, 0.999996241, 1, {0, 0}},
};

struct SortSetTests {
    const char* name;
    const SortSet* set;
    size_t count;
    SkPoint startPt;
};

#define TEST_ENTRY(name) #name, name, SK_ARRAY_COUNT(name)

static const SortSetTests tests[] = {
    { TEST_ENTRY(set17), {0, 0}},
    { TEST_ENTRY(set16), {130.090179f,11417.5957f} },
//    { TEST_ENTRY(set15), {0, 0}},
    { TEST_ENTRY(set14), {0, 0}},
//    { TEST_ENTRY(set13), {0, 0}},
    { TEST_ENTRY(set12), {0, 0}},
    { TEST_ENTRY(set11), {0, 0}},
    { TEST_ENTRY(set10), {0, 0}},
    { TEST_ENTRY(set9), {0, 0}},
    { TEST_ENTRY(set6a), {3.55555558f,2.77777767f} },
    { TEST_ENTRY(set8a), {1.5f,1} },
    { TEST_ENTRY(set8), {0, 0}},
    { TEST_ENTRY(set7), {0, 0}},
    { TEST_ENTRY(set6a), {3.55555558f,2.77777767f} },
    { TEST_ENTRY(set6), {0, 0}},
    { TEST_ENTRY(set5a), {306,596} },
    { TEST_ENTRY(set5), {0, 0}},
//    { TEST_ENTRY(set4), {0, 0}},
    { TEST_ENTRY(set3), {0, 0}},
    { TEST_ENTRY(set2), {0, 0}},
//    { TEST_ENTRY(set1a), {3.70370364f,3.14814806f} },
//    { TEST_ENTRY(set1), {0, 0}},
};

#undef TEST_ENTRY

static void setup(const SortSet* set, const size_t idx,
        SkOpSegment* seg, int* ts, const SkPoint& startPt) {
    SkPoint start, end;
    const SkPoint* data = set[idx].ptData;
    bool useIntersectPt = startPt.fX != 0 || startPt.fY != 0;
    if (useIntersectPt) {
        start = startPt;
        end = set[idx].endPt;
    }
    switch(set[idx].ptCount) {
        case 2: {
            SkASSERT(ValidPoints(data, 2));
            seg->addLine(data, false, false);
            SkDLine dLine;
            dLine.set(set[idx].ptData);
            SkASSERT(ValidLine(dLine));
            if (useIntersectPt) {
                break;
            }
            start = dLine.ptAtT(set[idx].tStart).asSkPoint();
            end = dLine.ptAtT(set[idx].tEnd).asSkPoint();
            } break;
        case 3: {
            SkASSERT(ValidPoints(data, 3));
            seg->addQuad(data, false, false);
            SkDQuad dQuad;
            dQuad.set(set[idx].ptData);
            SkASSERT(ValidQuad(dQuad));
             if (useIntersectPt) {
                break;
            }
            start = dQuad.ptAtT(set[idx].tStart).asSkPoint();
            end = dQuad.ptAtT(set[idx].tEnd).asSkPoint();
            } break;
        case 4: {
            SkASSERT(ValidPoints(data, 4));
            seg->addCubic(data, false, false);
            SkDCubic dCubic;
            dCubic.set(set[idx].ptData);
            SkASSERT(ValidCubic(dCubic));
            if (useIntersectPt) {
                break;
            }
            start = dCubic.ptAtT(set[idx].tStart).asSkPoint();
            end = dCubic.ptAtT(set[idx].tEnd).asSkPoint();
            } break;
    }
    double tStart = set[idx].tStart;
    double tEnd = set[idx].tEnd;
    seg->addT(NULL, start, tStart);
    seg->addT(NULL, end, tEnd);
    if (tStart != 0 && tEnd != 0) {
        seg->addT(NULL, set[idx].ptData[0], 0);
    }
    if (tStart != 1 && tEnd != 1) {
        seg->addT(NULL, set[idx].ptData[set[idx].ptCount - 1], 1);
    }
    int tIndex = 0;
    ts[0] = 0;
    ts[1] = 1;
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

static void testOne(skiatest::Reporter* reporter, const SortSetTests& test) {
    SkTDArray<SkOpAngle> angles;
    bool unsortable = false;
    bool unorderable = false;
    SkTArray<SkOpSegment> segs;
    for (size_t idx = 0; idx < test.count; ++idx) {
        int ts[2];
        const SortSet* set = test.set;
        SkOpSegment& seg = segs.push_back();
        setup(set, idx, &seg, ts, test.startPt);
        SkOpAngle* angle = angles.append();
        angle->set(&seg, ts[0], ts[1]);
#if DEBUG_ANGLE
        angle->setID(idx);
#endif
        if (angle->unsortable()) {
#if DEBUG_ANGLE
            SkDebugf("%s test[%s]:  angle[%d] unsortable\n", __FUNCTION__, test.name, idx);
#endif
            unsortable = true;
        }
        if (angle->unorderable()) {
#if DEBUG_ANGLE
            SkDebugf("%s test[%s]:  angle[%d] unorderable\n", __FUNCTION__, test.name, idx);
#endif
            unorderable = true;
        }
        reporter->bumpTestCount();
    }
    if (unsortable || unorderable) {
        return;
    }
#if DEBUG_ANGLE
    SkDebugf("%s test[%s]\n", __FUNCTION__, test.name);
#endif
    for (size_t idxL = 0; idxL < test.count; ++idxL) {
        const SkOpAngle& first = angles[idxL];
        for (size_t idxG = 0; idxG < test.count; ++idxG) {
            if (idxL == idxG) {
                continue;
            }
            const SkOpAngle& second = angles[idxG];
            bool compare = first < second;
            if (idxL < idxG) {
                if (!compare) {
                    SkDebugf("%s test[%s]:  first[%d] > second[%d]\n", __FUNCTION__,
                            test.name,  idxL,  idxG);
                    compare = first < second;
                }
                REPORTER_ASSERT(reporter, compare);
            } else {
                SkASSERT(idxL > idxG);
                if (compare) {
                    SkDebugf("%s test[%s]:  first[%d] < second[%d]\n", __FUNCTION__,
                            test.name,  idxL,  idxG);
                    compare = first < second;
                }
                REPORTER_ASSERT(reporter, !compare);
            }
            compare = second < first;
            if (idxL < idxG) {
                if (compare) {
                    SkDebugf("%s test[%s]:  second[%d] < first[%d]\n", __FUNCTION__,
                            test.name,  idxL,  idxG);
                    compare = second < first;
                }
                REPORTER_ASSERT(reporter, !compare);
            } else {
                SkASSERT(idxL > idxG);
                if (!compare) {
                    SkDebugf("%s test[%s]:  second[%d] > first[%d]\n", __FUNCTION__,
                            test.name,  idxL,  idxG);
                    compare = second < first;
                }
                REPORTER_ASSERT(reporter, compare);
            }
        }
    }
}

static void PathOpsAngleTest(skiatest::Reporter* reporter) {
    for (size_t index = 0; index < SK_ARRAY_COUNT(tests); ++index) {
        const SortSetTests& test = tests[index];
        testOne(reporter, test);
        reporter->bumpTestCount();
    }
}

static void PathOpsAngleTestOne(skiatest::Reporter* reporter) {
    size_t index = 0;
    const SortSetTests& test = tests[index];
    testOne(reporter, test);
}

#if 0
static int find_slop(double x, double y, double rx, double ry) {
    int slopBits = 0;
    bool less1, less2;
    double absX = fabs(x);
    double absY = fabs(y);
    double length = absX < absY ? absX / 2 + absY : absX + absY / 2;
    int exponent;
    (void) frexp(length, &exponent);
    double epsilon = ldexp(FLT_EPSILON, exponent);
    do {
        // get the length as the larger plus half the smaller (both same signs)
        // find the ulps of the length
        // compute the offsets from there
        double xSlop = epsilon * slopBits;
        double ySlop = x * y < 0 ? -xSlop : xSlop; // OPTIMIZATION: use copysign / _copysign ?
        double x1 = x - xSlop;
        double y1 = y + ySlop;
        double x_ry1 = x1 * ry;
        double rx_y1 = rx * y1;
        less1 = x_ry1 < rx_y1;
        double x2 = x + xSlop;
        double y2 = y - ySlop;
        double x_ry2 = x2 * ry;
        double rx_y2 = rx * y2;
        less2 = x_ry2 < rx_y2;
    } while (less1 == less2 && ++slopBits);
    return slopBits;
}

// from http://stackoverflow.com/questions/1427422/cheap-algorithm-to-find-measure-of-angle-between-vectors
static double diamond_angle(double y, double x)
{
    if (y >= 0)
        return (x >= 0 ? y/(x+y) : 1-x/(-x+y));
    else
        return (x < 0 ? 2-y/(-x-y) : 3+x/(x-y));
}

static const double slopTests[][4] = {
   // x                      y                       rx                      ry
    {-0.058554756452593892, -0.18804585843827226, -0.018568569646021160, -0.059615294434479438},
    {-0.0013717412948608398, 0.0041152238845825195, -0.00045837944195925573, 0.0013753175735478074},
    {-2.1033774145221198, -1.4046019261273715e-008, -0.70062688352066704, -1.2706324683777995e-008},
};

static void PathOpsAngleFindSlop(skiatest::Reporter* reporter) {
    for (size_t index = 0; index < SK_ARRAY_COUNT(slopTests); ++index) {
        const double* slopTest = slopTests[index];
        double x = slopTest[0];
        double y = slopTest[1];
        double rx = slopTest[2];
        double ry = slopTest[3];
        SkDebugf("%s  xy %d=%d\n", __FUNCTION__, (int) index, find_slop(x, y, rx, ry));
        SkDebugf("%s rxy %d=%d\n", __FUNCTION__, (int) index, find_slop(rx, ry, x, y));
        double angle = diamond_angle(y, x);
        double rAngle = diamond_angle(ry, rx);
        double diff = fabs(angle - rAngle);
        SkDebugf("%s diamond xy=%1.9g rxy=%1.9g diff=%1.9g factor=%d\n", __FUNCTION__,
                angle, rAngle, diff, (int) (diff / FLT_EPSILON));

    }
}
#endif

#include "TestClassDef.h"
DEFINE_TESTCLASS_SHORT(PathOpsAngleTest)

DEFINE_TESTCLASS_SHORT(PathOpsAngleTestOne)

// DEFINE_TESTCLASS_SHORT(PathOpsAngleFindSlop)
