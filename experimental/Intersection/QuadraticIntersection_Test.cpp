/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "CurveIntersection.h"
#include "CurveUtilities.h"
#include "Intersection_Tests.h"
#include "Intersections.h"
#include "LineIntersection.h"
#include "QuadraticIntersection_TestData.h"
#include "QuadraticUtilities.h"
#include "TestUtilities.h"

const int firstQuadIntersectionTest = 9;

static void standardTestCases() {
    for (size_t index = firstQuadIntersectionTest; index < quadraticTests_count; ++index) {
        const Quadratic& quad1 = quadraticTests[index][0];
        const Quadratic& quad2 = quadraticTests[index][1];
        Quadratic reduce1, reduce2;
        int order1 = reduceOrder(quad1, reduce1);
        int order2 = reduceOrder(quad2, reduce2);
        if (order1 < 3) {
            printf("[%d] quad1 order=%d\n", (int) index, order1);
        }
        if (order2 < 3) {
            printf("[%d] quad2 order=%d\n", (int) index, order2);
        }
        if (order1 == 3 && order2 == 3) {
            Intersections intersections, intersections2;
            intersect(reduce1, reduce2, intersections);
            intersect2(reduce1, reduce2, intersections2);
            SkASSERT(intersections.used() == intersections2.used());
            if (intersections.intersected()) {
                for (int pt = 0; pt < intersections.used(); ++pt) {
                    double tt1 = intersections.fT[0][pt];
                    double tx1, ty1;
                    xy_at_t(quad1, tt1, tx1, ty1);
                    double tt2 = intersections.fT[1][pt];
                    double tx2, ty2;
                    xy_at_t(quad2, tt2, tx2, ty2);
                    if (!approximately_equal(tx1, tx2)) {
                        printf("%s [%d,%d] x!= t1=%g (%g,%g) t2=%g (%g,%g)\n",
                            __FUNCTION__, (int)index, pt, tt1, tx1, ty1, tt2, tx2, ty2);
                    }
                    if (!approximately_equal(ty1, ty2)) {
                        printf("%s [%d,%d] y!= t1=%g (%g,%g) t2=%g (%g,%g)\n",
                            __FUNCTION__, (int)index, pt, tt1, tx1, ty1, tt2, tx2, ty2);
                    }
                    tt1 = intersections2.fT[0][pt];
                    SkASSERT(approximately_equal(intersections.fT[0][pt], tt1));
                    tt2 = intersections2.fT[1][pt];
                    SkASSERT(approximately_equal(intersections.fT[1][pt], tt2));
                }
            }
        }
    }
}

