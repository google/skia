/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkM44_DEFINED
#define SkM44_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkScalar.h"

struct SkV3 {
    float x, y, z;

    bool operator==(const SkV3& v) const {
        return x == v.x && y == v.y && z == v.z;
    }
    bool operator!=(const SkV3& v) const { return !(*this == v); }

    static SkScalar Dot(const SkV3& a, const SkV3& b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
    static SkV3   Cross(const SkV3& a, const SkV3& b) {
        return { a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x };
    }

    SkV3 operator-() const { return {-x, -y, -z}; }
    SkV3 operator+(const SkV3& v) const { return { x + v.x, y + v.y, z + v.z }; }
    SkV3 operator-(const SkV3& v) const { return { x - v.x, y - v.y, z - v.z }; }

    SkV3 operator*(const SkV3& v) const {
        return { x*v.x, y*v.y, z*v.z };
    }
    friend SkV3 operator*(const SkV3& v, SkScalar s) {
        return { v.x*s, v.y*s, v.z*s };
    }
    friend SkV3 operator*(SkScalar s, const SkV3& v) { return v*s; }

    SkScalar lengthSquared() const { return Dot(*this, *this); }
    SkScalar length() const { return SkScalarSqrt(Dot(*this, *this)); }

    SkScalar dot(const SkV3& v) const { return Dot(*this, v); }
    SkV3 cross(const SkV3& v) const { return Cross(*this, v); }

    const float* ptr() const { return &x; }
    float* ptr() { return &x; }
};

struct SkV4 {
    float x, y, z, w;

    bool operator==(const SkV4& v) const {
        return x == v.x && y == v.y && z == v.z && w == v.w;
    }
    bool operator!=(const SkV4& v) const { return !(*this == v); }

    SkV4 operator-() const { return {-x, -y, -z, -w}; }
    SkV4 operator+(const SkV4& v) const { return { x + v.x, y + v.y, z + v.z, w + v.w }; }
    SkV4 operator-(const SkV4& v) const { return { x - v.x, y - v.y, z - v.z, w - v.w }; }

    SkV4 operator*(const SkV4& v) const {
        return { x*v.x, y*v.y, z*v.z, w*v.w };
    }
    friend SkV4 operator*(const SkV4& v, SkScalar s) {
        return { v.x*s, v.y*s, v.z*s, v.w*s };
    }
    friend SkV4 operator*(SkScalar s, const SkV4& v) { return v*s; }

    const float* ptr() const { return &x; }
    float* ptr() { return &x; }
};

/**
 *  4x4 matrix used by SkCanvas and other parts of Skia.
 *
 *  Skia assumes a right-handed coordinate system:
 *      +X goes to the right
 *      +Y goes down
 *      +Z goes into the screen (away from the viewer)
 */
class SkM44 {
public:
    SkM44(const SkM44& src) = default;
    SkM44& operator=(const SkM44& src) = default;

    constexpr SkM44()
        : fMat{1, 0, 0, 0,
               0, 1, 0, 0,
               0, 0, 1, 0,
               0, 0, 0, 1}
        {}

    SkM44(const SkM44& a, const SkM44& b) {
        this->setConcat(a, b);
    }

    enum Uninitialized_Constructor {
        kUninitialized_Constructor
    };
    SkM44(Uninitialized_Constructor) {}

    enum NaN_Constructor {
        kNaN_Constructor
    };
    SkM44(NaN_Constructor)
        : fMat{SK_ScalarNaN, SK_ScalarNaN, SK_ScalarNaN, SK_ScalarNaN,
               SK_ScalarNaN, SK_ScalarNaN, SK_ScalarNaN, SK_ScalarNaN,
               SK_ScalarNaN, SK_ScalarNaN, SK_ScalarNaN, SK_ScalarNaN,
               SK_ScalarNaN, SK_ScalarNaN, SK_ScalarNaN, SK_ScalarNaN}
    {}

    /**
     *  Parameters are treated as row-major.
     */
    SkM44(SkScalar m0, SkScalar m4, SkScalar m8,  SkScalar m12,
          SkScalar m1, SkScalar m5, SkScalar m9,  SkScalar m13,
          SkScalar m2, SkScalar m6, SkScalar m10, SkScalar m14,
          SkScalar m3, SkScalar m7, SkScalar m11, SkScalar m15)
    {
        this->set44(m0, m4, m8,  m12,
                    m1, m5, m9,  m13,
                    m2, m6, m10, m14,
                    m3, m7, m11, m15);
    }

