// from http://tog.acm.org/resources/GraphicsGems/gems/Roots3And4.c
/*
 *  Roots3And4.c
 *
 *  Utility functions to find cubic and quartic roots,
 *  coefficients are passed like this:
 *
 *      c[0] + c[1]*x + c[2]*x^2 + c[3]*x^3 + c[4]*x^4 = 0
 *
 *  The functions return the number of non-complex roots and
 *  put the values into the s array.
 *
 *  Author:         Jochen Schwarze (schwarze@isa.de)
 *
 *  Jan 26, 1990    Version for Graphics Gems
 *  Oct 11, 1990    Fixed sign problem for negative q's in SolveQuartic
 *                  (reported by Mark Podlipec),
 *                  Old-style function definitions,
 *                  IsZero() as a macro
 *  Nov 23, 1990    Some systems do not declare acos() and cbrt() in
 *                  <math.h>, though the functions exist in the library.
 *                  If large coefficients are used, EQN_EPS should be
 *                  reduced considerably (e.g. to 1E-30), results will be
 *                  correct but multiple roots might be reported more
 *                  than once.
 */

#include    <math.h>
#include "CubicUtilities.h"
#include "QuadraticUtilities.h"
#include "QuarticRoot.h"

int reducedQuarticRoots(const double t4, const double t3, const double t2, const double t1,
        const double t0, const bool oneHint, double roots[4]) {
#if SK_DEBUG
    // create a string mathematica understands
    // GDB set print repe 15 # if repeated digits is a bother
    //     set print elements 400 # if line doesn't fit
    char str[1024];
    bzero(str, sizeof(str));
    sprintf(str, "Solve[%1.19g x^4 + %1.19g x^3 + %1.19g x^2 + %1.19g x + %1.19g == 0, x]",
        t4, t3, t2, t1, t0);
    mathematica_ize(str, sizeof(str));
#if ONE_OFF_DEBUG && ONE_OFF_DEBUG_MATHEMATICA
    SkDebugf("%s\n", str);
#endif
#endif
#if 0 && SK_DEBUG
    bool t4Or = approximately_zero_when_compared_to(t4, t0) // 0 is one root
            || approximately_zero_when_compared_to(t4, t1)
            || approximately_zero_when_compared_to(t4, t2);
    bool t4And = approximately_zero_when_compared_to(t4, t0) // 0 is one root
            && approximately_zero_when_compared_to(t4, t1)
            && approximately_zero_when_compared_to(t4, t2);
    if (t4Or != t4And) {
        SkDebugf("%s t4 or and\n", __FUNCTION__);
    }
    bool t3Or = approximately_zero_when_compared_to(t3, t0)
            || approximately_zero_when_compared_to(t3, t1)
            || approximately_zero_when_compared_to(t3, t2);
    bool t3And = approximately_zero_when_compared_to(t3, t0)
            && approximately_zero_when_compared_to(t3, t1)
            && approximately_zero_when_compared_to(t3, t2);
    if (t3Or != t3And) {
        SkDebugf("%s t3 or and\n", __FUNCTION__);
    }
    bool t0Or = approximately_zero_when_compared_to(t0, t1) // 0 is one root
            && approximately_zero_when_compared_to(t0, t2)
            && approximately_zero_when_compared_to(t0, t3)
            && approximately_zero_when_compared_to(t0, t4);
    bool t0And = approximately_zero_when_compared_to(t0, t1) // 0 is one root
            && approximately_zero_when_compared_to(t0, t2)
            && approximately_zero_when_compared_to(t0, t3)
            && approximately_zero_when_compared_to(t0, t4);
    if (t0Or != t0And) {
        SkDebugf("%s t0 or and\n", __FUNCTION__);
    }
#endif
    if (approximately_zero_when_compared_to(t4, t0) // 0 is one root
            && approximately_zero_when_compared_to(t4, t1)
            && approximately_zero_when_compared_to(t4, t2)) {
        if (approximately_zero_when_compared_to(t3, t0)
            && approximately_zero_when_compared_to(t3, t1)
            && approximately_zero_when_compared_to(t3, t2)) {
            return quadraticRootsReal(t2, t1, t0, roots);
        }
        if (approximately_zero_when_compared_to(t4, t3)) {
            return cubicRootsReal(t3, t2, t1, t0, roots);
        }
    }
    if ((approximately_zero_when_compared_to(t0, t1) || approximately_zero(t1))// 0 is one root
      //      && approximately_zero_when_compared_to(t0, t2)
            && approximately_zero_when_compared_to(t0, t3)
            && approximately_zero_when_compared_to(t0, t4)) {
        int num = cubicRootsReal(t4, t3, t2, t1, roots);
        for (int i = 0; i < num; ++i) {
            if (approximately_zero(roots[i])) {
                return num;
            }
        }
        roots[num++] = 0;
        return num;
    }
    if (oneHint) {
        SkASSERT(approximately_zero(t4 + t3 + t2 + t1 + t0)); // 1 is one root
        int num = cubicRootsReal(t4, t4 + t3, -(t1 + t0), -t0, roots); // note that -C==A+B+D+E
        for (int i = 0; i < num; ++i) {
            if (approximately_equal(roots[i], 1)) {
                return num;
            }
        }
        roots[num++] = 1;
        return num;
    }
    return -1;
}

