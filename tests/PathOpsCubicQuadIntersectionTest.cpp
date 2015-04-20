/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsTestCommon.h"
#include "SkIntersections.h"
#include "SkPathOpsCubic.h"
#include "SkPathOpsQuad.h"
#include "SkRandom.h"
#include "SkReduceOrder.h"
#include "Test.h"

static struct quadCubic {
    SkDCubic cubic;
    SkDQuad quad;
} quadCubicTests[] = {
    {{{{945.08099365234375, 747.1619873046875}, {982.5679931640625, 747.1619873046875}, {1013.6290283203125, 719.656005859375}, {1019.1910400390625, 683.72601318359375}}},
     {{{945, 747}, {976.0660400390625, 747}, {998.03302001953125, 725.03302001953125}}}},

    {{{{778, 14089}, {778, 14091.208984375}, {776.20916748046875, 14093}, {774, 14093}}},
     {{{778, 14089}, {777.99957275390625, 14090.65625}, {776.82843017578125, 14091.828125}}}},

    {{{{1020.08099,672.161987}, {1020.08002,630.73999}, {986.502014,597.161987}, {945.080994,597.161987}}},
     {{{1020,672}, {1020,640.93396}, {998.03302,618.96698}}}},

    {{{{778, 14089}, {778, 14091.208984375}, {776.20916748046875, 14093}, {774, 14093}}},
     {{{778, 14089}, {777.99957275390625, 14090.65625}, {776.82843017578125, 14091.828125}}}},

    {{{{1110, 817}, {1110.55225f, 817}, {1111, 817.447693f}, {1111, 818}}},
     {{{1110.70715f, 817.292908f}, {1110.41406f, 817.000122f}, {1110, 817}}}},

    {{{{1110, 817}, {1110.55225f, 817}, {1111, 817.447693f}, {1111, 818}}},
     {{{1111, 818}, {1110.99988f, 817.585876f}, {1110.70715f, 817.292908f}}}},

    {{{{55, 207}, {52.238574981689453, 207}, {50, 204.76142883300781}, {50, 202}}},
     {{{55, 207}, {52.929431915283203, 206.99949645996094},
       {51.464466094970703, 205.53553771972656}}}},

    {{{{49, 47}, {49, 74.614250183105469}, {26.614250183105469, 97}, {-1, 97}}},
     {{{-8.659739592076221e-015, 96.991401672363281}, {20.065492630004883, 96.645187377929688},
       {34.355339050292969, 82.355339050292969}}}},

    {{{{10,234}, {10,229.58172607421875}, {13.581720352172852,226}, {18,226}}},
     {{{18,226}, {14.686291694641113,226}, {12.342399597167969,228.3424072265625}}}},

    {{{{10,234}, {10,229.58172607421875}, {13.581720352172852,226}, {18,226}}},
     {{{12.342399597167969,228.3424072265625}, {10,230.68629455566406}, {10,234}}}},
};

static const int quadCubicTests_count = (int) SK_ARRAY_COUNT(quadCubicTests);

static void cubicQuadIntersection(skiatest::Reporter* reporter, int index) {
    int iIndex = static_cast<int>(index);
    const SkDCubic& cubic = quadCubicTests[index].cubic;
    SkASSERT(ValidCubic(cubic));
    const SkDQuad& quad = quadCubicTests[index].quad;
    SkASSERT(ValidQuad(quad));
    SkReduceOrder reduce1;
    SkReduceOrder reduce2;
    int order1 = reduce1.reduce(cubic, SkReduceOrder::kNo_Quadratics);
    int order2 = reduce2.reduce(quad);
    if (order1 != 4) {
        SkDebugf("[%d] cubic order=%d\n", iIndex, order1);
        REPORTER_ASSERT(reporter, 0);
    }
    if (order2 != 3) {
        SkDebugf("[%d] quad order=%d\n", iIndex, order2);
        REPORTER_ASSERT(reporter, 0);
    }
    SkIntersections i;
    int roots = i.intersect(cubic, quad);
    for (int pt = 0; pt < roots; ++pt) {
        double tt1 = i[0][pt];
        SkDPoint xy1 = cubic.ptAtT(tt1);
        double tt2 = i[1][pt];
        SkDPoint xy2 = quad.ptAtT(tt2);
        if (!xy1.approximatelyEqual(xy2)) {
            SkDebugf("%s [%d,%d] x!= t1=%g (%g,%g) t2=%g (%g,%g)\n",
                __FUNCTION__, iIndex, pt, tt1, xy1.fX, xy1.fY, tt2, xy2.fX, xy2.fY);
        }
        REPORTER_ASSERT(reporter, xy1.approximatelyEqual(xy2));
    }
    reporter->bumpTestCount();
}

DEF_TEST(PathOpsCubicQuadIntersection, reporter) {
    for (int index = 0; index < quadCubicTests_count; ++index) {
        cubicQuadIntersection(reporter, index);
        reporter->bumpTestCount();
    }
}

DEF_TEST(PathOpsCubicQuadIntersectionOneOff, reporter) {
    cubicQuadIntersection(reporter, 0);
}
