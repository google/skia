/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkMatrix44.h"
#include <type_traits>
#include <utility>

// Copying SkMatrix44 byte-wise is performance-critical to Blink. This class is
// contained in several Transform classes, which are copied multiple times
// during the rendering life cycle. See crbug.com/938563 for reference.
#if defined(SK_BUILD_FOR_WIN) || defined(SK_BUILD_FOR_MAC)
// std::is_trivially_copyable is not supported for some older clang versions,
// which (at least as of this patch) are in use for Chromecast.
static_assert(std::is_trivially_copyable<SkMatrix44>::value,
              "SkMatrix44 must be trivially copyable");
#endif

static inline bool eq4(const SkMScalar* SK_RESTRICT a,
                      const SkMScalar* SK_RESTRICT b) {
    return (a[0] == b[0]) & (a[1] == b[1]) & (a[2] == b[2]) & (a[3] == b[3]);
}

bool SkMatrix44::operator==(const SkMatrix44& other) const {
    if (this == &other) {
        return true;
    }

    if (this->isIdentity() && other.isIdentity()) {
        return true;
    }

    const SkMScalar* SK_RESTRICT a = &fMat[0][0];
    const SkMScalar* SK_RESTRICT b = &other.fMat[0][0];

#if 0
    for (int i = 0; i < 16; ++i) {
        if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
#else
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
#endif
}

///////////////////////////////////////////////////////////////////////////////
void SkMatrix44::recomputeTypeMask() {
    if (0 != perspX() || 0 != perspY() || 0 != perspZ() || 1 != fMat[3][3]) {
        fTypeMask = kTranslate_Mask | kScale_Mask | kAffine_Mask | kPerspective_Mask;
        return;
    }

    TypeMask mask = kIdentity_Mask;
    if (0 != transX() || 0 != transY() || 0 != transZ()) {
        mask |= kTranslate_Mask;
    }

    if (1 != scaleX() || 1 != scaleY() || 1 != scaleZ()) {
        mask |= kScale_Mask;
    }

    if (0 != fMat[1][0] || 0 != fMat[0][1] || 0 != fMat[0][2] ||
        0 != fMat[2][0] || 0 != fMat[1][2] || 0 != fMat[2][1]) {
            mask |= kAffine_Mask;
    }
    fTypeMask = mask;
}

///////////////////////////////////////////////////////////////////////////////

void SkMatrix44::asColMajorf(float dst[]) const {
    const SkMScalar* src = &fMat[0][0];
#ifdef SK_MSCALAR_IS_DOUBLE
    for (int i = 0; i < 16; ++i) {
        dst[i] = SkMScalarToFloat(src[i]);
    }
#elif defined SK_MSCALAR_IS_FLOAT
    memcpy(dst, src, 16 * sizeof(float));
#endif
}

void SkMatrix44::as3x4RowMajorf(float dst[]) const {
    dst[0] = fMat[0][0]; dst[1] = fMat[1][0]; dst[2]  = fMat[2][0]; dst[3]  = fMat[3][0];
    dst[4] = fMat[0][1]; dst[5] = fMat[1][1]; dst[6]  = fMat[2][1]; dst[7]  = fMat[3][1];
    dst[8] = fMat[0][2]; dst[9] = fMat[1][2]; dst[10] = fMat[2][2]; dst[11] = fMat[3][2];
}

void SkMatrix44::asColMajord(double dst[]) const {
    const SkMScalar* src = &fMat[0][0];
#ifdef SK_MSCALAR_IS_DOUBLE
    memcpy(dst, src, 16 * sizeof(double));
#elif defined SK_MSCALAR_IS_FLOAT
    for (int i = 0; i < 16; ++i) {
        dst[i] = SkMScalarToDouble(src[i]);
    }
#endif
}

void SkMatrix44::asRowMajorf(float dst[]) const {
    const SkMScalar* src = &fMat[0][0];
    for (int i = 0; i < 4; ++i) {
        dst[0] = SkMScalarToFloat(src[0]);
        dst[4] = SkMScalarToFloat(src[1]);
        dst[8] = SkMScalarToFloat(src[2]);
        dst[12] = SkMScalarToFloat(src[3]);
        src += 4;
        dst += 1;
    }
}

void SkMatrix44::asRowMajord(double dst[]) const {
    const SkMScalar* src = &fMat[0][0];
    for (int i = 0; i < 4; ++i) {
        dst[0] = SkMScalarToDouble(src[0]);
        dst[4] = SkMScalarToDouble(src[1]);
        dst[8] = SkMScalarToDouble(src[2]);
        dst[12] = SkMScalarToDouble(src[3]);
        src += 4;
        dst += 1;
    }
}

void SkMatrix44::setColMajorf(const float src[]) {
    SkMScalar* dst = &fMat[0][0];
#ifdef SK_MSCALAR_IS_DOUBLE
    for (int i = 0; i < 16; ++i) {
        dst[i] = SkMScalarToFloat(src[i]);
    }
#elif defined SK_MSCALAR_IS_FLOAT
    memcpy(dst, src, 16 * sizeof(float));
#endif

    this->recomputeTypeMask();
}

void SkMatrix44::setColMajord(const double src[]) {
    SkMScalar* dst = &fMat[0][0];
#ifdef SK_MSCALAR_IS_DOUBLE
    memcpy(dst, src, 16 * sizeof(double));
#elif defined SK_MSCALAR_IS_FLOAT
    for (int i = 0; i < 16; ++i) {
        dst[i] = SkDoubleToMScalar(src[i]);
    }
#endif

    this->recomputeTypeMask();
}

void SkMatrix44::setRowMajorf(const float src[]) {
    SkMScalar* dst = &fMat[0][0];
    for (int i = 0; i < 4; ++i) {
        dst[0] = SkMScalarToFloat(src[0]);
        dst[4] = SkMScalarToFloat(src[1]);
        dst[8] = SkMScalarToFloat(src[2]);
        dst[12] = SkMScalarToFloat(src[3]);
        src += 4;
        dst += 1;
    }
    this->recomputeTypeMask();
}

void SkMatrix44::setRowMajord(const double src[]) {
    SkMScalar* dst = &fMat[0][0];
    for (int i = 0; i < 4; ++i) {
        dst[0] = SkDoubleToMScalar(src[0]);
        dst[4] = SkDoubleToMScalar(src[1]);
        dst[8] = SkDoubleToMScalar(src[2]);
        dst[12] = SkDoubleToMScalar(src[3]);
        src += 4;
        dst += 1;
    }
    this->recomputeTypeMask();
}

///////////////////////////////////////////////////////////////////////////////

const SkMatrix44& SkMatrix44::I() {
    static constexpr SkMatrix44 gIdentity44(kIdentity_Constructor);
    return gIdentity44;
}

void SkMatrix44::setIdentity() {
    fMat[0][0] = 1;
    fMat[0][1] = 0;
    fMat[0][2] = 0;
    fMat[0][3] = 0;
    fMat[1][0] = 0;
    fMat[1][1] = 1;
    fMat[1][2] = 0;
    fMat[1][3] = 0;
    fMat[2][0] = 0;
    fMat[2][1] = 0;
    fMat[2][2] = 1;
    fMat[2][3] = 0;
    fMat[3][0] = 0;
    fMat[3][1] = 0;
    fMat[3][2] = 0;
    fMat[3][3] = 1;
    this->setTypeMask(kIdentity_Mask);
}

void SkMatrix44::set3x3(SkMScalar m_00, SkMScalar m_10, SkMScalar m_20,
                        SkMScalar m_01, SkMScalar m_11, SkMScalar m_21,
                        SkMScalar m_02, SkMScalar m_12, SkMScalar m_22) {
    fMat[0][0] = m_00; fMat[0][1] = m_10; fMat[0][2] = m_20; fMat[0][3] = 0;
    fMat[1][0] = m_01; fMat[1][1] = m_11; fMat[1][2] = m_21; fMat[1][3] = 0;
    fMat[2][0] = m_02; fMat[2][1] = m_12; fMat[2][2] = m_22; fMat[2][3] = 0;
    fMat[3][0] = 0;    fMat[3][1] = 0;    fMat[3][2] = 0;    fMat[3][3] = 1;
    this->recomputeTypeMask();
}

void SkMatrix44::set3x3RowMajorf(const float src[]) {
    fMat[0][0] = src[0]; fMat[0][1] = src[3]; fMat[0][2] = src[6]; fMat[0][3] = 0;
    fMat[1][0] = src[1]; fMat[1][1] = src[4]; fMat[1][2] = src[7]; fMat[1][3] = 0;
    fMat[2][0] = src[2]; fMat[2][1] = src[5]; fMat[2][2] = src[8]; fMat[2][3] = 0;
    fMat[3][0] = 0;      fMat[3][1] = 0;      fMat[3][2] = 0;      fMat[3][3] = 1;
    this->recomputeTypeMask();
}

void SkMatrix44::set3x4RowMajorf(const float src[]) {
    fMat[0][0] = src[0]; fMat[1][0] = src[1]; fMat[2][0] = src[2];  fMat[3][0] = src[3];
    fMat[0][1] = src[4]; fMat[1][1] = src[5]; fMat[2][1] = src[6];  fMat[3][1] = src[7];
    fMat[0][2] = src[8]; fMat[1][2] = src[9]; fMat[2][2] = src[10]; fMat[3][2] = src[11];
    fMat[0][3] = 0;      fMat[1][3] = 0;      fMat[2][3] = 0;       fMat[3][3] = 1;
    this->recomputeTypeMask();
}

void SkMatrix44::set4x4(SkMScalar m_00, SkMScalar m_10, SkMScalar m_20, SkMScalar m_30,
                SkMScalar m_01, SkMScalar m_11, SkMScalar m_21, SkMScalar m_31,
                SkMScalar m_02, SkMScalar m_12, SkMScalar m_22, SkMScalar m_32,
                SkMScalar m_03, SkMScalar m_13, SkMScalar m_23, SkMScalar m_33) {
    fMat[0][0] = m_00; fMat[0][1] = m_10; fMat[0][2] = m_20; fMat[0][3] = m_30;
    fMat[1][0] = m_01; fMat[1][1] = m_11; fMat[1][2] = m_21; fMat[1][3] = m_31;
    fMat[2][0] = m_02; fMat[2][1] = m_12; fMat[2][2] = m_22; fMat[2][3] = m_32;
    fMat[3][0] = m_03; fMat[3][1] = m_13; fMat[3][2] = m_23; fMat[3][3] = m_33;
    this->recomputeTypeMask();
}


///////////////////////////////////////////////////////////////////////////////

SkMatrix44& SkMatrix44::setTranslate(SkMScalar dx, SkMScalar dy, SkMScalar dz) {
    this->setIdentity();

    if (!dx && !dy && !dz) {
        return *this;
    }

    fMat[3][0] = dx;
    fMat[3][1] = dy;
    fMat[3][2] = dz;
    this->setTypeMask(kTranslate_Mask);
    return *this;
}

SkMatrix44& SkMatrix44::preTranslate(SkMScalar dx, SkMScalar dy, SkMScalar dz) {
    if (!dx && !dy && !dz) {
        return *this;
    }

    for (int i = 0; i < 4; ++i) {
        fMat[3][i] = fMat[0][i] * dx + fMat[1][i] * dy + fMat[2][i] * dz + fMat[3][i];
    }
    this->recomputeTypeMask();
    return *this;
}

SkMatrix44& SkMatrix44::postTranslate(SkMScalar dx, SkMScalar dy, SkMScalar dz) {
    if (!dx && !dy && !dz) {
        return *this;
    }

    if (this->getType() & kPerspective_Mask) {
        for (int i = 0; i < 4; ++i) {
            fMat[i][0] += fMat[i][3] * dx;
            fMat[i][1] += fMat[i][3] * dy;
            fMat[i][2] += fMat[i][3] * dz;
        }
    } else {
        fMat[3][0] += dx;
        fMat[3][1] += dy;
        fMat[3][2] += dz;
        this->recomputeTypeMask();
    }
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

SkMatrix44& SkMatrix44::setScale(SkMScalar sx, SkMScalar sy, SkMScalar sz) {
    this->setIdentity();

    if (1 == sx && 1 == sy && 1 == sz) {
        return *this;
    }

    fMat[0][0] = sx;
    fMat[1][1] = sy;
    fMat[2][2] = sz;
    this->setTypeMask(kScale_Mask);
    return *this;
}

SkMatrix44& SkMatrix44::preScale(SkMScalar sx, SkMScalar sy, SkMScalar sz) {
    if (1 == sx && 1 == sy && 1 == sz) {
        return *this;
    }

    // The implementation matrix * pureScale can be shortcut
    // by knowing that pureScale components effectively scale
    // the columns of the original matrix.
    for (int i = 0; i < 4; i++) {
        fMat[0][i] *= sx;
        fMat[1][i] *= sy;
        fMat[2][i] *= sz;
    }
    this->recomputeTypeMask();
    return *this;
}

SkMatrix44& SkMatrix44::postScale(SkMScalar sx, SkMScalar sy, SkMScalar sz) {
    if (1 == sx && 1 == sy && 1 == sz) {
        return *this;
    }

    for (int i = 0; i < 4; i++) {
        fMat[i][0] *= sx;
        fMat[i][1] *= sy;
        fMat[i][2] *= sz;
    }
    this->recomputeTypeMask();
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

void SkMatrix44::setRotateAbout(SkMScalar x, SkMScalar y, SkMScalar z,
                                SkMScalar radians) {
    double len2 = (double)x * x + (double)y * y + (double)z * z;
    if (1 != len2) {
        if (0 == len2) {
            this->setIdentity();
            return;
        }
        double scale = 1 / sqrt(len2);
        x = SkDoubleToMScalar(x * scale);
        y = SkDoubleToMScalar(y * scale);
        z = SkDoubleToMScalar(z * scale);
    }
    this->setRotateAboutUnit(x, y, z, radians);
}

void SkMatrix44::setRotateAboutUnit(SkMScalar x, SkMScalar y, SkMScalar z,
                                    SkMScalar radians) {
    double c = cos(radians);
    double s = sin(radians);
    double C = 1 - c;
    double xs = x * s;
    double ys = y * s;
    double zs = z * s;
    double xC = x * C;
    double yC = y * C;
    double zC = z * C;
    double xyC = x * yC;
    double yzC = y * zC;
    double zxC = z * xC;

    // if you're looking at wikipedia, remember that we're column major.
    this->set3x3(SkDoubleToMScalar(x * xC + c),     // scale x
                 SkDoubleToMScalar(xyC + zs),       // skew x
                 SkDoubleToMScalar(zxC - ys),       // trans x

                 SkDoubleToMScalar(xyC - zs),       // skew y
                 SkDoubleToMScalar(y * yC + c),     // scale y
                 SkDoubleToMScalar(yzC + xs),       // trans y

                 SkDoubleToMScalar(zxC + ys),       // persp x
                 SkDoubleToMScalar(yzC - xs),       // persp y
                 SkDoubleToMScalar(z * zC + c));    // persp 2
}

///////////////////////////////////////////////////////////////////////////////

static bool bits_isonly(int value, int mask) {
    return 0 == (value & ~mask);
}

void SkMatrix44::setConcat(const SkMatrix44& a, const SkMatrix44& b) {
    const SkMatrix44::TypeMask a_mask = a.getType();
    const SkMatrix44::TypeMask b_mask = b.getType();

    if (kIdentity_Mask == a_mask) {
        *this = b;
        return;
    }
    if (kIdentity_Mask == b_mask) {
        *this = a;
        return;
    }

    bool useStorage = (this == &a || this == &b);
    SkMScalar storage[16];
    SkMScalar* result = useStorage ? storage : &fMat[0][0];

    // Both matrices are at most scale+translate
    if (bits_isonly(a_mask | b_mask, kScale_Mask | kTranslate_Mask)) {
        result[0] = a.fMat[0][0] * b.fMat[0][0];
        result[1] = result[2] = result[3] = result[4] = 0;
        result[5] = a.fMat[1][1] * b.fMat[1][1];
        result[6] = result[7] = result[8] = result[9] = 0;
        result[10] = a.fMat[2][2] * b.fMat[2][2];
        result[11] = 0;
        result[12] = a.fMat[0][0] * b.fMat[3][0] + a.fMat[3][0];
        result[13] = a.fMat[1][1] * b.fMat[3][1] + a.fMat[3][1];
        result[14] = a.fMat[2][2] * b.fMat[3][2] + a.fMat[3][2];
        result[15] = 1;
    } else {
        for (int j = 0; j < 4; j++) {
            for (int i = 0; i < 4; i++) {
                double value = 0;
                for (int k = 0; k < 4; k++) {
                    value += SkMScalarToDouble(a.fMat[k][i]) * b.fMat[j][k];
                }
                *result++ = SkDoubleToMScalar(value);
            }
        }
    }

    if (useStorage) {
        memcpy(fMat, storage, sizeof(storage));
    }
    this->recomputeTypeMask();
}

///////////////////////////////////////////////////////////////////////////////

/** We always perform the calculation in doubles, to avoid prematurely losing
    precision along the way. This relies on the compiler automatically
    promoting our SkMScalar values to double (if needed).
 */
double SkMatrix44::determinant() const {
    if (this->isIdentity()) {
        return 1;
    }
    if (this->isScaleTranslate()) {
        return fMat[0][0] * fMat[1][1] * fMat[2][2] * fMat[3][3];
    }

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

static bool is_matrix_finite(const SkMatrix44& matrix) {
    SkMScalar accumulator = 0;
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            accumulator *= matrix.get(row, col);
        }
    }
    return accumulator == 0;
}

bool SkMatrix44::invert(SkMatrix44* storage) const {
    if (this->isIdentity()) {
        if (storage) {
            storage->setIdentity();
        }
        return true;
    }

    if (this->isTranslate()) {
        if (storage) {
            storage->setTranslate(-fMat[3][0], -fMat[3][1], -fMat[3][2]);
        }
        return true;
    }

    SkMatrix44 tmp;
    // Use storage if it's available and distinct from this matrix.
    SkMatrix44* inverse = (storage && storage != this) ? storage : &tmp;
    if (this->isScaleTranslate()) {
        if (0 == fMat[0][0] * fMat[1][1] * fMat[2][2]) {
            return false;
        }

        double invXScale = 1 / fMat[0][0];
        double invYScale = 1 / fMat[1][1];
        double invZScale = 1 / fMat[2][2];

        inverse->fMat[0][0] = SkDoubleToMScalar(invXScale);
        inverse->fMat[0][1] = 0;
        inverse->fMat[0][2] = 0;
        inverse->fMat[0][3] = 0;

        inverse->fMat[1][0] = 0;
        inverse->fMat[1][1] = SkDoubleToMScalar(invYScale);
        inverse->fMat[1][2] = 0;
        inverse->fMat[1][3] = 0;

        inverse->fMat[2][0] = 0;
        inverse->fMat[2][1] = 0;
        inverse->fMat[2][2] = SkDoubleToMScalar(invZScale);
        inverse->fMat[2][3] = 0;

        inverse->fMat[3][0] = SkDoubleToMScalar(-fMat[3][0] * invXScale);
        inverse->fMat[3][1] = SkDoubleToMScalar(-fMat[3][1] * invYScale);
        inverse->fMat[3][2] = SkDoubleToMScalar(-fMat[3][2] * invZScale);
        inverse->fMat[3][3] = 1;

        inverse->setTypeMask(this->getType());

        if (!is_matrix_finite(*inverse)) {
            return false;
        }
        if (storage && inverse != storage) {
            *storage = *inverse;
        }
        return true;
    }

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

    if (!(this->getType() & kPerspective_Mask)) {
        // If we know the matrix has no perspective, then the perspective
        // component is (0, 0, 0, 1). We can use this information to save a lot
        // of arithmetic that would otherwise be spent to compute the inverse
        // of a general matrix.

        SkASSERT(a03 == 0);
        SkASSERT(a13 == 0);
        SkASSERT(a23 == 0);
        SkASSERT(a33 == 1);

        double b00 = a00 * a11 - a01 * a10;
        double b01 = a00 * a12 - a02 * a10;
        double b03 = a01 * a12 - a02 * a11;
        double b06 = a20 * a31 - a21 * a30;
        double b07 = a20 * a32 - a22 * a30;
        double b08 = a20;
        double b09 = a21 * a32 - a22 * a31;
        double b10 = a21;
        double b11 = a22;

        // Calculate the determinant
        double det = b00 * b11 - b01 * b10 + b03 * b08;

        double invdet = sk_ieee_double_divide(1.0, det);
        // If det is zero, we want to return false. However, we also want to return false
        // if 1/det overflows to infinity (i.e. det is denormalized). Both of these are
        // handled by checking that 1/det is finite.
        if (!sk_float_isfinite(sk_double_to_float(invdet))) {
            return false;
        }

        b00 *= invdet;
        b01 *= invdet;
        b03 *= invdet;
        b06 *= invdet;
        b07 *= invdet;
        b08 *= invdet;
        b09 *= invdet;
        b10 *= invdet;
        b11 *= invdet;

        inverse->fMat[0][0] = SkDoubleToMScalar(a11 * b11 - a12 * b10);
        inverse->fMat[0][1] = SkDoubleToMScalar(a02 * b10 - a01 * b11);
        inverse->fMat[0][2] = SkDoubleToMScalar(b03);
        inverse->fMat[0][3] = 0;
        inverse->fMat[1][0] = SkDoubleToMScalar(a12 * b08 - a10 * b11);
        inverse->fMat[1][1] = SkDoubleToMScalar(a00 * b11 - a02 * b08);
        inverse->fMat[1][2] = SkDoubleToMScalar(-b01);
        inverse->fMat[1][3] = 0;
        inverse->fMat[2][0] = SkDoubleToMScalar(a10 * b10 - a11 * b08);
        inverse->fMat[2][1] = SkDoubleToMScalar(a01 * b08 - a00 * b10);
        inverse->fMat[2][2] = SkDoubleToMScalar(b00);
        inverse->fMat[2][3] = 0;
        inverse->fMat[3][0] = SkDoubleToMScalar(a11 * b07 - a10 * b09 - a12 * b06);
        inverse->fMat[3][1] = SkDoubleToMScalar(a00 * b09 - a01 * b07 + a02 * b06);
        inverse->fMat[3][2] = SkDoubleToMScalar(a31 * b01 - a30 * b03 - a32 * b00);
        inverse->fMat[3][3] = 1;

        inverse->setTypeMask(this->getType());
        if (!is_matrix_finite(*inverse)) {
            return false;
        }
        if (storage && inverse != storage) {
            *storage = *inverse;
        }
        return true;
    }

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

    inverse->fMat[0][0] = SkDoubleToMScalar(a11 * b11 - a12 * b10 + a13 * b09);
    inverse->fMat[0][1] = SkDoubleToMScalar(a02 * b10 - a01 * b11 - a03 * b09);
    inverse->fMat[0][2] = SkDoubleToMScalar(a31 * b05 - a32 * b04 + a33 * b03);
    inverse->fMat[0][3] = SkDoubleToMScalar(a22 * b04 - a21 * b05 - a23 * b03);
    inverse->fMat[1][0] = SkDoubleToMScalar(a12 * b08 - a10 * b11 - a13 * b07);
    inverse->fMat[1][1] = SkDoubleToMScalar(a00 * b11 - a02 * b08 + a03 * b07);
    inverse->fMat[1][2] = SkDoubleToMScalar(a32 * b02 - a30 * b05 - a33 * b01);
    inverse->fMat[1][3] = SkDoubleToMScalar(a20 * b05 - a22 * b02 + a23 * b01);
    inverse->fMat[2][0] = SkDoubleToMScalar(a10 * b10 - a11 * b08 + a13 * b06);
    inverse->fMat[2][1] = SkDoubleToMScalar(a01 * b08 - a00 * b10 - a03 * b06);
    inverse->fMat[2][2] = SkDoubleToMScalar(a30 * b04 - a31 * b02 + a33 * b00);
    inverse->fMat[2][3] = SkDoubleToMScalar(a21 * b02 - a20 * b04 - a23 * b00);
    inverse->fMat[3][0] = SkDoubleToMScalar(a11 * b07 - a10 * b09 - a12 * b06);
    inverse->fMat[3][1] = SkDoubleToMScalar(a00 * b09 - a01 * b07 + a02 * b06);
    inverse->fMat[3][2] = SkDoubleToMScalar(a31 * b01 - a30 * b03 - a32 * b00);
    inverse->fMat[3][3] = SkDoubleToMScalar(a20 * b03 - a21 * b01 + a22 * b00);
    inverse->setTypeMask(this->getType());
    if (!is_matrix_finite(*inverse)) {
        return false;
    }
    if (storage && inverse != storage) {
        *storage = *inverse;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void SkMatrix44::transpose() {
    if (!this->isIdentity()) {
        using std::swap;
        swap(fMat[0][1], fMat[1][0]);
        swap(fMat[0][2], fMat[2][0]);
        swap(fMat[0][3], fMat[3][0]);
        swap(fMat[1][2], fMat[2][1]);
        swap(fMat[1][3], fMat[3][1]);
        swap(fMat[2][3], fMat[3][2]);
        this->recomputeTypeMask();
    }
}

///////////////////////////////////////////////////////////////////////////////

void SkMatrix44::mapScalars(const SkScalar src[4], SkScalar dst[4]) const {
    SkScalar storage[4];
    SkScalar* result = (src == dst) ? storage : dst;

    for (int i = 0; i < 4; i++) {
        SkMScalar value = 0;
        for (int j = 0; j < 4; j++) {
            value += fMat[j][i] * src[j];
        }
        result[i] = SkMScalarToScalar(value);
    }

    if (storage == result) {
        memcpy(dst, storage, sizeof(storage));
    }
}

#ifdef SK_MSCALAR_IS_DOUBLE

void SkMatrix44::mapMScalars(const SkMScalar src[4], SkMScalar dst[4]) const {
    SkMScalar storage[4];
    SkMScalar* result = (src == dst) ? storage : dst;

    for (int i = 0; i < 4; i++) {
        SkMScalar value = 0;
        for (int j = 0; j < 4; j++) {
            value += fMat[j][i] * src[j];
        }
        result[i] = value;
    }

    if (storage == result) {
        memcpy(dst, storage, sizeof(storage));
    }
}

#endif

typedef void (*Map2Procf)(const SkMScalar mat[][4], const float src2[], int count, float dst4[]);
typedef void (*Map2Procd)(const SkMScalar mat[][4], const double src2[], int count, double dst4[]);

static void map2_if(const SkMScalar mat[][4], const float* SK_RESTRICT src2,
                    int count, float* SK_RESTRICT dst4) {
    for (int i = 0; i < count; ++i) {
        dst4[0] = src2[0];
        dst4[1] = src2[1];
        dst4[2] = 0;
        dst4[3] = 1;
        src2 += 2;
        dst4 += 4;
    }
}

static void map2_id(const SkMScalar mat[][4], const double* SK_RESTRICT src2,
                    int count, double* SK_RESTRICT dst4) {
    for (int i = 0; i < count; ++i) {
        dst4[0] = src2[0];
        dst4[1] = src2[1];
        dst4[2] = 0;
        dst4[3] = 1;
        src2 += 2;
        dst4 += 4;
    }
}

static void map2_tf(const SkMScalar mat[][4], const float* SK_RESTRICT src2,
                    int count, float* SK_RESTRICT dst4) {
    const float mat30 = SkMScalarToFloat(mat[3][0]);
    const float mat31 = SkMScalarToFloat(mat[3][1]);
    const float mat32 = SkMScalarToFloat(mat[3][2]);
    for (int n = 0; n < count; ++n) {
        dst4[0] = src2[0] + mat30;
        dst4[1] = src2[1] + mat31;
        dst4[2] = mat32;
        dst4[3] = 1;
        src2 += 2;
        dst4 += 4;
    }
}

static void map2_td(const SkMScalar mat[][4], const double* SK_RESTRICT src2,
                    int count, double* SK_RESTRICT dst4) {
    for (int n = 0; n < count; ++n) {
        dst4[0] = src2[0] + mat[3][0];
        dst4[1] = src2[1] + mat[3][1];
        dst4[2] = mat[3][2];
        dst4[3] = 1;
        src2 += 2;
        dst4 += 4;
    }
}

static void map2_sf(const SkMScalar mat[][4], const float* SK_RESTRICT src2,
                    int count, float* SK_RESTRICT dst4) {
    const float mat32 = SkMScalarToFloat(mat[3][2]);
    for (int n = 0; n < count; ++n) {
        dst4[0] = SkMScalarToFloat(mat[0][0] * src2[0] + mat[3][0]);
        dst4[1] = SkMScalarToFloat(mat[1][1] * src2[1] + mat[3][1]);
        dst4[2] = mat32;
        dst4[3] = 1;
        src2 += 2;
        dst4 += 4;
    }
}

static void map2_sd(const SkMScalar mat[][4], const double* SK_RESTRICT src2,
                    int count, double* SK_RESTRICT dst4) {
    for (int n = 0; n < count; ++n) {
        dst4[0] = mat[0][0] * src2[0] + mat[3][0];
        dst4[1] = mat[1][1] * src2[1] + mat[3][1];
        dst4[2] = mat[3][2];
        dst4[3] = 1;
        src2 += 2;
        dst4 += 4;
    }
}

static void map2_af(const SkMScalar mat[][4], const float* SK_RESTRICT src2,
                    int count, float* SK_RESTRICT dst4) {
    SkMScalar r;
    for (int n = 0; n < count; ++n) {
        SkMScalar sx = SkFloatToMScalar(src2[0]);
        SkMScalar sy = SkFloatToMScalar(src2[1]);
        r = mat[0][0] * sx + mat[1][0] * sy + mat[3][0];
        dst4[0] = SkMScalarToFloat(r);
        r = mat[0][1] * sx + mat[1][1] * sy + mat[3][1];
        dst4[1] = SkMScalarToFloat(r);
        r = mat[0][2] * sx + mat[1][2] * sy + mat[3][2];
        dst4[2] = SkMScalarToFloat(r);
        dst4[3] = 1;
        src2 += 2;
        dst4 += 4;
    }
}

static void map2_ad(const SkMScalar mat[][4], const double* SK_RESTRICT src2,
                    int count, double* SK_RESTRICT dst4) {
    for (int n = 0; n < count; ++n) {
        double sx = src2[0];
        double sy = src2[1];
        dst4[0] = mat[0][0] * sx + mat[1][0] * sy + mat[3][0];
        dst4[1] = mat[0][1] * sx + mat[1][1] * sy + mat[3][1];
        dst4[2] = mat[0][2] * sx + mat[1][2] * sy + mat[3][2];
        dst4[3] = 1;
        src2 += 2;
        dst4 += 4;
    }
}

static void map2_pf(const SkMScalar mat[][4], const float* SK_RESTRICT src2,
                    int count, float* SK_RESTRICT dst4) {
    SkMScalar r;
    for (int n = 0; n < count; ++n) {
        SkMScalar sx = SkFloatToMScalar(src2[0]);
        SkMScalar sy = SkFloatToMScalar(src2[1]);
        for (int i = 0; i < 4; i++) {
            r = mat[0][i] * sx + mat[1][i] * sy + mat[3][i];
            dst4[i] = SkMScalarToFloat(r);
        }
        src2 += 2;
        dst4 += 4;
    }
}

static void map2_pd(const SkMScalar mat[][4], const double* SK_RESTRICT src2,
                    int count, double* SK_RESTRICT dst4) {
    for (int n = 0; n < count; ++n) {
        double sx = src2[0];
        double sy = src2[1];
        for (int i = 0; i < 4; i++) {
            dst4[i] = mat[0][i] * sx + mat[1][i] * sy + mat[3][i];
        }
        src2 += 2;
        dst4 += 4;
    }
}

void SkMatrix44::map2(const float src2[], int count, float dst4[]) const {
    static const Map2Procf gProc[] = {
        map2_if, map2_tf, map2_sf, map2_sf, map2_af, map2_af, map2_af, map2_af
    };

    TypeMask mask = this->getType();
    Map2Procf proc = (mask & kPerspective_Mask) ? map2_pf : gProc[mask];
    proc(fMat, src2, count, dst4);
}

void SkMatrix44::map2(const double src2[], int count, double dst4[]) const {
    static const Map2Procd gProc[] = {
        map2_id, map2_td, map2_sd, map2_sd, map2_ad, map2_ad, map2_ad, map2_ad
    };

    TypeMask mask = this->getType();
    Map2Procd proc = (mask & kPerspective_Mask) ? map2_pd : gProc[mask];
    proc(fMat, src2, count, dst4);
}

bool SkMatrix44::preserves2dAxisAlignment (SkMScalar epsilon) const {

    // Can't check (mask & kPerspective_Mask) because Z isn't relevant here.
    if (0 != perspX() || 0 != perspY()) return false;

    // A matrix with two non-zeroish values in any of the upper right
    // rows or columns will skew.  If only one value in each row or
    // column is non-zeroish, we get a scale plus perhaps a 90-degree
    // rotation.
    int col0 = 0;
    int col1 = 0;
    int row0 = 0;
    int row1 = 0;

    // Must test against epsilon, not 0, because we can get values
    // around 6e-17 in the matrix that "should" be 0.

    if (SkMScalarAbs(fMat[0][0]) > epsilon) {
        col0++;
        row0++;
    }
    if (SkMScalarAbs(fMat[0][1]) > epsilon) {
        col1++;
        row0++;
    }
    if (SkMScalarAbs(fMat[1][0]) > epsilon) {
        col0++;
        row1++;
    }
    if (SkMScalarAbs(fMat[1][1]) > epsilon) {
        col1++;
        row1++;
    }
    if (col0 > 1 || col1 > 1 || row0 > 1 || row1 > 1) {
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void SkMatrix44::dump() const {
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

static void initFromMatrix(SkMScalar dst[4][4], const SkMatrix& src) {
    dst[0][0] = SkScalarToMScalar(src[SkMatrix::kMScaleX]);
    dst[1][0] = SkScalarToMScalar(src[SkMatrix::kMSkewX]);
    dst[2][0] = 0;
    dst[3][0] = SkScalarToMScalar(src[SkMatrix::kMTransX]);
    dst[0][1] = SkScalarToMScalar(src[SkMatrix::kMSkewY]);
    dst[1][1] = SkScalarToMScalar(src[SkMatrix::kMScaleY]);
    dst[2][1] = 0;
    dst[3][1] = SkScalarToMScalar(src[SkMatrix::kMTransY]);
    dst[0][2] = 0;
    dst[1][2] = 0;
    dst[2][2] = 1;
    dst[3][2] = 0;
    dst[0][3] = SkScalarToMScalar(src[SkMatrix::kMPersp0]);
    dst[1][3] = SkScalarToMScalar(src[SkMatrix::kMPersp1]);
    dst[2][3] = 0;
    dst[3][3] = SkScalarToMScalar(src[SkMatrix::kMPersp2]);
}

SkMatrix44::SkMatrix44(const SkMatrix& src) {
    this->operator=(src);
}

SkMatrix44& SkMatrix44::operator=(const SkMatrix& src) {
    initFromMatrix(fMat, src);

    if (src.isIdentity()) {
        this->setTypeMask(kIdentity_Mask);
    } else {
        this->recomputeTypeMask();
    }
    return *this;
}

SkMatrix44::operator SkMatrix() const {
    SkMatrix dst;

    dst[SkMatrix::kMScaleX] = SkMScalarToScalar(fMat[0][0]);
    dst[SkMatrix::kMSkewX]  = SkMScalarToScalar(fMat[1][0]);
    dst[SkMatrix::kMTransX] = SkMScalarToScalar(fMat[3][0]);

    dst[SkMatrix::kMSkewY]  = SkMScalarToScalar(fMat[0][1]);
    dst[SkMatrix::kMScaleY] = SkMScalarToScalar(fMat[1][1]);
    dst[SkMatrix::kMTransY] = SkMScalarToScalar(fMat[3][1]);

    dst[SkMatrix::kMPersp0] = SkMScalarToScalar(fMat[0][3]);
    dst[SkMatrix::kMPersp1] = SkMScalarToScalar(fMat[1][3]);
    dst[SkMatrix::kMPersp2] = SkMScalarToScalar(fMat[3][3]);

    return dst;
}
