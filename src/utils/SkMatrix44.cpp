
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#include "SkMatrix44.h"

SkMatrix44::SkMatrix44(const SkMatrix44& src) {
    memcpy(this, &src, sizeof(src));
}

SkMatrix44::SkMatrix44(const SkMatrix44& a, const SkMatrix44& b) {
    this->setConcat(a, b);
}

static inline bool eq4(const SkMScalar* SK_RESTRICT a,
                      const SkMScalar* SK_RESTRICT b) {
    return (a[0] == b[0]) & (a[1] == b[1]) & (a[2] == b[2]) & (a[3] == b[3]);
}

bool SkMatrix44::operator==(const SkMatrix44& other) const {
    if (this == &other) {
        return true;
    }

    if (this->isTriviallyIdentity() && other.isTriviallyIdentity()) {
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

int SkMatrix44::computeTypeMask() const {
    unsigned mask = 0;
    
    if (0 != perspX() || 0 != perspY() || 0 != perspZ() || 1 != fMat[3][3]) {
        return kTranslate_Mask | kScale_Mask | kAffine_Mask | kPerspective_Mask;
    }

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

    return mask;
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

    this->dirtyTypeMask();
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

    this->dirtyTypeMask();
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
    this->dirtyTypeMask();
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
    this->dirtyTypeMask();
}

///////////////////////////////////////////////////////////////////////////////

const SkMatrix44& SkMatrix44::I() {
    static SkMatrix44 gIdentity;
    return gIdentity;
}

void SkMatrix44::setIdentity() {
    sk_bzero(fMat, sizeof(fMat));
    fMat[0][0] = fMat[1][1] = fMat[2][2] = fMat[3][3] = 1;
    this->setTypeMask(kIdentity_Mask);
}

void SkMatrix44::set3x3(SkMScalar m00, SkMScalar m01, SkMScalar m02,
                        SkMScalar m10, SkMScalar m11, SkMScalar m12,
                        SkMScalar m20, SkMScalar m21, SkMScalar m22) {
    fMat[0][0] = m00; fMat[0][1] = m01; fMat[0][2] = m02; fMat[0][3] = 0;
    fMat[1][0] = m10; fMat[1][1] = m11; fMat[1][2] = m12; fMat[1][3] = 0;
    fMat[2][0] = m20; fMat[2][1] = m21; fMat[2][2] = m22; fMat[2][3] = 0;
    fMat[3][0] = 0;   fMat[3][1] = 0;   fMat[3][2] = 0;   fMat[3][3] = 1;
    this->dirtyTypeMask();
}

///////////////////////////////////////////////////////////////////////////////

void SkMatrix44::setTranslate(SkMScalar dx, SkMScalar dy, SkMScalar dz) {
    this->setIdentity();

    if (!dx && !dy && !dz) {
        return;
    }
    
    fMat[3][0] = dx;
    fMat[3][1] = dy;
    fMat[3][2] = dz;
    this->setTypeMask(kTranslate_Mask);
}

void SkMatrix44::preTranslate(SkMScalar dx, SkMScalar dy, SkMScalar dz) {
    if (!dx && !dy && !dz) {
        return;
    }

    const double X = SkMScalarToDouble(dx);
    const double Y = SkMScalarToDouble(dy);
    const double Z = SkMScalarToDouble(dz);

    double tmp;
    for (int i = 0; i < 4; ++i) {
        tmp = fMat[0][i] * X + fMat[1][i] * Y + fMat[2][i] * Z + fMat[3][i];
        fMat[3][i] = SkDoubleToMScalar(tmp);
    }
    this->dirtyTypeMask();
}

void SkMatrix44::postTranslate(SkMScalar dx, SkMScalar dy, SkMScalar dz) {
    if (!dx && !dy && !dz) {
        return;
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
        this->dirtyTypeMask();
    }
}

///////////////////////////////////////////////////////////////////////////////

void SkMatrix44::setScale(SkMScalar sx, SkMScalar sy, SkMScalar sz) {
    this->setIdentity();

    if (1 == sx && 1 == sy && 1 == sz) {
        return;
    }

    fMat[0][0] = sx;
    fMat[1][1] = sy;
    fMat[2][2] = sz;
    this->setTypeMask(kScale_Mask);
}

void SkMatrix44::preScale(SkMScalar sx, SkMScalar sy, SkMScalar sz) {
    if (1 == sx && 1 == sy && 1 == sz) {
        return;
    }
    
    SkMatrix44 tmp;
    tmp.setScale(sx, sy, sz);
    this->preConcat(tmp);
}

void SkMatrix44::postScale(SkMScalar sx, SkMScalar sy, SkMScalar sz) {
    if (1 == sx && 1 == sy && 1 == sz) {
        return;
    }
    
    for (int i = 0; i < 4; i++) {
        fMat[i][0] *= sx;
        fMat[i][1] *= sy;
        fMat[i][2] *= sz;
    }
    this->dirtyTypeMask();
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

    if (bits_isonly(a_mask | b_mask, kScale_Mask | kTranslate_Mask)) {
        sk_bzero(result, sizeof(storage));
        result[0] = a.fMat[0][0] * b.fMat[0][0];
        result[5] = a.fMat[1][1] * b.fMat[1][1];
        result[10] = a.fMat[2][2] * b.fMat[2][2];
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
    this->dirtyTypeMask();
}

///////////////////////////////////////////////////////////////////////////////

static inline SkMScalar det2x2(double m00, double m01, double m10, double m11) {
    return SkDoubleToMScalar(m00 * m11 - m10 * m01);
}

static inline double det3x3(double m00, double m01, double m02,
                            double m10, double m11, double m12,
                            double m20, double m21, double m22) {
    return  m00 * det2x2(m11, m12, m21, m22) -
    m10 * det2x2(m01, m02, m21, m22) +
    m20 * det2x2(m01, m02, m11, m12);
}

/** We always perform the calculation in doubles, to avoid prematurely losing
    precision along the way. This relies on the compiler automatically
    promoting our SkMScalar values to double (if needed).
 */
double SkMatrix44::determinant() const {
    return  fMat[0][0] * det3x3(fMat[1][1], fMat[1][2], fMat[1][3],
                                fMat[2][1], fMat[2][2], fMat[2][3],
                                fMat[3][1], fMat[3][2], fMat[3][3]) -
    fMat[1][0] * det3x3(fMat[0][1], fMat[0][2], fMat[0][3],
                        fMat[2][1], fMat[2][2], fMat[2][3],
                        fMat[3][1], fMat[3][2], fMat[3][3]) +
    fMat[2][0] * det3x3(fMat[0][1], fMat[0][2], fMat[0][3],
                        fMat[1][1], fMat[1][2], fMat[1][3],
                        fMat[3][1], fMat[3][2], fMat[3][3]) -
    fMat[3][0] * det3x3(fMat[0][1], fMat[0][2], fMat[0][3],
                        fMat[1][1], fMat[1][2], fMat[1][3],
                        fMat[2][1], fMat[2][2], fMat[2][3]);
}

///////////////////////////////////////////////////////////////////////////////

// just picked a small value. not sure how to pick the "right" one
#define TOO_SMALL_FOR_DETERMINANT   (1.e-8)

static inline double dabs(double x) {
    if (x < 0) {
        x = -x;
    }
    return x;
}

bool SkMatrix44::invert(SkMatrix44* inverse) const {
    const SkMatrix44::TypeMask mask = this->getType();

    if (kIdentity_Mask == mask) {
        if (inverse) {
            *inverse = *this;
            return true;
        }
    }

    if (kTranslate_Mask == mask) {
        if (inverse) {
            inverse->setTranslate(-fMat[3][0], -fMat[3][1], -fMat[3][2]);
        }
        return true;
    }

    if (bits_isonly(mask, kScale_Mask | kTranslate_Mask)) {
        if (0 == fMat[0][0] * fMat[1][1] * fMat[2][2]) {
            return false;
        }
        if (inverse) {
            sk_bzero(inverse->fMat, sizeof(inverse->fMat));

            inverse->fMat[3][0] = -fMat[3][0] / fMat[0][0];
            inverse->fMat[3][1] = -fMat[3][1] / fMat[1][1];
            inverse->fMat[3][2] = -fMat[3][2] / fMat[2][2];

            inverse->fMat[0][0] = 1 / fMat[0][0];
            inverse->fMat[1][1] = 1 / fMat[1][1];
            inverse->fMat[2][2] = 1 / fMat[2][2];
            inverse->fMat[3][3] = 1;

            inverse->setTypeMask(mask);
        }
        return true;
    }

    double det = this->determinant();
    if (dabs(det) < TOO_SMALL_FOR_DETERMINANT) {
        return false;
    }

    // We now we will succeed, so return early if the caller doesn't actually
    // want the computed inverse.
    if (NULL == inverse) {
        return true;
    }

    // we explicitly promote to doubles to keep the intermediate values in
    // higher precision (assuming SkMScalar isn't already a double)
    double m00 = fMat[0][0];
    double m01 = fMat[0][1];
    double m02 = fMat[0][2];
    double m03 = fMat[0][3];
    double m10 = fMat[1][0];
    double m11 = fMat[1][1];
    double m12 = fMat[1][2];
    double m13 = fMat[1][3];
    double m20 = fMat[2][0];
    double m21 = fMat[2][1];
    double m22 = fMat[2][2];
    double m23 = fMat[2][3];
    double m30 = fMat[3][0];
    double m31 = fMat[3][1];
    double m32 = fMat[3][2];
    double m33 = fMat[3][3];

    double tmp[4][4];

    tmp[0][0] = m12*m23*m31 - m13*m22*m31 + m13*m21*m32 - m11*m23*m32 - m12*m21*m33 + m11*m22*m33;
    tmp[0][1] = m03*m22*m31 - m02*m23*m31 - m03*m21*m32 + m01*m23*m32 + m02*m21*m33 - m01*m22*m33;
    tmp[0][2] = m02*m13*m31 - m03*m12*m31 + m03*m11*m32 - m01*m13*m32 - m02*m11*m33 + m01*m12*m33;
    tmp[0][3] = m03*m12*m21 - m02*m13*m21 - m03*m11*m22 + m01*m13*m22 + m02*m11*m23 - m01*m12*m23;
    tmp[1][0] = m13*m22*m30 - m12*m23*m30 - m13*m20*m32 + m10*m23*m32 + m12*m20*m33 - m10*m22*m33;
    tmp[1][1] = m02*m23*m30 - m03*m22*m30 + m03*m20*m32 - m00*m23*m32 - m02*m20*m33 + m00*m22*m33;
    tmp[1][2] = m03*m12*m30 - m02*m13*m30 - m03*m10*m32 + m00*m13*m32 + m02*m10*m33 - m00*m12*m33;
    tmp[1][3] = m02*m13*m20 - m03*m12*m20 + m03*m10*m22 - m00*m13*m22 - m02*m10*m23 + m00*m12*m23;
    tmp[2][0] = m11*m23*m30 - m13*m21*m30 + m13*m20*m31 - m10*m23*m31 - m11*m20*m33 + m10*m21*m33;
    tmp[2][1] = m03*m21*m30 - m01*m23*m30 - m03*m20*m31 + m00*m23*m31 + m01*m20*m33 - m00*m21*m33;
    tmp[2][2] = m01*m13*m30 - m03*m11*m30 + m03*m10*m31 - m00*m13*m31 - m01*m10*m33 + m00*m11*m33;
    tmp[2][3] = m03*m11*m20 - m01*m13*m20 - m03*m10*m21 + m00*m13*m21 + m01*m10*m23 - m00*m11*m23;
    tmp[3][0] = m12*m21*m30 - m11*m22*m30 - m12*m20*m31 + m10*m22*m31 + m11*m20*m32 - m10*m21*m32;
    tmp[3][1] = m01*m22*m30 - m02*m21*m30 + m02*m20*m31 - m00*m22*m31 - m01*m20*m32 + m00*m21*m32;
    tmp[3][2] = m02*m11*m30 - m01*m12*m30 - m02*m10*m31 + m00*m12*m31 + m01*m10*m32 - m00*m11*m32;
    tmp[3][3] = m01*m12*m20 - m02*m11*m20 + m02*m10*m21 - m00*m12*m21 - m01*m10*m22 + m00*m11*m22;

    double invDet = 1.0 / det;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            inverse->fMat[i][j] = SkDoubleToMScalar(tmp[i][j] * invDet);
        }
    }
    inverse->dirtyTypeMask();
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void SkMatrix44::transpose() {
    SkTSwap(fMat[0][1], fMat[1][0]);
    SkTSwap(fMat[0][2], fMat[2][0]);
    SkTSwap(fMat[0][3], fMat[3][0]);
    SkTSwap(fMat[1][2], fMat[2][1]);
    SkTSwap(fMat[1][3], fMat[3][1]);
    SkTSwap(fMat[2][3], fMat[3][2]);

    if (!this->isTriviallyIdentity()) {
        this->dirtyTypeMask();
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
    double r;
    for (int n = 0; n < count; ++n) {
        double sx = src2[0];
        double sy = src2[1];
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
    double r;
    for (int n = 0; n < count; ++n) {
        double sx = src2[0];
        double sy = src2[1];
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

///////////////////////////////////////////////////////////////////////////////

void SkMatrix44::dump() const {
    static const char* format =
        "[%g %g %g %g][%g %g %g %g][%g %g %g %g][%g %g %g %g]\n";
#if 0
    SkDebugf(format,
             fMat[0][0], fMat[1][0], fMat[2][0], fMat[3][0],
             fMat[0][1], fMat[1][1], fMat[2][1], fMat[3][1],
             fMat[0][2], fMat[1][2], fMat[2][2], fMat[3][2],
             fMat[0][3], fMat[1][3], fMat[2][3], fMat[3][3]);
#else
    SkDebugf(format,
             fMat[0][0], fMat[0][1], fMat[0][2], fMat[0][3],
             fMat[1][0], fMat[1][1], fMat[1][2], fMat[1][3],
             fMat[2][0], fMat[2][1], fMat[2][2], fMat[2][3],
             fMat[3][0], fMat[3][1], fMat[3][2], fMat[3][3]);
#endif
}

///////////////////////////////////////////////////////////////////////////////

// TODO: make this support src' perspective elements
//
static void initFromMatrix(SkMScalar dst[4][4], const SkMatrix& src) {
    sk_bzero(dst, 16 * sizeof(SkMScalar));
    dst[0][0] = SkScalarToMScalar(src[SkMatrix::kMScaleX]);
    dst[1][0] = SkScalarToMScalar(src[SkMatrix::kMSkewX]);
    dst[3][0] = SkScalarToMScalar(src[SkMatrix::kMTransX]);
    dst[0][1] = SkScalarToMScalar(src[SkMatrix::kMSkewY]);
    dst[1][1] = SkScalarToMScalar(src[SkMatrix::kMScaleY]);
    dst[3][1] = SkScalarToMScalar(src[SkMatrix::kMTransY]);
    dst[2][2] = dst[3][3] = 1;
}

SkMatrix44::SkMatrix44(const SkMatrix& src) {
    initFromMatrix(fMat, src);
}

SkMatrix44& SkMatrix44::operator=(const SkMatrix& src) {
    initFromMatrix(fMat, src);

    if (src.isIdentity()) {
        this->setTypeMask(kIdentity_Mask);
    } else {
        this->dirtyTypeMask();
    }
    return *this;
}

// TODO: make this support our perspective elements
//
SkMatrix44::operator SkMatrix() const {
    SkMatrix dst;
    dst.reset();    // setup our perspective correctly for identity

    dst[SkMatrix::kMScaleX]  = SkMScalarToScalar(fMat[0][0]);
    dst[SkMatrix::kMSkewX]  = SkMScalarToScalar(fMat[1][0]);
    dst[SkMatrix::kMTransX] = SkMScalarToScalar(fMat[3][0]);

    dst[SkMatrix::kMSkewY]  = SkMScalarToScalar(fMat[0][1]);
    dst[SkMatrix::kMScaleY] = SkMScalarToScalar(fMat[1][1]);
    dst[SkMatrix::kMTransY] = SkMScalarToScalar(fMat[3][1]);

    return dst;
}
