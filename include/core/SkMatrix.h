
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkMatrix_DEFINED
#define SkMatrix_DEFINED

#include "SkRect.h"

class SkString;

// TODO: can we remove these 3 (need to check chrome/android)
typedef SkScalar SkPersp;
#define SkScalarToPersp(x) (x)
#define SkPerspToScalar(x) (x)

/** \class SkMatrix

    The SkMatrix class holds a 3x3 matrix for transforming coordinates.
    SkMatrix does not have a constructor, so it must be explicitly initialized
    using either reset() - to construct an identity matrix, or one of the set
    functions (e.g. setTranslate, setRotate, etc.).
*/
class SK_API SkMatrix {
public:
    /** Enum of bit fields for the mask return by getType().
        Use this to identify the complexity of the matrix.
    */
    enum TypeMask {
        kIdentity_Mask      = 0,
        kTranslate_Mask     = 0x01,  //!< set if the matrix has translation
        kScale_Mask         = 0x02,  //!< set if the matrix has X or Y scale
        kAffine_Mask        = 0x04,  //!< set if the matrix skews or rotates
        kPerspective_Mask   = 0x08   //!< set if the matrix is in perspective
    };

    /** Returns a bitfield describing the transformations the matrix may
        perform. The bitfield is computed conservatively, so it may include
        false positives. For example, when kPerspective_Mask is true, all
        other bits may be set to true even in the case of a pure perspective
        transform.
   */
    TypeMask getType() const {
        if (fTypeMask & kUnknown_Mask) {
            fTypeMask = this->computeTypeMask();
        }
        // only return the public masks
        return (TypeMask)(fTypeMask & 0xF);
    }

    /** Returns true if the matrix is identity.
    */
    bool isIdentity() const {
        return this->getType() == 0;
    }

    /** Returns true if will map a rectangle to another rectangle. This can be
        true if the matrix is identity, scale-only, or rotates a multiple of
        90 degrees.
    */
    bool rectStaysRect() const {
        if (fTypeMask & kUnknown_Mask) {
            fTypeMask = this->computeTypeMask();
        }
        return (fTypeMask & kRectStaysRect_Mask) != 0;
    }
    // alias for rectStaysRect()
    bool preservesAxisAlignment() const { return this->rectStaysRect(); }

    /**
     *  Returns true if the matrix contains perspective elements.
     */
    bool hasPerspective() const {
        return SkToBool(this->getPerspectiveTypeMaskOnly() &
                        kPerspective_Mask);
    }

    /** Returns true if the matrix contains only translation, rotation or uniform scale
        Returns false if other transformation types are included or is degenerate
     */
    bool isSimilarity(SkScalar tol = SK_ScalarNearlyZero) const;

    /** Returns true if the matrix contains only translation, rotation or scale
        (non-uniform scale is allowed).
        Returns false if other transformation types are included or is degenerate
     */
    bool preservesRightAngles(SkScalar tol = SK_ScalarNearlyZero) const;

    enum {
        kMScaleX,
        kMSkewX,
        kMTransX,
        kMSkewY,
        kMScaleY,
        kMTransY,
        kMPersp0,
        kMPersp1,
        kMPersp2
    };

    /** Affine arrays are in column major order
        because that's how PDF and XPS like it.
     */
    enum {
        kAScaleX,
        kASkewY,
        kASkewX,
        kAScaleY,
        kATransX,
        kATransY
    };

    SkScalar operator[](int index) const {
        SkASSERT((unsigned)index < 9);
        return fMat[index];
    }

    SkScalar get(int index) const {
        SkASSERT((unsigned)index < 9);
        return fMat[index];
    }

    SkScalar getScaleX() const { return fMat[kMScaleX]; }
    SkScalar getScaleY() const { return fMat[kMScaleY]; }
    SkScalar getSkewY() const { return fMat[kMSkewY]; }
    SkScalar getSkewX() const { return fMat[kMSkewX]; }
    SkScalar getTranslateX() const { return fMat[kMTransX]; }
    SkScalar getTranslateY() const { return fMat[kMTransY]; }
    SkPersp getPerspX() const { return fMat[kMPersp0]; }
    SkPersp getPerspY() const { return fMat[kMPersp1]; }

    SkScalar& operator[](int index) {
        SkASSERT((unsigned)index < 9);
        this->setTypeMask(kUnknown_Mask);
        return fMat[index];
    }

