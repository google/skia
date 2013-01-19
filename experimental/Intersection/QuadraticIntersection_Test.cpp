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
#include "QuadraticIntersection_TestData.h"
#include "TestUtilities.h"
#include "SkTypes.h"

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

static void oneOffTest() {
    for (size_t outer = 0; outer < testSetCount - 1; ++outer) {
        for (size_t inner = outer + 1; inner < testSetCount; ++inner) {
            const Quadratic& quad1 = testSet[outer];
            const Quadratic& quad2 = testSet[inner];
            Intersections intersections2;
            intersect2(quad1, quad2, intersections2);
            if (intersections2.fUnsortable) {
                continue;
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
                SkDebugf("%s [%d][%d] t1=%1.9g (%1.9g, %1.9g) t2=%1.9g\n", __FUNCTION__,
                    outer, inner, tt1, tx1, tx2, tt2);
            }
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
    oneOffTest();
    coincidentTest();
    standardTestCases();
}
