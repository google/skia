/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkM4.h"
#include <type_traits>
#include <utility>

static inline bool eq4(const float* SK_RESTRICT a,
                      const float* SK_RESTRICT b) {
    return (a[0] == b[0]) & (a[1] == b[1]) & (a[2] == b[2]) & (a[3] == b[3]);
}

bool SkM4::operator==(const SkM4& other) const {
    if (this == &other) {
        return true;
    }

    const float* SK_RESTRICT a = &fMat[0][0];
    const float* SK_RESTRICT b = &other.fMat[0][0];
    // to reduce branch instructions, we compare 4 at a time.
    // see bench/Matrix44Bench.cpp for test.
    if (!eq4(&a[0], &b[0])) {
        return false;
    }
    if (!eq4(&a[4], &b[4])) {
        return false;
    }
    if (!eq4(&a[8], &b[8])) {
        return false;
    }
    return eq4(&a[12], &b[12]);
}

const SkM4& SkM4::I() {
    static constexpr SkM4 gIdentity4;
    return gIdentity4;
}

SkM4& SkM4::preTranslate(float dx, float dy, float dz) {
    for (int i = 0; i < 4; ++i) {
        fMat[3][i] = fMat[0][i] * dx + fMat[1][i] * dy + fMat[2][i] * dz + fMat[3][i];
    }
    return *this;
}

SkM4& SkM4::postTranslate(float dx, float dy, float dz) {
    for (int i = 0; i < 4; ++i) {
        fMat[i][0] += fMat[i][3] * dx;
        fMat[i][1] += fMat[i][3] * dy;
        fMat[i][2] += fMat[i][3] * dz;
    }
    return *this;
}

SkM4& SkM4::preScale(float sx, float sy, float sz) {
    for (int i = 0; i < 4; i++) {
        fMat[0][i] *= sx;
        fMat[1][i] *= sy;
        fMat[2][i] *= sz;
    }
    return *this;
}

SkM4& SkM4::postScale(float sx, float sy, float sz) {
    for (int i = 0; i < 4; i++) {
        fMat[i][0] *= sx;
        fMat[i][1] *= sy;
        fMat[i][2] *= sz;
    }
    return *this;
}

SkM4& SkM4::setConcat(const SkM4& a, const SkM4& b) {
    bool useStorage = (this == &a || this == &b);
    float storage[16];
    float* result = useStorage ? storage : &fMat[0][0];

    for (int j = 0; j < 4; j++) {
        for (int i = 0; i < 4; i++) {
            float value = 0;
            for (int k = 0; k < 4; k++) {
                value += a.fMat[k][i] * b.fMat[j][k];
            }
            *result++ = value;
        }
    }

    if (useStorage) {
        memcpy(fMat, storage, sizeof(storage));
    }
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

/** We always perform the calculation in doubles, to avoid prematurely losing
    precision along the way. This relies on the compiler automatically
    promoting our SkMScalar values to double (if needed).
 */
double SkM4::determinant() const {
    double a00 = fMat[0][0];
    double a01 = fMat[0][1];
    double a02 = fMat[0][2];
    double a03 = fMat[0][3];
    double a10 = fMat[1][0];
    double a11 = fMat[1][1];
    double a12 = fMat[1][2];
    double a13 = fMat[1][3];
    double a20 = fMat[2][0];
    double a21 = fMat[2][1];
    double a22 = fMat[2][2];
    double a23 = fMat[2][3];
    double a30 = fMat[3][0];
    double a31 = fMat[3][1];
    double a32 = fMat[3][2];
    double a33 = fMat[3][3];

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

static bool is_matrix_finite(const SkM4& matrix) {
    float accumulator = 0;
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            accumulator *= matrix.get(row, col);
        }
    }
    return accumulator == 0;
}

