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

bool AlmostEqualUlps(float A, float B, int maxUlpsDiff);
int UlpsDiff(float A, float B);
int FloatAsInt(float A);

#define USE_EPSILON 0

#if USE_EPSILON
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
#else
extern const int UlpsEpsilon;

#if defined(IN_TEST)
// FIXME: move to test-only header
const double PointEpsilon = 0.000001;
const double SquaredEpsilon = PointEpsilon * PointEpsilon;
#endif

inline bool approximately_zero(double x) {

    return fabs(x) < FLT_EPSILON;
}

inline bool approximately_zero(float x) {

    return fabs(x) < FLT_EPSILON;
}

inline bool approximately_equal(double x, double y) {
    if (approximately_zero(x - y)) {
        return true;
    }
    return AlmostEqualUlps((float) x, (float) y, UlpsEpsilon);
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

inline bool approximately_zero_squared(double x) {
    return approximately_zero(x);
}

inline bool approximately_greater_than_one(double x) {
    return x > 1 - FLT_EPSILON;
}

inline bool approximately_less_than_zero(double x) {
    return x < FLT_EPSILON;
}

inline bool approximately_negative(double x) {
    return x < FLT_EPSILON;
}

inline bool approximately_one_or_less(double x) {
    return x < 1 + FLT_EPSILON;
}

inline bool approximately_positive(double x) {
    return x > -FLT_EPSILON;
}

inline bool approximately_zero_or_more(double x) {
    return x > -FLT_EPSILON;
}


#endif

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

#endif // __DataTypes_h__