    void set(int index, SkScalar value) {
        SkASSERT((unsigned)index < 9);
        fMat[index] = value;
        this->setTypeMask(kUnknown_Mask);
    }

    void setScaleX(SkScalar v) { this->set(kMScaleX, v); }
    void setScaleY(SkScalar v) { this->set(kMScaleY, v); }
    void setSkewY(SkScalar v) { this->set(kMSkewY, v); }
    void setSkewX(SkScalar v) { this->set(kMSkewX, v); }
    void setTranslateX(SkScalar v) { this->set(kMTransX, v); }
    void setTranslateY(SkScalar v) { this->set(kMTransY, v); }
    void setPerspX(SkPersp v) { this->set(kMPersp0, v); }
    void setPerspY(SkPersp v) { this->set(kMPersp1, v); }

    void setAll(SkScalar scaleX, SkScalar skewX, SkScalar transX,
                SkScalar skewY, SkScalar scaleY, SkScalar transY,
                SkPersp persp0, SkPersp persp1, SkPersp persp2) {
        fMat[kMScaleX] = scaleX;
        fMat[kMSkewX]  = skewX;
        fMat[kMTransX] = transX;
        fMat[kMSkewY]  = skewY;
        fMat[kMScaleY] = scaleY;
        fMat[kMTransY] = transY;
        fMat[kMPersp0] = persp0;
        fMat[kMPersp1] = persp1;
        fMat[kMPersp2] = persp2;
        this->setTypeMask(kUnknown_Mask);
    }

    /** Set the matrix to identity
    */
    void reset();
    // alias for reset()
    void setIdentity() { this->reset(); }

    /** Set the matrix to translate by (dx, dy).
    */
    void setTranslate(SkScalar dx, SkScalar dy);
    void setTranslate(const SkVector& v) { this->setTranslate(v.fX, v.fY); }

    /** Set the matrix to scale by sx and sy, with a pivot point at (px, py).
        The pivot point is the coordinate that should remain unchanged by the
        specified transformation.
    */
    void setScale(SkScalar sx, SkScalar sy, SkScalar px, SkScalar py);
    /** Set the matrix to scale by sx and sy.
    */
    void setScale(SkScalar sx, SkScalar sy);
    /** Set the matrix to scale by 1/divx and 1/divy. Returns false and doesn't
        touch the matrix if either divx or divy is zero.
    */
    bool setIDiv(int divx, int divy);
    /** Set the matrix to rotate by the specified number of degrees, with a
        pivot point at (px, py). The pivot point is the coordinate that should
        remain unchanged by the specified transformation.
    */
    void setRotate(SkScalar degrees, SkScalar px, SkScalar py);
    /** Set the matrix to rotate about (0,0) by the specified number of degrees.
    */
    void setRotate(SkScalar degrees);
    /** Set the matrix to rotate by the specified sine and cosine values, with
        a pivot point at (px, py). The pivot point is the coordinate that
        should remain unchanged by the specified transformation.
    */
    void setSinCos(SkScalar sinValue, SkScalar cosValue,
                   SkScalar px, SkScalar py);
    /** Set the matrix to rotate by the specified sine and cosine values.
    */
    void setSinCos(SkScalar sinValue, SkScalar cosValue);
    /** Set the matrix to skew by sx and sy, with a pivot point at (px, py).
        The pivot point is the coordinate that should remain unchanged by the
        specified transformation.
    */
    void setSkew(SkScalar kx, SkScalar ky, SkScalar px, SkScalar py);
    /** Set the matrix to skew by sx and sy.
    */
    void setSkew(SkScalar kx, SkScalar ky);
    /** Set the matrix to the concatenation of the two specified matrices,
        returning true if the the result can be represented. Either of the
        two matrices may also be the target matrix. *this = a * b;
    */
    bool setConcat(const SkMatrix& a, const SkMatrix& b);

