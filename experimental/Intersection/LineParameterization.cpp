#include "CurveIntersection.h"

/* This rejects coincidence with two muls, two adds, and one cmp.
   Coincident candidates will take another four muls and two adds, but may still
   fail if they don't overlap. (The overlap test isn't performed here.)
 */
bool implicit_matches(const _Line& one, const _Line& two) {
    _Point oneD, twoD;
    tangent(one, oneD);
    tangent(two, twoD);
    /* See if the slopes match, i.e.
                        dx1 / dy1 ==               dx2 / dy2
          (dy1 * dy2) * dx1 / dy1 == (dy1 * dy2) * dx2 / dy2
                 dy2  * dx1       ==  dy1        * dx2
     */
    if (!approximately_equal(oneD.x * twoD.y, twoD.x * oneD.y)) {
        return false;
    }
    /* See if the axis intercepts match, i.e.
               y0 - x0 * dy / dx  ==       y1 - x1 * dy / dx
         dx * (y0 - x0 * dy / dx) == dx * (y1 - x1 * dy / dx)
         dx *  y0 - x0 * dy       == dx *  y1 - x1 * dy
     */
    if (!approximately_equal(oneD.x * one[0].y - oneD.y * one[0].x,
            oneD.x * two[0].y - oneD.y * two[0].x)) {
        return false;
    }
    return true;
}

bool implicit_matches_ulps(const _Line& one, const _Line& two, int ulps) {
    _Point oneD, twoD;
    tangent(one, oneD);
    tangent(two, twoD);
    /* See if the slopes match, i.e.
                        dx1 / dy1 ==               dx2 / dy2
          (dy1 * dy2) * dx1 / dy1 == (dy1 * dy2) * dx2 / dy2
                 dy2  * dx1       ==  dy1        * dx2
     */
    int diff = UlpsDiff((float) (oneD.x * twoD.y), (float) (twoD.x * oneD.y));
    if (diff < 0 || diff > ulps) {
        return false;
    }
    /* See if the axis intercepts match, i.e.
               y0 - x0 * dy / dx  ==       y1 - x1 * dy / dx
         dx * (y0 - x0 * dy / dx) == dx * (y1 - x1 * dy / dx)
         dx *  y0 - x0 * dy       == dx *  y1 - x1 * dy
     */
    diff = UlpsDiff((float) (oneD.x * one[0].y - oneD.y * one[0].x),
            (float) (oneD.x * two[0].y - oneD.y * two[0].x));
    return diff >= 0 && diff <= ulps;
}

void tangent(const _Line& line,  _Point& result) {
    result.x = line[0].x - line[1].x;
    result.y = line[0].y - line[1].y;
}
