/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkM44_DEFINED
#define SkM44_DEFINED

#include "include/core/SkScalar.h"

class SkMatrix;
class SkMatrix44;

struct SkV3 {
    float x, y, z;

    bool operator==(const SkV3& v) const {
        return x == v.x && y == v.y && z == v.z;
    }

    static SkScalar Dot(const SkV3& a, const SkV3& b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
    static SkV3   Cross(const SkV3& a, const SkV3& b) {
        return { a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x };
    }

    SkV3 operator-() const { return {-x, -y, -z}; }
    SkV3 operator+(const SkV3& v) const { return { x + v.x, y + v.y, z + v.z }; }
    SkV3 operator-(const SkV3& v) const { return { x - v.x, y - v.y, z - v.z }; }

    friend SkV3 operator*(const SkV3& v, SkScalar s) {
        return { v.x*s, v.y*s, v.z*s };
    }
    friend SkV3 operator*(SkScalar s, const SkV3& v) { return v*s; }

    SkScalar lengthSquared() const { return Dot(*this, *this); }
    SkScalar length() const { return SkScalarSqrt(Dot(*this, *this)); }

    SkScalar dot(const SkV3& v) const { return Dot(*this, v); }
    SkV3 cross(const SkV3& v) const { return Cross(*this, v); }
};

struct SkV4 {
    float x, y, z, w;

    bool operator==(const SkV4& v) const {
        return x == v.x && y == v.y && z == v.z && w == v.w;
    }
};

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

    /**
     *  Parameters are treated as row-major.
     */
    SkM44(SkScalar m0, SkScalar m4, SkScalar m8,  SkScalar m12,
          SkScalar m1, SkScalar m5, SkScalar m9,  SkScalar m13,
          SkScalar m2, SkScalar m6, SkScalar m10, SkScalar m14,
          SkScalar m3, SkScalar m7, SkScalar m11, SkScalar m15)
    {
        fMat[0] = m0; fMat[4] = m4; fMat[8]  = m8;  fMat[12] = m12;
        fMat[1] = m1; fMat[5] = m5; fMat[9]  = m9;  fMat[13] = m13;
        fMat[2] = m2; fMat[6] = m6; fMat[10] = m10; fMat[14] = m14;
        fMat[3] = m3; fMat[7] = m7; fMat[11] = m11; fMat[15] = m15;
    }

    SkM44(const SkMatrix44&);
    SkM44& operator=(const SkMatrix44&);

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

    /* Parameters are treated as row-major.
     */
    SkM44& setRowMajor(SkScalar m0, SkScalar m4, SkScalar m8,  SkScalar m12,
                       SkScalar m1, SkScalar m5, SkScalar m9,  SkScalar m13,
                       SkScalar m2, SkScalar m6, SkScalar m10, SkScalar m14,
                       SkScalar m3, SkScalar m7, SkScalar m11, SkScalar m15) {
        fMat[0] = m0; fMat[4] = m4; fMat[8]  = m8;  fMat[12] = m12;
        fMat[1] = m1; fMat[5] = m5; fMat[9]  = m9;  fMat[13] = m13;
        fMat[2] = m2; fMat[6] = m6; fMat[10] = m10; fMat[14] = m14;
        fMat[3] = m3; fMat[7] = m7; fMat[11] = m11; fMat[15] = m15;
        return *this;
    }

    SkScalar atColMajor(int index) const {
        SkASSERT(index >= 0 && index < 16);
        return fMat[index];
    }

    SkM44& setIdentity() {
        return this->setRowMajor(1, 0, 0, 0,
                                 0, 1, 0, 0,
                                 0, 0, 1, 0,
                                 0, 0, 0, 1);
    }

    SkM44& setTranslate(SkScalar x, SkScalar y, SkScalar z = 0) {
        return this->setRowMajor(1, 0, 0, x,
                                 0, 1, 0, y,
                                 0, 0, 1, z,
                                 0, 0, 0, 1);
    }

    SkM44& setScale(SkScalar x, SkScalar y, SkScalar z = 1) {
        return this->setRowMajor(x, 0, 0, 0,
                                 0, y, 0, 0,
                                 0, 0, z, 0,
                                 0, 0, 0, 1);
    }

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
    bool invert(SkM44* inverse) const;

    SkM44 transpose() const;

    void dump() const;

    ////////////

    SkV4 map(float x, float y, float z, float w) const;
    SkV4 operator*(const SkV4& v) const {
        return this->map(v.x, v.y, v.z, v.w);
    }
    SkV3 operator*(const SkV3& v) const {
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

    const SkScalar* asColMajor() const { return fMat; }
          SkScalar* asColMajor()       { return fMat; }

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
};

#endif
