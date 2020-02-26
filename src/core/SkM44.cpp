/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkM44.h"
#include "include/core/SkMatrix.h"
#include "include/private/SkVx.h"

typedef skvx::Vec<4, float> sk4f;

bool SkM44::operator==(const SkM44& other) const {
    if (this == &other) {
        return true;
    }

    sk4f a0 = sk4f::Load(fMat +  0);
    sk4f a1 = sk4f::Load(fMat +  4);
    sk4f a2 = sk4f::Load(fMat +  8);
    sk4f a3 = sk4f::Load(fMat + 12);

    sk4f b0 = sk4f::Load(other.fMat +  0);
    sk4f b1 = sk4f::Load(other.fMat +  4);
    sk4f b2 = sk4f::Load(other.fMat +  8);
    sk4f b3 = sk4f::Load(other.fMat + 12);

    auto eq = (a0 == b0) & (a1 == b1) & (a2 == b2) & (a3 == b3);
    return (eq[0] & eq[1] & eq[2] & eq[3]) == ~0;
}

static void transpose_arrays(SkScalar dst[], const SkScalar src[]) {
    dst[0]  = src[0]; dst[1]  = src[4]; dst[2]  = src[8];  dst[3]  = src[12];
    dst[4]  = src[1]; dst[5]  = src[5]; dst[6]  = src[9];  dst[7]  = src[13];
    dst[8]  = src[2]; dst[9]  = src[6]; dst[10] = src[10]; dst[11] = src[14];
    dst[12] = src[3]; dst[13] = src[7]; dst[14] = src[11]; dst[15] = src[15];
}

void SkM44::getRowMajor(SkScalar v[]) const {
    transpose_arrays(v, fMat);
}

SkM44& SkM44::setRowMajor(const SkScalar v[]) {
    transpose_arrays(fMat, v);
    return *this;
}

SkM44& SkM44::setConcat16(const SkM44& a, const SkScalar b[16]) {
    sk4f c0 = sk4f::Load(a.fMat +  0);
    sk4f c1 = sk4f::Load(a.fMat +  4);
    sk4f c2 = sk4f::Load(a.fMat +  8);
    sk4f c3 = sk4f::Load(a.fMat + 12);

    auto compute = [&](sk4f r) {
        return skvx::mad(c0, r[0], skvx::mad(c1, r[1], skvx::mad(c2, r[2], c3 * r[3])));
    };

    sk4f m0 = compute(sk4f::Load(b +  0));
    sk4f m1 = compute(sk4f::Load(b +  4));
    sk4f m2 = compute(sk4f::Load(b +  8));
    sk4f m3 = compute(sk4f::Load(b + 12));

    m0.store(fMat +  0);
    m1.store(fMat +  4);
    m2.store(fMat +  8);
    m3.store(fMat + 12);
    return *this;
}

SkM44& SkM44::preConcat(const SkMatrix& b) {
    sk4f c0 = sk4f::Load(fMat +  0);
    sk4f c1 = sk4f::Load(fMat +  4);
    sk4f c3 = sk4f::Load(fMat + 12);

    auto compute = [&](float r0, float r1, float r3) {
        return skvx::mad(c0, r0, skvx::mad(c1, r1, c3 * r3));
    };

    sk4f m0 = compute(b[0], b[3], b[6]);
    sk4f m1 = compute(b[1], b[4], b[7]);
    sk4f m3 = compute(b[2], b[5], b[8]);

    m0.store(fMat +  0);
    m1.store(fMat +  4);
    m3.store(fMat + 12);
    return *this;
}

SkM44& SkM44::preTranslate(SkScalar x, SkScalar y) {
    sk4f c0 = sk4f::Load(fMat +  0);
    sk4f c1 = sk4f::Load(fMat +  4);
    sk4f c3 = sk4f::Load(fMat + 12);

    // only need to update the last column
    skvx::mad(c0, x, skvx::mad(c1, y, c3)).store(fMat + 12);
    return *this;
}

SkM44& SkM44::preScale(SkScalar x, SkScalar y) {
    sk4f c0 = sk4f::Load(fMat +  0);
    sk4f c1 = sk4f::Load(fMat +  4);

    (c0 * x).store(fMat + 0);
    (c1 * y).store(fMat + 4);
    return *this;
}

