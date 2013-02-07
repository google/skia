// Another approach is to start with the implicit form of one curve and solve
// (seek implicit coefficients in QuadraticParameter.cpp
// by substituting in the parametric form of the other.
// The downside of this approach is that early rejects are difficult to come by.
// http://planetmath.org/encyclopedia/GaloisTheoreticDerivationOfTheQuarticFormula.html#step


#include "CubicUtilities.h"
#include "CurveIntersection.h"
#include "Intersections.h"
#include "QuadraticParameterization.h"
#include "QuarticRoot.h"
#include "QuadraticUtilities.h"
#include "TSearch.h"

#if SK_DEBUG
#include "LineUtilities.h"
#endif

/* given the implicit form 0 = Ax^2 + Bxy + Cy^2 + Dx + Ey + F
 * and given x = at^2 + bt + c  (the parameterized form)
 *           y = dt^2 + et + f
 * then
 * 0 = A(at^2+bt+c)(at^2+bt+c)+B(at^2+bt+c)(dt^2+et+f)+C(dt^2+et+f)(dt^2+et+f)+D(at^2+bt+c)+E(dt^2+et+f)+F
 */

static int findRoots(const QuadImplicitForm& i, const Quadratic& q2, double roots[4],
        bool oneHint) {
    double a, b, c;
    set_abc(&q2[0].x, a, b, c);
    double d, e, f;
    set_abc(&q2[0].y, d, e, f);
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
    int rootCount = reducedQuarticRoots(t4, t3, t2, t1, t0, oneHint, roots);
    if (rootCount >= 0) {
        return rootCount;
    }
    return quarticRootsReal(t4, t3, t2, t1, t0, roots);
}

static void addValidRoots(const double roots[4], const int count, const int side, Intersections& i) {
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
        i.insertOne(t, side);
    }
}

static bool onlyEndPtsInCommon(const Quadratic& q1, const Quadratic& q2, Intersections& i) {
// the idea here is to see at minimum do a quick reject by rotating all points
// to either side of the line formed by connecting the endpoints
// if the opposite curves points are on the line or on the other side, the
// curves at most intersect at the endpoints
    for (int oddMan = 0; oddMan < 3; ++oddMan) {
        const _Point* endPt[2];
        for (int opp = 1; opp < 3; ++opp) {
            int end = oddMan ^ opp;
            if (end == 3) {
                end = opp;
            }
            endPt[opp - 1] = &q1[end];
        }
        double origX = endPt[0]->x;
        double origY = endPt[0]->y;
        double adj = endPt[1]->x - origX;
        double opp = endPt[1]->y - origY;
        double sign = (q1[oddMan].y - origY) * adj - (q1[oddMan].x - origX) * opp;
        if (approximately_zero(sign)) {
            goto tryNextHalfPlane;
        }
        for (int n = 0; n < 3; ++n) {
            double test = (q2[n].y - origY) * adj - (q2[n].x - origX) * opp;
            if (test * sign > 0) {
                goto tryNextHalfPlane;
            }
        }
        for (int i1 = 0; i1 < 3; i1 += 2) {
            for (int i2 = 0; i2 < 3; i2 += 2) {
                if (q1[i1] == q2[i2]) {
                    i.insert(i1 >> 1, i2 >> 1);
                }
            }
        }
        SkASSERT(i.fUsed < 3);
        return true;
tryNextHalfPlane:
        ;
    }
    return false;
}

// returns false if there's more than one intercept or the intercept doesn't match the point
// returns true if the intercept was successfully added or if the
// original quads need to be subdivided
static bool addIntercept(const Quadratic& q1, const Quadratic& q2, double tMin, double tMax,
        Intersections& i, bool* subDivide) {
    double tMid = (tMin + tMax) / 2;
    _Point mid;
    xy_at_t(q2, tMid, mid.x, mid.y);
    _Line line;
    line[0] = line[1] = mid;
    _Point dxdy;
    dxdy_at_t(q2, tMid, dxdy);
    line[0].x -= dxdy.x;
    line[0].y -= dxdy.y;
    line[1].x += dxdy.x;
    line[1].y += dxdy.y;
    Intersections rootTs;
    int roots = intersect(q1, line, rootTs);
    if (roots == 0) {
        if (subDivide) {
            *subDivide = true;
        }
        return true;
    }
    if (roots == 2) {
        return false;
    }
    _Point pt2;
    xy_at_t(q1, rootTs.fT[0][0], pt2.x, pt2.y);
    if (!pt2.approximatelyEqual(mid)) {
        return false;
    }
    i.add(rootTs.fT[0][0], tMid);
    return true;
}

