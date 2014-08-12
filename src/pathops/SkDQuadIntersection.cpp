// Another approach is to start with the implicit form of one curve and solve
// (seek implicit coefficients in QuadraticParameter.cpp
// by substituting in the parametric form of the other.
// The downside of this approach is that early rejects are difficult to come by.
// http://planetmath.org/encyclopedia/GaloisTheoreticDerivationOfTheQuarticFormula.html#step

#include "SkDQuadImplicit.h"
#include "SkIntersections.h"
#include "SkPathOpsLine.h"
#include "SkQuarticRoot.h"
#include "SkTArray.h"
#include "SkTSort.h"

/* given the implicit form 0 = Ax^2 + Bxy + Cy^2 + Dx + Ey + F
 * and given x = at^2 + bt + c  (the parameterized form)
 *           y = dt^2 + et + f
 * then
 * 0 = A(at^2+bt+c)(at^2+bt+c)+B(at^2+bt+c)(dt^2+et+f)+C(dt^2+et+f)(dt^2+et+f)+D(at^2+bt+c)+E(dt^2+et+f)+F
 */

static int findRoots(const SkDQuadImplicit& i, const SkDQuad& quad, double roots[4],
        bool oneHint, bool flip, int firstCubicRoot) {
    SkDQuad flipped;
    const SkDQuad& q = flip ? (flipped = quad.flip()) : quad;
    double a, b, c;
    SkDQuad::SetABC(&q[0].fX, &a, &b, &c);
    double d, e, f;
    SkDQuad::SetABC(&q[0].fY, &d, &e, &f);
    const double t4 =     i.x2() *  a * a
                    +     i.xy() *  a * d
                    +     i.y2() *  d * d;
    const double t3 = 2 * i.x2() *  a * b
                    +     i.xy() * (a * e +     b * d)
                    + 2 * i.y2() *  d * e;
    const double t2 =     i.x2() * (b * b + 2 * a * c)
                    +     i.xy() * (c * d +     b * e + a * f)
                    +     i.y2() * (e * e + 2 * d * f)
                    +     i.x()  *  a
                    +     i.y()  *  d;
    const double t1 = 2 * i.x2() *  b * c
                    +     i.xy() * (c * e + b * f)
                    + 2 * i.y2() *  e * f
                    +     i.x()  *  b
                    +     i.y()  *  e;
    const double t0 =     i.x2() *  c * c
                    +     i.xy() *  c * f
                    +     i.y2() *  f * f
                    +     i.x()  *  c
                    +     i.y()  *  f
                    +     i.c();
    int rootCount = SkReducedQuarticRoots(t4, t3, t2, t1, t0, oneHint, roots);
    if (rootCount < 0) {
        rootCount = SkQuarticRootsReal(firstCubicRoot, t4, t3, t2, t1, t0, roots);
    }
    if (flip) {
        for (int index = 0; index < rootCount; ++index) {
            roots[index] = 1 - roots[index];
        }
    }
    return rootCount;
}

static int addValidRoots(const double roots[4], const int count, double valid[4]) {
    int result = 0;
    int index;
    for (index = 0; index < count; ++index) {
        if (!approximately_zero_or_more(roots[index]) || !approximately_one_or_less(roots[index])) {
            continue;
        }
        double t = 1 - roots[index];
        if (approximately_less_than_zero(t)) {
            t = 0;
        } else if (approximately_greater_than_one(t)) {
            t = 1;
        }
        valid[result++] = t;
    }
    return result;
}

static bool only_end_pts_in_common(const SkDQuad& q1, const SkDQuad& q2) {
// the idea here is to see at minimum do a quick reject by rotating all points
// to either side of the line formed by connecting the endpoints
// if the opposite curves points are on the line or on the other side, the
// curves at most intersect at the endpoints
    for (int oddMan = 0; oddMan < 3; ++oddMan) {
        const SkDPoint* endPt[2];
        for (int opp = 1; opp < 3; ++opp) {
            int end = oddMan ^ opp;  // choose a value not equal to oddMan
            if (3 == end) {  // and correct so that largest value is 1 or 2
                end = opp;
            }
            endPt[opp - 1] = &q1[end];
        }
        double origX = endPt[0]->fX;
        double origY = endPt[0]->fY;
        double adj = endPt[1]->fX - origX;
        double opp = endPt[1]->fY - origY;
        double sign = (q1[oddMan].fY - origY) * adj - (q1[oddMan].fX - origX) * opp;
        if (approximately_zero(sign)) {
            goto tryNextHalfPlane;
        }
        for (int n = 0; n < 3; ++n) {
            double test = (q2[n].fY - origY) * adj - (q2[n].fX - origX) * opp;
            if (test * sign > 0 && !precisely_zero(test)) {
                goto tryNextHalfPlane;
            }
        }
        return true;
tryNextHalfPlane:
        ;
    }
    return false;
}

