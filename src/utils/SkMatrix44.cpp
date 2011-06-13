/*
    Copyright 2011 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#include "SkMatrix44.h"

SkMatrix44::SkMatrix44() {
    this->setIdentity();
}

SkMatrix44::SkMatrix44(const SkMatrix44& src) {
    memcpy(this, &src, sizeof(src));
}

SkMatrix44::SkMatrix44(const SkMatrix44& a, const SkMatrix44& b) {
    this->setConcat(a, b);
}

SkMScalar SkMatrix44::get(int row, int col) const {
    SkASSERT(row <= 3 && row >= 0);
    SkASSERT(col <= 3 && col >= 0);
    return fMat[col][row];
}

void SkMatrix44::set(int row, int col, const SkMScalar& value) {
    SkASSERT(row <= 3 && row >= 0);
    SkASSERT(col <= 3 && col >= 0);
    fMat[col][row] = value;
}

///////////////////////////////////////////////////////////////////////////////

void SkMatrix44::asColMajorf(float dst[]) const {
    const SkMScalar* src = &fMat[0][0];
#ifdef SK_MSCALAR_IS_DOUBLE
    for (int i = 0; i < 16; ++i) {
        dst[i] = SkMScalarToFloat(src[i]);
    }
#else
    memcpy(dst, src, 16 * sizeof(float));
#endif
}

void SkMatrix44::asColMajord(double dst[]) const {
    const SkMScalar* src = &fMat[0][0];
#ifdef SK_MSCALAR_IS_DOUBLE
    memcpy(dst, src, 16 * sizeof(double));
#else
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

///////////////////////////////////////////////////////////////////////////////

static const SkMatrix44 gIdentity44;

bool SkMatrix44::isIdentity() const {
    return *this == gIdentity44;
}

///////////////////////////////////////////////////////////////////////////////

void SkMatrix44::setIdentity() {
    sk_bzero(fMat, sizeof(fMat));
    fMat[0][0] = fMat[1][1] = fMat[2][2] = fMat[3][3] = 1;
}

void SkMatrix44::set3x3(SkMScalar m00, SkMScalar m01, SkMScalar m02,
                        SkMScalar m10, SkMScalar m11, SkMScalar m12,
                        SkMScalar m20, SkMScalar m21, SkMScalar m22) {
    sk_bzero(fMat, sizeof(fMat));
    fMat[0][0] = m00; fMat[0][1] = m01; fMat[0][2] = m02; fMat[0][3] = 0;
    fMat[1][0] = m10; fMat[1][1] = m11; fMat[1][2] = m12; fMat[1][3] = 0;
    fMat[2][0] = m20; fMat[2][1] = m21; fMat[2][2] = m22; fMat[2][3] = 0;
    fMat[3][0] = 0;   fMat[3][1] = 0;   fMat[3][2] = 0;   fMat[3][3] = 1;
}

///////////////////////////////////////////////////////////////////////////////

void SkMatrix44::setTranslate(SkMScalar tx, SkMScalar ty, SkMScalar tz) {
    this->setIdentity();
    fMat[3][0] = tx;
    fMat[3][1] = ty;
    fMat[3][2] = tz;
    fMat[3][3] = 1;
}

void SkMatrix44::preTranslate(SkMScalar dx, SkMScalar dy, SkMScalar dz) {
    SkMatrix44 mat;
    mat.setTranslate(dx, dy, dz);
    this->preConcat(mat);
}

void SkMatrix44::postTranslate(SkMScalar dx, SkMScalar dy, SkMScalar dz) {
    fMat[3][0] += dx;
    fMat[3][1] += dy;
    fMat[3][2] += dz;
}

///////////////////////////////////////////////////////////////////////////////

void SkMatrix44::setScale(SkMScalar sx, SkMScalar sy, SkMScalar sz) {
    sk_bzero(fMat, sizeof(fMat));
    fMat[0][0] = sx;
    fMat[1][1] = sy;
    fMat[2][2] = sz;
    fMat[3][3] = 1;
}

void SkMatrix44::preScale(SkMScalar sx, SkMScalar sy, SkMScalar sz) {
    SkMatrix44 tmp;
    tmp.setScale(sx, sy, sz);
    this->preConcat(tmp);
}

void SkMatrix44::postScale(SkMScalar sx, SkMScalar sy, SkMScalar sz) {
    for (int i = 0; i < 4; i++) {
        fMat[i][0] *= sx;
        fMat[i][1] *= sy;
        fMat[i][2] *= sz;
    }
}

///////////////////////////////////////////////////////////////////////////////

void SkMatrix44::setRotateAbout(SkMScalar x, SkMScalar y, SkMScalar z,
                                SkMScalar radians) {
    double len2 = x * x + y * y + z * z;
    if (len2 != 1) {
        if (len2 == 0) {
            this->setIdentity();
            return;
        }
        double scale = 1 / sqrt(len2);
        x *= scale;
        y *= scale;
        z *= scale;
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
    this->set3x3(x * xC + c,    xyC + zs,       zxC - ys,
                 xyC - zs,      y * yC + c,     yzC + xs,
                 zxC + ys,      yzC - xs,       z * zC + c);
}

///////////////////////////////////////////////////////////////////////////////

void SkMatrix44::setConcat(const SkMatrix44& a, const SkMatrix44& b) {
    SkMScalar result[4][4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            double value = 0;
            for (int k = 0; k < 4; k++) {
                value += SkMScalarToDouble(a.fMat[k][i]) * b.fMat[j][k];
            }
            result[j][i] = SkDoubleToMScalar(value);
        }
    }
    memcpy(fMat, result, sizeof(result));
}

///////////////////////////////////////////////////////////////////////////////

static inline SkMScalar det2x2(double m00, double m01, double m10, double m11) {
    return m00 * m11 - m10 * m01;
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
    double det = this->determinant();
    if (dabs(det) < TOO_SMALL_FOR_DETERMINANT) {
        return false;
    }
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
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void SkMatrix44::map(const SkScalar src[4], SkScalar dst[4]) const {
    SkScalar result[4];
    for (int i = 0; i < 4; i++) {
        SkMScalar value = 0;
        for (int j = 0; j < 4; j++) {
            value += fMat[j][i] * src[j];
        }
        result[i] = value;
    }
    memcpy(dst, result, sizeof(result));
}

///////////////////////////////////////////////////////////////////////////////

void SkMatrix44::dump() const {
    SkDebugf("[%g %g %g %g][%g %g %g %g][%g %g %g %g][%g %g %g %g]\n",
#if 0
             fMat[0][0], fMat[0][1], fMat[0][2], fMat[0][3],
             fMat[1][0], fMat[1][1], fMat[1][2], fMat[1][3],
             fMat[2][0], fMat[2][1], fMat[2][2], fMat[2][3],
             fMat[3][0], fMat[3][1], fMat[3][2], fMat[3][3]);
#else
             fMat[0][0], fMat[1][0], fMat[2][0], fMat[3][0],
             fMat[0][1], fMat[1][1], fMat[2][1], fMat[3][1],
             fMat[0][2], fMat[1][2], fMat[2][2], fMat[3][2],
             fMat[0][3], fMat[1][3], fMat[2][3], fMat[3][3]);
#endif
}

///////////////////////////////////////////////////////////////////////////////

static void initFromMatrix(SkMScalar dst[4][4], const SkMatrix& src) {
    sk_bzero(dst, 16 * sizeof(SkMScalar));
    dst[0][0] = src[SkMatrix::kMScaleX];
    dst[1][0] = src[SkMatrix::kMSkewX];
    dst[3][0] = src[SkMatrix::kMTransX];
    dst[0][1] = src[SkMatrix::kMSkewY];
    dst[1][1] = src[SkMatrix::kMScaleY];
    dst[3][1] = src[SkMatrix::kMTransY];
    dst[2][2] = dst[3][3] = 1;
}

SkMatrix44::SkMatrix44(const SkMatrix& src) {
    initFromMatrix(fMat, src);
}

SkMatrix44& SkMatrix44::operator=(const SkMatrix& src) {
    initFromMatrix(fMat, src);
    return *this;
}

SkMatrix44::operator SkMatrix() const {
    SkMatrix dst;
    dst.reset();    // setup our perspective correctly for identity

    dst[SkMatrix::kMScaleX]  = SkMScalarToFloat(fMat[0][0]);
    dst[SkMatrix::kMSkewX]  = SkMScalarToFloat(fMat[1][0]);
    dst[SkMatrix::kMTransX] = SkMScalarToFloat(fMat[3][0]);

    dst[SkMatrix::kMSkewY]  = SkMScalarToFloat(fMat[0][1]);
    dst[SkMatrix::kMScaleY] = SkMScalarToFloat(fMat[1][1]);
    dst[SkMatrix::kMTransY] = SkMScalarToFloat(fMat[3][1]);

    return dst;
}



