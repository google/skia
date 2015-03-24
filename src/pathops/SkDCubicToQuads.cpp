/*
http://stackoverflow.com/questions/2009160/how-do-i-convert-the-2-control-points-of-a-cubic-curve-to-the-single-control-poi
*/

/*
Let's call the control points of the cubic Q0..Q3 and the control points of the quadratic P0..P2.
Then for degree elevation, the equations are:

Q0 = P0
Q1 = 1/3 P0 + 2/3 P1
Q2 = 2/3 P1 + 1/3 P2
Q3 = P2
In your case you have Q0..Q3 and you're solving for P0..P2. There are two ways to compute P1 from
 the equations above:

P1 = 3/2 Q1 - 1/2 Q0
P1 = 3/2 Q2 - 1/2 Q3
If this is a degree-elevated cubic, then both equations will give the same answer for P1. Since
 it's likely not, your best bet is to average them. So,

P1 = -1/4 Q0 + 3/4 Q1 + 3/4 Q2 - 1/4 Q3

SkDCubic defined by: P1/2 - anchor points, C1/C2 control points
|x| is the euclidean norm of x
mid-point approx of cubic: a quad that shares the same anchors with the cubic and has the
 control point at C = (3·C2 - P2 + 3·C1 - P1)/4

Algorithm

pick an absolute precision (prec)
Compute the Tdiv as the root of (cubic) equation
sqrt(3)/18 · |P2 - 3·C2 + 3·C1 - P1|/2 · Tdiv ^ 3 = prec
if Tdiv < 0.5 divide the cubic at Tdiv. First segment [0..Tdiv] can be approximated with by a
 quadratic, with a defect less than prec, by the mid-point approximation.
 Repeat from step 2 with the second resulted segment (corresponding to 1-Tdiv)
0.5<=Tdiv<1 - simply divide the cubic in two. The two halves can be approximated by the mid-point
 approximation
Tdiv>=1 - the entire cubic can be approximated by the mid-point approximation

confirmed by (maybe stolen from)
http://www.caffeineowl.com/graphics/2d/vectorial/cubic2quad01.html
// maybe in turn derived from  http://www.cccg.ca/proceedings/2004/36.pdf
// also stored at http://www.cis.usouthal.edu/~hain/general/Publications/Bezier/bezier%20cccg04%20paper.pdf

*/

#include "SkPathOpsCubic.h"
#include "SkPathOpsLine.h"
#include "SkPathOpsQuad.h"
#include "SkReduceOrder.h"
#include "SkTArray.h"
#include "SkTSort.h"

#define USE_CUBIC_END_POINTS 1

static double calc_t_div(const SkDCubic& cubic, double precision, double start) {
    const double adjust = sqrt(3.) / 36;
    SkDCubic sub;
    const SkDCubic* cPtr;
    if (start == 0) {
        cPtr = &cubic;
    } else {
        // OPTIMIZE: special-case half-split ?
        sub = cubic.subDivide(start, 1);
        cPtr = &sub;
    }
    const SkDCubic& c = *cPtr;
    double dx = c[3].fX - 3 * (c[2].fX - c[1].fX) - c[0].fX;
    double dy = c[3].fY - 3 * (c[2].fY - c[1].fY) - c[0].fY;
    double dist = sqrt(dx * dx + dy * dy);
    double tDiv3 = precision / (adjust * dist);
    double t = SkDCubeRoot(tDiv3);
    if (start > 0) {
        t = start + (1 - start) * t;
    }
    return t;
}

SkDQuad SkDCubic::toQuad() const {
    SkDQuad quad;
    quad[0] = fPts[0];
    const SkDPoint fromC1 = {(3 * fPts[1].fX - fPts[0].fX) / 2, (3 * fPts[1].fY - fPts[0].fY) / 2};
    const SkDPoint fromC2 = {(3 * fPts[2].fX - fPts[3].fX) / 2, (3 * fPts[2].fY - fPts[3].fY) / 2};
    quad[1].fX = (fromC1.fX + fromC2.fX) / 2;
    quad[1].fY = (fromC1.fY + fromC2.fY) / 2;
    quad[2] = fPts[3];
    return quad;
}

