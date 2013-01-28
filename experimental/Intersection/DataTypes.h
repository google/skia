/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef __DataTypes_h__
#define __DataTypes_h__

#include <assert.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>

extern bool AlmostEqualUlps(float A, float B);
inline bool AlmostEqualUlps(double A, double B) { return AlmostEqualUlps((float) A, (float) B); }

// FIXME: delete
int UlpsDiff(float A, float B);

// FLT_EPSILON == 1.19209290E-07 == 1 / (2 ^ 23)
const double FLT_EPSILON_CUBED = FLT_EPSILON * FLT_EPSILON * FLT_EPSILON;
const double FLT_EPSILON_SQUARED = FLT_EPSILON * FLT_EPSILON;
const double FLT_EPSILON_SQRT = sqrt(FLT_EPSILON);
const double FLT_EPSILON_INVERSE = 1 / FLT_EPSILON;

inline bool approximately_zero(double x) {

    return fabs(x) < FLT_EPSILON;
}

inline bool precisely_zero(double x) {

    return fabs(x) < DBL_EPSILON;
}

inline bool approximately_zero(float x) {

    return fabs(x) < FLT_EPSILON;
}

inline bool approximately_zero_cubed(double x) {
    return fabs(x) < FLT_EPSILON_CUBED;
}

inline bool approximately_zero_squared(double x) {
    return fabs(x) < FLT_EPSILON_SQUARED;
}

inline bool approximately_zero_sqrt(double x) {
    return fabs(x) < FLT_EPSILON_SQRT;
}

inline bool approximately_zero_inverse(double x) {
    return fabs(x) > FLT_EPSILON_INVERSE;
}

// Use this for comparing Ts in the range of 0 to 1. For general numbers (larger and smaller) use
// AlmostEqualUlps instead.
inline bool approximately_equal(double x, double y) {
#if 1
    return approximately_zero(x - y);
#else
// see http://visualstudiomagazine.com/blogs/tool-tracker/2011/11/compare-floating-point-numbers.aspx
// this allows very small (e.g. degenerate) values to compare unequally, but in this case,
// AlmostEqualUlps should be used instead.
    if (x == y) {
        return true;
    }
    double absY = fabs(y);
    if (x == 0) {
        return absY < FLT_EPSILON;
    }
    double absX = fabs(x);
    if (y == 0) {
        return absX < FLT_EPSILON;
    }
    return fabs(x - y) < (absX > absY ? absX : absY) * FLT_EPSILON;
#endif
}

inline bool approximately_equal_squared(double x, double y) {
    return approximately_equal(x, y);
}

inline bool approximately_greater(double x, double y) {
    return approximately_equal(x, y) ? false : x > y;
}

inline bool approximately_lesser(double x, double y) {
    return approximately_equal(x, y) ? false : x < y;
}

inline double approximately_pin(double x) {
    return approximately_zero(x) ? 0 : x;
}

inline float approximately_pin(float x) {
    return approximately_zero(x) ? 0 : x;
}

inline bool approximately_greater_than_one(double x) {
    return x > 1 - FLT_EPSILON;
}

inline bool precisely_greater_than_one(double x) {
    return x > 1 - DBL_EPSILON;
}

inline bool approximately_less_than_zero(double x) {
    return x < FLT_EPSILON;
}

inline bool precisely_less_than_zero(double x) {
    return x < DBL_EPSILON;
}

inline bool approximately_negative(double x) {
    return x < FLT_EPSILON;
}

inline bool precisely_negative(double x) {
    return x < DBL_EPSILON;
}

inline bool approximately_one_or_less(double x) {
    return x < 1 + FLT_EPSILON;
}

inline bool approximately_positive(double x) {
    return x > -FLT_EPSILON;
}

inline bool approximately_positive_squared(double x) {
    return x > -(FLT_EPSILON_SQUARED);
}

inline bool approximately_zero_or_more(double x) {
    return x > -FLT_EPSILON;
}

inline bool approximately_between(double a, double b, double c) {
    assert(a <= c);
    return a <= c ? approximately_negative(a - b) && approximately_negative(b - c)
            : approximately_negative(b - a) && approximately_negative(c - b);
}

// returns true if (a <= b <= c) || (a >= b >= c)
inline bool between(double a, double b, double c) {
    assert(((a <= b && b <= c) || (a >= b && b >= c)) == ((a - b) * (c - b) <= 0));
    return (a - b) * (c - b) <= 0;
}

struct _Point {
    double x;
    double y;

    friend _Point operator-(const _Point& a, const _Point& b) {
        _Point v = {a.x - b.x, a.y - b.y};
        return v;
    }

    void operator+=(const _Point& v) {
        x += v.x;
        y += v.y;
    }

    void operator-=(const _Point& v) {
        x -= v.x;
        y -= v.y;
    }

    void operator/=(const double s) {
        x /= s;
        y /= s;
    }

    void operator*=(const double s) {
        x *= s;
        y *= s;
    }

    friend bool operator==(const _Point& a, const _Point& b) {
        return a.x == b.x && a.y == b.y;
    }

    friend bool operator!=(const _Point& a, const _Point& b) {
        return a.x != b.x || a.y != b.y;
    }

    // note: this can not be implemented with
    // return approximately_equal(a.y, y) && approximately_equal(a.x, x);
    // because that will not take the magnitude of the values
    bool approximatelyEqual(const _Point& a) const {
        return AlmostEqualUlps((float) x, (float) a.x)
                && AlmostEqualUlps((float) y, (float) a.y);
    }

    double cross(const _Point& a) const {
        return x * a.y - y * a.x;
    }

    double dot(const _Point& a) const {
        return x * a.x + y * a.y;
    }

    double length() const {
        return sqrt(lengthSquared());
    }

    double lengthSquared() const {
        return x * x + y * y;
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

    // FIXME: used by debugging only ?
    bool contains(const _Point& pt) const {
        return approximately_between(left, pt.x, right)
                && approximately_between(top, pt.y, bottom);
    }

    bool intersects(_Rect& r) const {
        assert(left <= right);
        assert(top <= bottom);
        assert(r.left <= r.right);
        assert(r.top <= r.bottom);
        return r.left <= right && left <= r.right && r.top <= bottom && top <= r.bottom;
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

// FIXME: move these into SkTypes.h
template <typename T> inline T SkTMax(T a, T b) {
    if (a < b)
        a = b;
    return a;
}

template <typename T> inline T SkTMin(T a, T b) {
    if (a > b)
        a = b;
    return a;
}

#endif // __DataTypes_h__