static const Quadratic testSet[] = {
  {{1.7465749139282332,1.9930452039527999}, {1.8320006564080331,1.859481345189089}, {1.8731035127758437,1.6344055934266613}},
  {{1.8731035127758437,1.6344055934266613}, {1.89928170345231,1.5006405518943067}, {1.9223833226085514,1.3495796165215643}},
  {{1.74657491,1.9930452}, {1.87407679,1.76762853}, {1.92238332,1.34957962}},
  {{0.60797907,1.68776977}, {1.0447864,1.50810914}, {1.87464474,1.63655092}},
  {{1.87464474,1.63655092}, {2.70450308,1.76499271}, {4,3}},

{{1.2071879545809394,0.82163474041730045}, {1.1534203513372994,0.52790870069930229}, {1.0880000000000001,0.29599999999999982}}, //t=0.63155333662549329,0.80000000000000004
{{0.33333333333333326,0.81481481481481488}, {0.63395173631977997,0.68744136726313931}, {1.205684411948591,0.81344322326274499}},
{{0.33333333333333326,0.81481481481481488}, {0.63396444791444551,0.68743368362444768}, {1.205732763658403,0.81345617746834109}},//t=0.33333333333333331,0.63396444791444551
{{1.205684411948591,0.81344322326274499}, {1.2057085875611198,0.81344969999329253}, {1.205732763658403,0.81345617746834109}},

  {{1.20718795,0.82163474}, {1.15342035,0.527908701}, {1.088,0.296}},
  {{1.20568441,0.813443223}, {1.20570859,0.8134497}, {1.20573276,0.813456177}},

  {{41.5072916,87.1234036}, {28.2747836,80.9545395}, {23.5780771,69.3344126}},
  {{72.9633878,95.6593007}, {42.7738746,88.4730382}, {31.1932785,80.2458029}},

  {{31.1663962,54.7302484}, {31.1662882,54.7301074}, {31.1663969,54.7302485}},
  {{26.0404936,45.4260361}, {27.7887523,33.1863051}, {40.8833242,26.0301855}},

  {{29.9404074,49.1672596}, {44.3131071,45.3915253}, {58.1067559,59.5061814}},
  {{72.6510251,64.2972928}, {53.6989659,60.1862397}, {35.2053722,44.8391126}},

{{52.14807018377202, 65.012420045148644}, {44.778669050208237, 66.315562705604378}, {51.619118408823567, 63.787827046262684}},
{{30.004993234763383, 93.921296668202288}, {53.384822003076991, 60.732180341802753}, {58.652998934338584, 43.111073088306185}},

{{80.897794748143198, 49.236332042718459}, {81.082078218891212, 64.066749904488631}, {69.972305057149981, 72.968595519850993}},
{{72.503745601281395, 32.952320736577882}, {88.030880716061645, 38.137194847810164}, {73.193774825517906, 67.773492479591397}},

{{67.426548091427676, 37.993772624988935}, {51.129513170665035, 57.542281234563646}, {44.594748190899189, 65.644267382683879}},
{{61.336508189019057, 82.693132843213675}, {54.825078921449354, 71.663932799212432}, {47.727444217558926, 61.4049645128392}},

{{67.4265481,37.9937726}, {51.1295132,57.5422812}, {44.5947482,65.6442674}},
{{61.3365082,82.6931328}, {54.8250789,71.6639328}, {47.7274442,61.4049645}},

{{53.774852327053594, 53.318060789841951}, {45.787877803416805, 51.393492026284981}, {46.703936967162392, 53.06860709822206}},
{{46.703936967162392, 53.06860709822206}, {47.619996130907957, 54.74372217015916}, {53.020051653535361, 48.633140968832024}},

{{50.934805397717923, 51.52391952648901}, {56.803308902971423, 44.246234610627596}, {69.776888596721406, 40.166645096692555}},
{{50.230212796400401, 38.386469101526998}, {49.855620812184917, 38.818990392153609}, {56.356567496227363, 47.229909093319407}},

{{36.148792695174222, 70.336952793070424}, {36.141613037691357, 70.711654739870085}, {36.154708826402597, 71.088492662905836}},
{{35.216235592661825, 70.580199617313212}, {36.244476835123969, 71.010897787304074}, {37.230244263238326, 71.423156953613102}},

// this pair is nearly coincident, and causes the quartic code to produce bad
// data. Mathematica doesn't think they touch. Graphically, I can't tell.
// it may not be so bad to pretend that they don't touch, if I can detect that
{{369.848602,145.680267}, {382.360413,121.298294}, {406.207703,121.298294}},
{{369.850525,145.675964}, {382.362915,121.29287}, {406.211273,121.29287}},

{{33.567436351153468, 62.336347586395924}, {35.200980274619084, 65.038561460144479}, {36.479571811084995, 67.632178905412445}},
{{41.349524945572696, 67.886658677862641}, {39.125562529359087, 67.429772735149214}, {35.600314083992416, 66.705372160552685}},

{{67.25299631583178, 21.109080184767524}, {43.617595267398613, 33.658034168577529}, {33.38371819435676, 44.214192553988745}},
{{40.476838859398541, 39.543209911285999}, {36.701186108431131, 34.8817994016458}, {30.102144288878023, 26.739063172945315}},

{{25.367434474345036, 50.4712103169743}, {17.865013304933097, 37.356741010559439}, {16.818988838905465, 37.682915484123129}},
{{16.818988838905465, 37.682915484123129}, {15.772964372877833, 38.009089957686811}, {20.624104547604965, 41.825131596683121}},

{{26.440225044088567, 79.695009812848298}, {26.085525979582247, 83.717928354134784}, {27.075079976297072, 84.820633667838905}},
{{27.075079976297072, 84.820633667838905}, {28.276546859574015, 85.988574184029034}, {25.649263209500006, 87.166762066617025}},

{{34.879150914024962, 83.862726601601125}, {35.095810134304429, 83.693473210169543}, {35.359284111931586, 83.488069234177502}},
{{54.503204203015471, 76.094098492518242}, {51.366889541918894, 71.609856061299155}, {46.53086955445437, 69.949863036494207}},

{{0, 0}, {1, 0}, {0, 3}},
{{1, 0}, {0, 1}, {1, 1}},
{{369.961151,137.980698}, {383.970093,121.298294}, {406.213287,121.298294}},
{{353.2948,194.351074}, {353.2948,173.767563}, {364.167572,160.819855}},
{{360.416077,166.795715}, {370.126831,147.872162}, {388.635406,147.872162}},
{{406.236359,121.254936}, {409.445679,121.254936}, {412.975952,121.789818}},
{{406.235992,121.254936}, {425.705902,121.254936}, {439.71994,137.087616}},

{{369.8543701171875, 145.66734313964844}, {382.36788940429688, 121.28203582763672}, {406.21844482421875, 121.28203582763672}},
{{369.96469116210938, 137.96672058105469}, {383.97555541992188, 121.28203582763672}, {406.2218017578125, 121.28203582763672}},

    {{369.962311, 137.976044}, {383.971893, 121.29287}, {406.216125, 121.29287}},

    {{400.121704, 149.468719}, {391.949493, 161.037186}, {391.949493, 181.202423}},
    {{391.946747, 181.839218}, {391.946747, 155.62442}, {406.115479, 138.855438}},
    {{360.048828125, 229.2578125}, {360.048828125, 224.4140625}, {362.607421875, 221.3671875}},
    {{362.607421875, 221.3671875}, {365.166015625, 218.3203125}, {369.228515625, 218.3203125}},
    {{8, 8}, {10, 10}, {8, -10}},
    {{8, 8}, {12, 12}, {14, 4}},
    {{8, 8}, {9, 9}, {10, 8}}
};

