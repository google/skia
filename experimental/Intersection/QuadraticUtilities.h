/* Parameterization form, given A*t*t + 2*B*t*(1-t) + C*(1-t)*(1-t)
 *
 * a = A - 2*B +   C
 * b =     2*B - 2*C
 * c =             C
 */
inline void set_abc(const double* quad, double& a, double& b, double& c) {
    a = quad[0];     // a = A
    b = 2 * quad[2]; // b =     2*B
    c = quad[4];     // c =             C
    b -= c;          // b =     2*B -   C
    a -= b;          // a = A - 2*B +   C
    b -= c;          // b =     2*B - 2*C
}

int quadraticRoots(double A, double B, double C, double t[2]);
