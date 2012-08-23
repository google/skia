#include "CurveIntersection.h"
#include "CurveUtilities.h"
#include "IntersectionUtilities.h"

/* Given a cubic, find the convex hull described by the end and control points.
   The hull may have 3 or 4 points. Cubics that degenerate into a point or line
   are not considered.

   The hull is computed by assuming that three points, if unique and non-linear,
   form a triangle. The fourth point may replace one of the first three, may be
   discarded if in the triangle or on an edge, or may be inserted between any of
   the three to form a convex quadralateral.

   The indices returned in order describe the convex hull.
*/
int convex_hull(const Cubic& cubic, char order[4]) {
    size_t index;
    // find top point
    size_t yMin = 0;
    for (index = 1; index < 4; ++index) {
        if (cubic[yMin].y > cubic[index].y || (cubic[yMin].y == cubic[index].y
                && cubic[yMin].x > cubic[index].x)) {
            yMin = index;
        }
    }
    order[0] = yMin;
    int midX = -1;
    int backupYMin = -1;
    for (int pass = 0; pass < 2; ++pass) {
        for (index = 0; index < 4; ++index) {
            if (index == yMin) {
                continue;
            }
            // rotate line from (yMin, index) to axis
            // see if remaining two points are both above or below
            // use this to find mid
            int mask = other_two(yMin, index);
            int side1 = yMin ^ mask;
            int side2 = index ^ mask;
            Cubic rotPath;
            if (!rotate(cubic, yMin, index, rotPath)) { // ! if cbc[yMin]==cbc[idx]
                order[1] = side1;
                order[2] = side2;
                return 3;
            }
            int sides = side(rotPath[side1].y - rotPath[yMin].y);
            sides ^= side(rotPath[side2].y - rotPath[yMin].y);
            if (sides == 2) { // '2' means one remaining point <0, one >0
                if (midX >= 0) {
                    printf("%s unexpected mid\n", __FUNCTION__); // there can be only one mid
                }
                midX = index;
            } else if (sides == 0) { // '0' means both to one side or the other
                backupYMin = index;
            }
        }
        if (midX >= 0) {
            break;
        }
        if (backupYMin < 0) {
            break;
        }
        yMin = backupYMin;
        backupYMin = -1;
    }
    if (midX < 0) {
        midX = yMin ^ 3; // choose any other point
    }
    int mask = other_two(yMin, midX);
    int least = yMin ^ mask;
    int most = midX ^ mask;
    order[0] = yMin;
    order[1] = least;

    // see if mid value is on same side of line (least, most) as yMin
    Cubic midPath;
    if (!rotate(cubic, least, most, midPath)) { // ! if cbc[least]==cbc[most]
        order[2] = midX;
        return 3;
    }
    int midSides = side(midPath[yMin].y - midPath[least].y);
    midSides ^= side(midPath[midX].y - midPath[least].y);
    if (midSides != 2) {  // if mid point is not between
        order[2] = most;
        return 3; // result is a triangle
    }
    order[2] = midX;
    order[3] = most;
    return 4; // result is a quadralateral
}

/* Find the convex hull for cubics with the x-axis interval regularly spaced.
   Cubics computed as distance functions are formed this way.

   connectTo0[0], connectTo0[1] are the point indices that cubic[0] connects to.
   connectTo3[0], connectTo3[1] are the point indices that cubic[3] connects to.

   Returns true if cubic[1] to cubic[2] also forms part of the hull.
*/
bool convex_x_hull(const Cubic& cubic, char connectTo0[2], char connectTo3[2]) {
    double projectedY[4];
    projectedY[0] = 0;
    int index;
    for (index = 1; index < 4; ++index) {
        projectedY[index] = (cubic[index].y - cubic[0].y) * (3.0 / index);
    }
    int lower0Index = 1;
    int upper0Index = 1;
    for (index = 2; index < 4; ++index) {
        if (approximately_greater(projectedY[lower0Index], projectedY[index])) {
            lower0Index = index;
        }
        if (approximately_lesser(projectedY[upper0Index], projectedY[index])) {
            upper0Index = index;
        }
    }
    connectTo0[0] = lower0Index;
    connectTo0[1] = upper0Index;
    for (index = 0; index < 3; ++index) {
        projectedY[index] = (cubic[3].y - cubic[index].y) * (3.0 / (3 - index));
    }
    projectedY[3] = 0;
    int lower3Index = 2;
    int upper3Index = 2;
    for (index = 1; index > -1; --index) {
        if (approximately_greater(projectedY[lower3Index], projectedY[index])) {
            lower3Index = index;
        }
        if (approximately_lesser(projectedY[upper3Index], projectedY[index])) {
            upper3Index = index;
        }
    }
    connectTo3[0] = lower3Index;
    connectTo3[1] = upper3Index;
    return (1 << lower0Index | 1 << upper0Index
            | 1 << lower3Index | 1 << upper3Index) == 0x0F;
}

