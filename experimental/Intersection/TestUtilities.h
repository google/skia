#include "DataTypes.h"

bool controls_inside(const Cubic& );
void find_tight_bounds(const Cubic& , _Rect& );
void quad_to_cubic(const Quadratic& , Cubic& );
void xy_at_t(const Cubic& , double t, double& x, double& y);
void xy_at_t(const _Line& , double t, double& x, double& y);
void xy_at_t(const Quadratic& , double t, double& x, double& y);
