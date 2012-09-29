#include "DataTypes.h"

class QuadImplicitForm {
public:
    QuadImplicitForm(const Quadratic& q);
    bool implicit_match(const QuadImplicitForm& two) const;

    double x2() const { return p[xx_coeff]; }
    double xy() const { return p[xy_coeff]; }
    double y2() const { return p[yy_coeff]; }
    double x() const { return p[x_coeff]; }
    double y() const { return p[y_coeff]; }
    double c() const { return p[c_coeff]; }

private:
    enum Coeffs {
        xx_coeff,
        xy_coeff,
        yy_coeff,
        x_coeff,
        y_coeff,
        c_coeff,
        coeff_count
    };

    double p[coeff_count];
};
