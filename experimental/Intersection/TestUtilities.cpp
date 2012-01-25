#include "CubicIntersection.h"
#include "TestUtilities.h"

void quad_to_cubic(const Quadratic& quad, Cubic& cubic) {
    cubic[0] = quad[0];
    cubic[1].x = quad[0].x / 3 + quad[1].x * 2 / 3;
    cubic[1].y = quad[0].y / 3 + quad[1].y * 2 / 3;
    cubic[2].x = quad[2].x / 3 + quad[1].x * 2 / 3;
    cubic[2].y = quad[2].y / 3 + quad[1].y * 2 / 3;
    cubic[3] = quad[2];
}

static bool tiny(const Cubic& cubic) {
    int index, minX, maxX, minY, maxY;
    minX = maxX = minY = maxY = 0;
    for (index = 1; index < 4; ++index) {
        if (cubic[minX].x > cubic[index].x) {
            minX = index;
        }
        if (cubic[minY].y > cubic[index].y) {
            minY = index;
        }
        if (cubic[maxX].x < cubic[index].x) {
            maxX = index;
        }
        if (cubic[maxY].y < cubic[index].y) {
            maxY = index;
        }
    }
    return     approximately_equal(cubic[maxX].x, cubic[minX].x)
            && approximately_equal(cubic[maxY].y, cubic[minY].y);
}

void find_tight_bounds(const Cubic& cubic, _Rect& bounds) {
    CubicPair cubicPair;
    chop_at(cubic, cubicPair, 0.5);
    if (!tiny(cubicPair.first()) && !controls_inside(cubicPair.first())) {
        find_tight_bounds(cubicPair.first(), bounds);
    } else {
        bounds.add(cubicPair.first()[0]);
        bounds.add(cubicPair.first()[3]);
    }
    if (!tiny(cubicPair.second()) && !controls_inside(cubicPair.second())) {
        find_tight_bounds(cubicPair.second(), bounds);
    } else {
        bounds.add(cubicPair.second()[0]);
        bounds.add(cubicPair.second()[3]);
    }
}

bool controls_inside(const Cubic& cubic) {
    return
        ((cubic[0].x <= cubic[1].x && cubic[0].x <= cubic[2].x && cubic[1].x <= cubic[3].x && cubic[2].x <= cubic[3].x)
     ||  (cubic[0].x >= cubic[1].x && cubic[0].x >= cubic[2].x && cubic[1].x >= cubic[3].x && cubic[2].x >= cubic[3].x))
     && ((cubic[0].y <= cubic[1].y && cubic[0].y <= cubic[2].y && cubic[1].y <= cubic[3].y && cubic[2].y <= cubic[3].y)
     ||  (cubic[0].y >= cubic[1].y && cubic[0].y >= cubic[2].y && cubic[1].y >= cubic[3].y && cubic[2].x >= cubic[3].y));
}

void xy_at_t(const Cubic& cubic, double t, double& x, double& y) {
    double one_t = 1 - t;
    double one_t2 = one_t * one_t;
    double a = one_t2 * one_t;
    double b = 3 * one_t2 * t;
    double t2 = t * t;
    double c = 3 * one_t * t2;
    double d = t2 * t;
    x = a * cubic[0].x + b * cubic[1].x + c * cubic[2].x + d * cubic[3].x;
    y = a * cubic[0].y + b * cubic[1].y + c * cubic[2].y + d * cubic[3].y;
}

void xy_at_t(const _Line& line, double t, double& x, double& y) {
    double one_t = 1 - t;
    x = one_t * line[0].x + t * line[1].x;
    y = one_t * line[0].y + t * line[1].y;
}

void xy_at_t(const Quadratic& quad, double t, double& x, double& y) {
    double one_t = 1 - t;
    double a = one_t * one_t;
    double b = 2 * one_t * t;
    double c = t * t;
    x = a * quad[0].x + b * quad[1].x + c * quad[2].x;
    y = a * quad[0].y + b * quad[1].y + c * quad[2].y;
}

