#include "DataTypes.h"

const double PointEpsilon = 0.000001;
const double SquaredEpsilon = PointEpsilon * PointEpsilon;

bool rotate(const Cubic& cubic, int zero, int index, Cubic& rotPath) {
    double dy = cubic[index].y - cubic[zero].y;
    double dx = cubic[index].x - cubic[zero].x;
    if (approximately_equal(dy, 0)) {
        if (approximately_equal(dx, 0)) {
            return false;
        }
        memcpy(rotPath, cubic, sizeof(Cubic));
        return true;
    }
    for (int index = 0; index < 4; ++index) {
        rotPath[index].x = cubic[index].x * dx + cubic[index].y * dy;
        rotPath[index].y = cubic[index].y * dx - cubic[index].x * dy;
    }
    return true;
}

double t_at(const _Line& line, const _Point& pt) {
    double dx = line[1].x - line[0].x;
    double dy = line[1].y - line[0].y;
    if (fabs(dx) > fabs(dy)) {
        if (approximately_zero(dx)) {
            return 0;
        }
        return (pt.x - line[0].x) / dx;
    }
    if (approximately_zero(dy)) {
        return 0;
    }
    return (pt.y - line[0].y) / dy;
}

static void setMinMax(double x, double y1, double y2, int flags,
        double& minX, double& maxX) {
    if (minX > x && (flags & (kFindTopMin | kFindBottomMin))) {
        minX = x;
    }
    if (maxX < x && (flags & (kFindTopMax | kFindBottomMax))) {
        maxX = x;
    }
}

void x_at(const _Point& p1, const _Point& p2, double top, double bottom,
        int flags, double& minX, double& maxX) {
    if (approximately_equal(p1.y, p2.y)) {
        // It should be OK to bail early in this case. There's another edge
        // which shares this end point which can intersect without failing to 
        // have a slope ... maybe
        return;
    }
    
    // p2.x is always greater than p1.x -- the part of points (p1, p2) are
    // moving from the start of the cubic towards its end.
    // if p1.y < p2.y, minX can be affected
    // if p1.y > p2.y, maxX can be affected 
    double slope = (p2.x - p1.x) / (p2.y - p1.y);
    int topFlags = flags & (kFindTopMin | kFindTopMax);
    if (topFlags && (top <= p1.y && top >= p2.y
            || top >= p1.y && top <= p2.y)) {
        double x = p1.x + (top - p1.y) * slope;
        setMinMax(x, p1.y, p2.y, topFlags, minX, maxX);
    }
    int bottomFlags = flags & (kFindBottomMin | kFindBottomMax);
    if (bottomFlags && (bottom <= p1.y && bottom >= p2.y
            || bottom >= p1.y && bottom <= p2.y)) {
        double x = p1.x + (bottom - p1.y) * slope;
        setMinMax(x, p1.y, p2.y, bottomFlags, minX, maxX);
    }
}