    /** Preconcats the matrix with the specified translation.
        M' = M * T(dx, dy)
    */
    bool preTranslate(SkScalar dx, SkScalar dy);
    /** Preconcats the matrix with the specified scale.
        M' = M * S(sx, sy, px, py)
    */
    bool preScale(SkScalar sx, SkScalar sy, SkScalar px, SkScalar py);
    /** Preconcats the matrix with the specified scale.
        M' = M * S(sx, sy)
    */
    bool preScale(SkScalar sx, SkScalar sy);
    /** Preconcats the matrix with the specified rotation.
        M' = M * R(degrees, px, py)
    */
    bool preRotate(SkScalar degrees, SkScalar px, SkScalar py);
    /** Preconcats the matrix with the specified rotation.
        M' = M * R(degrees)
    */
    bool preRotate(SkScalar degrees);
    /** Preconcats the matrix with the specified skew.
        M' = M * K(kx, ky, px, py)
    */
    bool preSkew(SkScalar kx, SkScalar ky, SkScalar px, SkScalar py);
    /** Preconcats the matrix with the specified skew.
        M' = M * K(kx, ky)
    */
    bool preSkew(SkScalar kx, SkScalar ky);
    /** Preconcats the matrix with the specified matrix.
        M' = M * other
    */
    bool preConcat(const SkMatrix& other);

    /** Postconcats the matrix with the specified translation.
        M' = T(dx, dy) * M
    */
    bool postTranslate(SkScalar dx, SkScalar dy);
    /** Postconcats the matrix with the specified scale.
        M' = S(sx, sy, px, py) * M
    */
    bool postScale(SkScalar sx, SkScalar sy, SkScalar px, SkScalar py);
    /** Postconcats the matrix with the specified scale.
        M' = S(sx, sy) * M
    */
    bool postScale(SkScalar sx, SkScalar sy);
    /** Postconcats the matrix by dividing it by the specified integers.
        M' = S(1/divx, 1/divy, 0, 0) * M
    */
    bool postIDiv(int divx, int divy);
    /** Postconcats the matrix with the specified rotation.
        M' = R(degrees, px, py) * M
    */
    bool postRotate(SkScalar degrees, SkScalar px, SkScalar py);
    /** Postconcats the matrix with the specified rotation.
        M' = R(degrees) * M
    */
    bool postRotate(SkScalar degrees);
    /** Postconcats the matrix with the specified skew.
        M' = K(kx, ky, px, py) * M
    */
    bool postSkew(SkScalar kx, SkScalar ky, SkScalar px, SkScalar py);
    /** Postconcats the matrix with the specified skew.
        M' = K(kx, ky) * M
    */
    bool postSkew(SkScalar kx, SkScalar ky);
    /** Postconcats the matrix with the specified matrix.
        M' = other * M
    */
    bool postConcat(const SkMatrix& other);

    enum ScaleToFit {
        /**
         * Scale in X and Y independently, so that src matches dst exactly.
         * This may change the aspect ratio of the src.
         */
        kFill_ScaleToFit,
        /**
         * Compute a scale that will maintain the original src aspect ratio,
         * but will also ensure that src fits entirely inside dst. At least one
         * axis (X or Y) will fit exactly. kStart aligns the result to the
         * left and top edges of dst.
         */
        kStart_ScaleToFit,
        /**
         * Compute a scale that will maintain the original src aspect ratio,
         * but will also ensure that src fits entirely inside dst. At least one
         * axis (X or Y) will fit exactly. The result is centered inside dst.
         */
        kCenter_ScaleToFit,
        /**
         * Compute a scale that will maintain the original src aspect ratio,
         * but will also ensure that src fits entirely inside dst. At least one
         * axis (X or Y) will fit exactly. kEnd aligns the result to the
         * right and bottom edges of dst.
         */
        kEnd_ScaleToFit
    };

    /** Set the matrix to the scale and translate values that map the source
        rectangle to the destination rectangle, returning true if the the result
        can be represented.
        @param src the source rectangle to map from.
        @param dst the destination rectangle to map to.
        @param stf the ScaleToFit option
        @return true if the matrix can be represented by the rectangle mapping.
    */
    bool setRectToRect(const SkRect& src, const SkRect& dst, ScaleToFit stf);

    /** Set the matrix such that the specified src points would map to the
        specified dst points. count must be within [0..4].
        @param src  The array of src points
        @param dst  The array of dst points
        @param count The number of points to use for the transformation
        @return true if the matrix was set to the specified transformation
    */
    bool setPolyToPoly(const SkPoint src[], const SkPoint dst[], int count);

    /** If this matrix can be inverted, return true and if inverse is not null,
        set inverse to be the inverse of this matrix. If this matrix cannot be
        inverted, ignore inverse and return false
    */
    bool SK_WARN_UNUSED_RESULT invert(SkMatrix* inverse) const {
        // Allow the trivial case to be inlined.
        if (this->isIdentity()) {
            if (NULL != inverse) {
                inverse->reset();
            }
            return true;
        }
        return this->invertNonIdentity(inverse);
    }