SkV4 SkM44::map(float x, float y, float z, float w) const {
    sk4f c0 = sk4f::Load(fMat +  0);
    sk4f c1 = sk4f::Load(fMat +  4);
    sk4f c2 = sk4f::Load(fMat +  8);
    sk4f c3 = sk4f::Load(fMat + 12);

    SkV4 v;
    skvx::mad(c0, x, skvx::mad(c1, y, skvx::mad(c2, z, c3 * w))).store(&v.x);
    return v;
}

///////////////////////////////////////////////////////////////////////////////

/** We always perform the calculation in doubles, to avoid prematurely losing
    precision along the way. This relies on the compiler automatically
    promoting our SkScalar values to double (if needed).
 */
double SkM44::determinant() const {
    double a00 = fMat[0];
    double a01 = fMat[1];
    double a02 = fMat[2];
    double a03 = fMat[3];
    double a10 = fMat[4];
    double a11 = fMat[5];
    double a12 = fMat[6];
    double a13 = fMat[7];
    double a20 = fMat[8];
    double a21 = fMat[9];
    double a22 = fMat[10];
    double a23 = fMat[11];
    double a30 = fMat[12];
    double a31 = fMat[13];
    double a32 = fMat[14];
    double a33 = fMat[15];

    double b00 = a00 * a11 - a01 * a10;
    double b01 = a00 * a12 - a02 * a10;
    double b02 = a00 * a13 - a03 * a10;
    double b03 = a01 * a12 - a02 * a11;
    double b04 = a01 * a13 - a03 * a11;
    double b05 = a02 * a13 - a03 * a12;
    double b06 = a20 * a31 - a21 * a30;
    double b07 = a20 * a32 - a22 * a30;
    double b08 = a20 * a33 - a23 * a30;
    double b09 = a21 * a32 - a22 * a31;
    double b10 = a21 * a33 - a23 * a31;
    double b11 = a22 * a33 - a23 * a32;

    // Calculate the determinant
    return b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;
}

///////////////////////////////////////////////////////////////////////////////

bool SkM44::invert(SkM44* inverse) const {
    double a00 = fMat[0];
    double a01 = fMat[1];
    double a02 = fMat[2];
    double a03 = fMat[3];
    double a10 = fMat[4];
    double a11 = fMat[5];
    double a12 = fMat[6];
    double a13 = fMat[7];
    double a20 = fMat[8];
    double a21 = fMat[9];
    double a22 = fMat[10];
    double a23 = fMat[11];
    double a30 = fMat[12];
    double a31 = fMat[13];
    double a32 = fMat[14];
    double a33 = fMat[15];

    double b00 = a00 * a11 - a01 * a10;
    double b01 = a00 * a12 - a02 * a10;
    double b02 = a00 * a13 - a03 * a10;
    double b03 = a01 * a12 - a02 * a11;
    double b04 = a01 * a13 - a03 * a11;
    double b05 = a02 * a13 - a03 * a12;
    double b06 = a20 * a31 - a21 * a30;
    double b07 = a20 * a32 - a22 * a30;
    double b08 = a20 * a33 - a23 * a30;
    double b09 = a21 * a32 - a22 * a31;
    double b10 = a21 * a33 - a23 * a31;
    double b11 = a22 * a33 - a23 * a32;

    // Calculate the determinant
    double det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;

    double invdet = sk_ieee_double_divide(1.0, det);
    // If det is zero, we want to return false. However, we also want to return false
    // if 1/det overflows to infinity (i.e. det is denormalized). Both of these are
    // handled by checking that 1/det is finite.
    if (!SkScalarIsFinite(SkScalar(invdet))) {
        return false;
    }

    b00 *= invdet;
    b01 *= invdet;
    b02 *= invdet;
    b03 *= invdet;
    b04 *= invdet;
    b05 *= invdet;
    b06 *= invdet;
    b07 *= invdet;
    b08 *= invdet;
    b09 *= invdet;
    b10 *= invdet;
    b11 *= invdet;

    SkScalar tmp[16] = {
        SkDoubleToScalar(a11 * b11 - a12 * b10 + a13 * b09),
        SkDoubleToScalar(a02 * b10 - a01 * b11 - a03 * b09),
        SkDoubleToScalar(a31 * b05 - a32 * b04 + a33 * b03),
        SkDoubleToScalar(a22 * b04 - a21 * b05 - a23 * b03),
        SkDoubleToScalar(a12 * b08 - a10 * b11 - a13 * b07),
        SkDoubleToScalar(a00 * b11 - a02 * b08 + a03 * b07),
        SkDoubleToScalar(a32 * b02 - a30 * b05 - a33 * b01),
        SkDoubleToScalar(a20 * b05 - a22 * b02 + a23 * b01),
        SkDoubleToScalar(a10 * b10 - a11 * b08 + a13 * b06),
        SkDoubleToScalar(a01 * b08 - a00 * b10 - a03 * b06),
        SkDoubleToScalar(a30 * b04 - a31 * b02 + a33 * b00),
        SkDoubleToScalar(a21 * b02 - a20 * b04 - a23 * b00),
        SkDoubleToScalar(a11 * b07 - a10 * b09 - a12 * b06),
        SkDoubleToScalar(a00 * b09 - a01 * b07 + a02 * b06),
        SkDoubleToScalar(a31 * b01 - a30 * b03 - a32 * b00),
        SkDoubleToScalar(a20 * b03 - a21 * b01 + a22 * b00),
    };
    if (!SkScalarsAreFinite(tmp, 16)) {
        return false;
    }
    memcpy(inverse->fMat, tmp, sizeof(tmp));
    return true;
}