    static SkM44 Rows(const SkV4& r0, const SkV4& r1, const SkV4& r2, const SkV4& r3) {
        SkM44 m(kUninitialized_Constructor);
        m.setRow(0, r0);
        m.setRow(1, r1);
        m.setRow(2, r2);
        m.setRow(3, r3);
        return m;
    }
    static SkM44 Cols(const SkV4& c0, const SkV4& c1, const SkV4& c2, const SkV4& c3) {
        SkM44 m(kUninitialized_Constructor);
        m.setCol(0, c0);
        m.setCol(1, c1);
        m.setCol(2, c2);
        m.setCol(3, c3);
        return m;
    }

    static SkM44 Translate(SkScalar x, SkScalar y, SkScalar z = 0) {
        return SkM44(1, 0, 0, x,
                     0, 1, 0, y,
                     0, 0, 1, z,
                     0, 0, 0, 1);
    }

    static SkM44 Scale(SkScalar x, SkScalar y, SkScalar z = 1) {
        return SkM44(x, 0, 0, 0,
                     0, y, 0, 0,
                     0, 0, z, 0,
                     0, 0, 0, 1);
    }

    static SkM44 Rotate(SkV3 axis, SkScalar radians) {
        SkM44 m(kUninitialized_Constructor);
        m.setRotate(axis, radians);
        return m;
    }

    bool operator==(const SkM44& other) const;
    bool operator!=(const SkM44& other) const {
        return !(other == *this);
    }

    void getColMajor(SkScalar v[]) const {
        memcpy(v, fMat, sizeof(fMat));
    }
    void getRowMajor(SkScalar v[]) const;

    SkM44& setColMajor(const SkScalar v[]) {
        memcpy(fMat, v, sizeof(fMat));
        return *this;
    }
    SkM44& setRowMajor(const SkScalar v[]);

    /* Parameters in same order as constructor.
     */
    SkM44& set44(SkScalar m0, SkScalar m4, SkScalar m8,  SkScalar m12,
                 SkScalar m1, SkScalar m5, SkScalar m9,  SkScalar m13,
                 SkScalar m2, SkScalar m6, SkScalar m10, SkScalar m14,
                 SkScalar m3, SkScalar m7, SkScalar m11, SkScalar m15) {
        fMat[0] = m0; fMat[4] = m4; fMat[8]  = m8;  fMat[12] = m12;
        fMat[1] = m1; fMat[5] = m5; fMat[9]  = m9;  fMat[13] = m13;
        fMat[2] = m2; fMat[6] = m6; fMat[10] = m10; fMat[14] = m14;
        fMat[3] = m3; fMat[7] = m7; fMat[11] = m11; fMat[15] = m15;
        return *this;
    }

    SkScalar rc(int r, int c) const {
        SkASSERT(r >= 0 && r <= 3);
        SkASSERT(c >= 0 && c <= 3);
        return fMat[c*4 + r];
    }
    void setRC(int r, int c, SkScalar value) {
        SkASSERT(r >= 0 && r <= 3);
        SkASSERT(c >= 0 && c <= 3);
        fMat[c*4 + r] = value;
    }

    SkV4 row(int i) const {
        SkASSERT(i >= 0 && i <= 3);
        return {fMat[i + 0], fMat[i + 4], fMat[i + 8], fMat[i + 12]};
    }
    SkV4 col(int i) const {
        SkASSERT(i >= 0 && i <= 3);
        return {fMat[i*4 + 0], fMat[i*4 + 1], fMat[i*4 + 2], fMat[i*4 + 3]};
    }

    void setRow(int i, const SkV4& v) {
        SkASSERT(i >= 0 && i <= 3);
        fMat[i + 0]  = v.x;
        fMat[i + 4]  = v.y;
        fMat[i + 8]  = v.z;
        fMat[i + 12] = v.w;
    }
    void setCol(int i, const SkV4& v) {
        SkASSERT(i >= 0 && i <= 3);
        memcpy(&fMat[i*4], v.ptr(), sizeof(v));
    }

    SkM44& setIdentity() {
        return this->set44(1, 0, 0, 0,
                           0, 1, 0, 0,
                           0, 0, 1, 0,
                           0, 0, 0, 1);
    }

    SkM44& setTranslate(SkScalar x, SkScalar y, SkScalar z = 0) {
        return this->set44(1, 0, 0, x,
                           0, 1, 0, y,
                           0, 0, 1, z,
                           0, 0, 0, 1);
    }

