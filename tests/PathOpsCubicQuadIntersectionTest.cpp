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
#include "SkReduceOrder.h"
#include "Test.h"

static struct lineCubic {
    SkDCubic cubic;
    SkDQuad quad;
    int answerCount;
    SkDPoint answers[2];
} quadCubicTests[] = {
    {{{{778, 14089}, {778, 14091.208984375}, {776.20916748046875, 14093}, {774, 14093}}},
     {{{778, 14089}, {777.99957275390625, 14090.65625}, {776.82843017578125, 14091.828125}}}, 2,
     {{778, 14089}, {776.82855609581270,14091.828250841330}}},

    {{{{1110, 817}, {1110.55225f, 817}, {1111, 817.447693f}, {1111, 818}}},
     {{{1110.70715f, 817.292908f}, {1110.41406f, 817.000122f}, {1110, 817}}}, 2,
      {{1110, 817}, {1110.70715f, 817.292908f}}},

    {{{{1110, 817}, {1110.55225f, 817}, {1111, 817.447693f}, {1111, 818}}},
     {{{1111, 818}, {1110.99988f, 817.585876f}, {1110.70715f, 817.292908f}}}, 2,
      {{1110.70715f, 817.292908f}, {1111, 818}}},

    {{{{55, 207}, {52.238574981689453, 207}, {50, 204.76142883300781}, {50, 202}}},
     {{{55, 207}, {52.929431915283203, 206.99949645996094},
       {51.464466094970703, 205.53553771972656}}}, 2,
      {{55, 207}, {51.464466094970703, 205.53553771972656}}},

    {{{{49, 47}, {49, 74.614250183105469}, {26.614250183105469, 97}, {-1, 97}}},
     {{{-8.659739592076221e-015, 96.991401672363281}, {20.065492630004883, 96.645187377929688},
       {34.355339050292969, 82.355339050292969}}}, 2,
      {{34.355339050292969,82.355339050292969}, {34.28654835573549, 82.424006509351585}}},

    {{{{10,234}, {10,229.58172607421875}, {13.581720352172852,226}, {18,226}}},
     {{{18,226}, {14.686291694641113,226}, {12.342399597167969,228.3424072265625}}}, 1,
      {{18,226}, {0,0}}},

    {{{{10,234}, {10,229.58172607421875}, {13.581720352172852,226}, {18,226}}},
     {{{12.342399597167969,228.3424072265625}, {10,230.68629455566406}, {10,234}}}, 1,
      {{10,234}, {0,0}}},
};

static const size_t quadCubicTests_count = SK_ARRAY_COUNT(quadCubicTests);

static void PathOpsCubicQuadIntersectionTest(skiatest::Reporter* reporter) {
    for (size_t index = 0; index < quadCubicTests_count; ++index) {
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
        SkASSERT(roots == quadCubicTests[index].answerCount);
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
            bool found = false;
            for (int idx2 = 0; idx2 < quadCubicTests[index].answerCount; ++idx2) {
                found |= quadCubicTests[index].answers[idx2].approximatelyEqual(xy1);
            }
            if (!found) {
                SkDebugf("%s [%d,%d] xy1=(%g,%g) != \n",
                    __FUNCTION__, iIndex, pt, xy1.fX, xy1.fY);
            }
            REPORTER_ASSERT(reporter, found);
        }
        reporter->bumpTestCount();
    }
}

#include "TestClassDef.h"
DEFINE_TESTCLASS_SHORT(PathOpsCubicQuadIntersectionTest)
