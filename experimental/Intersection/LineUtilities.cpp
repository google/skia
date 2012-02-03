#include "CurveIntersection.h"
#include "LineUtilities.h"

bool implicitLine(const _Line& line, double& slope, double& axisIntercept) {
    _Point delta;
    tangent(line, delta);
    bool moreHorizontal = fabs(delta.x) > fabs(delta.y);
    if (moreHorizontal) {
        slope = delta.y / delta.x;
        axisIntercept = line[0].y - slope * line[0].x;
    } else {
        slope = delta.x / delta.y;
        axisIntercept = line[0].x - slope * line[0].y;
    }
    return moreHorizontal;
}

int reduceOrder(const _Line& line, _Line& reduced) {
    reduced[0] = line[0];
    int different = line[0] != line[1];
    reduced[1] = line[different];
    return 1 + different;
}