static bool isLinearInner(const Quadratic& q1, double t1s, double t1e, const Quadratic& q2,
        double t2s, double t2e, Intersections& i, bool* subDivide) {
    Quadratic hull;
    sub_divide(q1, t1s, t1e, hull);
    _Line line = {hull[2], hull[0]};
    const _Line* testLines[] = { &line, (const _Line*) &hull[0], (const _Line*) &hull[1] };
    size_t testCount = sizeof(testLines) / sizeof(testLines[0]);
    SkTDArray<double> tsFound;
    for (size_t index = 0; index < testCount; ++index) {
        Intersections rootTs;
        int roots = intersect(q2, *testLines[index], rootTs);
        for (int idx2 = 0; idx2 < roots; ++idx2) {
            double t = rootTs.fT[0][idx2];
#if SK_DEBUG
        _Point qPt, lPt;
        xy_at_t(q2, t, qPt.x, qPt.y);
        xy_at_t(*testLines[index], rootTs.fT[1][idx2], lPt.x, lPt.y);
        SkASSERT(qPt.approximatelyEqual(lPt));
#endif
            if (approximately_negative(t - t2s) || approximately_positive(t - t2e)) {
                continue;
            }
            *tsFound.append() = rootTs.fT[0][idx2];
        }
    }
    int tCount = tsFound.count();
    if (!tCount) {
        return true;
    }
    double tMin, tMax;
    _Point dxy1, dxy2;
    if (tCount == 1) {
        tMin = tMax = tsFound[0];
    } else if (tCount > 1) {
        QSort<double>(tsFound.begin(), tsFound.end() - 1);
        tMin = tsFound[0];
        tMax = tsFound[1];
    }
    _Point end;
    xy_at_t(q2, t2s, end.x, end.y);
    bool startInTriangle = point_in_hull(hull, end);
    if (startInTriangle) {
        tMin = t2s;
    }
    xy_at_t(q2, t2e, end.x, end.y);
    bool endInTriangle = point_in_hull(hull, end);
    if (endInTriangle) {
        tMax = t2e;
    }
    int split = 0;
    if (tMin != tMax || tCount > 2) {
        dxdy_at_t(q2, tMin, dxy2);
        for (int index = 1; index < tCount; ++index) {
            dxy1 = dxy2;
            dxdy_at_t(q2, tsFound[index], dxy2);
            double dot = dxy1.dot(dxy2);
            if (dot < 0) {
                split = index - 1;
                break;
            }
        }

    }
    if (split == 0) { // there's one point
        if (addIntercept(q1, q2, tMin, tMax, i, subDivide)) {
            return true;
        }
        i.swap();
        return isLinearInner(q2, tMin, tMax, q1, t1s, t1e, i, subDivide);
    }
    // At this point, we have two ranges of t values -- treat each separately at the split
    bool result;
    if (addIntercept(q1, q2, tMin, tsFound[split - 1], i, subDivide)) {
        result = true;
    } else {
        i.swap();
        result = isLinearInner(q2, tMin, tsFound[split - 1], q1, t1s, t1e, i, subDivide);
    }
    if (addIntercept(q1, q2, tsFound[split], tMax, i, subDivide)) {
        result = true;
    } else {
        i.swap();
        result |= isLinearInner(q2, tsFound[split], tMax, q1, t1s, t1e, i, subDivide);
    }
    return result;
}

