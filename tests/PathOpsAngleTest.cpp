/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsTestCommon.h"
#include "SkIntersections.h"
#include "SkOpSegment.h"
#include "SkPathOpsTriangle.h"
#include "SkRandom.h"
#include "SkTArray.h"
#include "SkTSort.h"
#include "Test.h"

static bool gDisableAngleTests = true;

static float next(float f)
{
    int fBits = SkFloatAs2sCompliment(f);
    ++fBits;
    float fNext = Sk2sComplimentAsFloat(fBits);
    return fNext;
}

static float prev(float f)
{
    int fBits = SkFloatAs2sCompliment(f);
    --fBits;
    float fNext = Sk2sComplimentAsFloat(fBits);
    return fNext;
}

DEF_TEST(PathOpsAngleFindCrossEpsilon, reporter) {
    if (gDisableAngleTests) {
        return;
    }
    SkRandom ran;
    int maxEpsilon = 0;
    for (int index = 0; index < 10000000; ++index) {
        SkDLine line = {{{0, 0}, {ran.nextRangeF(0.0001f, 1000), ran.nextRangeF(0.0001f, 1000)}}};
        for (int inner = 0; inner < 10; ++inner) {
            float t = ran.nextRangeF(0.0001f, 1);
            SkDPoint dPt = line.ptAtT(t);
            SkPoint pt = dPt.asSkPoint();
            float xs[3] = { prev(pt.fX), pt.fX, next(pt.fX) };
            float ys[3] = { prev(pt.fY), pt.fY, next(pt.fY) };
            for (int xIdx = 0; xIdx < 3; ++xIdx) {
                for (int yIdx = 0; yIdx < 3; ++yIdx) {
                    SkPoint test = { xs[xIdx], ys[yIdx] };
                    float p1 = SkDoubleToScalar(line[1].fX * test.fY);
                    float p2 = SkDoubleToScalar(line[1].fY * test.fX);
                    int p1Bits = SkFloatAs2sCompliment(p1);
                    int p2Bits = SkFloatAs2sCompliment(p2);
                    int epsilon = abs(p1Bits - p2Bits);
                    if (maxEpsilon < epsilon) {
                        SkDebugf("line={{0, 0}, {%1.7g, %1.7g}} t=%1.7g pt={%1.7g, %1.7g}"
                            " epsilon=%d\n",
                            line[1].fX, line[1].fY, t, test.fX, test.fY, epsilon);
                        maxEpsilon = epsilon;
                    }
                }
            }
        }
    }
}

