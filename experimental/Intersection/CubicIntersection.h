#include "DataTypes.h"

class Intersections;

// unit-testable utilities
bool bezier_clip(const Cubic& cubic1, const Cubic& cubic2, double& minT, double& maxT);
bool bezier_clip(const Quadratic& q1, const Quadratic& q2, double& minT, double& maxT);
void chop_at(const Cubic& src, CubicPair& dst, double t);
void chop_at(const Quadratic& src, QuadraticPair& dst, double t);
int convex_hull(const Cubic& cubic, char order[4]);
bool convex_x_hull(const Cubic& cubic, char connectTo0[2], char connectTo3[2]);
double cube_root(double x);
int cubic_roots(const double coeff[4], double tValues[3]);
bool implicit_matches(const Cubic& cubic1, const Cubic& cubic2);
bool implicit_matches(const Quadratic& quad1, const Quadratic& quad2);
int quadratic_roots(const double coeff[3], double tValues[2]);
void sub_divide(const Cubic& src, double t1, double t2, Cubic& dst);
void sub_divide(const Quadratic& src, double t1, double t2, Quadratic& dst);
void tangent(const Cubic& cubic, double t, _Point& result);
void tangent(const Quadratic& cubic, double t, _Point& result);

// main functions
enum ReduceOrder_Flags {
    kReduceOrder_NoQuadraticsAllowed,
    kReduceOrder_QuadraticsAllowed
};
int reduceOrder(const Cubic& cubic, Cubic& reduction, ReduceOrder_Flags );
int reduceOrder(const Quadratic& quad, Quadratic& reduction);
bool intersectStart(const Cubic& cubic1, const Cubic& cubic2, Intersections& );
bool intersectStartT(const Cubic& cubic1, const Cubic& cubic2, Intersections& );
bool intersectStart(const Quadratic& q1, const Quadratic& q2, Intersections& );