const size_t testSetCount = sizeof(testSet) / sizeof(testSet[0]);

#define ONE_OFF_DEBUG 1

static void oneOffTest1(size_t outer, size_t inner) {
    const Quadratic& quad1 = testSet[outer];
    const Quadratic& quad2 = testSet[inner];
    Intersections intersections2;
    intersect2(quad1, quad2, intersections2);
    if (intersections2.fUnsortable) {
        SkASSERT(0);
        return;
    }
    for (int pt = 0; pt < intersections2.used(); ++pt) {
        double tt1 = intersections2.fT[0][pt];
        double tx1, ty1;
        xy_at_t(quad1, tt1, tx1, ty1);
        int pt2 = intersections2.fFlip ? intersections2.used() - pt - 1 : pt;
        double tt2 = intersections2.fT[1][pt2];
        double tx2, ty2;
        xy_at_t(quad2, tt2, tx2, ty2);
        if (!AlmostEqualUlps(tx1, tx2)) {
            SkDebugf("%s [%d,%d] x!= t1=%g (%g,%g) t2=%g (%g,%g)\n",
                __FUNCTION__, (int)outer, (int)inner, tt1, tx1, ty1, tt2, tx2, ty2);
            SkASSERT(0);
        }
        if (!AlmostEqualUlps(ty1, ty2)) {
            SkDebugf("%s [%d,%d] y!= t1=%g (%g,%g) t2=%g (%g,%g)\n",
                __FUNCTION__, (int)outer, (int)inner, tt1, tx1, ty1, tt2, tx2, ty2);
            SkASSERT(0);
        }
#if ONE_OFF_DEBUG
        SkDebugf("%s [%d][%d] t1=%1.9g (%1.9g, %1.9g) t2=%1.9g\n", __FUNCTION__,
            outer, inner, tt1, tx1, ty1, tt2);
#endif
    }
}

void QuadraticIntersection_OneOffTest() {
    oneOffTest1(0, 3);
    oneOffTest1(0, 4);
    oneOffTest1(1, 3);
    oneOffTest1(1, 4);
}

static void oneOffTests() {
    for (size_t outer = 0; outer < testSetCount - 1; ++outer) {
        for (size_t inner = outer + 1; inner < testSetCount; ++inner) {
            oneOffTest1(outer, inner);
        }
    }
}

static const Quadratic coincidentTestSet[] = {
    {{369.850525, 145.675964}, {382.362915, 121.29287}, {406.211273, 121.29287}},
    {{369.850525, 145.675964}, {382.362915, 121.29287}, {406.211273, 121.29287}},
    {{8, 8}, {10, 10}, {8, -10}},
    {{8, -10}, {10, 10}, {8, 8}},
};

const size_t coincidentTestSetCount = sizeof(coincidentTestSet) / sizeof(coincidentTestSet[0]);