DEF_TEST(PathOpsAngleFindQuadEpsilon, reporter) {
    if (gDisableAngleTests) {
        return;
    }
    SkRandom ran;
    int maxEpsilon = 0;
    double maxAngle = 0;
    for (int index = 0; index < 100000; ++index) {
        SkDLine line = {{{0, 0}, {ran.nextRangeF(0.0001f, 1000), ran.nextRangeF(0.0001f, 1000)}}};
        float t = ran.nextRangeF(0.0001f, 1);
        SkDPoint dPt = line.ptAtT(t);
        float t2 = ran.nextRangeF(0.0001f, 1);
        SkDPoint qPt = line.ptAtT(t2);
        float t3 = ran.nextRangeF(0.0001f, 1);
        SkDPoint qPt2 = line.ptAtT(t3);
        qPt.fX += qPt2.fY;
        qPt.fY -= qPt2.fX;
        SkDQuad quad = {{line[0], dPt, qPt}};
        // binary search for maximum movement of quad[1] towards test that still has 1 intersection
        double moveT = 0.5f;
        double deltaT = moveT / 2;
        SkDPoint last;
        do {
            last = quad[1];
            quad[1].fX = dPt.fX - line[1].fY * moveT;
            quad[1].fY = dPt.fY + line[1].fX * moveT;
            SkIntersections i;
            i.intersect(quad, line);
            REPORTER_ASSERT(reporter, i.used() > 0);
            if (i.used() == 1) {
                moveT += deltaT;
            } else {
                moveT -= deltaT;
            }
            deltaT /= 2;
        } while (last.asSkPoint() != quad[1].asSkPoint());
        float p1 = SkDoubleToScalar(line[1].fX * last.fY);
        float p2 = SkDoubleToScalar(line[1].fY * last.fX);
        int p1Bits = SkFloatAs2sCompliment(p1);
        int p2Bits = SkFloatAs2sCompliment(p2);
        int epsilon = abs(p1Bits - p2Bits);
        if (maxEpsilon < epsilon) {
            SkDebugf("line={{0, 0}, {%1.7g, %1.7g}} t=%1.7g/%1.7g/%1.7g moveT=%1.7g"
                    " pt={%1.7g, %1.7g} epsilon=%d\n",
                    line[1].fX, line[1].fY, t, t2, t3, moveT, last.fX, last.fY, epsilon);
            maxEpsilon = epsilon;
        }
        double a1 = atan2(line[1].fY, line[1].fX);
        double a2 = atan2(last.fY, last.fX);
        double angle = fabs(a1 - a2);
        if (maxAngle < angle) {
            SkDebugf("line={{0, 0}, {%1.7g, %1.7g}} t=%1.7g/%1.7g/%1.7g moveT=%1.7g"
                    " pt={%1.7g, %1.7g} angle=%1.7g\n",
                    line[1].fX, line[1].fY, t, t2, t3, moveT, last.fX, last.fY, angle);
            maxAngle = angle;
        }
    }
}

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

DEF_TEST(PathOpsAngleFindSlop, reporter) {
    if (gDisableAngleTests) {
        return;
    }
    for (int index = 0; index < (int) SK_ARRAY_COUNT(slopTests); ++index) {
        const double* slopTest = slopTests[index];
        double x = slopTest[0];
        double y = slopTest[1];
        double rx = slopTest[2];
        double ry = slopTest[3];
        SkDebugf("%s  xy %d=%d\n", __FUNCTION__, index, find_slop(x, y, rx, ry));
        SkDebugf("%s rxy %d=%d\n", __FUNCTION__, index, find_slop(rx, ry, x, y));
        double angle = diamond_angle(y, x);
        double rAngle = diamond_angle(ry, rx);
        double diff = fabs(angle - rAngle);
        SkDebugf("%s diamond xy=%1.9g rxy=%1.9g diff=%1.9g factor=%d\n", __FUNCTION__,
                angle, rAngle, diff, (int) (diff / FLT_EPSILON));
    }
}

class PathOpsAngleTester {
public:
    static int After(const SkOpAngle& lh, const SkOpAngle& rh) {
        return lh.after(&rh);
    }

    static int ConvexHullOverlaps(const SkOpAngle& lh, const SkOpAngle& rh) {
        return lh.convexHullOverlaps(rh);
    }

    static int Orderable(const SkOpAngle& lh, const SkOpAngle& rh) {
        return lh.orderable(rh);
    }

    static int EndsIntersect(const SkOpAngle& lh, const SkOpAngle& rh) {
        return lh.endsIntersect(rh);
    }

    static void SetNext(SkOpAngle& lh, SkOpAngle& rh) {
        lh.fNext = &rh;
    }
};

class PathOpsSegmentTester {
public:
    static void ConstructCubic(SkOpSegment* segment, SkPoint shortCubic[4]) {
        segment->debugConstructCubic(shortCubic);
    }

    static void ConstructLine(SkOpSegment* segment, SkPoint shortLine[2]) {
        segment->debugConstructLine(shortLine);
    }

    static void ConstructQuad(SkOpSegment* segment, SkPoint shortQuad[3]) {
        segment->debugConstructQuad(shortQuad);
    }

    static void DebugReset(SkOpSegment* segment) {
        segment->debugReset();
    }
};

struct CircleData {
    const SkDCubic fPts;
    const int fPtCount;
    SkPoint fShortPts[4];
};