static bool add_simple_ts(const SkDCubic& cubic, double precision, SkTArray<double, true>* ts) {
    double tDiv = calc_t_div(cubic, precision, 0);
    if (tDiv >= 1) {
        return true;
    }
    if (tDiv >= 0.5) {
        ts->push_back(0.5);
        return true;
    }
    return false;
}

static void addTs(const SkDCubic& cubic, double precision, double start, double end,
        SkTArray<double, true>* ts) {
    double tDiv = calc_t_div(cubic, precision, 0);
    double parts = ceil(1.0 / tDiv);
    for (double index = 0; index < parts; ++index) {
        double newT = start + (index / parts) * (end - start);
        if (newT > 0 && newT < 1) {
            ts->push_back(newT);
        }
    }
}

// flavor that returns T values only, deferring computing the quads until they are needed
// FIXME: when called from recursive intersect 2, this could take the original cubic
// and do a more precise job when calling chop at and sub divide by computing the fractional ts.
// it would still take the prechopped cubic for reduce order and find cubic inflections
void SkDCubic::toQuadraticTs(double precision, SkTArray<double, true>* ts) const {
    SkReduceOrder reducer;
    int order = reducer.reduce(*this, SkReduceOrder::kAllow_Quadratics);
    if (order < 3) {
        return;
    }
    double inflectT[5];
    int inflections = findInflections(inflectT);
    SkASSERT(inflections <= 2);
    if (!endsAreExtremaInXOrY()) {
        inflections += findMaxCurvature(&inflectT[inflections]);
        SkASSERT(inflections <= 5);
    }
    SkTQSort<double>(inflectT, &inflectT[inflections - 1]);
    // OPTIMIZATION: is this filtering common enough that it needs to be pulled out into its
    // own subroutine?
    while (inflections && approximately_less_than_zero(inflectT[0])) {
        memmove(inflectT, &inflectT[1], sizeof(inflectT[0]) * --inflections);
    }
    int start = 0;
    int next = 1;
    while (next < inflections) {
        if (!approximately_equal(inflectT[start], inflectT[next])) {
            ++start;
        ++next;
            continue;
        }
        memmove(&inflectT[start], &inflectT[next], sizeof(inflectT[0]) * (--inflections - start));
    }

    while (inflections && approximately_greater_than_one(inflectT[inflections - 1])) {
        --inflections;
    }
    SkDCubicPair pair;
    if (inflections == 1) {
        pair = chopAt(inflectT[0]);
        int orderP1 = reducer.reduce(pair.first(), SkReduceOrder::kNo_Quadratics);
        if (orderP1 < 2) {
            --inflections;
        } else {
            int orderP2 = reducer.reduce(pair.second(), SkReduceOrder::kNo_Quadratics);
            if (orderP2 < 2) {
                --inflections;
            }
        }
    }
    if (inflections == 0 && add_simple_ts(*this, precision, ts)) {
        return;
    }
    if (inflections == 1) {
        pair = chopAt(inflectT[0]);
        addTs(pair.first(), precision, 0, inflectT[0], ts);
        addTs(pair.second(), precision, inflectT[0], 1, ts);
        return;
    }
    if (inflections > 1) {
        SkDCubic part = subDivide(0, inflectT[0]);
        addTs(part, precision, 0, inflectT[0], ts);
        int last = inflections - 1;
        for (int idx = 0; idx < last; ++idx) {
            part = subDivide(inflectT[idx], inflectT[idx + 1]);
            addTs(part, precision, inflectT[idx], inflectT[idx + 1], ts);
        }
        part = subDivide(inflectT[last], 1);
        addTs(part, precision, inflectT[last], 1, ts);
        return;
    }
    addTs(*this, precision, 0, 1, ts);
}