static void coincidentTest() {
    for (size_t testIndex = 0; testIndex < coincidentTestSetCount - 1; testIndex += 2) {
        const Quadratic& quad1 = coincidentTestSet[testIndex];
        const Quadratic& quad2 = coincidentTestSet[testIndex + 1];
        Intersections intersections2;
        intersect2(quad1, quad2, intersections2);
        SkASSERT(intersections2.coincidentUsed() == 2);
        for (int pt = 0; pt < intersections2.coincidentUsed(); ++pt) {
            double tt1 = intersections2.fT[0][pt];
            double tt2 = intersections2.fT[1][pt];
            SkASSERT(approximately_equal(1, tt1) || approximately_zero(tt1));
            SkASSERT(approximately_equal(1, tt2) || approximately_zero(tt2));
        }
    }
}

void QuadraticIntersection_Test() {
    oneOffTests();
    coincidentTest();
    standardTestCases();
}

static int floatSign(double x) {
    return x < 0 ? -1 : x > 0 ? 1 : 0;
}

static const Quadratic pointFinderTestSet[] = {
                                                                                                                                //>=0.633974464         0.633974846 <=
{{1.2071879545809394,0.82163474041730045}, {1.1534203513372994,0.52790870069930229}, {1.0880000000000001,0.29599999999999982}}, //t=0.63155333662549329,0.80000000000000004
{{1.2071879545809394,0.82163474041730045}, {1.2065040319428038,0.81766753259119995}, {1.2058123269101506,0.81370135061854221}}, //t=0.63155333662549329,0.6339049773632347
{{1.2058123269101506,0.81370135061854221}, {1.152376363978022,0.5244097415381026}, {1.0880000000000001,0.29599999999999982}},   //t=0.6339049773632347, 0.80000000000000004
                                                                                                                                //>=0.633974083         0.633975227 <=
{{0.33333333333333326,0.81481481481481488}, {0.63395173631977997,0.68744136726313931}, {1.205684411948591,0.81344322326274499}},//t=0.33333333333333331,0.63395173631977986
{{0.33333333333333326,0.81481481481481488}, {0.63396444791444551,0.68743368362444768}, {1.205732763658403,0.81345617746834109}},//t=0.33333333333333331,0.63396444791444551
{{1.205684411948591,0.81344322326274499}, {1.2057085875611198,0.81344969999329253}, {1.205732763658403,0.81345617746834109}},   //t=0.63395173631977986,0.63396444791444551
{{1.205732763658403,0.81345617746834109}, {1.267928895828891,0.83008534558465619}, {1.3333333333333333,0.85185185185185175}},   //t=0.63396444791444551,0.66666666666666663
};

static void pointFinder(const Quadratic& q1, const Quadratic& q2) {
    for (int index = 0; index < 3; ++index) {
        double t = nearestT(q1, q2[index]);
        _Point onQuad;
        xy_at_t(q1, t, onQuad.x, onQuad.y);
        SkDebugf("%s t=%1.9g (%1.9g,%1.9g) dist=%1.9g\n", __FUNCTION__, t, onQuad.x, onQuad.y,
                onQuad.distance(q2[index]));
        double left[3];
        left[0] = is_left((const _Line&) q1[0], q2[index]);
        left[1] = is_left((const _Line&) q1[1], q2[index]);
        _Line diag = {q1[0], q1[2]};
        left[2] = is_left(diag, q2[index]);
        SkDebugf("%s left=(%d, %d, %d) inHull=%s\n", __FUNCTION__, floatSign(left[0]),
                floatSign(left[1]), floatSign(left[2]),
                point_in_hull(q1, q2[index]) ? "true" : "false");
    }
    SkDebugf("\n");
}

static void hullIntersect(const Quadratic& q1, const Quadratic& q2) {
    SkDebugf("%s", __FUNCTION__);
    double aRange[2], bRange[2];
    for (int i1 = 0; i1 < 3; ++i1) {
        _Line l1 = {q1[i1], q1[(i1 + 1) % 3]};
        for (int i2 = 0; i2 < 3; ++i2) {
            _Line l2 = {q2[i2], q2[(i2 + 1) % 3]};
            if (intersect(l1, l2, aRange, bRange)) {
                SkDebugf(" [%d,%d]", i1, i2);
            }
        }
    }
    SkDebugf("\n");
}

void QuadraticIntersection_PointFinder() {
    pointFinder(pointFinderTestSet[0], pointFinderTestSet[4]);
    pointFinder(pointFinderTestSet[4], pointFinderTestSet[0]);
    pointFinder(pointFinderTestSet[0], pointFinderTestSet[6]);
    pointFinder(pointFinderTestSet[6], pointFinderTestSet[0]);
    hullIntersect(pointFinderTestSet[0], pointFinderTestSet[4]);
    hullIntersect(pointFinderTestSet[0], pointFinderTestSet[6]);
}