// returns false if there's more than one intercept or the intercept doesn't match the point
// returns true if the intercept was successfully added or if the
// original quads need to be subdivided
static bool add_intercept(const SkDQuad& q1, const SkDQuad& q2, double tMin, double tMax,
                          SkIntersections* i, bool* subDivide) {
    double tMid = (tMin + tMax) / 2;
    SkDPoint mid = q2.ptAtT(tMid);
    SkDLine line;
    line[0] = line[1] = mid;
    SkDVector dxdy = q2.dxdyAtT(tMid);
    line[0] -= dxdy;
    line[1] += dxdy;
    SkIntersections rootTs;
    rootTs.allowNear(false);
    int roots = rootTs.intersect(q1, line);
    if (roots == 0) {
        if (subDivide) {
            *subDivide = true;
        }
        return true;
    }
    if (roots == 2) {
        return false;
    }
    SkDPoint pt2 = q1.ptAtT(rootTs[0][0]);
    if (!pt2.approximatelyEqual(mid)) {
        return false;
    }
    i->insertSwap(rootTs[0][0], tMid, pt2);
    return true;
}

static bool is_linear_inner(const SkDQuad& q1, double t1s, double t1e, const SkDQuad& q2,
                            double t2s, double t2e, SkIntersections* i, bool* subDivide) {
    SkDQuad hull = q1.subDivide(t1s, t1e);
    SkDLine line = {{hull[2], hull[0]}};
    const SkDLine* testLines[] = { &line, (const SkDLine*) &hull[0], (const SkDLine*) &hull[1] };
    const size_t kTestCount = SK_ARRAY_COUNT(testLines);
    SkSTArray<kTestCount * 2, double, true> tsFound;
    for (size_t index = 0; index < kTestCount; ++index) {
        SkIntersections rootTs;
        rootTs.allowNear(false);
        int roots = rootTs.intersect(q2, *testLines[index]);
        for (int idx2 = 0; idx2 < roots; ++idx2) {
            double t = rootTs[0][idx2];
#if 0 // def SK_DEBUG   // FIXME : accurate for error = 16, error of 17.5 seen
// {{{136.08723965397621, 1648.2814535211637}, {593.49031197259478, 1190.8784277439891}, {593.49031197259478, 544.0128173828125}}}
// {{{-968.181396484375, 544.0128173828125}, {592.2825927734375, 870.552490234375}, {593.435302734375, 557.8828125}}}

            SkDPoint qPt = q2.ptAtT(t);
            SkDPoint lPt = testLines[index]->ptAtT(rootTs[1][idx2]);
            SkASSERT(qPt.approximatelyDEqual(lPt));
#endif
            if (approximately_negative(t - t2s) || approximately_positive(t - t2e)) {
                continue;
            }
            tsFound.push_back(rootTs[0][idx2]);
        }
    }
    int tCount = tsFound.count();
    if (tCount <= 0) {
        return true;
    }
    double tMin, tMax;
    if (tCount == 1) {
        tMin = tMax = tsFound[0];
    } else {
        SkASSERT(tCount > 1);
        SkTQSort<double>(tsFound.begin(), tsFound.end() - 1);
        tMin = tsFound[0];
        tMax = tsFound[tsFound.count() - 1];
    }
    SkDPoint end = q2.ptAtT(t2s);
    bool startInTriangle = hull.pointInHull(end);
    if (startInTriangle) {
        tMin = t2s;
    }
    end = q2.ptAtT(t2e);
    bool endInTriangle = hull.pointInHull(end);
    if (endInTriangle) {
        tMax = t2e;
    }
    int split = 0;
    SkDVector dxy1, dxy2;
    if (tMin != tMax || tCount > 2) {
        dxy2 = q2.dxdyAtT(tMin);
        for (int index = 1; index < tCount; ++index) {
            dxy1 = dxy2;
            dxy2 = q2.dxdyAtT(tsFound[index]);
            double dot = dxy1.dot(dxy2);
            if (dot < 0) {
                split = index - 1;
                break;
            }
        }
    }
    if (split == 0) {  // there's one point
        if (add_intercept(q1, q2, tMin, tMax, i, subDivide)) {
            return true;
        }
        i->swap();
        return is_linear_inner(q2, tMin, tMax, q1, t1s, t1e, i, subDivide);
    }
    // At this point, we have two ranges of t values -- treat each separately at the split
    bool result;
    if (add_intercept(q1, q2, tMin, tsFound[split - 1], i, subDivide)) {
        result = true;
    } else {
        i->swap();
        result = is_linear_inner(q2, tMin, tsFound[split - 1], q1, t1s, t1e, i, subDivide);
    }
    if (add_intercept(q1, q2, tsFound[split], tMax, i, subDivide)) {
        result = true;
    } else {
        i->swap();
        result |= is_linear_inner(q2, tsFound[split], tMax, q1, t1s, t1e, i, subDivide);
    }
    return result;
}