    SkM44& setScale(SkScalar x, SkScalar y, SkScalar z = 1) {
        return this->set44(x, 0, 0, 0,
                           0, y, 0, 0,
                           0, 0, z, 0,
                           0, 0, 0, 1);
    }

    /**
     *  Set this matrix to rotate about the specified unit-length axis vector,
     *  by an angle specified by its sin() and cos().
     *
     *  This does not attempt to verify that axis.length() == 1 or that the sin,cos values
     *  are correct.
     */
    SkM44& setRotateUnitSinCos(SkV3 axis, SkScalar sinAngle, SkScalar cosAngle);

    /**
     *  Set this matrix to rotate about the specified unit-length axis vector,
     *  by an angle specified in radians.
     *
     *  This does not attempt to verify that axis.length() == 1.
     */
    SkM44& setRotateUnit(SkV3 axis, SkScalar radians) {
        return this->setRotateUnitSinCos(axis, SkScalarSin(radians), SkScalarCos(radians));
    }

    /**
     *  Set this matrix to rotate about the specified axis vector,
     *  by an angle specified in radians.
     *
     *  Note: axis is not assumed to be unit-length, so it will be normalized internally.
     *        If axis is already unit-length, call setRotateAboutUnitRadians() instead.
     */
    SkM44& setRotate(SkV3 axis, SkScalar radians);

    SkM44& setConcat16(const SkM44& a, const SkScalar colMajor[16]);

    SkM44& setConcat(const SkM44& a, const SkM44& b) {
        return this->setConcat16(a, b.fMat);
    }

    friend SkM44 operator*(const SkM44& a, const SkM44& b) {
        return SkM44(a, b);
    }

    SkM44& preConcat16(const SkScalar colMajor[16]) {
        return this->setConcat16(*this, colMajor);
    }

    /** If this is invertible, return that in inverse and return true. If it is
     *  not invertible, return false and leave the inverse parameter unchanged.
     */
    bool SK_WARN_UNUSED_RESULT invert(SkM44* inverse) const;

    SkM44 transpose() const;

    void dump() const;

    ////////////

    SkV4 map(float x, float y, float z, float w) const;
    SkV4 operator*(const SkV4& v) const {
        return this->map(v.x, v.y, v.z, v.w);
    }
    SkV3 operator*(SkV3 v) const {
        auto v4 = this->map(v.x, v.y, v.z, 0);
        return {v4.x, v4.y, v4.z};
    }

    ////////////////////// Converting to/from SkMatrix

    /* When converting from SkM44 to SkMatrix, the third row and
     * column is dropped.  When converting from SkMatrix to SkM44
     * the third row and column remain as identity:
     * [ a b c ]      [ a b 0 c ]
     * [ d e f ]  ->  [ d e 0 f ]
     * [ g h i ]      [ 0 0 1 0 ]
     *                [ g h 0 i ]
     */
    SkMatrix asM33() const {
        return SkMatrix::MakeAll(fMat[0], fMat[4], fMat[12],
                                 fMat[1], fMat[5], fMat[13],
                                 fMat[3], fMat[7], fMat[15]);
    }

    SkM44(const SkMatrix& src)
    : SkM44(src[SkMatrix::kMScaleX], src[SkMatrix::kMSkewX],  0, src[SkMatrix::kMTransX],
            src[SkMatrix::kMSkewY],  src[SkMatrix::kMScaleY], 0, src[SkMatrix::kMTransY],
            0,                       0,                       1, 0,
            src[SkMatrix::kMPersp0], src[SkMatrix::kMPersp1], 0, src[SkMatrix::kMPersp2])
    {}

    SkM44& operator=(const SkMatrix& src) {
        *this = SkM44(src);
        return *this;
    }

    SkM44& preTranslate(SkScalar x, SkScalar y);
    SkM44& preScale(SkScalar x, SkScalar y);
    SkM44& preConcat(const SkMatrix&);

private:
    /* Stored in column-major.
     *  Indices
     *  0  4  8  12        1 0 0 trans_x
     *  1  5  9  13  e.g.  0 1 0 trans_y
     *  2  6 10  14        0 0 1 trans_z
     *  3  7 11  15        0 0 0 1
     */
    SkScalar fMat[16];

    double determinant() const;

    friend class SkMatrixPriv;
};

SkM44 Sk3LookAt(const SkV3& eye, const SkV3& center, const SkV3& up);
SkM44 Sk3Perspective(float near, float far, float angle);

#endif
