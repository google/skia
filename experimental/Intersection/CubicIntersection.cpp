#include "CubicIntersection.h"
#include "Intersections.h"
#include "LineIntersection.h"

static bool chop(const Cubic& smaller, const Cubic& larger,
        Intersections& intersections, double minT, double maxT);
        
static bool intersect(const Cubic& smaller, const Cubic& larger,
        Intersections& intersections) {
    // FIXME: carry order with cubic so we don't call it repeatedly
    Cubic smallResult;
    if (reduceOrder(smaller, smallResult, kReduceOrder_NoQuadraticsAllowed) <= 2) {
        Cubic largeResult;
        if (reduceOrder(larger, largeResult, kReduceOrder_NoQuadraticsAllowed) <= 2) {
            _Point pt;
            const _Line& smallLine = (const _Line&) smallResult;
            const _Line& largeLine = (const _Line&) largeResult;
            if (!lineIntersect(smallLine, largeLine, &pt)) {
                return false;
            }
            double smallT = t_at(smallLine, pt);
            double largeT = t_at(largeLine, pt);
            intersections.add(smallT, largeT);
            return true;
        }
    }
    double minT, maxT;
    if (!bezier_clip(smaller, larger, minT, maxT)) {
        if (minT == maxT) {
            intersections.add(minT, 0.5);
            return true;
        }
        return false;
    }
    return chop(larger, smaller, intersections, minT, maxT);
}

bool chop(const Cubic& smaller, const Cubic& larger,
        Intersections& intersections, double minT, double maxT) {
    intersections.swap();
    if (maxT - minT > 0.8) { // http://cagd.cs.byu.edu/~tom/papers/bezclip.pdf Multiple intersections
        CubicPair cubicPair;
        chop_at(larger, cubicPair, 0.5);
        int used = intersections.used();
        if (intersect(cubicPair.first(), smaller, intersections)) {
            intersections.offset(used, 0, 0.5);
        }
        used = intersections.used();
        if (intersect(cubicPair.second(), smaller, intersections)) {
            intersections.offset(used, 0.5, 1);
        }
        intersections.swap();
        return intersections.intersected();
    }
    Cubic cut;
    sub_divide(smaller, minT, maxT, cut);
    int used = intersections.used();
    bool result = intersect(cut, larger, intersections);
    if (result) {
        intersections.offset(used, minT, maxT);
    }
    intersections.swap();
    return result;
}

bool intersectStart(const Cubic& cubic1, const Cubic& cubic2,
        Intersections& intersections) {
    double minT1, minT2, maxT1, maxT2;
    if (!bezier_clip(cubic2, cubic1, minT1, maxT1)) {
        return false;
    }
    if (!bezier_clip(cubic1, cubic2, minT2, maxT2)) {
        return false;
    }
    if (maxT1 - minT1 < maxT2 - minT2) {
        intersections.swap();
        return chop(cubic1, cubic2, intersections, minT1, maxT1);
    } 
    return chop(cubic2, cubic1, intersections, minT2, maxT2);
}
