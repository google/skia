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


Cubic defined by: P1/2 - anchor points, C1/C2 control points
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

#include "CubicUtilities.h"
#include "CurveIntersection.h"
#include "LineIntersection.h"
#include "TSearch.h"

const bool AVERAGE_END_POINTS = true; // results in better fitting curves

#define USE_CUBIC_END_POINTS 1

static double calcTDiv(const Cubic& cubic, double precision, double start) {
    const double adjust = sqrt(3) / 36;
    Cubic sub;
    const Cubic* cPtr;
    if (start == 0) {
        cPtr = &cubic;
    } else {
        // OPTIMIZE: special-case half-split ?
        sub_divide(cubic, start, 1, sub);
        cPtr = &sub;
    }
    const Cubic& c = *cPtr;
    double dx = c[3].x - 3 * (c[2].x - c[1].x) - c[0].x;
    double dy = c[3].y - 3 * (c[2].y - c[1].y) - c[0].y;
    double dist = sqrt(dx * dx + dy * dy);
    double tDiv3 = precision / (adjust * dist);
    double t = cube_root(tDiv3);
    if (start > 0) {
        t = start + (1 - start) * t;
    }
    return t;
}

void demote_cubic_to_quad(const Cubic& cubic, Quadratic& quad) {
    quad[0] = cubic[0];
if (AVERAGE_END_POINTS) {
    const _Point fromC1 = { (3 * cubic[1].x - cubic[0].x) / 2, (3 * cubic[1].y - cubic[0].y) / 2 };
    const _Point fromC2 = { (3 * cubic[2].x - cubic[3].x) / 2, (3 * cubic[2].y - cubic[3].y) / 2 };
    quad[1].x = (fromC1.x + fromC2.x) / 2;
    quad[1].y = (fromC1.y + fromC2.y) / 2;
} else {
    lineIntersect((const _Line&) cubic[0], (const _Line&) cubic[2], quad[1]);
}
    quad[2] = cubic[3];
}

int cubic_to_quadratics(const Cubic& cubic, double precision, SkTDArray<Quadratic>& quadratics) {
    SkTDArray<double> ts;
    cubic_to_quadratics(cubic, precision, ts);
    int tsCount = ts.count();
    double t1Start = 0;
    int order = 0;
    for (int idx = 0; idx <= tsCount; ++idx) {
        double t1 = idx < tsCount ? ts[idx] : 1;
        Cubic part;
        sub_divide(cubic, t1Start, t1, part);
        Quadratic q1;
        demote_cubic_to_quad(part, q1);
        Quadratic s1;
        int o1 = reduceOrder(q1, s1, kReduceOrder_TreatAsFill);
        if (order < o1) {
            order = o1;
        }
        memcpy(quadratics.append(), o1 < 2 ? s1 : q1, sizeof(Quadratic));
        t1Start = t1;
    }
    return order;
}

static bool addSimpleTs(const Cubic& cubic, double precision, SkTDArray<double>& ts) {
    double tDiv = calcTDiv(cubic, precision, 0);
    if (tDiv >= 1) {
        return true;
    }
    if (tDiv >= 0.5) {
        *ts.append() = 0.5;
        return true;
    }
    return false;
}

static void addTs(const Cubic& cubic, double precision, double start, double end,
        SkTDArray<double>& ts) {
    double tDiv = calcTDiv(cubic, precision, 0);
    double parts = ceil(1.0 / tDiv);
    for (double index = 0; index < parts; ++index) {
        double newT = start + (index / parts) * (end - start);
        if (newT > 0 && newT < 1) {
            *ts.append() = newT;
        }
    }
}

// flavor that returns T values only, deferring computing the quads until they are needed
// FIXME: when called from recursive intersect 2, this could take the original cubic
// and do a more precise job when calling chop at and sub divide by computing the fractional ts.
// it would still take the prechopped cubic for reduce order and find cubic inflections
void cubic_to_quadratics(const Cubic& cubic, double precision, SkTDArray<double>& ts) {
    Cubic reduced;
    int order = reduceOrder(cubic, reduced, kReduceOrder_QuadraticsAllowed,
            kReduceOrder_TreatAsFill);
    if (order < 3) {
        return;
    }
    double inflectT[5];
    int inflections = find_cubic_inflections(cubic, inflectT);
    SkASSERT(inflections <= 2);
    if (!ends_are_extrema_in_x_or_y(cubic)) {
        inflections += find_cubic_max_curvature(cubic, &inflectT[inflections]);
        SkASSERT(inflections <= 5);
    }
    QSort<double>(inflectT, &inflectT[inflections - 1]);
    // OPTIMIZATION: is this filtering common enough that it needs to be pulled out into its
    // own subroutine?
    while (inflections && approximately_less_than_zero(inflectT[0])) {
        memcpy(inflectT, &inflectT[1], sizeof(inflectT[0]) * --inflections);
    }
    int start = 0;
    do {
        int next = start + 1;
        if (next >= inflections) {
            break;
        }
        if (!approximately_equal(inflectT[start], inflectT[next])) {
            ++start;
            continue;
        }
        memcpy(&inflectT[start], &inflectT[next], sizeof(inflectT[0]) * (--inflections - start));
    } while (true);
    while (inflections && approximately_greater_than_one(inflectT[inflections - 1])) {
        --inflections;
    }
    CubicPair pair;
    if (inflections == 1) {
        chop_at(cubic, pair, inflectT[0]);
        int orderP1 = reduceOrder(pair.first(), reduced, kReduceOrder_NoQuadraticsAllowed,
                kReduceOrder_TreatAsFill);
        if (orderP1 < 2) {
            --inflections;
        } else {
            int orderP2 = reduceOrder(pair.second(), reduced, kReduceOrder_NoQuadraticsAllowed,
                    kReduceOrder_TreatAsFill);
            if (orderP2 < 2) {
                --inflections;
            }
        }
    }
    if (inflections == 0 && addSimpleTs(cubic, precision, ts)) {
        return;
    }
    if (inflections == 1) {
        chop_at(cubic, pair, inflectT[0]);
        addTs(pair.first(), precision, 0, inflectT[0], ts);
        addTs(pair.second(), precision, inflectT[0], 1, ts);
        return;
    }
    if (inflections > 1) {
        Cubic part;
        sub_divide(cubic, 0, inflectT[0], part);
        addTs(part, precision, 0, inflectT[0], ts);
        int last = inflections - 1;
        for (int idx = 0; idx < last; ++idx) {
            sub_divide(cubic, inflectT[idx], inflectT[idx + 1], part);
            addTs(part, precision, inflectT[idx], inflectT[idx + 1], ts);
        }
        sub_divide(cubic, inflectT[last], 1, part);
        addTs(part, precision, inflectT[last], 1, ts);
        return;
    }
    addTs(cubic, precision, 0, 1, ts);
}