static CircleData circleDataSet[] = {
    { {{{313.0155029296875, 207.90290832519531}, {320.05078125, 227.58743286132812}}}, 2, {} },
    { {{{313.0155029296875, 207.90290832519531}, {313.98246891063195, 219.33615203830394},
            {320.05078125, 227.58743286132812}}}, 3, {} },
};

static const int circleDataSetSize = (int) SK_ARRAY_COUNT(circleDataSet);

DEF_TEST(PathOpsAngleCircle, reporter) {
    SkOpSegment segment[2];
    for (int index = 0; index < circleDataSetSize; ++index) {
        CircleData& data = circleDataSet[index];
        for (int idx2 = 0; idx2 < data.fPtCount; ++idx2) {
            data.fShortPts[idx2] = data.fPts.fPts[idx2].asSkPoint();
        }
        switch (data.fPtCount) {
            case 2:
                PathOpsSegmentTester::ConstructLine(&segment[index], data.fShortPts);
                break;
            case 3:
                PathOpsSegmentTester::ConstructQuad(&segment[index], data.fShortPts);
                break;
            case 4:
                PathOpsSegmentTester::ConstructCubic(&segment[index], data.fShortPts);
                break;
        }
    }
    PathOpsAngleTester::Orderable(*segment[0].debugLastAngle(), *segment[1].debugLastAngle());
}

struct IntersectData {
    const SkDCubic fPts;
    const int fPtCount;
    double fTStart;
    double fTEnd;
    SkPoint fShortPts[4];
};

static IntersectData intersectDataSet1[] = {
    { {{{322.935669,231.030273}, {312.832214,220.393295}, {312.832214,203.454178}}}, 3,
            0.865309956, 0.154740299, {} },
    { {{{322.12738,233.397751}, {295.718353,159.505829}}}, 2,
            0.345028807, 0.0786326511, {} },
    { {{{322.935669,231.030273}, {312.832214,220.393295}, {312.832214,203.454178}}}, 3,
            0.865309956, 1, {} },
    { {{{322.12738,233.397751}, {295.718353,159.505829}}}, 2,
            0.345028807, 1, {} },
};

static IntersectData intersectDataSet2[] = {
    { {{{364.390686,157.898193}, {375.281769,136.674606}, {396.039917,136.674606}}}, 3,
            0.578520747, 1, {} },
    { {{{364.390686,157.898193}, {375.281769,136.674606}, {396.039917,136.674606}}}, 3,
            0.578520747, 0.536512973, {} },
    { {{{366.608826,151.196014}, {378.803101,136.674606}, {398.164948,136.674606}}}, 3,
            0.490456543, 1, {} },
};

static IntersectData intersectDataSet3[] = {
    { {{{2.000000,0.000000}, {1.33333333,0.66666667}}}, 2, 1, 0, {} },
    { {{{1.33333333,0.66666667}, {0.000000,2.000000}}}, 2, 0, 0.25, {} },
    { {{{2.000000,2.000000}, {1.33333333,0.66666667}}}, 2, 1, 0, {} },
};

static IntersectData intersectDataSet4[] = {
    { {{{1.3333333,0.6666667}, {0.000,2.000}}}, 2, 0.250000006, 0, {} },
    { {{{1.000,0.000}, {1.000,1.000}}}, 2, 1, 0, {} },
    { {{{1.000,1.000}, {0.000,0.000}}}, 2, 0, 1, {} },
};

static IntersectData intersectDataSet5[] = {
    { {{{0.000,0.000}, {1.000,0.000}, {1.000,1.000}}}, 3, 1, 0.666666667, {} },
    { {{{0.000,0.000}, {2.000,1.000}, {0.000,2.000}}}, 3, 0.5, 1, {} },
    { {{{0.000,0.000}, {2.000,1.000}, {0.000,2.000}}}, 3, 0.5, 0, {} },
};

