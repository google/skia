#include "DataTypes.h"

double cube_root(double x);
void coefficients(const double* cubic, double& A, double& B, double& C, double& D);
int cubicRoots(double A, double B, double C, double D, double t[3]);
double derivativeAtT(const double* cubic, double t);
// competing version that should produce same results
double derivativeAtT_2(const double* cubic, double t);
void dxdy_at_t(const Cubic& , double t, double& x, double& y);
bool rotate(const Cubic& cubic, int zero, int index, Cubic& rotPath);
double secondDerivativeAtT(const double* cubic, double t);
void xy_at_t(const Cubic& , double t, double& x, double& y);
