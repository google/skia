#include "CubicIntersection.h"
#include "Intersections.h"
#include "IntersectionUtilities.h"
#include "LineIntersection.h"

class CubicIntersections : public Intersections {
public:

CubicIntersections(const Cubic& c1, const Cubic& c2, Intersections& i) 
    : cubic1(c1)
    , cubic2(c2)
    , intersections(i)
    , depth(0) 
    , splits(0) {
}

bool intersect() {
    double minT1, minT2, maxT1, maxT2;
    if (!bezier_clip(cubic2, cubic1, minT1, maxT1)) {
        return false;
    }
    if (!bezier_clip(cubic1, cubic2, minT2, maxT2)) {
        return false;
    }
    int split;
    if (maxT1 - minT1 < maxT2 - minT2) {
        intersections.swap();
        minT2 = 0;
        maxT2 = 1;
        split = maxT1 - minT1 > tClipLimit;
    } else {
        minT1 = 0;
        maxT1 = 1;
        split = (maxT2 - minT2 > tClipLimit) << 1;
    }
    return chop(minT1, maxT1, minT2, maxT2, split);
}

protected:
        
bool intersect(double minT1, double maxT1, double minT2, double maxT2) {
    Cubic smaller, larger;
    // FIXME: carry last subdivide and reduceOrder result with cubic 
    sub_divide(cubic1, minT1, maxT1, intersections.swapped() ? larger : smaller);
    sub_divide(cubic2, minT2, maxT2, intersections.swapped() ? smaller : larger);
    Cubic smallResult;
    if (reduceOrder(smaller, smallResult,
            kReduceOrder_NoQuadraticsAllowed) <= 2) {
        Cubic largeResult;
        if (reduceOrder(larger, largeResult,
                kReduceOrder_NoQuadraticsAllowed) <= 2) {
            _Point pt;
            const _Line& smallLine = (const _Line&) smallResult;
            const _Line& largeLine = (const _Line&) largeResult;
            if (!lineIntersect(smallLine, largeLine, &pt)) {
                return false;
            }
            double smallT = t_at(smallLine, pt);
            double largeT = t_at(largeLine, pt);
            if (intersections.swapped()) {
                smallT = interp(minT2, maxT2, smallT); 
                largeT = interp(minT1, maxT1, largeT); 
            } else {
                smallT = interp(minT1, maxT1, smallT); 
                largeT = interp(minT2, maxT2, largeT); 
            }
            intersections.add(smallT, largeT);
            return true;
        }
    }
    double minT, maxT;
    if (!bezier_clip(smaller, larger, minT, maxT)) {
        if (minT == maxT) {
            if (intersections.swapped()) {
                minT1 = (minT1 + maxT1) / 2;
                minT2 = interp(minT2, maxT2, minT);
            } else {
                minT1 = interp(minT1, maxT1, minT);
                minT2 = (minT2 + maxT2) / 2;
            }
            intersections.add(minT1, minT2);
            return true;
        }
        return false;
    }
    
    int split;
    if (intersections.swapped()) {
        double newMinT1 = interp(minT1, maxT1, minT);
        double newMaxT1 = interp(minT1, maxT1, maxT);
        split = (newMaxT1 - newMinT1 > (maxT1 - minT1) * tClipLimit) << 1;
#define VERBOSE 0
#if VERBOSE
        printf("%s d=%d s=%d new1=(%g,%g) old1=(%g,%g) split=%d\n",
                __FUNCTION__, depth, splits, newMinT1, newMaxT1, minT1, maxT1,
                split);
#endif
        minT1 = newMinT1;
        maxT1 = newMaxT1;
    } else {
        double newMinT2 = interp(minT2, maxT2, minT);
        double newMaxT2 = interp(minT2, maxT2, maxT);
        split = newMaxT2 - newMinT2 > (maxT2 - minT2) * tClipLimit;
#if VERBOSE
        printf("%s d=%d s=%d new2=(%g,%g) old2=(%g,%g) split=%d\n",
                __FUNCTION__, depth, splits, newMinT2, newMaxT2, minT2, maxT2,
                split);
#endif
        minT2 = newMinT2;
        maxT2 = newMaxT2;
    }
    return chop(minT1, maxT1, minT2, maxT2, split);
}

bool chop(double minT1, double maxT1, double minT2, double maxT2, int split) {
    ++depth;
    intersections.swap();
    if (split) {
        ++splits;
        if (split & 2) {
            double middle1 = (maxT1 + minT1) / 2;
            intersect(minT1, middle1, minT2, maxT2);
            intersect(middle1, maxT1, minT2, maxT2);
        } else {
            double middle2 = (maxT2 + minT2) / 2;
            intersect(minT1, maxT1, minT2, middle2);
            intersect(minT1, maxT1, middle2, maxT2);
        }
        --splits;
        intersections.swap();
        --depth;
        return intersections.intersected();
    }
    bool result = intersect(minT1, maxT1, minT2, maxT2);
    intersections.swap();
    --depth;
    return result;
}

private:

static const double tClipLimit = 0.8; // http://cagd.cs.byu.edu/~tom/papers/bezclip.pdf see Multiple intersections
const Cubic& cubic1;
const Cubic& cubic2;
Intersections& intersections;
int depth;
int splits;
};

bool intersectStartT(const Cubic& c1, const Cubic& c2, Intersections& i) {
    CubicIntersections c(c1, c2, i);
    return c.intersect();
}