static IntersectData intersectDataSet6[] = { // pathops_visualizer.htm:3658
    { {{{0.000,1.000}, {3.000,4.000}, {1.000,0.000}, {3.000,0.000}}}, 4, 0.0925339054, 0, {} }, // pathops_visualizer.htm:3616
    { {{{0.000,1.000}, {0.000,3.000}, {1.000,0.000}, {4.000,3.000}}}, 4, 0.453872386, 0, {} }, // pathops_visualizer.htm:3616
    { {{{0.000,1.000}, {3.000,4.000}, {1.000,0.000}, {3.000,0.000}}}, 4, 0.0925339054, 0.417096368, {} }, // pathops_visualizer.htm:3616
};

static IntersectData intersectDataSet7[] = { // pathops_visualizer.htm:3748
    { {{{2.000,1.000}, {0.000,1.000}}}, 2, 0.5, 0, {} }, // pathops_visualizer.htm:3706
    { {{{2.000,0.000}, {0.000,2.000}}}, 2, 0.5, 1, {} }, // pathops_visualizer.htm:3706
    { {{{0.000,1.000}, {0.000,2.000}, {2.000,0.000}, {2.000,1.000}}}, 4, 0.5, 1, {} }, // pathops_visualizer.htm:3706
}; //

static IntersectData intersectDataSet8[] = { // pathops_visualizer.htm:4194
    { {{{0.000,1.000}, {2.000,3.000}, {5.000,1.000}, {4.000,3.000}}}, 4, 0.311007457, 0.285714286, {} }, // pathops_visualizer.htm:4152
    { {{{1.000,5.000}, {3.000,4.000}, {1.000,0.000}, {3.000,2.000}}}, 4, 0.589885081, 0.999982974, {} }, // pathops_visualizer.htm:4152
    { {{{1.000,5.000}, {3.000,4.000}, {1.000,0.000}, {3.000,2.000}}}, 4, 0.589885081, 0.576935809, {} }, // pathops_visualizer.htm:4152
}; //

static IntersectData intersectDataSet9[] = { // pathops_visualizer.htm:4142
    { {{{0.000,1.000}, {2.000,3.000}, {5.000,1.000}, {4.000,3.000}}}, 4, 0.476627072, 0.311007457, {} }, // pathops_visualizer.htm:4100
    { {{{1.000,5.000}, {3.000,4.000}, {1.000,0.000}, {3.000,2.000}}}, 4, 0.999982974, 1, {} }, // pathops_visualizer.htm:4100
    { {{{0.000,1.000}, {2.000,3.000}, {5.000,1.000}, {4.000,3.000}}}, 4, 0.476627072, 1, {} }, // pathops_visualizer.htm:4100
}; //

static IntersectData intersectDataSet10[] = { // pathops_visualizer.htm:4186
    { {{{0.000,1.000}, {1.000,6.000}, {1.000,0.000}, {1.000,0.000}}}, 4, 0.788195121, 0.726275769, {} }, // pathops_visualizer.htm:4144
    { {{{0.000,1.000}, {0.000,1.000}, {1.000,0.000}, {6.000,1.000}}}, 4, 0.473378977, 1, {} }, // pathops_visualizer.htm:4144
    { {{{0.000,1.000}, {1.000,6.000}, {1.000,0.000}, {1.000,0.000}}}, 4, 0.788195121, 1, {} }, // pathops_visualizer.htm:4144
}; //

static IntersectData intersectDataSet11[] = { // pathops_visualizer.htm:4704
    { {{{979.305,561.000}, {1036.695,291.000}}}, 2, 0.888888874, 0.11111108, {} }, // pathops_visualizer.htm:4662
    { {{{1006.695,291.000}, {1023.264,291.000}, {1033.840,304.431}, {1030.318,321.000}}}, 4, 1, 0, {} }, // pathops_visualizer.htm:4662
    { {{{979.305,561.000}, {1036.695,291.000}}}, 2, 0.888888874, 1, {} }, // pathops_visualizer.htm:4662
}; //