static double flatMeasure(const Quadratic& q) {
    _Point mid = q[1];
    mid -= q[0];
    _Point dxy = q[2];
    dxy -= q[0];
    double length = dxy.length(); // OPTIMIZE: get rid of sqrt
    return fabs(mid.cross(dxy) / length);
}

// FIXME ? should this measure both and then use the quad that is the flattest as the line?
static bool isLinear(const Quadratic& q1, const Quadratic& q2, Intersections& i) {
    double measure = flatMeasure(q1);
    // OPTIMIZE: (get rid of sqrt) use approximately_zero
    if (!approximately_zero_sqrt(measure)) {
        return false;
    }
    return isLinearInner(q1, 0, 1, q2, 0, 1, i, NULL);
}

// FIXME: if flat measure is sufficiently large, then probably the quartic solution failed
static void relaxedIsLinear(const Quadratic& q1, const Quadratic& q2, Intersections& i) {
    double m1 = flatMeasure(q1);
    double m2 = flatMeasure(q2);
#if SK_DEBUG
    double min = SkTMin(m1, m2);
    if (min > 5) {
        SkDebugf("%s maybe not flat enough.. %1.9g\n", __FUNCTION__, min);
    }
#endif
    i.reset();
    const Quadratic& rounder = m2 < m1 ? q1 : q2;
    const Quadratic& flatter = m2 < m1 ? q2 : q1;
    bool subDivide = false;
    isLinearInner(flatter, 0, 1, rounder, 0, 1, i, &subDivide);
    if (subDivide) {
        QuadraticPair pair;
        chop_at(flatter, pair, 0.5);
        Intersections firstI, secondI;
        relaxedIsLinear(pair.first(), rounder, firstI);
        for (int index = 0; index < firstI.used(); ++index) {
            i.insert(firstI.fT[0][index] * 0.5, firstI.fT[1][index]);
        }
        relaxedIsLinear(pair.second(), rounder, secondI);
        for (int index = 0; index < secondI.used(); ++index) {
            i.insert(0.5 + secondI.fT[0][index] * 0.5, secondI.fT[1][index]);
        }
    }
    if (m2 < m1) {
        i.swapPts();
    }
}

#if 0
static void unsortableExpanse(const Quadratic& q1, const Quadratic& q2, Intersections& i) {
    const Quadratic* qs[2] = { &q1, &q2 };
    // need t values for start and end of unsortable expanse on both curves
    // try projecting lines parallel to the end points
    i.fT[0][0] = 0;
    i.fT[0][1] = 1;
    int flip = -1; // undecided
    for (int qIdx = 0; qIdx < 2; qIdx++) {
        for (int t = 0; t < 2; t++) {
            _Point dxdy;
            dxdy_at_t(*qs[qIdx], t, dxdy);
            _Line perp;
            perp[0] = perp[1] = (*qs[qIdx])[t == 0 ? 0 : 2];
            perp[0].x += dxdy.y;
            perp[0].y -= dxdy.x;
            perp[1].x -= dxdy.y;
            perp[1].y += dxdy.x;
            Intersections hitData;
            int hits = intersectRay(*qs[qIdx ^ 1], perp, hitData);
            SkASSERT(hits <= 1);
            if (hits) {
                if (flip < 0) {
                    _Point dxdy2;
                    dxdy_at_t(*qs[qIdx ^ 1], hitData.fT[0][0], dxdy2);
                    double dot = dxdy.dot(dxdy2);
                    flip = dot < 0;
                    i.fT[1][0] = flip;
                    i.fT[1][1] = !flip;
                }
                i.fT[qIdx ^ 1][t ^ flip] = hitData.fT[0][0];
            }
        }
    }
    i.fUnsortable = true; // failed, probably coincident or near-coincident
    i.fUsed = 2;
}
#endif