    /** Fills the passed array with affine identity values
        in column major order.
        @param affine  The array to fill with affine identity values.
        Must not be NULL.
    */
    static void SetAffineIdentity(SkScalar affine[6]);

    /** Fills the passed array with the affine values in column major order.
        If the matrix is a perspective transform, returns false
        and does not change the passed array.
        @param affine  The array to fill with affine values. Ignored if NULL.
    */
    bool asAffine(SkScalar affine[6]) const;

    /** Apply this matrix to the array of points specified by src, and write
        the transformed points into the array of points specified by dst.
        dst[] = M * src[]
        @param dst  Where the transformed coordinates are written. It must
                    contain at least count entries
        @param src  The original coordinates that are to be transformed. It
                    must contain at least count entries
        @param count The number of points in src to read, and then transform
                     into dst.
    */
    void mapPoints(SkPoint dst[], const SkPoint src[], int count) const;

    /** Apply this matrix to the array of points, overwriting it with the
        transformed values.
        dst[] = M * pts[]
        @param pts  The points to be transformed. It must contain at least
                    count entries
        @param count The number of points in pts.
    */
    void mapPoints(SkPoint pts[], int count) const {
        this->mapPoints(pts, pts, count);
    }

    /** Like mapPoints but with custom byte stride between the points. Stride
     *  should be a multiple of sizeof(SkScalar).
     */
    void mapPointsWithStride(SkPoint pts[], size_t stride, int count) const {
        SkASSERT(stride >= sizeof(SkPoint));
        SkASSERT(0 == stride % sizeof(SkScalar));
        for (int i = 0; i < count; ++i) {
            this->mapPoints(pts, pts, 1);
            pts = (SkPoint*)((intptr_t)pts + stride);
        }
    }

    /** Like mapPoints but with custom byte stride between the points.
    */
    void mapPointsWithStride(SkPoint dst[], SkPoint src[],
                             size_t stride, int count) const {
        SkASSERT(stride >= sizeof(SkPoint));
        SkASSERT(0 == stride % sizeof(SkScalar));
        for (int i = 0; i < count; ++i) {
            this->mapPoints(dst, src, 1);
            src = (SkPoint*)((intptr_t)src + stride);
            dst = (SkPoint*)((intptr_t)dst + stride);
        }
    }

    /** Apply this matrix to the array of homogeneous points, specified by src,
        where a homogeneous point is defined by 3 contiguous scalar values,
        and write the transformed points into the array of scalars specified by dst.
        dst[] = M * src[]
        @param dst  Where the transformed coordinates are written. It must
                    contain at least 3 * count entries
        @param src  The original coordinates that are to be transformed. It
                    must contain at least 3 * count entries
        @param count The number of triples (homogeneous points) in src to read,
                     and then transform into dst.
    */
    void mapHomogeneousPoints(SkScalar dst[], const SkScalar src[], int count) const;

    void mapXY(SkScalar x, SkScalar y, SkPoint* result) const {
        SkASSERT(result);
        this->getMapXYProc()(*this, x, y, result);
    }

    /** Apply this matrix to the array of vectors specified by src, and write
        the transformed vectors into the array of vectors specified by dst.
        This is similar to mapPoints, but ignores any translation in the matrix.
        @param dst  Where the transformed coordinates are written. It must
                    contain at least count entries
        @param src  The original coordinates that are to be transformed. It
                    must contain at least count entries
        @param count The number of vectors in src to read, and then transform
                     into dst.
    */
    void mapVectors(SkVector dst[], const SkVector src[], int count) const;

    /** Apply this matrix to the array of vectors specified by src, and write
        the transformed vectors into the array of vectors specified by dst.
        This is similar to mapPoints, but ignores any translation in the matrix.
        @param vecs The vectors to be transformed. It must contain at least
                    count entries
        @param count The number of vectors in vecs.
    */
    void mapVectors(SkVector vecs[], int count) const {
        this->mapVectors(vecs, vecs, count);
    }

    /** Apply this matrix to the src rectangle, and write the transformed
        rectangle into dst. This is accomplished by transforming the 4 corners
        of src, and then setting dst to the bounds of those points.
        @param dst  Where the transformed rectangle is written.
        @param src  The original rectangle to be transformed.
        @return the result of calling rectStaysRect()
    */
    bool mapRect(SkRect* dst, const SkRect& src) const;

