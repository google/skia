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

#include <initializer_list>

/* The values are stored column-major (for purposes of operator[]), but methods that
 *  take 16 values present them in row-major fashion.
 *
 *  0  4  8  12
 *  1  5  9  13
 *  2  6 10  14
 *  3  7 11  15
 */
class SK_API SkM4 {
public:
    SkM4(const SkM4& src) = default;
    SkM4& operator=(const SkM4& src) = default;

    constexpr SkM4()
        : fMat{1, 0, 0, 0,
               0, 1, 0, 0,
               0, 0, 1, 0,
               0, 0, 0, 1}
        {}


    SkM4(const SkM4& a, const SkM4& b) {
        this->setConcat(a, b);
    }

    /**
     *  Parameters are taken as row-major.
     */
    SkM4(SkScalar m0, SkScalar m4, SkScalar m8,  SkScalar m12,
         SkScalar m1, SkScalar m5, SkScalar m9,  SkScalar m13,
         SkScalar m2, SkScalar m6, SkScalar m10, SkScalar m14,
         SkScalar m3, SkScalar m7, SkScalar m11, SkScalar m15)
    {
        fMat[0] = m0; fMat[4] = m4; fMat[8]  = m8;  fMat[12] = m12;
        fMat[1] = m1; fMat[5] = m5; fMat[9]  = m9;  fMat[13] = m13;
        fMat[2] = m2; fMat[6] = m6; fMat[10] = m10; fMat[14] = m14;
        fMat[3] = m3; fMat[7] = m7; fMat[11] = m11; fMat[15] = m15;
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

    static const SkM4 kI;

    // assumes column major
    SkScalar operator[](int index) const {
        SkASSERT(index >= 0 && index < 16);
        return fMat[index];
    }

    void getColMajor(SkScalar v[]) const {
        memcpy(v, fMat, sizeof(fMat));
    }

    SkM4& setColMajor(const SkScalar v[]) {
        memcpy(fMat, v, sizeof(fMat));
        return *this;
    }

    /* The matrix is column-major, but these parameters are interpreted as row-major.
     * This allows the call-site to "look" as we might write it on paper. The corresponding
     * indices for the parameters are:
     *
     *  set4x4(0, 4,  8, 12,
     *         1, 5,  9, 13,
     *         2, 6, 10, 14,
     *         3, 7, 11, 15)
     */
    SkM4& set4x4(SkScalar m0, SkScalar m4, SkScalar m8, SkScalar m12,
                 SkScalar m1, SkScalar m5, SkScalar m9, SkScalar m13,
                 SkScalar m2, SkScalar m6, SkScalar m10, SkScalar m14,
                 SkScalar m3, SkScalar m7, SkScalar m11, SkScalar m15) {
        fMat[0] = m0; fMat[4] = m4; fMat[8]  = m8;  fMat[12] = m12;
        fMat[1] = m1; fMat[5] = m5; fMat[9]  = m9;  fMat[13] = m13;
        fMat[2] = m2; fMat[6] = m6; fMat[10] = m10; fMat[14] = m14;
        fMat[3] = m3; fMat[7] = m7; fMat[11] = m11; fMat[15] = m15;
        return *this;
    }

    SkM4& setIdentity() {
        *this = SkM4(1, 0, 0, 0,
                     0, 1, 0, 0,
                     0, 0, 1, 0,
                     0, 0, 0, 1);
        return *this;
    }

    SkM4 setTranslate(SkScalar x, SkScalar y, SkScalar z = 0) {
        *this = SkM4(1, 0, 0, x,
                     0, 1, 0, y,
                     0, 0, 1, z,
                     0, 0, 0, 1);
        return *this;
    }

    SkM4 setScale(SkScalar x, SkScalar y, SkScalar z = 1) {
        *this = SkM4(x, 0, 0, 0,
                     0, y, 0, 0,
                     0, 0, z, 0,
                     0, 0, 0, 1);
        return *this;
    }

    SkM4& setConcat(const SkM4& a, const SkM4& b);

    friend SkM4 operator*(const SkM4& a, const SkM4& b) {
        return SkM4(a, b);
    }

    /** If this is invertible, return that in inverse and return true. If it is
        not invertible, return false and leave the inverse parameter in an
        unspecified state.
     */
    bool invert(SkM4* inverse) const;

    void dump() const;

    ////////////////////// Converting to/from SkMatrix

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

private:
    SkScalar fMat[16];

    double determinant() const;
};

#endif
