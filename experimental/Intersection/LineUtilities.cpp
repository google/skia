#include "LineUtilities.h"

bool implicitLine(const _Line& line, double& slope, double& axisIntercept) {
    double lineDx = line[1].x - line[0].x;
    double lineDy = line[1].y - line[0].y;
    bool moreHorizontal = fabs(lineDx) > fabs(lineDy);
    if (moreHorizontal) {
        slope = lineDy / lineDx;
        axisIntercept = line[0].y - slope * line[0].x;
    } else {
        slope = lineDx / lineDy;
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