    /** Apply this matrix to the rectangle, and write the transformed rectangle
        back into it. This is accomplished by transforming the 4 corners of
        rect, and then setting it to the bounds of those points
        @param rect The rectangle to transform.
        @return the result of calling rectStaysRect()
    */
    bool mapRect(SkRect* rect) const {
        return this->mapRect(rect, *rect);
    }

    /** Apply this matrix to the src rectangle, and write the four transformed
        points into dst. The points written to dst will be the original top-left, top-right,
        bottom-right, and bottom-left points transformed by the matrix.
        @param dst  Where the transformed quad is written.
        @param rect The original rectangle to be transformed.
    */
    void mapRectToQuad(SkPoint dst[4], const SkRect& rect) const {
        // This could potentially be faster if we only transformed each x and y of the rect once.
        rect.toQuad(dst);
        this->mapPoints(dst, 4);
    }

    /** Return the mean radius of a circle after it has been mapped by
        this matrix. NOTE: in perspective this value assumes the circle
        has its center at the origin.
    */
    SkScalar mapRadius(SkScalar radius) const;

    typedef void (*MapXYProc)(const SkMatrix& mat, SkScalar x, SkScalar y,
                                 SkPoint* result);

    static MapXYProc GetMapXYProc(TypeMask mask) {
        SkASSERT((mask & ~kAllMasks) == 0);
        return gMapXYProcs[mask & kAllMasks];
    }

    MapXYProc getMapXYProc() const {
        return GetMapXYProc(this->getType());
    }

    typedef void (*MapPtsProc)(const SkMatrix& mat, SkPoint dst[],
                                  const SkPoint src[], int count);

    static MapPtsProc GetMapPtsProc(TypeMask mask) {
        SkASSERT((mask & ~kAllMasks) == 0);
        return gMapPtsProcs[mask & kAllMasks];
    }

    MapPtsProc getMapPtsProc() const {
        return GetMapPtsProc(this->getType());
    }

    /** If the matrix can be stepped in X (not complex perspective)
        then return true and if step[XY] is not null, return the step[XY] value.
        If it cannot, return false and ignore step.
    */
    bool fixedStepInX(SkScalar y, SkFixed* stepX, SkFixed* stepY) const;

    /** Efficient comparison of two matrices. It distinguishes between zero and
     *  negative zero. It will return false when the sign of zero values is the
     *  only difference between the two matrices. It considers NaN values to be
     *  equal to themselves. So a matrix full of NaNs is "cheap equal" to
     *  another matrix full of NaNs iff the NaN values are bitwise identical
     *  while according to strict the strict == test a matrix with a NaN value
     *  is equal to nothing, including itself.
     */
    bool cheapEqualTo(const SkMatrix& m) const {
        return 0 == memcmp(fMat, m.fMat, sizeof(fMat));
    }

    friend bool operator==(const SkMatrix& a, const SkMatrix& b);
    friend bool operator!=(const SkMatrix& a, const SkMatrix& b) {
        return !(a == b);
    }

    enum {
        // writeTo/readFromMemory will never return a value larger than this
        kMaxFlattenSize = 9 * sizeof(SkScalar) + sizeof(uint32_t)
    };
    // return the number of bytes written, whether or not buffer is null
    size_t writeToMemory(void* buffer) const;
    /**
     * Reads data from the buffer parameter
     *
     * @param buffer Memory to read from
     * @param length Amount of memory available in the buffer
     * @return number of bytes read (must be a multiple of 4) or
     *         0 if there was not enough memory available
     */
    size_t readFromMemory(const void* buffer, size_t length);

    SkDEVCODE(void dump() const;)
    SK_TO_STRING_NONVIRT()

    /**
     * Calculates the minimum stretching factor of the matrix. If the matrix has
     * perspective -1 is returned.
     *
     * @return minumum strecthing factor
     */
    SkScalar getMinStretch() const;

    /**
     * Calculates the maximum stretching factor of the matrix. If the matrix has
     * perspective -1 is returned.
     *
     * @return maximum strecthing factor
     */
    SkScalar getMaxStretch() const;

    /**
     *  Return a reference to a const identity matrix
     */
    static const SkMatrix& I();

    /**
     *  Return a reference to a const matrix that is "invalid", one that could
     *  never be used.
     */
    static const SkMatrix& InvalidMatrix();

