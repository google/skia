/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkM4_DEFINED
#define SkM4_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkScalar.h"

class SK_API SkM4 {
public:

    constexpr SkM4()
        : fMat{{ 1, 0, 0, 0, },
               { 0, 1, 0, 0, },
               { 0, 0, 1, 0, },
               { 0, 0, 0, 1, }}
        {}

    SkM4(const SkM4& src) = default;
    SkM4& operator=(const SkM4& src) = default;

    SkM4(const SkM4& a, const SkM4& b) {
        this->setConcat(a, b);
    }

    SkM4(float m_00, float m_10, float m_20, float m_30,
         float m_01, float m_11, float m_21, float m_31,
         float m_02, float m_12, float m_22, float m_32,
         float m_03, float m_13, float m_23, float m_33) {
        fMat[0][0] = m_00; fMat[0][1] = m_10; fMat[0][2] = m_20; fMat[0][3] = m_30;
        fMat[1][0] = m_01; fMat[1][1] = m_11; fMat[1][2] = m_21; fMat[1][3] = m_31;
        fMat[2][0] = m_02; fMat[2][1] = m_12; fMat[2][2] = m_22; fMat[2][3] = m_32;
        fMat[3][0] = m_03; fMat[3][1] = m_13; fMat[3][2] = m_23; fMat[3][3] = m_33;
    }

    static SkM4 Translate(SkScalar x, SkScalar y, SkScalar z = 0) {
        return SkM4(1, 0, 0, x,
                    0, 1, 0, y,
                    0, 0, 1, z,
                    0, 0, 0, 1);
    }

    static SkM4 Scale(SkScalar x, SkScalar y, SkScalar z = 1) {
        return SkM4(x, 0, 0, 0,
                    0, y, 0, 0,
                    0, 0, z, 0,
                    0, 0, 0, 1);
    }

    bool operator==(const SkM4& other) const;
    bool operator!=(const SkM4& other) const {
        return !(other == *this);
    }

    /* When converting from SkM4 to SkMatrix, the third row and
     * column is dropped.  When converting from SkMatrix to SkM4
     * the third row and column remain as identity:
     * [ a b c ]      [ a b 0 c ]
     * [ d e f ]  ->  [ d e 0 f ]
     * [ g h i ]      [ 0 0 1 0 ]
     *                [ g h 0 i ]
     */
    SkM4(const SkMatrix&);
    SkM4& operator=(const SkMatrix& src);
    operator SkMatrix() const;

    /**
     *  Return a reference to a const identity matrix
     */
    static const SkM4& I();

    float operator[](unsigned index) const {
        SkASSERT(index < 16);
        return (&fMat[0][0])[index];
    }

    SkM4& setIdentity() {
        *this = SkM4(1, 0, 0, 0,
                     0, 1, 0, 0,
                     0, 0, 1, 0,
                     0, 0, 0, 1);
        return *this;
    }

    /**
     *  get a value from the matrix. The row,col parameters work as follows:
     *  (0, 0)  scale-x
     *  (0, 3)  translate-x
     *  (3, 0)  perspective-x
     */
    inline float get(int row, int col) const {
        SkASSERT((unsigned)row <= 3);
        SkASSERT((unsigned)col <= 3);
        return fMat[col][row];
    }

    /**
     *  set a value in the matrix. The row,col parameters work as follows:
     *  (0, 0)  scale-x
     *  (0, 3)  translate-x
     *  (3, 0)  perspective-x
     */
    inline void set(int row, int col, float value) {
        SkASSERT((unsigned)row <= 3);
        SkASSERT((unsigned)col <= 3);
        fMat[col][row] = value;
    }

    void get16(float v[]) const {
        memcpy(v, fMat, 16 * sizeof(float));
    }
    SkM4& set16(const float v[]) {
        memcpy(fMat, v, 16 * sizeof(float));
        return *this;
    }

    SkM4& set4x4(float m_00, float m_10, float m_20, float m_30,
                 float m_01, float m_11, float m_21, float m_31,
                 float m_02, float m_12, float m_22, float m_32,
                 float m_03, float m_13, float m_23, float m_33) {
        fMat[0][0] = m_00; fMat[0][1] = m_10; fMat[0][2] = m_20; fMat[0][3] = m_30;
        fMat[1][0] = m_01; fMat[1][1] = m_11; fMat[1][2] = m_21; fMat[1][3] = m_31;
        fMat[2][0] = m_02; fMat[2][1] = m_12; fMat[2][2] = m_22; fMat[2][3] = m_32;
        fMat[3][0] = m_03; fMat[3][1] = m_13; fMat[3][2] = m_23; fMat[3][3] = m_33;
        return *this;
    }

    SkM4 setTranslate(float x, float y, float z = 0) {
        *this = SkM4(1, 0, 0, x,
                     0, 1, 0, y,
                     0, 0, 1, z,
                     0, 0, 0, 1);
        return *this;
    }
    SkM4& preTranslate(float dx, float dy, float dz);
    SkM4& postTranslate(float dx, float dy, float dz);

    SkM4 setScale(float x, float y, float z = 1) {
        *this = SkM4(x, 0, 0, 0,
                     0, y, 0, 0,
                     0, 0, z, 0,
                     0, 0, 0, 1);
        return *this;
    }
    SkM4& preScale(float sx, float sy, float sz);
    SkM4& postScale(float sx, float sy, float sz);

    SkM4& setConcat(const SkM4& a, const SkM4& b);
    inline SkM4& preConcat(const SkM4& m) {
        return this->setConcat(*this, m);
    }
    inline SkM4& postConcat(const SkM4& m) {
        return this->setConcat(m, *this);
    }

    friend SkM4 operator*(const SkM4& a, const SkM4& b) {
        return SkM4(a, b);
    }

    /** If this is invertible, return that in inverse and return true. If it is
        not invertible, return false and leave the inverse parameter in an
        unspecified state.
     */
    bool invert(SkM4* inverse) const;

    void dump() const;

private:
    float fMat[4][4];

    double determinant() const;

    float transX() const { return fMat[3][0]; }
    float transY() const { return fMat[3][1]; }
    float transZ() const { return fMat[3][2]; }

    float scaleX() const { return fMat[0][0]; }
    float scaleY() const { return fMat[1][1]; }
    float scaleZ() const { return fMat[2][2]; }

    float perspX() const { return fMat[0][3]; }
    float perspY() const { return fMat[1][3]; }
    float perspZ() const { return fMat[2][3]; }
};

#endif
