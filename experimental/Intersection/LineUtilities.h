#include "DataTypes.h"

bool implicitLine(const _Line& line, double& slope, double& axisIntercept);
int reduceOrder(const _Line& line, _Line& reduced);

double t_at(const _Line&, const _Point& );
void xy_at_t(const _Line& , double t, double& x, double& y);

enum x_at_flags {
    kFindTopMin = 1,
    kFindTopMax = 2,
    kFindBottomMin = 4,
    kFindBottomMax = 8
};

void x_at(const _Point& p1, const _Point& p2, double minY, double maxY,
        int flags, double& tMin, double& tMax);

