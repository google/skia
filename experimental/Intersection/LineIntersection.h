#ifndef LineIntersection_DEFINE
#define LineIntersection_DEFINE

#include "DataTypes.h"

int horizontalIntersect(const _Line& line, double y, double tRange[2]);
int horizontalLineIntersect(const _Line& line, double left, double right,
        double y, double tRange[2]);
int verticalLineIntersect(const _Line& line, double top, double bottom,
        double x, double tRange[2]);
int intersect(const _Line& a, const _Line& b, double aRange[2], double bRange[2]);
bool testIntersect(const _Line& a, const _Line& b);

#endif