bool SkM4::invert(SkM4* storage) const {
    SkM4 tmp;
    // Use storage if it's available and distinct from this matrix.
    SkM4* inverse = (storage && storage != this) ? storage : &tmp;

    double a00 = fMat[0][0];
    double a01 = fMat[0][1];
    double a02 = fMat[0][2];
    double a03 = fMat[0][3];
    double a10 = fMat[1][0];
    double a11 = fMat[1][1];
    double a12 = fMat[1][2];
    double a13 = fMat[1][3];
    double a20 = fMat[2][0];
    double a21 = fMat[2][1];
    double a22 = fMat[2][2];
    double a23 = fMat[2][3];
    double a30 = fMat[3][0];
    double a31 = fMat[3][1];
    double a32 = fMat[3][2];
    double a33 = fMat[3][3];

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
    if (!sk_float_isfinite(sk_double_to_float(invdet))) {
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

    inverse->fMat[0][0] = float(a11 * b11 - a12 * b10 + a13 * b09);
    inverse->fMat[0][1] = float(a02 * b10 - a01 * b11 - a03 * b09);
    inverse->fMat[0][2] = float(a31 * b05 - a32 * b04 + a33 * b03);
    inverse->fMat[0][3] = float(a22 * b04 - a21 * b05 - a23 * b03);
    inverse->fMat[1][0] = float(a12 * b08 - a10 * b11 - a13 * b07);
    inverse->fMat[1][1] = float(a00 * b11 - a02 * b08 + a03 * b07);
    inverse->fMat[1][2] = float(a32 * b02 - a30 * b05 - a33 * b01);
    inverse->fMat[1][3] = float(a20 * b05 - a22 * b02 + a23 * b01);
    inverse->fMat[2][0] = float(a10 * b10 - a11 * b08 + a13 * b06);
    inverse->fMat[2][1] = float(a01 * b08 - a00 * b10 - a03 * b06);
    inverse->fMat[2][2] = float(a30 * b04 - a31 * b02 + a33 * b00);
    inverse->fMat[2][3] = float(a21 * b02 - a20 * b04 - a23 * b00);
    inverse->fMat[3][0] = float(a11 * b07 - a10 * b09 - a12 * b06);
    inverse->fMat[3][1] = float(a00 * b09 - a01 * b07 + a02 * b06);
    inverse->fMat[3][2] = float(a31 * b01 - a30 * b03 - a32 * b00);
    inverse->fMat[3][3] = float(a20 * b03 - a21 * b01 + a22 * b00);
    if (!is_matrix_finite(*inverse)) {
        return false;
    }
    if (storage && inverse != storage) {
        *storage = *inverse;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void SkM4::dump() const {
    static const char* format = "|%g %g %g %g|\n"
                                "|%g %g %g %g|\n"
                                "|%g %g %g %g|\n"
                                "|%g %g %g %g|\n";
    SkDebugf(format,
             fMat[0][0], fMat[1][0], fMat[2][0], fMat[3][0],
             fMat[0][1], fMat[1][1], fMat[2][1], fMat[3][1],
             fMat[0][2], fMat[1][2], fMat[2][2], fMat[3][2],
             fMat[0][3], fMat[1][3], fMat[2][3], fMat[3][3]);
}

///////////////////////////////////////////////////////////////////////////////

SkM4::SkM4(const SkMatrix& src) {
    this->operator=(src);
}

SkM4& SkM4::operator=(const SkMatrix& src) {
    fMat[0][0] = src[SkMatrix::kMScaleX];
    fMat[1][0] = src[SkMatrix::kMSkewX];
    fMat[2][0] = 0;
    fMat[3][0] = src[SkMatrix::kMTransX];
    fMat[0][1] = src[SkMatrix::kMSkewY];
    fMat[1][1] = src[SkMatrix::kMScaleY];
    fMat[2][1] = 0;
    fMat[3][1] = src[SkMatrix::kMTransY];
    fMat[0][2] = 0;
    fMat[1][2] = 0;
    fMat[2][2] = 1;
    fMat[3][2] = 0;
    fMat[0][3] = src[SkMatrix::kMPersp0];
    fMat[1][3] = src[SkMatrix::kMPersp1];
    fMat[2][3] = 0;
    fMat[3][3] = src[SkMatrix::kMPersp2];
    return *this;
}

SkM4::operator SkMatrix() const {
    SkMatrix dst;

    dst[SkMatrix::kMScaleX] = fMat[0][0];
    dst[SkMatrix::kMSkewX]  = fMat[1][0];
    dst[SkMatrix::kMTransX] = fMat[3][0];

    dst[SkMatrix::kMSkewY]  = fMat[0][1];
    dst[SkMatrix::kMScaleY] = fMat[1][1];
    dst[SkMatrix::kMTransY] = fMat[3][1];

    dst[SkMatrix::kMPersp0] = fMat[0][3];
    dst[SkMatrix::kMPersp1] = fMat[1][3];
    dst[SkMatrix::kMPersp2] = fMat[3][3];

    return dst;
}

//////////////////////////////////////////////////////////////////////////////

SkM4& SkM4::preTranslate(SkScalar x, SkScalar y) {
    SkM4 tmp;
    tmp.setTranslate(x, y, 0);
    return this->preConcat(tmp);
}

SkM4& SkM4::preScale(SkScalar sx, SkScalar sy) {
    SkM4 tmp;
    tmp.setScale(sx, sy, 1);
    return this->preConcat(tmp);
}

SkM4& SkM4::preConcat(const SkMatrix& m) {
    SkM4 tmp(m);
    return this->preConcat(tmp);
}

bool SkM4::rectStaysRect() const {
    return false;
}

bool SkM4::mapRect(SkRect* dst, const SkRect& src) const {
    return ((const SkMatrix&)*this).mapRect(dst, src);
}