static IntersectData intersectDataSet12[] = { // pathops_visualizer.htm:5481
    { {{{67.000,912.000}, {67.000,913.000}}}, 2, 1, 0, {} }, // pathops_visualizer.htm:5439
    { {{{67.000,913.000}, {67.000,917.389}, {67.224,921.726}, {67.662,926.000}}}, 4, 0, 1, {} }, // pathops_visualizer.htm:5439
    { {{{194.000,1041.000}, {123.860,1041.000}, {67.000,983.692}, {67.000,913.000}}}, 4, 1, 0, {} }, // pathops_visualizer.htm:5439
}; //

static IntersectData intersectDataSet13[] = { // pathops_visualizer.htm:5735
    { {{{6.000,0.000}, {0.000,4.000}}}, 2, 0.625, 0.25, {} }, // pathops_visualizer.htm:5693
    { {{{0.000,1.000}, {0.000,6.000}, {4.000,0.000}, {6.000,1.000}}}, 4, 0.5, 0.833333333, {} }, // pathops_visualizer.htm:5693
    { {{{0.000,1.000}, {0.000,6.000}, {4.000,0.000}, {6.000,1.000}}}, 4, 0.5, 0.379043969, {} }, // pathops_visualizer.htm:5693
}; //

static IntersectData intersectDataSet14[] = { // pathops_visualizer.htm:5875
    { {{{0.000,1.000}, {4.000,6.000}, {2.000,1.000}, {2.000,0.000}}}, 4, 0.0756502183, 0.0594570973, {} }, // pathops_visualizer.htm:5833
    { {{{1.000,2.000}, {0.000,2.000}, {1.000,0.000}, {6.000,4.000}}}, 4, 0.0756502184, 0, {} }, // pathops_visualizer.htm:5833
    { {{{0.000,1.000}, {4.000,6.000}, {2.000,1.000}, {2.000,0.000}}}, 4, 0.0756502183, 0.531917258, {} }, // pathops_visualizer.htm:5833
}; //

static IntersectData intersectDataSet15[] = { // pathops_visualizer.htm:6580
    { {{{490.435,879.407}, {405.593,909.436}}}, 2, 0.500554405, 1, {} }, // pathops_visualizer.htm:6538
    { {{{447.967,894.438}, {448.007,894.424}, {448.014,894.422}}}, 3, 0, 1, {} }, // pathops_visualizer.htm:6538
    { {{{490.435,879.407}, {405.593,909.436}}}, 2, 0.500554405, 0.500000273, {} }, // pathops_visualizer.htm:6538
}; //

static IntersectData intersectDataSet16[] = { // pathops_visualizer.htm:7419
    { {{{1.000,4.000}, {4.000,5.000}, {3.000,2.000}, {6.000,3.000}}}, 4, 0.5, 0, {} }, // pathops_visualizer.htm:7377
    { {{{2.000,3.000}, {3.000,6.000}, {4.000,1.000}, {5.000,4.000}}}, 4, 0.5, 0.112701665, {} }, // pathops_visualizer.htm:7377
    { {{{5.000,4.000}, {2.000,3.000}}}, 2, 0.5, 0, {} }, // pathops_visualizer.htm:7377
}; //

#define I(x) intersectDataSet##x

static IntersectData* intersectDataSets[] = {
    I(1), I(2), I(3), I(4), I(5), I(6), I(7), I(8), I(9), I(10),
    I(11), I(12), I(13), I(14), I(15), I(16),
};

#undef I
#define I(x) (int) SK_ARRAY_COUNT(intersectDataSet##x)

static const int intersectDataSetSizes[] = {
    I(1), I(2), I(3), I(4), I(5), I(6), I(7), I(8), I(9), I(10),
    I(11), I(12), I(13), I(14), I(15), I(16),
};

#undef I

static const int intersectDataSetsSize = (int) SK_ARRAY_COUNT(intersectDataSetSizes);

