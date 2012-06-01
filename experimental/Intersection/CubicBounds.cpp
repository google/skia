#include "CurveIntersection.h"
#include "Extrema.h"

static int isBoundedByEndPoints(double a, double b, double c, double d)
{
    return (a <= b && a <= c && b <= d && c <= d)
            || (a >= b && a >= c && b >= d && c >= d);
}

double leftMostT(const Cubic& cubic, double startT, double endT) {
    double leftTs[2];
    _Point pt[2];
    int results = findExtrema(cubic[0].x, cubic[1].x, cubic[2].x, cubic[3].x,
            leftTs);
    int best = -1;
    for (int index = 0; index < results; ++index) {
        if (startT > leftTs[index] || leftTs[index] > endT) {
            continue;
        }
        if (best < 0) {
            best = index;
            continue;
        }
        xy_at_t(cubic, leftTs[0], pt[0].x, pt[0].y);
        xy_at_t(cubic, leftTs[1], pt[1].x, pt[1].y);
        if (pt[0].x > pt[1].x) {
            best = 1;
        }
    }
    if (best >= 0) {
        return leftTs[best];
    }
    xy_at_t(cubic, startT, pt[0].x, pt[0].y);
    xy_at_t(cubic, endT, pt[1].x, pt[1].y);
    return pt[0].x <= pt[1].x ? startT : endT;
}

void _Rect::setBounds(const Cubic& cubic) {
    set(cubic[0]);
    add(cubic[3]);
    double tValues[4];
    int roots = 0;
    if (!isBoundedByEndPoints(cubic[0].x, cubic[1].x, cubic[2].x, cubic[3].x)) {
        roots = findExtrema(cubic[0].x, cubic[1].x, cubic[2].x,
                cubic[3].x, tValues);
    }
    if (!isBoundedByEndPoints(cubic[0].y, cubic[1].y, cubic[2].y, cubic[3].y)) {
        roots += findExtrema(cubic[0].y, cubic[1].y, cubic[2].y,
                cubic[3].y, &tValues[roots]);
    }
    for (int x = 0; x < roots; ++x) {
        _Point result;
        xy_at_t(cubic, tValues[x], result.x, result.y);
        add(result);
    }
}

void _Rect::setRawBounds(const Cubic& cubic) {
    set(cubic[0]);
    for (int x = 1; x < 4; ++x) {
        add(cubic[x]);
    }
}