bool intersect2(const Quadratic& q1, const Quadratic& q2, Intersections& i) {
    // if the quads share an end point, check to see if they overlap

    if (onlyEndPtsInCommon(q1, q2, i)) {
        return i.intersected();
    }
    if (onlyEndPtsInCommon(q2, q1, i)) {
        i.swapPts();
        return i.intersected();
    }
    // see if either quad is really a line
    if (isLinear(q1, q2, i)) {
        return i.intersected();
    }
    if (isLinear(q2, q1, i)) {
        i.swapPts();
        return i.intersected();
    }
    QuadImplicitForm i1(q1);
    QuadImplicitForm i2(q2);
    if (i1.implicit_match(i2)) {
        // FIXME: compute T values
        // compute the intersections of the ends to find the coincident span
        bool useVertical = fabs(q1[0].x - q1[2].x) < fabs(q1[0].y - q1[2].y);
        double t;
        if ((t = axialIntersect(q1, q2[0], useVertical)) >= 0) {
            i.addCoincident(t, 0);
        }
        if ((t = axialIntersect(q1, q2[2], useVertical)) >= 0) {
            i.addCoincident(t, 1);
        }
        useVertical = fabs(q2[0].x - q2[2].x) < fabs(q2[0].y - q2[2].y);
        if ((t = axialIntersect(q2, q1[0], useVertical)) >= 0) {
            i.addCoincident(0, t);
        }
        if ((t = axialIntersect(q2, q1[2], useVertical)) >= 0) {
            i.addCoincident(1, t);
        }
        SkASSERT(i.fCoincidentUsed <= 2);
        return i.fCoincidentUsed > 0;
    }
    double roots1[4], roots2[4];
    bool useCubic = q1[0] == q2[0] || q1[0] == q2[2] || q1[2] == q2[0];
    int rootCount = findRoots(i2, q1, roots1, useCubic);
    // OPTIMIZATION: could short circuit here if all roots are < 0 or > 1
    int rootCount2 = findRoots(i1, q2, roots2, useCubic);
    addValidRoots(roots1, rootCount, 0, i);
    addValidRoots(roots2, rootCount2, 1, i);
    if (i.insertBalanced() && i.fUsed <= 1) {
        if (i.fUsed == 1) {
            _Point xy1, xy2;
            xy_at_t(q1, i.fT[0][0], xy1.x, xy1.y);
            xy_at_t(q2, i.fT[1][0], xy2.x, xy2.y);
            if (!xy1.approximatelyEqual(xy2)) {
                --i.fUsed;
                --i.fUsed2;
            }
        }
        return i.intersected();
    }
    _Point pts[4];
    int closest[4];
    double dist[4];
    int index, ndex2;
    for (ndex2 = 0; ndex2 < i.fUsed2; ++ndex2) {
        xy_at_t(q2, i.fT[1][ndex2], pts[ndex2].x, pts[ndex2].y);
    }
    bool foundSomething = false;
    for (index = 0; index < i.fUsed; ++index) {
        _Point xy;
        xy_at_t(q1, i.fT[0][index], xy.x, xy.y);
        dist[index] = DBL_MAX;
        closest[index] = -1;
        for (ndex2 = 0; ndex2 < i.fUsed2; ++ndex2) {
            if (!pts[ndex2].approximatelyEqual(xy)) {
                continue;
            }
            double dx = pts[ndex2].x - xy.x;
            double dy = pts[ndex2].y - xy.y;
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
    if (i.fUsed && i.fUsed2 && !foundSomething) {
        relaxedIsLinear(q1, q2, i);
        return i.intersected();
    }
    double roots1Copy[4], roots2Copy[4];
    memcpy(roots1Copy, i.fT[0], i.fUsed * sizeof(double));
    memcpy(roots2Copy, i.fT[1], i.fUsed2 * sizeof(double));
    int used = 0;
    do {
        double lowest = DBL_MAX;
        int lowestIndex = -1;
        for (index = 0; index < i.fUsed; ++index) {
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
        i.fT[0][used] = roots1Copy[lowestIndex];
        i.fT[1][used] = roots2Copy[closest[lowestIndex]];
        closest[lowestIndex] = -1;
    } while (++used < i.fUsed);
    i.fUsed = i.fUsed2 = used;
    i.fFlip = false;
    return i.intersected();
}