static double flat_measure(const SkDQuad& q) {
    SkDVector mid = q[1] - q[0];
    SkDVector dxy = q[2] - q[0];
    double length = dxy.length();  // OPTIMIZE: get rid of sqrt
    return fabs(mid.cross(dxy) / length);
}

// FIXME ? should this measure both and then use the quad that is the flattest as the line?
static bool is_linear(const SkDQuad& q1, const SkDQuad& q2, SkIntersections* i) {
    double measure = flat_measure(q1);
    // OPTIMIZE: (get rid of sqrt) use approximately_zero
    if (!approximately_zero_sqrt(measure)) {
        return false;
    }
    return is_linear_inner(q1, 0, 1, q2, 0, 1, i, NULL);
}

// FIXME: if flat measure is sufficiently large, then probably the quartic solution failed
// avoid imprecision incurred with chopAt
static void relaxed_is_linear(const SkDQuad* q1, double s1, double e1, const SkDQuad* q2,
        double s2, double e2, SkIntersections* i) {
    double m1 = flat_measure(*q1);
    double m2 = flat_measure(*q2);
    i->reset();
    const SkDQuad* rounder, *flatter;
    double sf, midf, ef, sr, er;
    if (m2 < m1) {
        rounder = q1;
        sr = s1;
        er = e1;
        flatter = q2;
        sf = s2;
        midf = (s2 + e2) / 2;
        ef = e2;
    } else {
        rounder = q2;
        sr = s2;
        er = e2;
        flatter = q1;
        sf = s1;
        midf = (s1 + e1) / 2;
        ef = e1;
    }
    bool subDivide = false;
    is_linear_inner(*flatter, sf, ef, *rounder, sr, er, i, &subDivide);
    if (subDivide) {
        relaxed_is_linear(flatter, sf, midf, rounder, sr, er, i);
        relaxed_is_linear(flatter, midf, ef, rounder, sr, er, i);
    }
    if (m2 < m1) {
        i->swapPts();
    }
}

// each time through the loop, this computes values it had from the last loop
// if i == j == 1, the center values are still good
// otherwise, for i != 1 or j != 1, four of the values are still good
// and if i == 1 ^ j == 1, an additional value is good
static bool binary_search(const SkDQuad& quad1, const SkDQuad& quad2, double* t1Seed,
                          double* t2Seed, SkDPoint* pt) {
    double tStep = ROUGH_EPSILON;
    SkDPoint t1[3], t2[3];
    int calcMask = ~0;
    do {
        if (calcMask & (1 << 1)) t1[1] = quad1.ptAtT(*t1Seed);
        if (calcMask & (1 << 4)) t2[1] = quad2.ptAtT(*t2Seed);
        if (t1[1].approximatelyEqual(t2[1])) {
            *pt = t1[1];
    #if ONE_OFF_DEBUG
            SkDebugf("%s t1=%1.9g t2=%1.9g (%1.9g,%1.9g) == (%1.9g,%1.9g)\n", __FUNCTION__,
                    t1Seed, t2Seed, t1[1].fX, t1[1].fY, t2[1].fX, t2[1].fY);
    #endif
            return true;
        }
        if (calcMask & (1 << 0)) t1[0] = quad1.ptAtT(SkTMax(0., *t1Seed - tStep));
        if (calcMask & (1 << 2)) t1[2] = quad1.ptAtT(SkTMin(1., *t1Seed + tStep));
        if (calcMask & (1 << 3)) t2[0] = quad2.ptAtT(SkTMax(0., *t2Seed - tStep));
        if (calcMask & (1 << 5)) t2[2] = quad2.ptAtT(SkTMin(1., *t2Seed + tStep));
        double dist[3][3];
        // OPTIMIZE: using calcMask value permits skipping some distance calcuations
        //   if prior loop's results are moved to correct slot for reuse
        dist[1][1] = t1[1].distanceSquared(t2[1]);
        int best_i = 1, best_j = 1;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                if (i == 1 && j == 1) {
                    continue;
                }
                dist[i][j] = t1[i].distanceSquared(t2[j]);
                if (dist[best_i][best_j] > dist[i][j]) {
                    best_i = i;
                    best_j = j;
                }
            }
        }
        if (best_i == 1 && best_j == 1) {
            tStep /= 2;
            if (tStep < FLT_EPSILON_HALF) {
                break;
            }
            calcMask = (1 << 0) | (1 << 2) | (1 << 3) | (1 << 5);
            continue;
        }
        if (best_i == 0) {
            *t1Seed -= tStep;
            t1[2] = t1[1];
            t1[1] = t1[0];
            calcMask = 1 << 0;
        } else if (best_i == 2) {
            *t1Seed += tStep;
            t1[0] = t1[1];
            t1[1] = t1[2];
            calcMask = 1 << 2;
        } else {
            calcMask = 0;
        }
        if (best_j == 0) {
            *t2Seed -= tStep;
            t2[2] = t2[1];
            t2[1] = t2[0];
            calcMask |= 1 << 3;
        } else if (best_j == 2) {
            *t2Seed += tStep;
            t2[0] = t2[1];
            t2[1] = t2[2];
            calcMask |= 1 << 5;
        }
    } while (true);
