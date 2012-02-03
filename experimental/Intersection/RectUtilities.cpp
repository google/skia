#include "DataTypes.h"
#include "Extrema.h"

void _Rect::setBounds(const Cubic& cubic) {
    set(cubic[0]);
    add(cubic[3]);
    double tValues[4];
    int roots = SkFindCubicExtrema(cubic[0].x, cubic[1].x, cubic[2].x,
            cubic[3].x, tValues);
    roots += SkFindCubicExtrema(cubic[0].y, cubic[1].y, cubic[2].y, cubic[3].y,
            &tValues[roots]);
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

void _Rect::setBounds(const Quadratic& quad) {
    set(quad[0]);
    add(quad[2]);
    double tValues[2];
    int roots = SkFindQuadExtrema(quad[0].x, quad[1].x, quad[2].x, tValues);
    roots += SkFindQuadExtrema(quad[0].y, quad[1].y, quad[2].y, &tValues[roots]);
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