SkM44 SkM44::transpose() const {
    SkM44 trans(SkM44::kUninitialized_Constructor);
    transpose_arrays(trans.fMat, fMat);
    return trans;
}

SkM44& SkM44::setRotateUnitSinCos(SkV3 axis, SkScalar sinAngle, SkScalar cosAngle) {
    // Taken from "Essential Mathematics for Games and Interactive Applications"
    //             James M. Van Verth and Lars M. Bishop -- third edition
    SkScalar x = axis.x;
    SkScalar y = axis.y;
    SkScalar z = axis.z;
    SkScalar c = cosAngle;
    SkScalar s = sinAngle;
    SkScalar t = 1 - c;

    return this->set44(t*x*x + c,   t*x*y - s*z, t*x*z + s*y, 0,
                       t*x*y + s*z, t*y*y + c,   t*y*z - s*x, 0,
                       t*x*z - s*y, t*y*z + s*x, t*z*z + c,   0,
                       0,           0,           0,           1);
}

SkM44& SkM44::setRotate(SkV3 axis, SkScalar radians) {
    SkScalar len = axis.length();
    if (len > 0 && SkScalarIsFinite(len)) {
        this->setRotateUnit(axis * (SK_Scalar1 / len), radians);
    } else {
        this->setIdentity();
    }
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

void SkM44::dump() const {
    static const char* format = "|%g %g %g %g|\n"
                                "|%g %g %g %g|\n"
                                "|%g %g %g %g|\n"
                                "|%g %g %g %g|\n";
    SkDebugf(format,
             fMat[0], fMat[4], fMat[8],  fMat[12],
             fMat[1], fMat[5], fMat[9],  fMat[13],
             fMat[2], fMat[6], fMat[10], fMat[14],
             fMat[3], fMat[7], fMat[11], fMat[15]);
}

static SkV3 normalize(SkV3 v) { return v * (1.0f / v.length()); }

static SkV4 v4(SkV3 v, SkScalar w) { return {v.x, v.y, v.z, w}; }

SkM44 Sk3LookAt(const SkV3& eye, const SkV3& center, const SkV3& up) {
    SkV3 f = normalize(center - eye);
    SkV3 u = normalize(up);
    SkV3 s = normalize(f.cross(u));

    SkM44 m(SkM44::kUninitialized_Constructor);
    if (!SkM44::Cols(v4(s, 0), v4(s.cross(f), 0), v4(-f, 0), v4(eye, 1)).invert(&m)) {
        m.setIdentity();
    }
    return m;
}

SkM44 Sk3Perspective(float near, float far, float angle) {
    SkASSERT(far > near);

    float denomInv = sk_ieee_float_divide(1, far - near);
    float halfAngle = angle * 0.5f;
    float cot = sk_float_cos(halfAngle) / sk_float_sin(halfAngle);

    SkM44 m;
    m.setRC(0, 0, cot);
    m.setRC(1, 1, cot);
    m.setRC(2, 2, (far + near) * denomInv);
    m.setRC(2, 3, 2 * far * near * denomInv);
    m.setRC(3, 2, -1);
    return m;
}
