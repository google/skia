/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMatrix44_DEFINED
#define SkMatrix44_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkScalar.h"

#include <atomic>
#include <cstring>

#ifdef SK_MSCALAR_IS_DOUBLE
#ifdef SK_MSCALAR_IS_FLOAT
    #error "can't define MSCALAR both as DOUBLE and FLOAT"
#endif
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
    static inline double SkMScalarAbs(double x) {
        return fabs(x);
    }
    static const SkMScalar SK_MScalarPI = 3.141592653589793;

    #define SkMScalarFloor(x)           sk_double_floor(x)
    #define SkMScalarCeil(x)            sk_double_ceil(x)
    #define SkMScalarRound(x)           sk_double_round(x)

    #define SkMScalarFloorToInt(x)      sk_double_floor2int(x)
    #define SkMScalarCeilToInt(x)       sk_double_ceil2int(x)
    #define SkMScalarRoundToInt(x)      sk_double_round2int(x)


#elif defined SK_MSCALAR_IS_FLOAT
#ifdef SK_MSCALAR_IS_DOUBLE
    #error "can't define MSCALAR both as DOUBLE and FLOAT"
#endif
    typedef float SkMScalar;

    static inline float SkFloatToMScalar(float x) {
        return x;
    }
    static inline float SkMScalarToFloat(float x) {
        return x;
    }
    static inline float SkDoubleToMScalar(double x) {
        return sk_double_to_float(x);
    }
    static inline double SkMScalarToDouble(float x) {
        return static_cast<double>(x);
    }
    static inline float SkMScalarAbs(float x) {
        return sk_float_abs(x);
    }
    static const SkMScalar SK_MScalarPI = 3.14159265f;

    #define SkMScalarFloor(x)           sk_float_floor(x)
    #define SkMScalarCeil(x)            sk_float_ceil(x)
    #define SkMScalarRound(x)           sk_float_round(x)

    #define SkMScalarFloorToInt(x)      sk_float_floor2int(x)
    #define SkMScalarCeilToInt(x)       sk_float_ceil2int(x)
    #define SkMScalarRoundToInt(x)      sk_float_round2int(x)

#endif

#define SkIntToMScalar(n)       static_cast<SkMScalar>(n)

#define SkMScalarToScalar(x)    SkMScalarToFloat(x)
#define SkScalarToMScalar(x)    SkFloatToMScalar(x)

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

/** \class SkMatrix44

    The SkMatrix44 class holds a 4x4 matrix.

*/
class SK_API SkMatrix44 {
public:

    enum Uninitialized_Constructor {
        kUninitialized_Constructor
    };
    enum Identity_Constructor {
        kIdentity_Constructor
    };

    SkMatrix44(Uninitialized_Constructor) {}  // ironically, cannot be constexpr

    constexpr SkMatrix44(Identity_Constructor)
        : fMat{{ 1, 0, 0, 0, },
               { 0, 1, 0, 0, },
               { 0, 0, 1, 0, },
               { 0, 0, 0, 1, }}
        , fTypeMask(kIdentity_Mask)
    {}

    constexpr SkMatrix44() : SkMatrix44{kIdentity_Constructor} {}

    SkMatrix44(const SkMatrix44& src) = default;

    SkMatrix44& operator=(const SkMatrix44& src) = default;

    SkMatrix44(const SkMatrix44& a, const SkMatrix44& b) {
        this->setConcat(a, b);
    }

    bool operator==(const SkMatrix44& other) const;
    bool operator!=(const SkMatrix44& other) const {
        return !(other == *this);
    }

    /* When converting from SkMatrix44 to SkMatrix, the third row and
     * column is dropped.  When converting from SkMatrix to SkMatrix44
     * the third row and column remain as identity:
     * [ a b c ]      [ a b 0 c ]
     * [ d e f ]  ->  [ d e 0 f ]
     * [ g h i ]      [ 0 0 1 0 ]
     *                [ g h 0 i ]
     */
    SkMatrix44(const SkMatrix&);
    SkMatrix44& operator=(const SkMatrix& src);
    operator SkMatrix() const;

    /**
     *  Return a reference to a const identity matrix
     */
    static const SkMatrix44& I();

    using TypeMask = uint8_t;
    enum : TypeMask {
        kIdentity_Mask = 0,
        kTranslate_Mask = 1 << 0,    //!< set if the matrix has translation
        kScale_Mask = 1 << 1,        //!< set if the matrix has any scale != 1
        kAffine_Mask = 1 << 2,       //!< set if the matrix skews or rotates
        kPerspective_Mask = 1 << 3,  //!< set if the matrix is in perspective
    };

