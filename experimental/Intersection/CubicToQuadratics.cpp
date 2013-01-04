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

*/

#include "CubicUtilities.h"
#include "CurveIntersection.h"

static double calcTDiv(const Cubic& cubic, double start) {
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
    double tDiv3 = FLT_EPSILON / (adjust * dist);
    return cube_root(tDiv3);
}

static void demote(const Cubic& cubic, Quadratic& quad) {
    quad[0] = cubic[0];
    quad[1].x = (cubic[3].x - 3 * (cubic[2].x - cubic[1].x) - cubic[0].x) / 4;
    quad[1].y = (cubic[3].y - 3 * (cubic[2].y - cubic[1].y) - cubic[0].y) / 4;
    quad[2] = cubic[3];
}

int cubic_to_quadratics(const Cubic& cubic, SkTDArray<Quadratic>& quadratics) {
    quadratics.setCount(1); // FIXME: every place I have setCount(), I also want setReserve()
    Cubic reduced;
    int order = reduceOrder(cubic, reduced, kReduceOrder_QuadraticsAllowed);
    if (order < 3) {
        memcpy(quadratics[0], reduced, order * sizeof(_Point));
        return order;
    }
    double tDiv = calcTDiv(cubic, 0);
    if (approximately_greater_than_one(tDiv)) {
        demote(cubic, quadratics[0]);
        return 2;
    }
    if (tDiv >= 0.5) {
        CubicPair pair;
        chop_at(cubic, pair, 0.5);
        quadratics.setCount(2);
        demote(pair.first(), quadratics[0]);
        demote(pair.second(), quadratics[1]);
        return 2;
    }
    double start = 0;
    int index = -1;
    // if we iterate but make little progress, check to see if the curve is flat
    // or if the control points are tangent to each other
    do {
        Cubic part;
        sub_divide(part, start, tDiv, part);
        quadratics.append();
        demote(part, quadratics[++index]);
        if (tDiv == 1) {
            break;
        }
        start = tDiv;
        tDiv = calcTDiv(cubic, start);
        if (tDiv > 1) {
            tDiv = 1;
        }
    } while (true);
    return 2;
}