    /**
     * Testing routine; the matrix's type cache should never need to be
     * manually invalidated during normal use.
     */
    void dirtyMatrixTypeCache() {
        this->setTypeMask(kUnknown_Mask);
    }

private:
    enum {
        /** Set if the matrix will map a rectangle to another rectangle. This
            can be true if the matrix is scale-only, or rotates a multiple of
            90 degrees.

            This bit will be set on identity matrices
        */
        kRectStaysRect_Mask = 0x10,

        /** Set if the perspective bit is valid even though the rest of
            the matrix is Unknown.
        */
        kOnlyPerspectiveValid_Mask = 0x40,

        kUnknown_Mask = 0x80,

        kORableMasks =  kTranslate_Mask |
                        kScale_Mask |
                        kAffine_Mask |
                        kPerspective_Mask,

        kAllMasks = kTranslate_Mask |
                    kScale_Mask |
                    kAffine_Mask |
                    kPerspective_Mask |
                    kRectStaysRect_Mask
    };

    SkScalar         fMat[9];
    mutable uint32_t fTypeMask;

    uint8_t computeTypeMask() const;
    uint8_t computePerspectiveTypeMask() const;

    void setTypeMask(int mask) {
        // allow kUnknown or a valid mask
        SkASSERT(kUnknown_Mask == mask || (mask & kAllMasks) == mask ||
                 ((kUnknown_Mask | kOnlyPerspectiveValid_Mask) & mask)
                 == (kUnknown_Mask | kOnlyPerspectiveValid_Mask));
        fTypeMask = SkToU8(mask);
    }

    void orTypeMask(int mask) {
        SkASSERT((mask & kORableMasks) == mask);
        fTypeMask = SkToU8(fTypeMask | mask);
    }

    void clearTypeMask(int mask) {
        // only allow a valid mask
        SkASSERT((mask & kAllMasks) == mask);
        fTypeMask &= ~mask;
    }

    TypeMask getPerspectiveTypeMaskOnly() const {
        if ((fTypeMask & kUnknown_Mask) &&
            !(fTypeMask & kOnlyPerspectiveValid_Mask)) {
            fTypeMask = this->computePerspectiveTypeMask();
        }
        return (TypeMask)(fTypeMask & 0xF);
    }

    /** Returns true if we already know that the matrix is identity;
        false otherwise.
    */
    bool isTriviallyIdentity() const {
        if (fTypeMask & kUnknown_Mask) {
            return false;
        }
        return ((fTypeMask & 0xF) == 0);
    }

    bool SK_WARN_UNUSED_RESULT invertNonIdentity(SkMatrix* inverse) const;

    static bool Poly2Proc(const SkPoint[], SkMatrix*, const SkPoint& scale);
    static bool Poly3Proc(const SkPoint[], SkMatrix*, const SkPoint& scale);
    static bool Poly4Proc(const SkPoint[], SkMatrix*, const SkPoint& scale);

    static void Identity_xy(const SkMatrix&, SkScalar, SkScalar, SkPoint*);
    static void Trans_xy(const SkMatrix&, SkScalar, SkScalar, SkPoint*);
    static void Scale_xy(const SkMatrix&, SkScalar, SkScalar, SkPoint*);
    static void ScaleTrans_xy(const SkMatrix&, SkScalar, SkScalar, SkPoint*);
    static void Rot_xy(const SkMatrix&, SkScalar, SkScalar, SkPoint*);
    static void RotTrans_xy(const SkMatrix&, SkScalar, SkScalar, SkPoint*);
    static void Persp_xy(const SkMatrix&, SkScalar, SkScalar, SkPoint*);

    static const MapXYProc gMapXYProcs[];

    static void Identity_pts(const SkMatrix&, SkPoint[], const SkPoint[], int);
    static void Trans_pts(const SkMatrix&, SkPoint dst[], const SkPoint[], int);
    static void Scale_pts(const SkMatrix&, SkPoint dst[], const SkPoint[], int);
    static void ScaleTrans_pts(const SkMatrix&, SkPoint dst[], const SkPoint[],
                               int count);
    static void Rot_pts(const SkMatrix&, SkPoint dst[], const SkPoint[], int);
    static void RotTrans_pts(const SkMatrix&, SkPoint dst[], const SkPoint[],
                             int count);
    static void Persp_pts(const SkMatrix&, SkPoint dst[], const SkPoint[], int);

    static const MapPtsProc gMapPtsProcs[];

    friend class SkPerspIter;
};

#endif
