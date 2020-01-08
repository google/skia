/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkMatrix44.h"
#include <type_traits>
#include <utility>
#include "include/private/SkVx.h"

typedef skvx::Vec<4, float> sk4f;

// Copying SkMatrix44 byte-wise is performance-critical to Blink. This class is
// contained in several Transform classes, which are copied multiple times
// during the rendering life cycle. See crbug.com/938563 for reference.
#if defined(SK_BUILD_FOR_WIN) || defined(SK_BUILD_FOR_MAC)
// std::is_trivially_copyable is not supported for some older clang versions,
// which (at least as of this patch) are in use for Chromecast.
static_assert(std::is_trivially_copyable<SkMatrix44>::value,
              "SkMatrix44 must be trivially copyable");
#endif

bool SkMatrix44::operator==(const SkMatrix44& other) const {
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

///////////////////////////////////////////////////////////////////////////////
void SkMatrix44::recomputeTypeMask() {
    if (0 != perspX() || 0 != perspY() || 0 != perspZ() || 1 != fMat[15]) {
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

    if (0 != fMat[1] || 0 != fMat[2] || 0 != fMat[4] ||
        0 != fMat[6] || 0 != fMat[8] || 0 != fMat[9]) {
            mask |= kAffine_Mask;
    }
    fTypeMask = mask;
}

///////////////////////////////////////////////////////////////////////////////

void SkMatrix44::asColMajorf(float dst[]) const {
    const SkScalar* src = fMat;
    for (int i = 0; i < 16; ++i) {
        dst[i] = float(src[i]);
    }
}

void SkMatrix44::as3x4RowMajorf(float dst[]) const {
    dst[0] = fMat[0]; dst[1] = fMat[4]; dst[2]  = fMat[8];  dst[3]  = fMat[12];
    dst[4] = fMat[1]; dst[5] = fMat[5]; dst[6]  = fMat[9];  dst[7]  = fMat[13];
    dst[8] = fMat[2]; dst[9] = fMat[6]; dst[10] = fMat[10]; dst[11] = fMat[14];
}

void SkMatrix44::asColMajord(double dst[]) const {
    const SkScalar* src = fMat;
    for (int i = 0; i < 16; ++i) {
        dst[i] = double(src[i]);
    }
}

void SkMatrix44::asRowMajorf(float dst[]) const {
    const SkScalar* src = fMat;
    for (int i = 0; i < 4; ++i) {
        dst[0] = float(src[0]);
        dst[4] = float(src[1]);
        dst[8] = float(src[2]);
        dst[12] = float(src[3]);
        src += 4;
        dst += 1;
    }
}

void SkMatrix44::asRowMajord(double dst[]) const {
    const SkScalar* src = fMat;
    for (int i = 0; i < 4; ++i) {
        dst[0] = double(src[0]);
        dst[4] = double(src[1]);
        dst[8] = double(src[2]);
        dst[12] = double(src[3]);
        src += 4;
        dst += 1;
    }
}

void SkMatrix44::setColMajorf(const float src[]) {
    SkScalar* dst = fMat;
    for (int i = 0; i < 16; ++i) {
        dst[i] = SkScalar(src[i]);
    }

    this->recomputeTypeMask();
}

void SkMatrix44::setColMajord(const double src[]) {
    SkScalar* dst = fMat;
    for (int i = 0; i < 16; ++i) {
        dst[i] = SkScalar(src[i]);
    }

    this->recomputeTypeMask();
}

void SkMatrix44::setRowMajorf(const float src[]) {
    SkScalar* dst = fMat;
    for (int i = 0; i < 4; ++i) {
        dst[0] = SkScalar(src[0]);
        dst[4] = SkScalar(src[1]);
        dst[8] = SkScalar(src[2]);
        dst[12] = SkScalar(src[3]);
        src += 4;
        dst += 1;
    }
    this->recomputeTypeMask();
}

void SkMatrix44::setRowMajord(const double src[]) {
    SkScalar* dst = fMat;
    for (int i = 0; i < 4; ++i) {
        dst[0] = SkScalar(src[0]);
        dst[4] = SkScalar(src[1]);
        dst[8] = SkScalar(src[2]);
        dst[12] = SkScalar(src[3]);
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
    fMat[0] = 1;
    fMat[1] = 0;
    fMat[2] = 0;
    fMat[3] = 0;
    fMat[4] = 0;
    fMat[5] = 1;
    fMat[6] = 0;
    fMat[7] = 0;
    fMat[8] = 0;
    fMat[9] = 0;
    fMat[10] = 1;
    fMat[11] = 0;
    fMat[12] = 0;
    fMat[13] = 0;
    fMat[14] = 0;
    fMat[15] = 1;
    this->setTypeMask(kIdentity_Mask);
}

void SkMatrix44::set3x3(SkScalar m_00, SkScalar m_10, SkScalar m_20,
                        SkScalar m_01, SkScalar m_11, SkScalar m_21,
                        SkScalar m_02, SkScalar m_12, SkScalar m_22) {
    fMat[0]  = m_00; fMat[1]  = m_10; fMat[2]  = m_20; fMat[3]  = 0;
    fMat[4]  = m_01; fMat[5]  = m_11; fMat[6]  = m_21; fMat[7]  = 0;
    fMat[8]  = m_02; fMat[9]  = m_12; fMat[10] = m_22; fMat[11] = 0;
    fMat[12] = 0;    fMat[13] = 0;    fMat[14] = 0;    fMat[15] = 1;
    this->recomputeTypeMask();
}

void SkMatrix44::set3x3RowMajorf(const float src[]) {
    fMat[0]  = src[0]; fMat[1]  = src[3]; fMat[2]  = src[6]; fMat[3]  = 0;
    fMat[4]  = src[1]; fMat[5]  = src[4]; fMat[6]  = src[7]; fMat[7]  = 0;
    fMat[8]  = src[2]; fMat[9]  = src[5]; fMat[10] = src[8]; fMat[11] = 0;
    fMat[12] = 0;      fMat[13] = 0;      fMat[14] = 0;      fMat[15] = 1;
    this->recomputeTypeMask();
}

void SkMatrix44::set3x4RowMajorf(const float src[]) {
    fMat[0] = src[0]; fMat[4] = src[1]; fMat[8]  = src[2];  fMat[12] = src[3];
    fMat[1] = src[4]; fMat[5] = src[5]; fMat[9]  = src[6];  fMat[13] = src[7];
    fMat[2] = src[8]; fMat[6] = src[9]; fMat[10] = src[10]; fMat[14] = src[11];
    fMat[3] = 0;      fMat[7] = 0;      fMat[11] = 0;       fMat[15] = 1;
    this->recomputeTypeMask();
}

void SkMatrix44::set4x4(SkScalar m_00, SkScalar m_10, SkScalar m_20, SkScalar m_30,
                        SkScalar m_01, SkScalar m_11, SkScalar m_21, SkScalar m_31,
                        SkScalar m_02, SkScalar m_12, SkScalar m_22, SkScalar m_32,
                        SkScalar m_03, SkScalar m_13, SkScalar m_23, SkScalar m_33) {
    fMat[0]  = m_00; fMat[1]  = m_10; fMat[2]  = m_20; fMat[3] = m_30;
    fMat[4]  = m_01; fMat[5]  = m_11; fMat[6]  = m_21; fMat[7] = m_31;
    fMat[8]  = m_02; fMat[9]  = m_12; fMat[10] = m_22; fMat[11] = m_32;
    fMat[12] = m_03; fMat[13] = m_13; fMat[14] = m_23; fMat[15] = m_33;
    this->recomputeTypeMask();
}


///////////////////////////////////////////////////////////////////////////////

SkMatrix44& SkMatrix44::setTranslate(SkScalar dx, SkScalar dy, SkScalar dz) {
    this->setIdentity();

    if (!dx && !dy && !dz) {
        return *this;
    }

    fMat[12] = dx;
    fMat[13] = dy;
    fMat[14] = dz;
    this->setTypeMask(kTranslate_Mask);
    return *this;
}

SkMatrix44& SkMatrix44::preTranslate(SkScalar dx, SkScalar dy, SkScalar dz) {
    if (!dx && !dy && !dz) {
        return *this;
    }

    for (int i = 0; i < 4; ++i) {
        fMat[12 + i] = fMat[i] * dx + fMat[4 + i] * dy + fMat[8 + i] * dz + fMat[12 + i];
    }
    this->recomputeTypeMask();
    return *this;
}

SkMatrix44& SkMatrix44::postTranslate(SkScalar dx, SkScalar dy, SkScalar dz) {
    if (!dx && !dy && !dz) {
        return *this;
    }

    if (this->getType() & kPerspective_Mask) {
        for (int i = 0; i < 4; ++i) {
            fMat[i*4 + 0] += fMat[i*4 + 3] * dx;
            fMat[i*4 + 1] += fMat[i*4 + 3] * dy;
            fMat[i*4 + 2] += fMat[i*4 + 3] * dz;
        }
    } else {
        fMat[12] += dx;
        fMat[13] += dy;
        fMat[14] += dz;
        this->recomputeTypeMask();
    }
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

SkMatrix44& SkMatrix44::setScale(SkScalar sx, SkScalar sy, SkScalar sz) {
    this->setIdentity();

    if (1 == sx && 1 == sy && 1 == sz) {
        return *this;
    }

    fMat[0]  = sx;
    fMat[5]  = sy;
    fMat[10] = sz;
    this->setTypeMask(kScale_Mask);
    return *this;
}

SkMatrix44& SkMatrix44::preScale(SkScalar sx, SkScalar sy, SkScalar sz) {
    if (1 == sx && 1 == sy && 1 == sz) {
        return *this;
    }

    // The implementation matrix * pureScale can be shortcut
    // by knowing that pureScale components effectively scale
    // the columns of the original matrix.
    for (int i = 0; i < 4; i++) {
        fMat[0 + i] *= sx;
        fMat[4 + i] *= sy;
        fMat[8 + i] *= sz;
    }
    this->recomputeTypeMask();
    return *this;
}

SkMatrix44& SkMatrix44::postScale(SkScalar sx, SkScalar sy, SkScalar sz) {
    if (1 == sx && 1 == sy && 1 == sz) {
        return *this;
    }

    for (int i = 0; i < 4; i++) {
        fMat[i*4 + 0] *= sx;
        fMat[i*4 + 1] *= sy;
        fMat[i*4 + 2] *= sz;
    }
    this->recomputeTypeMask();
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

void SkMatrix44::setRotateAbout(SkScalar x, SkScalar y, SkScalar z, SkScalar radians) {
    double len2 = (double)x * x + (double)y * y + (double)z * z;
    if (1 != len2) {
        if (0 == len2) {
            this->setIdentity();
            return;
        }
        double scale = 1 / sqrt(len2);
        x = SkScalar(x * scale);
        y = SkScalar(y * scale);
        z = SkScalar(z * scale);
    }
    this->setRotateAboutUnit(x, y, z, radians);
}

void SkMatrix44::setRotateAboutUnit(SkScalar x, SkScalar y, SkScalar z, SkScalar radians) {
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
    this->set3x3(SkScalar(x * xC + c),     // scale x
                 SkScalar(xyC + zs),       // skew x
                 SkScalar(zxC - ys),       // trans x

                 SkScalar(xyC - zs),       // skew y
                 SkScalar(y * yC + c),     // scale y
                 SkScalar(yzC + xs),       // trans y

                 SkScalar(zxC + ys),       // persp x
                 SkScalar(yzC - xs),       // persp y
                 SkScalar(z * zC + c));    // persp 2
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
    SkScalar storage[16];
    SkScalar* result = useStorage ? storage : fMat;

    // Both matrices are at most scale+translate
    if (bits_isonly(a_mask | b_mask, kScale_Mask | kTranslate_Mask)) {
        result[0] = a.fMat[0] * b.fMat[0];
        result[1] = result[2] = result[3] = result[4] = 0;
        result[5] = a.fMat[5] * b.fMat[5];
        result[6] = result[7] = result[8] = result[9] = 0;
        result[10] = a.fMat[10] * b.fMat[10];
        result[11] = 0;
        result[12] = a.fMat[0]  * b.fMat[12] + a.fMat[12];
        result[13] = a.fMat[5]  * b.fMat[13] + a.fMat[13];
        result[14] = a.fMat[10] * b.fMat[14] + a.fMat[14];
        result[15] = 1;
    } else {
        for (int j = 0; j < 4; j++) {
            for (int i = 0; i < 4; i++) {
                double value = 0;
                for (int k = 0; k < 4; k++) {
                    value += double(a.fMat[k*4 + i]) * b.fMat[j*4 + k];
                }
                *result++ = SkScalar(value);
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
        return fMat[0] * fMat[5] * fMat[10] * fMat[15];
    }

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

static bool is_matrix_finite(const SkMatrix44& matrix) {
    SkScalar accumulator = 0;
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
            storage->setTranslate(-fMat[12], -fMat[13], -fMat[14]);
        }
        return true;
    }

    SkMatrix44 tmp;
    // Use storage if it's available and distinct from this matrix.
    SkMatrix44* inverse = (storage && storage != this) ? storage : &tmp;
    if (this->isScaleTranslate()) {
        if (0 == fMat[0] * fMat[5] * fMat[10]) {
            return false;
        }

        double invXScale = 1 / fMat[0];
        double invYScale = 1 / fMat[5];
        double invZScale = 1 / fMat[10];

        inverse->fMat[0] = SkScalar(invXScale);
        inverse->fMat[1] = 0;
        inverse->fMat[2] = 0;
        inverse->fMat[3] = 0;

        inverse->fMat[4] = 0;
        inverse->fMat[5] = SkScalar(invYScale);
        inverse->fMat[6] = 0;
        inverse->fMat[7] = 0;

        inverse->fMat[8]  = 0;
        inverse->fMat[9]  = 0;
        inverse->fMat[10] = SkScalar(invZScale);
        inverse->fMat[11] = 0;

        inverse->fMat[12] = SkScalar(-fMat[12] * invXScale);
        inverse->fMat[13] = SkScalar(-fMat[13] * invYScale);
        inverse->fMat[14] = SkScalar(-fMat[14] * invZScale);
        inverse->fMat[15] = 1;

        inverse->setTypeMask(this->getType());

        if (!is_matrix_finite(*inverse)) {
            return false;
        }
        if (storage && inverse != storage) {
            *storage = *inverse;
        }
        return true;
    }

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

        inverse->fMat[0] = SkScalar(a11 * b11 - a12 * b10);
        inverse->fMat[1] = SkScalar(a02 * b10 - a01 * b11);
        inverse->fMat[2] = SkScalar(b03);
        inverse->fMat[3] = 0;
        inverse->fMat[4] = SkScalar(a12 * b08 - a10 * b11);
        inverse->fMat[5] = SkScalar(a00 * b11 - a02 * b08);
        inverse->fMat[6] = SkScalar(-b01);
        inverse->fMat[7] = 0;
        inverse->fMat[8] = SkScalar(a10 * b10 - a11 * b08);
        inverse->fMat[9] = SkScalar(a01 * b08 - a00 * b10);
        inverse->fMat[10] = SkScalar(b00);
        inverse->fMat[11] = 0;
        inverse->fMat[12] = SkScalar(a11 * b07 - a10 * b09 - a12 * b06);
        inverse->fMat[13] = SkScalar(a00 * b09 - a01 * b07 + a02 * b06);
        inverse->fMat[14] = SkScalar(a31 * b01 - a30 * b03 - a32 * b00);
        inverse->fMat[15] = 1;

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

    inverse->fMat[0] = SkScalar(a11 * b11 - a12 * b10 + a13 * b09);
    inverse->fMat[1] = SkScalar(a02 * b10 - a01 * b11 - a03 * b09);
    inverse->fMat[2] = SkScalar(a31 * b05 - a32 * b04 + a33 * b03);
    inverse->fMat[3] = SkScalar(a22 * b04 - a21 * b05 - a23 * b03);
    inverse->fMat[4] = SkScalar(a12 * b08 - a10 * b11 - a13 * b07);
    inverse->fMat[5] = SkScalar(a00 * b11 - a02 * b08 + a03 * b07);
    inverse->fMat[6] = SkScalar(a32 * b02 - a30 * b05 - a33 * b01);
    inverse->fMat[7] = SkScalar(a20 * b05 - a22 * b02 + a23 * b01);
    inverse->fMat[8] = SkScalar(a10 * b10 - a11 * b08 + a13 * b06);
    inverse->fMat[9] = SkScalar(a01 * b08 - a00 * b10 - a03 * b06);
    inverse->fMat[10] = SkScalar(a30 * b04 - a31 * b02 + a33 * b00);
    inverse->fMat[11] = SkScalar(a21 * b02 - a20 * b04 - a23 * b00);
    inverse->fMat[12] = SkScalar(a11 * b07 - a10 * b09 - a12 * b06);
    inverse->fMat[13] = SkScalar(a00 * b09 - a01 * b07 + a02 * b06);
    inverse->fMat[14] = SkScalar(a31 * b01 - a30 * b03 - a32 * b00);
    inverse->fMat[15] = SkScalar(a20 * b03 - a21 * b01 + a22 * b00);
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
        swap(fMat[ 1], fMat[ 4]);
        swap(fMat[ 2], fMat[ 8]);
        swap(fMat[ 3], fMat[12]);
        swap(fMat[ 6], fMat[ 9]);
        swap(fMat[ 7], fMat[13]);
        swap(fMat[11], fMat[14]);
        this->recomputeTypeMask();
    }
}

///////////////////////////////////////////////////////////////////////////////

void SkMatrix44::mapScalars(const SkScalar src[4], SkScalar dst[4]) const {
    SkScalar storage[4];
    SkScalar* result = (src == dst) ? storage : dst;

    for (int i = 0; i < 4; i++) {
        SkScalar value = 0;
        for (int j = 0; j < 4; j++) {
            value += fMat[j*4 + i] * src[j];
        }
        result[i] = value;
    }

    if (storage == result) {
        memcpy(dst, storage, sizeof(storage));
    }
}

typedef void (*Map2Procf)(const SkScalar mat[16], const float src2[], int count, float dst4[]);
typedef void (*Map2Procd)(const SkScalar mat[16], const double src2[], int count, double dst4[]);

static void map2_if(const SkScalar mat[16], const float* SK_RESTRICT src2,
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

static void map2_id(const SkScalar mat[16], const double* SK_RESTRICT src2,
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

static void map2_tf(const SkScalar mat[16], const float* SK_RESTRICT src2,
                    int count, float* SK_RESTRICT dst4) {
    const float mat30 = float(mat[12]);
    const float mat31 = float(mat[13]);
    const float mat32 = float(mat[14]);
    for (int n = 0; n < count; ++n) {
        dst4[0] = src2[0] + mat30;
        dst4[1] = src2[1] + mat31;
        dst4[2] = mat32;
        dst4[3] = 1;
        src2 += 2;
        dst4 += 4;
    }
}

static void map2_td(const SkScalar mat[16], const double* SK_RESTRICT src2,
                    int count, double* SK_RESTRICT dst4) {
    for (int n = 0; n < count; ++n) {
        dst4[0] = src2[0] + mat[12];
        dst4[1] = src2[1] + mat[13];
        dst4[2] = mat[14];
        dst4[3] = 1;
        src2 += 2;
        dst4 += 4;
    }
}

static void map2_sf(const SkScalar mat[16], const float* SK_RESTRICT src2,
                    int count, float* SK_RESTRICT dst4) {
    const float mat32 = float(mat[14]);
    for (int n = 0; n < count; ++n) {
        dst4[0] = float(mat[0] * src2[0] + mat[12]);
        dst4[1] = float(mat[5] * src2[1] + mat[13]);
        dst4[2] = mat32;
        dst4[3] = 1;
        src2 += 2;
        dst4 += 4;
    }
}

static void map2_sd(const SkScalar mat[16], const double* SK_RESTRICT src2,
                    int count, double* SK_RESTRICT dst4) {
    for (int n = 0; n < count; ++n) {
        dst4[0] = mat[0] * src2[0] + mat[12];
        dst4[1] = mat[5] * src2[1] + mat[13];
        dst4[2] = mat[14];
        dst4[3] = 1;
        src2 += 2;
        dst4 += 4;
    }
}

static void map2_af(const SkScalar mat[16], const float* SK_RESTRICT src2,
                    int count, float* SK_RESTRICT dst4) {
    SkScalar r;
    for (int n = 0; n < count; ++n) {
        SkScalar sx = src2[0];
        SkScalar sy = src2[1];
        r = mat[0] * sx + mat[4] * sy + mat[12];
        dst4[0] = float(r);
        r = mat[1] * sx + mat[5] * sy + mat[13];
        dst4[1] = float(r);
        r = mat[2] * sx + mat[6] * sy + mat[14];
        dst4[2] = float(r);
        dst4[3] = 1;
        src2 += 2;
        dst4 += 4;
    }
}

static void map2_ad(const SkScalar mat[16], const double* SK_RESTRICT src2,
                    int count, double* SK_RESTRICT dst4) {
    for (int n = 0; n < count; ++n) {
        double sx = src2[0];
        double sy = src2[1];
        dst4[0] = mat[0] * sx + mat[4] * sy + mat[12];
        dst4[1] = mat[1] * sx + mat[5] * sy + mat[13];
        dst4[2] = mat[2] * sx + mat[6] * sy + mat[14];
        dst4[3] = 1;
        src2 += 2;
        dst4 += 4;
    }
}

static void map2_pf(const SkScalar mat[16], const float* SK_RESTRICT src2,
                    int count, float* SK_RESTRICT dst4) {
    SkScalar r;
    for (int n = 0; n < count; ++n) {
        SkScalar sx = src2[0];
        SkScalar sy = src2[1];
        for (int i = 0; i < 4; i++) {
            r = mat[0 + i] * sx + mat[4 + i] * sy + mat[12 + i];
            dst4[i] = float(r);
        }
        src2 += 2;
        dst4 += 4;
    }
}

static void map2_pd(const SkScalar mat[16], const double* SK_RESTRICT src2,
                    int count, double* SK_RESTRICT dst4) {
    for (int n = 0; n < count; ++n) {
        double sx = src2[0];
        double sy = src2[1];
        for (int i = 0; i < 4; i++) {
            dst4[i] = mat[0 + i] * sx + mat[4 + i] * sy + mat[12 + i];
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

bool SkMatrix44::preserves2dAxisAlignment (SkScalar epsilon) const {

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

    if (SkScalarAbs(fMat[0]) > epsilon) {
        col0++;
        row0++;
    }
    if (SkScalarAbs(fMat[1]) > epsilon) {
        col1++;
        row0++;
    }
    if (SkScalarAbs(fMat[4]) > epsilon) {
        col0++;
        row1++;
    }
    if (SkScalarAbs(fMat[5]) > epsilon) {
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
             fMat[0], fMat[4], fMat[ 8], fMat[12],
             fMat[1], fMat[5], fMat[ 9], fMat[13],
             fMat[2], fMat[6], fMat[10], fMat[14],
             fMat[3], fMat[7], fMat[11], fMat[15]);
}

///////////////////////////////////////////////////////////////////////////////

static void initFromMatrix(SkScalar dst[16], const SkMatrix& src) {
    dst[ 0] = src[SkMatrix::kMScaleX];
    dst[ 4] = src[SkMatrix::kMSkewX];
    dst[ 8] = 0;
    dst[12] = src[SkMatrix::kMTransX];
    dst[ 1] = src[SkMatrix::kMSkewY];
    dst[ 5] = src[SkMatrix::kMScaleY];
    dst[ 9] = 0;
    dst[13] = src[SkMatrix::kMTransY];
    dst[ 2] = 0;
    dst[ 6] = 0;
    dst[10] = 1;
    dst[14] = 0;
    dst[ 3] = src[SkMatrix::kMPersp0];
    dst[ 7] = src[SkMatrix::kMPersp1];
    dst[11] = 0;
    dst[15] = src[SkMatrix::kMPersp2];
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
    return SkMatrix::MakeAll(fMat[0], fMat[4], fMat[12],
                             fMat[1], fMat[5], fMat[13],
                             fMat[3], fMat[7], fMat[15]);
}
