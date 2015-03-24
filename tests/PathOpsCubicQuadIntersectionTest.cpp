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
    int answerCount;
    SkDPoint answers[2];
} quadCubicTests[] = {
#if 0  // FIXME : this should not fail (root problem behind skpcarrot_is24 )
    {{{{1020.08099,672.161987}, {1020.08002,630.73999}, {986.502014,597.161987}, {945.080994,597.161987}}},
     {{{1020,672}, {1020,640.93396}, {998.03302,618.96698}}}, 1,
      {{1019.421, 662.449}}},
#endif

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

DEF_TEST(PathOpsCubicQuadIntersection, reporter) {
    for (int index = 0; index < quadCubicTests_count; ++index) {
        cubicQuadIntersection(reporter, index);
        reporter->bumpTestCount();
    }
}

DEF_TEST(PathOpsCubicQuadIntersectionOneOff, reporter) {
    cubicQuadIntersection(reporter, 0);
}

static bool gPathOpCubicQuadSlopVerbose = false;
static const int kCubicToQuadSubdivisionDepth = 8; // slots reserved for cubic to quads subdivision

// determine that slop required after quad/quad finds a candidate intersection
// use the cross of the tangents plus the distance from 1 or 0 as knobs
DEF_TEST(PathOpsCubicQuadSlop, reporter) {
    // create a random non-selfintersecting cubic
    // break it into quadratics
    // offset the quadratic, measuring the slop required to find the intersection
    if (!gPathOpCubicQuadSlopVerbose) {  // takes a while to run -- so exclude it by default
        return;
    }
    int results[101];
    sk_bzero(results, sizeof(results));
    double minCross[101];
    sk_bzero(minCross, sizeof(minCross));
    double maxCross[101];
    sk_bzero(maxCross, sizeof(maxCross));
    double sumCross[101];
    sk_bzero(sumCross, sizeof(sumCross));
    int foundOne = 0;
    int slopCount = 1;
    SkRandom ran;
    for (int index = 0; index < 10000000; ++index) {
        if (index % 1000 == 999) SkDebugf(".");
        SkDCubic cubic = {{
                {ran.nextRangeF(-1000, 1000), ran.nextRangeF(-1000, 1000)},
                {ran.nextRangeF(-1000, 1000), ran.nextRangeF(-1000, 1000)},
                {ran.nextRangeF(-1000, 1000), ran.nextRangeF(-1000, 1000)},
                {ran.nextRangeF(-1000, 1000), ran.nextRangeF(-1000, 1000)}
        }};
        SkIntersections i;
        if (i.intersect(cubic)) {
            continue;
        }
        SkSTArray<kCubicToQuadSubdivisionDepth, double, true> ts;
        cubic.toQuadraticTs(cubic.calcPrecision(), &ts);
        double tStart = 0;
        int tsCount = ts.count();
        for (int i1 = 0; i1 <= tsCount; ++i1) {
            const double tEnd = i1 < tsCount ? ts[i1] : 1;
            SkDCubic part = cubic.subDivide(tStart, tEnd);
            SkDQuad quad = part.toQuad();
            SkReduceOrder reducer;
            int order = reducer.reduce(quad);
            if (order != 3) {
                continue;
            }
            for (int i2 = 0; i2 < 100; ++i2) {
                SkDPoint endDisplacement = {ran.nextRangeF(-100, 100), ran.nextRangeF(-100, 100)};
                SkDQuad nearby = {{
                        {quad[0].fX + endDisplacement.fX, quad[0].fY + endDisplacement.fY},
                        {quad[1].fX + ran.nextRangeF(-100, 100), quad[1].fY + ran.nextRangeF(-100, 100)},
                        {quad[2].fX - endDisplacement.fX, quad[2].fY - endDisplacement.fY}
                }};
                order = reducer.reduce(nearby);
                if (order != 3) {
                    continue;
                }
                SkIntersections locals;
                locals.allowNear(false);
                locals.intersect(quad, nearby);
                if (locals.used() != 1) {
                    continue;
                }
                // brute force find actual intersection
                SkDLine cubicLine = {{ {0, 0}, {cubic[0].fX, cubic[0].fY } }};
                SkIntersections liner;
                int i3;
                int found = -1;
                int foundErr = true;
                for (i3 = 1; i3 <= 1000; ++i3) {
                    cubicLine[0] = cubicLine[1];
                    cubicLine[1] = cubic.ptAtT(i3 / 1000.);
                    liner.reset();
                    liner.allowNear(false);
                    liner.intersect(nearby, cubicLine);
                    if (liner.used() == 0) {
                        continue;
                    }
                    if (liner.used() > 1) {
                        foundErr = true;
                        break;
                    }
                    if (found > 0) {
                        foundErr = true;
                        break;
                    }
                    foundErr = false;
                    found = i3;
                }
                if (foundErr) {
                    continue;
                }
                SkDVector dist = liner.pt(0) - locals.pt(0);
                SkDVector qV = nearby.dxdyAtT(locals[0][0]);
                double cubicT = (found - 1 + liner[1][0]) / 1000.;
                SkDVector cV = cubic.dxdyAtT(cubicT);
                double qxc = qV.crossCheck(cV);
                double qvLen = qV.length();
                double cvLen = cV.length();
                double maxLen = SkTMax(qvLen, cvLen);
                qxc /= maxLen;
                double quadT = tStart + (tEnd - tStart) * locals[0][0];
                double diffT = fabs(cubicT - quadT);
                int diffIdx = (int) (diffT * 100);
                results[diffIdx]++;
                double absQxc = fabs(qxc);
                if (sumCross[diffIdx] == 0) {
                    minCross[diffIdx] = maxCross[diffIdx] = sumCross[diffIdx] = absQxc;
                } else {
                    minCross[diffIdx] = SkTMin(minCross[diffIdx], absQxc);
                    maxCross[diffIdx] = SkTMax(maxCross[diffIdx], absQxc);
                    sumCross[diffIdx] +=  absQxc;
                }
                if (diffIdx >= 20) {
#if 01
                    SkDebugf("cubic={{{%1.9g,%1.9g}, {%1.9g,%1.9g}, {%1.9g,%1.9g}, {%1.9g,%1.9g}}}"
                        " quad={{{%1.9g,%1.9g}, {%1.9g,%1.9g}, {%1.9g,%1.9g}}}"
                        " {{{%1.9g,%1.9g}, {%1.9g,%1.9g}}}"
                        " qT=%1.9g cT=%1.9g dist=%1.9g cross=%1.9g\n",
                        cubic[0].fX, cubic[0].fY, cubic[1].fX, cubic[1].fY,
                        cubic[2].fX, cubic[2].fY, cubic[3].fX, cubic[3].fY,
                        nearby[0].fX, nearby[0].fY, nearby[1].fX, nearby[1].fY,
                        nearby[2].fX, nearby[2].fY,
                        liner.pt(0).fX, liner.pt(0).fY,
                        locals.pt(0).fX, locals.pt(0).fY, quadT, cubicT, dist.length(), qxc);
#else
                    SkDebugf("qT=%1.9g cT=%1.9g dist=%1.9g cross=%1.9g\n",
                        quadT, cubicT, dist.length(), qxc);
                    SkDebugf("<div id=\"slop%d\">\n", ++slopCount);
                    SkDebugf("{{{%1.9g,%1.9g}, {%1.9g,%1.9g}, {%1.9g,%1.9g}, {%1.9g,%1.9g}}}\n"
                        "{{{%1.9g,%1.9g}, {%1.9g,%1.9g}, {%1.9g,%1.9g}}}\n"
                        "{{{%1.9g,%1.9g}, {%1.9g,%1.9g}}}\n",
                        cubic[0].fX, cubic[0].fY, cubic[1].fX, cubic[1].fY,
                        cubic[2].fX, cubic[2].fY, cubic[3].fX, cubic[3].fY,
                        nearby[0].fX, nearby[0].fY, nearby[1].fX, nearby[1].fY,
                        nearby[2].fX, nearby[2].fY,
                        liner.pt(0).fX, liner.pt(0).fY,
                        locals.pt(0).fX, locals.pt(0).fY);
                    SkDebugf("</div>\n\n");
#endif
                }
                ++foundOne;
            }
            tStart = tEnd;
        }
        if (++foundOne >= 100000) {
            break;
        }
    }
#if 01
    SkDebugf("slopCount=%d\n", slopCount);
    int max = 100;
    while (results[max] == 0) {
        --max;
    }
    for (int i = 0; i <= max; ++i) {
        if (i > 0 && i % 10 == 0) {
            SkDebugf("\n");
        }
        SkDebugf("%d ", results[i]);
    }
    SkDebugf("min\n");
    for (int i = 0; i <= max; ++i) {
        if (i > 0 && i % 10 == 0) {
            SkDebugf("\n");
        }
        SkDebugf("%1.9g ", minCross[i]);
    }
    SkDebugf("max\n");
    for (int i = 0; i <= max; ++i) {
        if (i > 0 && i % 10 == 0) {
            SkDebugf("\n");
        }
        SkDebugf("%1.9g ", maxCross[i]);
    }
    SkDebugf("avg\n");
    for (int i = 0; i <= max; ++i) {
        if (i > 0 && i % 10 == 0) {
            SkDebugf("\n");
        }
        SkDebugf("%1.9g ", sumCross[i] / results[i]);
    }
#else
    for (int i = 1; i < slopCount; ++i) {
        SkDebugf("        slop%d,\n", i);
    }
#endif
    SkDebugf("\n");
}
