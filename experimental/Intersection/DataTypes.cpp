#include "DataTypes.h"

#include <sys/types.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

void	*memcpy(void *, const void *, size_t);

#ifdef __cplusplus
}
#endif


#if USE_EPSILON
const double PointEpsilon = 0.000001;
const double SquaredEpsilon = PointEpsilon * PointEpsilon;
#endif

const int UlpsEpsilon = 16;

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

void xy_at_t(const Cubic& cubic, double t, double& x, double& y) {
    double one_t = 1 - t;
    double one_t2 = one_t * one_t;
    double a = one_t2 * one_t;
    double b = 3 * one_t2 * t;
    double t2 = t * t;
    double c = 3 * one_t * t2;
    double d = t2 * t;
    if (&x) {
        x = a * cubic[0].x + b * cubic[1].x + c * cubic[2].x + d * cubic[3].x;
    }
    if (&y) {
        y = a * cubic[0].y + b * cubic[1].y + c * cubic[2].y + d * cubic[3].y;
    }
}

void xy_at_t(const _Line& line, double t, double& x, double& y) {
    double one_t = 1 - t;
    if (&x) {
        x = one_t * line[0].x + t * line[1].x;
    }
    if (&y) {
        y = one_t * line[0].y + t * line[1].y;
    }
}

void xy_at_t(const Quadratic& quad, double t, double& x, double& y) {
    double one_t = 1 - t;
    double a = one_t * one_t;
    double b = 2 * one_t * t;
    double c = t * t;
    if (&x) {
        x = a * quad[0].x + b * quad[1].x + c * quad[2].x;
    }
    if (&y) {
        y = a * quad[0].y + b * quad[1].y + c * quad[2].y;
    }
}


// from http://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
union Float_t
{
    Float_t(float num = 0.0f) : f(num) {}
    // Portable extraction of components.
    bool Negative() const { return (i >> 31) != 0; }
    int32_t RawMantissa() const { return i & ((1 << 23) - 1); }
    int32_t RawExponent() const { return (i >> 23) & 0xFF; }
 
    int32_t i;
    float f;
#ifdef _DEBUG
    struct
    {   // Bitfields for exploration. Do not use in production code.
        uint32_t mantissa : 23;
        uint32_t exponent : 8;
        uint32_t sign : 1;
    } parts;
#endif
};
 
bool AlmostEqualUlps(float A, float B, int maxUlpsDiff)
{
    Float_t uA(A);
    Float_t uB(B);
 
    // Different signs means they do not match.
    if (uA.Negative() != uB.Negative())
    {
        // Check for equality to make sure +0==-0
        return A == B;
    }
 
    // Find the difference in ULPs.
    int ulpsDiff = abs(uA.i - uB.i);
    return ulpsDiff <= maxUlpsDiff;
}

int UlpsDiff(float A, float B) 
{
    Float_t uA(A);
    Float_t uB(B);

    return abs(uA.i - uB.i);
}

int FloatAsInt(float A)
{
    Float_t uA(A);
    return uA.i;
}


