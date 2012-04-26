#include "DataTypes.h"
#include "Extrema.h"

static int isBoundedByEndPoints(double a, double b, double c)
{
    return (a <= b && b <= c) || (a >= b && b >= c);
}

double leftMostT(const Quadratic& quad, double startT, double endT) {
    double leftT;
    if (findExtrema(quad[0].x, quad[1].x, quad[2].x, &leftT)
            && startT <= leftT && leftT <= endT) {
        return leftT;
    }
    _Point startPt;
    xy_at_t(quad, startT, startPt.x, startPt.y);
    _Point endPt;
    xy_at_t(quad, endT, endPt.x, endPt.y);
    return startPt.x <= endPt.x ? startT : endT;
}

void _Rect::setBounds(const Quadratic& quad) {
    set(quad[0]);
    add(quad[2]);
    double tValues[2];
    int roots = 0;
    if (!isBoundedByEndPoints(quad[0].x, quad[1].x, quad[2].x)) {
        roots = findExtrema(quad[0].x, quad[1].x, quad[2].x, tValues);
    }
    if (!isBoundedByEndPoints(quad[0].y, quad[1].y, quad[2].y)) {
        roots += findExtrema(quad[0].y, quad[1].y, quad[2].y,
                &tValues[roots]);
    }
    for (int x = 0; x < roots; ++x) {
        _Point result;
        xy_at_t(quad, tValues[x], result.x, result.y);
        add(result);
    }
}

void _Rect::setRawBounds(const Quadratic& quad) {
    set(quad[0]);
    for (int x = 1; x < 3; ++x) {
        add(quad[x]);
    }
}