#if ONE_OFF_DEBUG
    SkDebugf("%s t1=%1.9g t2=%1.9g (%1.9g,%1.9g) != (%1.9g,%1.9g) %s\n", __FUNCTION__,
        t1Seed, t2Seed, t1[1].fX, t1[1].fY, t1[2].fX, t1[2].fY);
#endif
    return false;
}

static void lookNearEnd(const SkDQuad& q1, const SkDQuad& q2, int testT,
        const SkIntersections& orig, bool swap, SkIntersections* i) {
    if (orig.used() == 1 && orig[!swap][0] == testT) {
        return;
    }
    if (orig.used() == 2 && orig[!swap][1] == testT) {
        return;
    }
    SkDLine tmpLine;
    int testTIndex = testT << 1;
    tmpLine[0] = tmpLine[1] = q2[testTIndex];
    tmpLine[1].fX += q2[1].fY - q2[testTIndex].fY;
    tmpLine[1].fY -= q2[1].fX - q2[testTIndex].fX;
    SkIntersections impTs;
    impTs.intersectRay(q1, tmpLine);
    for (int index = 0; index < impTs.used(); ++index) {
        SkDPoint realPt = impTs.pt(index);
        if (!tmpLine[0].approximatelyPEqual(realPt)) {
            continue;
        }
        if (swap) {
            i->insert(testT, impTs[0][index], tmpLine[0]);
        } else {
            i->insert(impTs[0][index], testT, tmpLine[0]);
        }
    }
}