DEF_TEST(PathOpsAngleAfter, reporter) {
    for (int index = intersectDataSetsSize - 1; index >= 0; --index) {
        IntersectData* dataArray = intersectDataSets[index];
        const int dataSize = intersectDataSetSizes[index];
        SkOpSegment segment[3];
        for (int index2 = 0; index2 < dataSize - 2; ++index2) {
            for (int temp = 0; temp < (int) SK_ARRAY_COUNT(segment); ++temp) {
                PathOpsSegmentTester::DebugReset(&segment[temp]);
            }
            for (int index3 = 0; index3 < (int) SK_ARRAY_COUNT(segment); ++index3) {
                IntersectData& data = dataArray[index2 + index3];
                SkPoint temp[4];
                for (int idx2 = 0; idx2 < data.fPtCount; ++idx2) {
                    temp[idx2] = data.fPts.fPts[idx2].asSkPoint();
                }
                switch (data.fPtCount) {
                    case 2: {
                        SkDLine seg = SkDLine::SubDivide(temp, data.fTStart,
                                data.fTStart < data.fTEnd ? 1 : 0);
                        data.fShortPts[0] = seg[0].asSkPoint();
                        data.fShortPts[1] = seg[1].asSkPoint();
                        PathOpsSegmentTester::ConstructLine(&segment[index3], data.fShortPts);
                        } break;
                    case 3: {
                        SkDQuad seg = SkDQuad::SubDivide(temp, data.fTStart, data.fTEnd);
                        data.fShortPts[0] = seg[0].asSkPoint();
                        data.fShortPts[1] = seg[1].asSkPoint();
                        data.fShortPts[2] = seg[2].asSkPoint();
                        PathOpsSegmentTester::ConstructQuad(&segment[index3], data.fShortPts);
                        } break;
                    case 4: {
                        SkDCubic seg = SkDCubic::SubDivide(temp, data.fTStart, data.fTEnd);
                        data.fShortPts[0] = seg[0].asSkPoint();
                        data.fShortPts[1] = seg[1].asSkPoint();
                        data.fShortPts[2] = seg[2].asSkPoint();
                        data.fShortPts[3] = seg[3].asSkPoint();
                        PathOpsSegmentTester::ConstructCubic(&segment[index3], data.fShortPts);
                        } break;
                }
            }
            SkOpAngle& angle1 = *const_cast<SkOpAngle*>(segment[0].debugLastAngle());
            SkOpAngle& angle2 = *const_cast<SkOpAngle*>(segment[1].debugLastAngle());
            SkOpAngle& angle3 = *const_cast<SkOpAngle*>(segment[2].debugLastAngle());
            PathOpsAngleTester::SetNext(angle1, angle3);
       // These data sets are seeded when the set itself fails, so likely the dataset does not
       // match the expected result. The tests above return 1 when first added, but
       // return 0 after the bug is fixed.
            SkDEBUGCODE(int result =) PathOpsAngleTester::After(angle2, angle1);
            SkASSERT(result == 0 || result == 1);
        }
    }
}

void SkOpSegment::debugConstruct() {
    addStartSpan(1);
    addEndSpan(1);
    debugAddAngle(0, 1);
}

void SkOpSegment::debugAddAngle(int start, int end) {
    SkASSERT(start != end);
    SkOpAngle& angle = fAngles.push_back();
    angle.set(this, start, end);
}

void SkOpSegment::debugConstructCubic(SkPoint shortQuad[4]) {
    addCubic(shortQuad, false, false);
    addT(NULL, shortQuad[0], 0);
    addT(NULL, shortQuad[3], 1);
    debugConstruct();
}

void SkOpSegment::debugConstructLine(SkPoint shortQuad[2]) {
    addLine(shortQuad, false, false);
    addT(NULL, shortQuad[0], 0);
    addT(NULL, shortQuad[1], 1);
    debugConstruct();
}

void SkOpSegment::debugConstructQuad(SkPoint shortQuad[3]) {
    addQuad(shortQuad, false, false);
    addT(NULL, shortQuad[0], 0);
    addT(NULL, shortQuad[2], 1);
    debugConstruct();
}
