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


#ifndef SkMatrix44_DEFINED
#define SkMatrix44_DEFINED

#include "SkMatrix.h"
#include "SkScalar.h"

#ifdef SK_MSCALAR_IS_DOUBLE
    typedef double SkMScalar;
    static inline double SkFloatToMScalar(float x) {
        return static_cast<double>(x);
    }
    static inline float SkMScalarToFloat(double x) {
        return static_cast<float>(x);
    }
    static inline double SkDoubleToMScalar(double x) {
        return x;
    }
    static inline double SkMScalarToDouble(double x) {
        return x;
    }
    static const SkMScalar SK_MScalarPI = 3.141592653589793;
#else
    typedef float SkMScalar;
    static inline float SkFloatToMScalar(float x) {
        return x;
    }
    static inline float SkMScalarToFloat(float x) {
        return x;
    }
    static inline float SkDoubleToMScalar(double x) {
        return static_cast<float>(x);
    }
    static inline double SkMScalarToDouble(float x) {
        return static_cast<double>(x);
    }
    static const SkMScalar SK_MScalarPI = 3.14159265f;
#endif

static const SkMScalar SK_MScalar1 = 1;

///////////////////////////////////////////////////////////////////////////////

struct SkVector4 {
    SkScalar fData[4];

    SkVector4() {
        this->set(0, 0, 0, 1);
    }
    SkVector4(const SkVector4& src) {
        memcpy(fData, src.fData, sizeof(fData));
    }
    SkVector4(SkScalar x, SkScalar y, SkScalar z, SkScalar w = SK_Scalar1) {
        fData[0] = x;
        fData[1] = y;
        fData[2] = z;
        fData[3] = w;
    }

    SkVector4& operator=(const SkVector4& src) {
        memcpy(fData, src.fData, sizeof(fData));
        return *this;
    }

    bool operator==(const SkVector4& v) {
        return fData[0] == v.fData[0] && fData[1] == v.fData[1] &&
               fData[2] == v.fData[2] && fData[3] == v.fData[3];
    }
    bool operator!=(const SkVector4& v) {
        return !(*this == v);
    }
    bool equals(SkScalar x, SkScalar y, SkScalar z, SkScalar w = SK_Scalar1) {
        return fData[0] == x && fData[1] == y &&
               fData[2] == z && fData[3] == w;
    }

    void set(SkScalar x, SkScalar y, SkScalar z, SkScalar w = SK_Scalar1) {
        fData[0] = x;
        fData[1] = y;
        fData[2] = z;
        fData[3] = w;
    }
};

class SkMatrix44 {
public:
    SkMatrix44();
    SkMatrix44(const SkMatrix44&);
    SkMatrix44(const SkMatrix44& a, const SkMatrix44& b);

    SkMatrix44& operator=(const SkMatrix44& src) {
        memcpy(this, &src, sizeof(*this));
        return *this;
    }

    bool operator==(const SkMatrix44& other) const {
        return !memcmp(this, &other, sizeof(*this));
    }
    bool operator!=(const SkMatrix44& other) const {
        return !!memcmp(this, &other, sizeof(*this));
    }

    SkMatrix44(const SkMatrix&);
    SkMatrix44& operator=(const SkMatrix& src);
    operator SkMatrix() const;

    SkMScalar get(int row, int col) const;
    void set(int row, int col, const SkMScalar& value);

    void asColMajorf(float[]) const;
    void asColMajord(double[]) const;
    void asRowMajorf(float[]) const;
    void asRowMajord(double[]) const;

    bool isIdentity() const;
    void setIdentity();
    void reset() { this->setIdentity();}

    void set3x3(SkMScalar m00, SkMScalar m01, SkMScalar m02,
                SkMScalar m10, SkMScalar m11, SkMScalar m12,
                SkMScalar m20, SkMScalar m21, SkMScalar m22);

    void setTranslate(SkMScalar dx, SkMScalar dy, SkMScalar dz);
    void preTranslate(SkMScalar dx, SkMScalar dy, SkMScalar dz);
    void postTranslate(SkMScalar dx, SkMScalar dy, SkMScalar dz);

    void setScale(SkMScalar sx, SkMScalar sy, SkMScalar sz);
    void preScale(SkMScalar sx, SkMScalar sy, SkMScalar sz);
    void postScale(SkMScalar sx, SkMScalar sy, SkMScalar sz);

    void setScale(SkMScalar scale) {
        this->setScale(scale, scale, scale);
    }
    void preScale(SkMScalar scale) {
        this->preScale(scale, scale, scale);
    }
    void postScale(SkMScalar scale) {
        this->postScale(scale, scale, scale);
    }

    void setRotateDegreesAbout(SkMScalar x, SkMScalar y, SkMScalar z,
                               SkMScalar degrees) {
        this->setRotateAbout(x, y, z, degrees * SK_MScalarPI / 180);
    }

    /** Rotate about the vector [x,y,z]. If that vector is not unit-length,
        it will be automatically resized.
     */
    void setRotateAbout(SkMScalar x, SkMScalar y, SkMScalar z,
                        SkMScalar radians);
    /** Rotate about the vector [x,y,z]. Does not check the length of the
        vector, assuming it is unit-length.
     */
    void setRotateAboutUnit(SkMScalar x, SkMScalar y, SkMScalar z,
                            SkMScalar radians);

    void setConcat(const SkMatrix44& a, const SkMatrix44& b);
    void preConcat(const SkMatrix44& m) {
        this->setConcat(*this, m);
    }
    void postConcat(const SkMatrix44& m) {
        this->setConcat(m, *this);
    }

    friend SkMatrix44 operator*(const SkMatrix44& a, const SkMatrix44& b) {
        return SkMatrix44(a, b);
    }

    /** If this is invertible, return that in inverse and return true. If it is
        not invertible, return false and ignore the inverse parameter.
     */
    bool invert(SkMatrix44* inverse) const;

    /** Apply the matrix to the src vector, returning the new vector in dst.
        It is legal for src and dst to point to the same memory.
     */
    void map(const SkScalar src[4], SkScalar dst[4]) const;
    void map(SkScalar vec[4]) const {
        this->map(vec, vec);
    }

    friend SkVector4 operator*(const SkMatrix44& m, const SkVector4& src) {
        SkVector4 dst;
        m.map(src.fData, dst.fData);
        return dst;
    }

    void dump() const;

private:
    /*  Stored in the same order as opengl:
         [3][0] = tx
         [3][1] = ty
         [3][2] = tz
     */
    SkMScalar fMat[4][4];

    double determinant() const;
};

#endif