    /**
     *  Returns a bitfield describing the transformations the matrix may
     *  perform. The bitfield is computed conservatively, so it may include
     *  false positives. For example, when kPerspective_Mask is true, all
     *  other bits may be set to true even in the case of a pure perspective
     *  transform.
     */
    inline TypeMask getType() const { return fTypeMask; }

    /**
     *  Return true if the matrix is identity.
     */
    inline bool isIdentity() const {
        return kIdentity_Mask == this->getType();
    }

    /**
     *  Return true if the matrix contains translate or is identity.
     */
    inline bool isTranslate() const {
        return !(this->getType() & ~kTranslate_Mask);
    }

    /**
     *  Return true if the matrix only contains scale or translate or is identity.
     */
    inline bool isScaleTranslate() const {
        return !(this->getType() & ~(kScale_Mask | kTranslate_Mask));
    }

    /**
     *  Returns true if the matrix only contains scale or is identity.
     */
    inline bool isScale() const {
            return !(this->getType() & ~kScale_Mask);
    }

    inline bool hasPerspective() const {
        return SkToBool(this->getType() & kPerspective_Mask);
    }

    void setIdentity();
    inline void reset() { this->setIdentity();}

    /**
     *  get a value from the matrix. The row,col parameters work as follows:
     *  (0, 0)  scale-x
     *  (0, 3)  translate-x
     *  (3, 0)  perspective-x
     */
    inline SkMScalar get(int row, int col) const {
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
    inline void set(int row, int col, SkMScalar value) {
        SkASSERT((unsigned)row <= 3);
        SkASSERT((unsigned)col <= 3);
        fMat[col][row] = value;
        this->recomputeTypeMask();
    }

    inline double getDouble(int row, int col) const {
        return SkMScalarToDouble(this->get(row, col));
    }
    inline void setDouble(int row, int col, double value) {
        this->set(row, col, SkDoubleToMScalar(value));
    }
    inline float getFloat(int row, int col) const {
        return SkMScalarToFloat(this->get(row, col));
    }
    inline void setFloat(int row, int col, float value) {
        this->set(row, col, SkFloatToMScalar(value));
    }

    /** These methods allow one to efficiently read matrix entries into an
     *  array. The given array must have room for exactly 16 entries. Whenever
     *  possible, they will try to use memcpy rather than an entry-by-entry
     *  copy.
     *
     *  Col major indicates that consecutive elements of columns will be stored
     *  contiguously in memory.  Row major indicates that consecutive elements
     *  of rows will be stored contiguously in memory.
     */
    void asColMajorf(float[]) const;
    void asColMajord(double[]) const;
    void asRowMajorf(float[]) const;
    void asRowMajord(double[]) const;

    /** These methods allow one to efficiently set all matrix entries from an
     *  array. The given array must have room for exactly 16 entries. Whenever
     *  possible, they will try to use memcpy rather than an entry-by-entry
     *  copy.
     *
     *  Col major indicates that input memory will be treated as if consecutive
     *  elements of columns are stored contiguously in memory.  Row major
     *  indicates that input memory will be treated as if consecutive elements
     *  of rows are stored contiguously in memory.
     */
    void setColMajorf(const float[]);
    void setColMajord(const double[]);
    void setRowMajorf(const float[]);
    void setRowMajord(const double[]);

#ifdef SK_MSCALAR_IS_FLOAT
    void setColMajor(const SkMScalar data[]) { this->setColMajorf(data); }
    void setRowMajor(const SkMScalar data[]) { this->setRowMajorf(data); }
#else
    void setColMajor(const SkMScalar data[]) { this->setColMajord(data); }
    void setRowMajor(const SkMScalar data[]) { this->setRowMajord(data); }
#endif

    /* This sets the top-left of the matrix and clears the translation and
     * perspective components (with [3][3] set to 1).  m_ij is interpreted
     * as the matrix entry at row = i, col = j. */
    void set3x3(SkMScalar m_00, SkMScalar m_10, SkMScalar m_20,
                SkMScalar m_01, SkMScalar m_11, SkMScalar m_21,
                SkMScalar m_02, SkMScalar m_12, SkMScalar m_22);
    void set3x3RowMajorf(const float[]);

    void set4x4(SkMScalar m_00, SkMScalar m_10, SkMScalar m_20, SkMScalar m_30,
                SkMScalar m_01, SkMScalar m_11, SkMScalar m_21, SkMScalar m_31,
                SkMScalar m_02, SkMScalar m_12, SkMScalar m_22, SkMScalar m_32,
                SkMScalar m_03, SkMScalar m_13, SkMScalar m_23, SkMScalar m_33);

    void setTranslate(SkMScalar dx, SkMScalar dy, SkMScalar dz);
    void preTranslate(SkMScalar dx, SkMScalar dy, SkMScalar dz);
    void postTranslate(SkMScalar dx, SkMScalar dy, SkMScalar dz);

    void setScale(SkMScalar sx, SkMScalar sy, SkMScalar sz);
    void preScale(SkMScalar sx, SkMScalar sy, SkMScalar sz);
    void postScale(SkMScalar sx, SkMScalar sy, SkMScalar sz);

    inline void setScale(SkMScalar scale) {
        this->setScale(scale, scale, scale);
    }
    inline void preScale(SkMScalar scale) {
        this->preScale(scale, scale, scale);
    }
    inline void postScale(SkMScalar scale) {
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
    inline void preConcat(const SkMatrix44& m) {
        this->setConcat(*this, m);
    }
    inline void postConcat(const SkMatrix44& m) {
        this->setConcat(m, *this);
    }

    friend SkMatrix44 operator*(const SkMatrix44& a, const SkMatrix44& b) {
        return SkMatrix44(a, b);
    }

    /** If this is invertible, return that in inverse and return true. If it is
        not invertible, return false and leave the inverse parameter in an
        unspecified state.
     */
    bool invert(SkMatrix44* inverse) const;

    /** Transpose this matrix in place. */
    void transpose();

    /** Apply the matrix to the src vector, returning the new vector in dst.
        It is legal for src and dst to point to the same memory.
     */
    void mapScalars(const SkScalar src[4], SkScalar dst[4]) const;
    inline void mapScalars(SkScalar vec[4]) const {
        this->mapScalars(vec, vec);
    }

#ifdef SK_MSCALAR_IS_DOUBLE
    void mapMScalars(const SkMScalar src[4], SkMScalar dst[4]) const;
#elif defined SK_MSCALAR_IS_FLOAT
    inline void mapMScalars(const SkMScalar src[4], SkMScalar dst[4]) const {
        this->mapScalars(src, dst);
    }
#endif
    inline void mapMScalars(SkMScalar vec[4]) const {
        this->mapMScalars(vec, vec);
    }

    friend SkVector4 operator*(const SkMatrix44& m, const SkVector4& src) {
        SkVector4 dst;
        m.mapScalars(src.fData, dst.fData);
        return dst;
    }

    /**
     *  map an array of [x, y, 0, 1] through the matrix, returning an array
     *  of [x', y', z', w'].
     *
     *  @param src2     array of [x, y] pairs, with implied z=0 and w=1
     *  @param count    number of [x, y] pairs in src2
     *  @param dst4     array of [x', y', z', w'] quads as the output.
     */
    void map2(const float src2[], int count, float dst4[]) const;
    void map2(const double src2[], int count, double dst4[]) const;

    /** Returns true if transformating an axis-aligned square in 2d by this matrix
        will produce another 2d axis-aligned square; typically means the matrix
        is a scale with perhaps a 90-degree rotation. A 3d rotation through 90
        degrees into a perpendicular plane collapses a square to a line, but
        is still considered to be axis-aligned.

        By default, tolerates very slight error due to float imprecisions;
        a 90-degree rotation can still end up with 10^-17 of
        "non-axis-aligned" result.
     */
    bool preserves2dAxisAlignment(SkMScalar epsilon = SK_ScalarNearlyZero) const;

    void dump() const;

    double determinant() const;

private:
    /* This is indexed by [col][row]. */
    SkMScalar fMat[4][4];
    TypeMask fTypeMask;

    static constexpr int kAllPublic_Masks = 0xF;

    void as3x4RowMajorf(float[]) const;
    void set3x4RowMajorf(const float[]);

    SkMScalar transX() const { return fMat[3][0]; }
    SkMScalar transY() const { return fMat[3][1]; }
    SkMScalar transZ() const { return fMat[3][2]; }

    SkMScalar scaleX() const { return fMat[0][0]; }
    SkMScalar scaleY() const { return fMat[1][1]; }
    SkMScalar scaleZ() const { return fMat[2][2]; }

    SkMScalar perspX() const { return fMat[0][3]; }
    SkMScalar perspY() const { return fMat[1][3]; }
    SkMScalar perspZ() const { return fMat[2][3]; }

    void recomputeTypeMask();

    inline void setTypeMask(TypeMask mask) {
        SkASSERT(0 == (~kAllPublic_Masks & mask));
        fTypeMask = mask;
    }

    inline const SkMScalar* values() const { return &fMat[0][0]; }

    friend class SkColorSpace;
};

#endif