int SkIntersections::intersect(const SkDQuad& q1, const SkDQuad& q2) {
    fMax = 4;
    // if the quads share an end point, check to see if they overlap
    for (int i1 = 0; i1 < 3; i1 += 2) {
        for (int i2 = 0; i2 < 3; i2 += 2) {
            if (q1[i1].asSkPoint() == q2[i2].asSkPoint()) {
                insert(i1 >> 1, i2 >> 1, q1[i1]);
            }
        }
    }
    SkASSERT(fUsed < 3);
    if (only_end_pts_in_common(q1, q2)) {
        return fUsed;
    }
    if (only_end_pts_in_common(q2, q1)) {
        return fUsed;
    }
    // see if either quad is really a line
    // FIXME: figure out why reduce step didn't find this earlier
    if (is_linear(q1, q2, this)) {
        return fUsed;
    }
    SkIntersections swapped;
    swapped.setMax(fMax);
    if (is_linear(q2, q1, &swapped)) {
        swapped.swapPts();
        *this = swapped;
        return fUsed;
    }
    SkIntersections copyI(*this);
    lookNearEnd(q1, q2, 0, *this, false, &copyI);
    lookNearEnd(q1, q2, 1, *this, false, &copyI);
    lookNearEnd(q2, q1, 0, *this, true, &copyI);
    lookNearEnd(q2, q1, 1, *this, true, &copyI);
    int innerEqual = 0;
    if (copyI.fUsed >= 2) {
        SkASSERT(copyI.fUsed <= 4);
        double width = copyI[0][1] - copyI[0][0];
        int midEnd = 1;
        for (int index = 2; index < copyI.fUsed; ++index) {
            double testWidth = copyI[0][index] - copyI[0][index - 1];
            if (testWidth <= width) {
                continue;
            }
            midEnd = index;
        }
        for (int index = 0; index < 2; ++index) {
            double testT = (copyI[0][midEnd] * (index + 1)
                    + copyI[0][midEnd - 1] * (2 - index)) / 3;
            SkDPoint testPt1 = q1.ptAtT(testT);
            testT = (copyI[1][midEnd] * (index + 1) + copyI[1][midEnd - 1] * (2 - index)) / 3;
            SkDPoint testPt2 = q2.ptAtT(testT);
            innerEqual += testPt1.approximatelyEqual(testPt2);
        }
    }
    bool expectCoincident = copyI.fUsed >= 2 && innerEqual == 2;
    if (expectCoincident) {
        reset();
        insertCoincident(copyI[0][0], copyI[1][0], copyI.fPt[0]);
        int last = copyI.fUsed - 1;
        insertCoincident(copyI[0][last], copyI[1][last], copyI.fPt[last]);
        return fUsed;
    }
    SkDQuadImplicit i1(q1);
    SkDQuadImplicit i2(q2);
    int index;
    bool flip1 = q1[2] == q2[0];
    bool flip2 = q1[0] == q2[2];
    bool useCubic = q1[0] == q2[0];
    double roots1[4];
    int rootCount = findRoots(i2, q1, roots1, useCubic, flip1, 0);
    // OPTIMIZATION: could short circuit here if all roots are < 0 or > 1
    double roots1Copy[4];
    int r1Count = addValidRoots(roots1, rootCount, roots1Copy);
    SkDPoint pts1[4];
    for (index = 0; index < r1Count; ++index) {
        pts1[index] = q1.ptAtT(roots1Copy[index]);
    }
    double roots2[4];
    int rootCount2 = findRoots(i1, q2, roots2, useCubic, flip2, 0);
    double roots2Copy[4];
    int r2Count = addValidRoots(roots2, rootCount2, roots2Copy);
    SkDPoint pts2[4];
    for (index = 0; index < r2Count; ++index) {
        pts2[index] = q2.ptAtT(roots2Copy[index]);
    }
    if (r1Count == r2Count && r1Count <= 1) {
        if (r1Count == 1 && used() == 0) {
            if (pts1[0].approximatelyEqual(pts2[0])) {
                insert(roots1Copy[0], roots2Copy[0], pts1[0]);
            } else {
                // find intersection by chasing t
                if (binary_search(q1, q2, roots1Copy, roots2Copy, pts1)) {
                    insert(roots1Copy[0], roots2Copy[0], pts1[0]);
                }
            }
        }
        return fUsed;
    }
    int closest[4];
    double dist[4];
    bool foundSomething = false;
    for (index = 0; index < r1Count; ++index) {
        dist[index] = DBL_MAX;
        closest[index] = -1;
        for (int ndex2 = 0; ndex2 < r2Count; ++ndex2) {
            if (!pts2[ndex2].approximatelyEqual(pts1[index])) {
                continue;
            }
            double dx = pts2[ndex2].fX - pts1[index].fX;
            double dy = pts2[ndex2].fY - pts1[index].fY;
            double distance = dx * dx + dy * dy;
            if (dist[index] <= distance) {
                continue;
            }
            for (int outer = 0; outer < index; ++outer) {
                if (closest[outer] != ndex2) {
                    continue;
                }
                if (dist[outer] < distance) {
                    goto next;
                }
                closest[outer] = -1;
            }
            dist[index] = distance;
            closest[index] = ndex2;
            foundSomething = true;
        next:
            ;
        }
    }
    if (r1Count && r2Count && !foundSomething) {
        relaxed_is_linear(&q1, 0, 1, &q2, 0, 1, this);
        return fUsed;
    }
    int used = 0;
    do {
        double lowest = DBL_MAX;
        int lowestIndex = -1;
        for (index = 0; index < r1Count; ++index) {
            if (closest[index] < 0) {
                continue;
            }
            if (roots1Copy[index] < lowest) {
                lowestIndex = index;
                lowest = roots1Copy[index];
            }
        }
        if (lowestIndex < 0) {
            break;
        }
        insert(roots1Copy[lowestIndex], roots2Copy[closest[lowestIndex]],
                pts1[lowestIndex]);
        closest[lowestIndex] = -1;
    } while (++used < r1Count);
    return fUsed;
}
