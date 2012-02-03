#include "CurveIntersection.h"
#include "IntersectionUtilities.h"

/*
 Given a cubic c, t1, and t2, find a small cubic segment.
 
 The new cubic is defined as points A, B, C, and D, where
 s1 = 1 - t1
 s2 = 1 - t2
 A = c[0]*s1*s1*s1 + 3*c[1]*s1*s1*t1 + 3*c[2]*s1*t1*t1 + c[3]*t1*t1*t1
 D = c[0]*s2*s2*s2 + 3*c[1]*s2*s2*t2 + 3*c[2]*s2*t2*t2 + c[3]*t2*t2*t2
 
 We don't have B or C. So We define two equations to isolate them.
 First, compute two reference T values 1/3 and 2/3 from t1 to t2:
 
 c(at (2*t1 + t2)/3) == E
 c(at (t1 + 2*t2)/3) == F
 
 Next, compute where those values must be if we know the values of B and C:
 
 _12   =  A*2/3 + B*1/3
 12_   =  A*1/3 + B*2/3
 _23   =  B*2/3 + C*1/3
 23_   =  B*1/3 + C*2/3
 _34   =  C*2/3 + D*1/3
 34_   =  C*1/3 + D*2/3
 _123  = (A*2/3 + B*1/3)*2/3 + (B*2/3 + C*1/3)*1/3 = A*4/9 + B*4/9 + C*1/9
 123_  = (A*1/3 + B*2/3)*1/3 + (B*1/3 + C*2/3)*2/3 = A*1/9 + B*4/9 + C*4/9
 _234  = (B*2/3 + C*1/3)*2/3 + (C*2/3 + D*1/3)*1/3 = B*4/9 + C*4/9 + D*1/9
 234_  = (B*1/3 + C*2/3)*1/3 + (C*1/3 + D*2/3)*2/3 = B*1/9 + C*4/9 + D*4/9
 _1234 = (A*4/9 + B*4/9 + C*1/9)*2/3 + (B*4/9 + C*4/9 + D*1/9)*1/3
       =  A*8/27 + B*12/27 + C*6/27 + D*1/27
       =  E
 1234_ = (A*1/9 + B*4/9 + C*4/9)*1/3 + (B*1/9 + C*4/9 + D*4/9)*2/3
       =  A*1/27 + B*6/27 + C*12/27 + D*8/27
       =  F
 E*27  =  A*8    + B*12   + C*6     + D
 F*27  =  A      + B*6    + C*12    + D*8
 
Group the known values on one side:
       
 M       = E*27 - A*8 - D     = B*12 + C* 6
 N       = F*27 - A   - D*8   = B* 6 + C*12
 M*2 - N = B*18
 N*2 - M = C*18
 B       = (M*2 - N)/18
 C       = (N*2 - M)/18
 */
 
static double interp_cubic_coords(const double* src, double t)
{
    double ab = interp(src[0], src[2], t);
    double bc = interp(src[2], src[4], t);
    double cd = interp(src[4], src[6], t);
    double abc = interp(ab, bc, t);
    double bcd = interp(bc, cd, t);
    double abcd = interp(abc, bcd, t);
    return abcd;
}
 
void sub_divide(const Cubic& src, double t1, double t2, Cubic& dst) {
    double ax = dst[0].x = interp_cubic_coords(&src[0].x, t1);
    double ay = dst[0].y = interp_cubic_coords(&src[0].y, t1);
    double ex = interp_cubic_coords(&src[0].x, (t1*2+t2)/3);
    double ey = interp_cubic_coords(&src[0].y, (t1*2+t2)/3);
    double fx = interp_cubic_coords(&src[0].x, (t1+t2*2)/3);
    double fy = interp_cubic_coords(&src[0].y, (t1+t2*2)/3);
    double dx = dst[3].x = interp_cubic_coords(&src[0].x, t2);
    double dy = dst[3].y = interp_cubic_coords(&src[0].y, t2);
    double mx = ex * 27 - ax * 8 - dx;
    double my = ey * 27 - ay * 8 - dy;
    double nx = fx * 27 - ax - dx * 8;
    double ny = fy * 27 - ay - dy * 8;
    /* bx = */ dst[1].x = (mx * 2 - nx) / 18;
    /* by = */ dst[1].y = (my * 2 - ny) / 18;
    /* cx = */ dst[2].x = (nx * 2 - mx) / 18;
    /* cy = */ dst[2].y = (ny * 2 - my) / 18;
}

/* classic one t subdivision */
static void interp_cubic_coords(const double* src, double* dst, double t)
{
    double ab = interp(src[0], src[2], t);
    double bc = interp(src[2], src[4], t);
    double cd = interp(src[4], src[6], t);
    double abc = interp(ab, bc, t);
    double bcd = interp(bc, cd, t);
    double abcd = interp(abc, bcd, t);

    dst[0] = src[0];
    dst[2] = ab;
    dst[4] = abc;
    dst[6] = abcd;
    dst[8] = bcd;
    dst[10] = cd;
    dst[12] = src[6];
}

void chop_at(const Cubic& src, CubicPair& dst, double t)
{
    interp_cubic_coords(&src[0].x, &dst.pts[0].x, t);
    interp_cubic_coords(&src[0].y, &dst.pts[0].y, t);
}
