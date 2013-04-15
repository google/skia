/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPathOpsPoint_DEFINED
#define SkPathOpsPoint_DEFINED

#include "SkPathOpsTypes.h"
#include "SkPoint.h"

struct SkDVector {
    double fX, fY;

    friend SkDPoint operator+(const SkDPoint& a, const SkDVector& b);

    void operator+=(const SkDVector& v) {
        fX += v.fX;
        fY += v.fY;
    }

    void operator-=(const SkDVector& v) {
        fX -= v.fX;
        fY -= v.fY;
    }

    void operator/=(const double s) {
        fX /= s;
        fY /= s;
    }

    void operator*=(const double s) {
        fX *= s;
        fY *= s;
    }

    SkVector asSkVector() const {
        SkVector v = {SkDoubleToScalar(fX), SkDoubleToScalar(fY)};
        return v;
    }

    double cross(const SkDVector& a) const {
        return fX * a.fY - fY * a.fX;
    }

    double dot(const SkDVector& a) const {
        return fX * a.fX + fY * a.fY;
    }

    double length() const {
        return sqrt(lengthSquared());
    }

    double lengthSquared() const {
        return fX * fX + fY * fY;
    }
};

struct SkDPoint {
    double fX;
    double fY;

    void set(const SkPoint& pt) {
        fX = pt.fX;
        fY = pt.fY;
    }

    friend SkDVector operator-(const SkDPoint& a, const SkDPoint& b);

    friend bool operator==(const SkDPoint& a, const SkDPoint& b) {
        return a.fX == b.fX && a.fY == b.fY;
    }

    friend bool operator!=(const SkDPoint& a, const SkDPoint& b) {
        return a.fX != b.fX || a.fY != b.fY;
    }

    void operator=(const SkPoint& pt) {
        fX = pt.fX;
        fY = pt.fY;
    }


    void operator+=(const SkDVector& v) {
        fX += v.fX;
        fY += v.fY;
    }

    void operator-=(const SkDVector& v) {
        fX -= v.fX;
        fY -= v.fY;
    }

    // note: this can not be implemented with
    // return approximately_equal(a.fY, fY) && approximately_equal(a.fX, fX);
    // because that will not take the magnitude of the values
    bool approximatelyEqual(const SkDPoint& a) const {
        double denom = SkTMax<double>(fabs(fX), SkTMax<double>(fabs(fY),
                SkTMax<double>(fabs(a.fX), fabs(a.fY))));
        if (denom == 0) {
            return true;
        }
        double inv = 1 / denom;
        return approximately_equal(fX * inv, a.fX * inv)
                && approximately_equal(fY * inv, a.fY * inv);
    }

    bool approximatelyEqual(const SkPoint& a) const {
        double denom = SkTMax<double>(fabs(fX), SkTMax<double>(fabs(fY),
                SkScalarToDouble(SkTMax<SkScalar>(fabsf(a.fX), fabsf(a.fY)))));
        if (denom == 0) {
            return true;
        }
        double inv = 1 / denom;
        return approximately_equal(fX * inv, a.fX * inv)
                && approximately_equal(fY * inv, a.fY * inv);
    }

    bool approximatelyEqualHalf(const SkDPoint& a) const {
        double denom = SkTMax<double>(fabs(fX), SkTMax<double>(fabs(fY),
                SkTMax<double>(fabs(a.fX), fabs(a.fY))));
        if (denom == 0) {
            return true;
        }
        double inv = 1 / denom;
        return approximately_equal_half(fX * inv, a.fX * inv)
                && approximately_equal_half(fY * inv, a.fY * inv);
    }

    bool approximatelyZero() const {
        return approximately_zero(fX) && approximately_zero(fY);
    }

    SkPoint asSkPoint() const {
        SkPoint pt = {SkDoubleToScalar(fX), SkDoubleToScalar(fY)};
        return pt;
    }

    double distance(const SkDPoint& a) const {
        SkDVector temp = *this - a;
        return temp.length();
    }

    double distanceSquared(const SkDPoint& a) const {
        SkDVector temp = *this - a;
        return temp.lengthSquared();
    }

    double moreRoughlyEqual(const SkDPoint& a) const {
        return more_roughly_equal(a.fY, fY) && more_roughly_equal(a.fX, fX);
    }

    double roughlyEqual(const SkDPoint& a) const {
        return roughly_equal(a.fY, fY) && roughly_equal(a.fX, fX);
    }
};

#endif
