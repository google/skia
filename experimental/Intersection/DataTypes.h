#ifndef __DataTypes_h__
#define __DataTypes_h__

extern const double PointEpsilon;
extern const double SquaredEpsilon;

inline bool approximately_equal(double x, double y) {
    return fabs(x - y) < PointEpsilon;
}

inline bool approximately_equal_squared(double x, double y) {
    return fabs(x - y) < SquaredEpsilon;
}

inline bool approximately_greater(double x, double y) {
    return x > y - PointEpsilon;
}

inline bool approximately_lesser(double x, double y) {
    return x < y + PointEpsilon;
}

inline bool approximately_zero(double x) {
    return fabs(x) < PointEpsilon;
}

inline bool approximately_zero_squared(double x) {
    return fabs(x) < SquaredEpsilon;
}

inline bool approximately_negative(double x) {
    return x < PointEpsilon;
}

struct _Point {
    double x;
    double y;

    void operator-=(const _Point& v) {
        x -= v.x;
        y -= v.y;
    }

    friend bool operator==(const _Point& a, const _Point& b) {
        return a.x == b.x && a.y == b.y;
    }

    friend bool operator!=(const _Point& a, const _Point& b) {
        return a.x!= b.x || a.y != b.y;
    }
    
    bool approximatelyEqual(const _Point& a) const {
        return approximately_equal(a.y, y) && approximately_equal(a.x, x);
    }

};

typedef _Point _Line[2];
typedef _Point Quadratic[3];
typedef _Point Cubic[4];

struct _Rect {
    double left;
    double top;
    double right;
    double bottom;
    
    void add(const _Point& pt) {
        if (left > pt.x) {
            left = pt.x;
        }
        if (top > pt.y) {
            top = pt.y;
        }
        if (right < pt.x) {
            right = pt.x;
        }
        if (bottom < pt.y) {
            bottom = pt.y;
        }
    }
    
    void set(const _Point& pt) {
        left = right = pt.x;
        top = bottom = pt.y;
    }
    
    void setBounds(const _Line& line) {
        set(line[0]);
        add(line[1]);
    }
    
    void setBounds(const Cubic& );
    void setBounds(const Quadratic& );
    void setRawBounds(const Cubic& );
    void setRawBounds(const Quadratic& );
};

struct CubicPair {
    const Cubic& first() const { return (const Cubic&) pts[0]; }
    const Cubic& second() const { return (const Cubic&) pts[3]; }
    _Point pts[7];
};

struct QuadraticPair {
    const Quadratic& first() const { return (const Quadratic&) pts[0]; }
    const Quadratic& second() const { return (const Quadratic&) pts[2]; }
    _Point pts[5];
};

enum x_at_flags {
    kFindTopMin = 1,
    kFindTopMax = 2,
    kFindBottomMin = 4,
    kFindBottomMax = 8
};

bool rotate(const Cubic& cubic, int zero, int index, Cubic& rotPath);
double t_at(const _Line&, const _Point& );
void x_at(const _Point& p1, const _Point& p2, double minY, double maxY,
        int flags, double& tMin, double& tMax);
void xy_at_t(const Cubic& , double t, double& x, double& y);
void xy_at_t(const _Line& , double t, double& x, double& y);
void xy_at_t(const Quadratic& , double t, double& x, double& y);

#endif // __DataTypes_h__