int quarticRootsReal(int firstCubicRoot, const double A, const double B, const double C,
        const double D, const double E, double s[4]) {
    double  u, v;
    /* normal form: x^4 + Ax^3 + Bx^2 + Cx + D = 0 */
    const double invA = 1 / A;
    const double a = B * invA;
    const double b = C * invA;
    const double c = D * invA;
    const double d = E * invA;
    /*  substitute x = y - a/4 to eliminate cubic term:
    x^4 + px^2 + qx + r = 0 */
    const double a2 = a * a;
    const double p = -3 * a2 / 8 + b;
    const double q = a2 * a / 8 - a * b / 2 + c;
    const double r = -3 * a2 * a2 / 256 + a2 * b / 16 - a * c / 4 + d;
    int num;
    if (approximately_zero(r)) {
    /* no absolute term: y(y^3 + py + q) = 0 */
        num = cubicRootsReal(1, 0, p, q, s);
        s[num++] = 0;
    } else {
        /* solve the resolvent cubic ... */
        double cubicRoots[3];
        int roots = cubicRootsReal(1, -p / 2, -r, r * p / 2 - q * q / 8, cubicRoots);
        int index;
    #if 0 && SK_DEBUG // enable to verify that any cubic root is as good as any other
        double tries[3][4];
        int nums[3];
        for (index = 0; index < roots; ++index) {
            /* ... and take one real solution ... */
            const double z = cubicRoots[index];
            /* ... to build two quadric equations */
            u = z * z - r;
            v = 2 * z - p;
            if (approximately_zero_squared(u)) {
                u = 0;
            } else if (u > 0) {
                u = sqrt(u);
            } else {
                SkDebugf("%s u=%1.9g <0\n", __FUNCTION__, u);
                continue;
            }
            if (approximately_zero_squared(v)) {
                v = 0;
            } else if (v > 0) {
                v = sqrt(v);
            } else {
                SkDebugf("%s v=%1.9g <0\n", __FUNCTION__, v);
                continue;
            }
            nums[index] = quadraticRootsReal(1, q < 0 ? -v : v, z - u, tries[index]);
            nums[index] += quadraticRootsReal(1, q < 0 ? v : -v, z + u, tries[index] + nums[index]);
            /* resubstitute */
            const double sub = a / 4;
            for (int i = 0; i < nums[index]; ++i) {
                tries[index][i] -= sub;
            }
        }
        for (index = 0; index < roots; ++index) {
            SkDebugf("%s", __FUNCTION__);
            for (int idx2 = 0; idx2 < nums[index]; ++idx2) {
                SkDebugf(" %1.9g", tries[index][idx2]);
            }
            SkDebugf("\n");
        }
    #endif
        /* ... and take one real solution ... */
        double z;
        num = 0;
        int num2 = 0;
        for (index = firstCubicRoot; index < roots; ++index) {
            z = cubicRoots[index];
            /* ... to build two quadric equations */
            u = z * z - r;
            v = 2 * z - p;
            if (approximately_zero_squared(u)) {
                u = 0;
            } else if (u > 0) {
                u = sqrt(u);
            } else {
                continue;
            }
            if (approximately_zero_squared(v)) {
                v = 0;
            } else if (v > 0) {
                v = sqrt(v);
            } else {
                continue;
            }
            num = quadraticRootsReal(1, q < 0 ? -v : v, z - u, s);
            num2 = quadraticRootsReal(1, q < 0 ? v : -v, z + u, s + num);
            if (!((num | num2) & 1)) {
                break; // prefer solutions without single quad roots
            }
        }
        num += num2;
        if (!num) {
            return 0; // no valid cubic root
        }
    }
    /* resubstitute */
    const double sub = a / 4;
    for (int i = 0; i < num; ++i) {
        s[i] -= sub;
    }
    // eliminate duplicates
    for (int i = 0; i < num - 1; ++i) {
        for (int j = i + 1; j < num; ) {
            if (AlmostEqualUlps(s[i], s[j])) {
                if (j < --num) {
                    s[j] = s[num];
                }
            } else {
                ++j;
            }
        }
    }
    return num;
}
