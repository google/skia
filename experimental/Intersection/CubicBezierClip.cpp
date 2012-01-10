#include "CubicIntersection.h"
#include "LineParameters.h"
#include <algorithm> // used for std::swap

// return false if unable to clip (e.g., unable to create implicit line)
// caller should subdivide, or create degenerate if the values are too small
bool bezier_clip(const Cubic& cubic1, const Cubic& cubic2, double& minT, double& maxT) {
    minT = 1;
    maxT = 0;
    // determine normalized implicit line equation for pt[0] to pt[3]
    //   of the form ax + by + c = 0, where a*a + b*b == 1
    
    // find the implicit line equation parameters
    LineParameters endLine;
    endLine.cubicEndPoints(cubic1);
    if (!endLine.normalize()) {
        printf("line cannot be normalized: need more code here\n");
        return false;
    }

    double distance[2];
    endLine.controlPtDistance(cubic1, distance);
    
    // find fat line
    double top = distance[0];
    double bottom = distance[1];
    if (top > bottom) {
        std::swap(top, bottom);
    }
    if (top * bottom >= 0) {
        const double scale = 3/4.0; // http://cagd.cs.byu.edu/~tom/papers/bezclip.pdf (13)
        if (top < 0) {
            top *= scale;
            bottom = 0;
        } else {
            top = 0;
            bottom *= scale;
        }
    } else {
        const double scale = 4/9.0; // http://cagd.cs.byu.edu/~tom/papers/bezclip.pdf (15)
        top *= scale;
        bottom *= scale;
    }
    
    // compute intersecting candidate distance
    Cubic distance2y; // points with X of (0, 1/3, 2/3, 1)
    endLine.cubicDistanceY(cubic2, distance2y);
    
    int flags = 0;
    if (approximately_lesser(distance2y[0].y, top)) {
        flags |= kFindTopMin;
    } else if (approximately_greater(distance2y[0].y, bottom)) {
        flags |= kFindBottomMin;
    } else {
        minT = 0;
    }

    if (approximately_lesser(distance2y[3].y, top)) {
        flags |= kFindTopMax;
    } else if (approximately_greater(distance2y[3].y, bottom)) {
        flags |= kFindBottomMax;
    } else {
        maxT = 1;
    }
    // Find the intersection of distance convex hull and fat line.
    char to_0[2];
    char to_3[2];
    bool do_1_2_edge = convex_x_hull(distance2y, to_0, to_3);
    x_at(distance2y[0], distance2y[to_0[0]], top, bottom, flags, minT, maxT);
    if (to_0[0] != to_0[1]) {
        x_at(distance2y[0], distance2y[to_0[1]], top, bottom, flags, minT, maxT);
    }
    x_at(distance2y[to_3[0]], distance2y[3], top, bottom, flags, minT, maxT);
    if (to_3[0] != to_3[1]) {
        x_at(distance2y[to_3[1]], distance2y[3], top, bottom, flags, minT, maxT);
    }
    if (do_1_2_edge) {
        x_at(distance2y[1], distance2y[2], top, bottom, flags, minT, maxT);
    }
    
    return minT < maxT; // returns false if distance shows no intersection
}

