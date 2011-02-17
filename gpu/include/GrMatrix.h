/*
    Copyright 2010 Google Inc.

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


#ifndef GrMatrix_DEFINED
#define GrMatrix_DEFINED

#include "GrPoint.h"

struct GrRect;

/*
 * 3x3 matrix
 */
class GrMatrix {
public:
    static const GrMatrix& I() {
        static const GrMatrix I = GrMatrix(GR_Scalar1, 0, 0,
                                           0, GR_Scalar1, 0,
                                           0, 0, gRESCALE);
        return I;
    };
    static const GrMatrix& InvalidMatrix() {
        static const GrMatrix INV = 
            GrMatrix(GR_ScalarMax, GR_ScalarMax, GR_ScalarMax,
                     GR_ScalarMax, GR_ScalarMax, GR_ScalarMax,
                     GR_ScalarMax, GR_ScalarMax, GR_ScalarMax);
        return INV;
    }
    /** 
     * Handy index constants
     */
    enum {
        kScaleX,
        kSkewX,
        kTransX,
        kSkewY,
        kScaleY,
        kTransY,
        kPersp0,
        kPersp1,
        kPersp2
    };
    
    /**
     * Create an uninitialized matrix
     */
    GrMatrix() {
        fTypeMask = 0;
    }
    
    /**
     * Create a matrix from an array of values
     * @param values    row-major array of matrix components
     */
    explicit GrMatrix(const GrScalar values[]) {
        setToArray(values);
    }
    
    /**
     * Create a matrix from values
     * @param scaleX    (0,0) matrix element
     * @param skewX     (0,1) matrix element
     * @param transX    (0,2) matrix element
     * @param skewY     (1,0) matrix element
     * @param scaleY    (1,1) matrix element
     * @param transY    (1,2) matrix element
     * @param persp0    (2,0) matrix element
     * @param persp1    (2,1) matrix element
     * @param persp2    (2,2) matrix element
     */
    GrMatrix(GrScalar scaleX,
             GrScalar skewX,
             GrScalar transX,
             GrScalar skewY,
             GrScalar scaleY,
             GrScalar transY,
             GrScalar persp0,
             GrScalar persp1,
             GrScalar persp2) {
        setAll(scaleX, skewX,  transX,
               skewY,  scaleY, transY,
               persp0, persp1, persp2);
    }

    /**
     * access matrix component
     * @return matrix component value
     */
    const GrScalar& operator[] (int idx) const {
        GrAssert((unsigned)idx < 9);
        return fM[idx];
    }

    /**
     * Set a matrix from an array of values
     * @param values    row-major array of matrix components
     */
    void setToArray(const GrScalar values[]) {
        for (int i = 0; i < 9; ++i) {
            fM[i] = values[i];
        }
        this->computeTypeMask();
    }
    
    /**
     * Create a matrix from values
     * @param scaleX    (0,0) matrix element
     * @param skewX     (0,1) matrix element
     * @param transX    (0,2) matrix element
     * @param skewY     (1,0) matrix element
     * @param scaleY    (1,1) matrix element
     * @param transY    (1,2) matrix element
     * @param persp0    (2,0) matrix element
     * @param persp1    (2,1) matrix element
     * @param persp2    (2,2) matrix element
     */
    void setAll(GrScalar scaleX,
                GrScalar skewX,
                GrScalar transX,
                GrScalar skewY,
                GrScalar scaleY,
                GrScalar transY,
                GrScalar persp0,
                GrScalar persp1,                
                GrScalar persp2) {
        fM[kScaleX] = scaleX;
        fM[kSkewX]  = skewX;
        fM[kTransX] = transX;
        fM[kSkewY]  = skewY;
        fM[kScaleY] = scaleY;
        fM[kTransY] = transY;
        fM[kPersp0] = persp0;
        fM[kPersp1] = persp1;
        fM[kPersp2] = persp2;
        
        this->computeTypeMask();
    }
    
    /**
     * set matrix component
     * @param idx    index of component to set
     * @param value  value to set component to
     */
    inline void set(int idx, GrScalar value);
    
    /**
     * make this matrix an identity matrix
     */
    void setIdentity();

    /**
     * overwrite entire matrix to be a translation matrix
     * @param dx    amount to translate by in x
     * @param dy    amount to translate by in y
     */
    void setTranslate(GrScalar dx, GrScalar dy);

    /**
     * overwrite entire matrix to be a scaling matrix
     * @param sx    x scale factor
     * @param sy    y scale factor
     */
    void setScale(GrScalar sx, GrScalar sy);

    /**
     * overwrite entire matrix to be a skew matrix
     * @param skx   x skew factor
     * @param sky   y skew factor
     */
    void setSkew(GrScalar skx, GrScalar sky);

    /**
     * set this matrix to be a concantenation of two
     * matrices (a*b). Either a, b, or both can be this matrix.
     * @param a     first matrix to multiply
     * @param b     second matrix to multiply
     */
    void setConcat(const GrMatrix& a, const GrMatrix& b);

    /**
     * Set this matrix to this*m
     * @param m     matrix to concatenate
     */
    void preConcat(const GrMatrix& m);

    /**
     * Set this matrix to m*this
     * @param m     matrix to concatenate
     */
    void postConcat(const GrMatrix& m);

    /**
     *  Compute the inverse of this matrix, and return true if it is invertible,
     *  or false if not.
     *
     *  If inverted is not null, and the matrix is invertible, then the inverse
     *  is written into it. If the matrix is not invertible (this method returns
     *  false) then inverted is left unchanged.
     */
    bool invert(GrMatrix* inverted) const;
    
    /**
     * Transforms a point by the matrix
     *
     * @param src   the point to transform
     * @return the transformed point
     */
    GrPoint mapPoint(const GrPoint& src) const {
        GrPoint result;
        (this->*gMapProcs[fTypeMask])(&result, &src, 1);
        return result;
    }
    
    /**
     * Transforms an array of points by the matrix.
     *
     * @param dstPts the array to write transformed points into
     * @param srcPts the array of points to transform
     @ @param count the number of points to transform
     */
    void mapPoints(GrPoint dstPts[], 
                   const GrPoint srcPts[], 
                   uint32_t count) const {
        (this->*gMapProcs[fTypeMask])(dstPts, srcPts, count);
    }

    /**
     * Transforms pts with arbitrary stride in place.
     *
     * @param start  pointer to first point to transform
     * @param stride distance in bytes between consecutive points
     @ @param count the number of points to transform
     */
    void mapPointsWithStride(GrPoint* start, 
                             size_t stride, 
                             uint32_t count) const {
        for (uint32_t i = 0; i < count; ++i) {            
            this->mapPoints(start, start, 1);
            start = (GrPoint*)((intptr_t)start + stride);
        }
    }
    
    /**
     *  Transform the 4 corners of the src rect, and return the bounding rect
     *  in the dst rect. Note: src and dst may point to the same memory.
     */
    void mapRect(GrRect* dst, const GrRect& src) const;

    /**
     *  Transform the 4 corners of the rect, and return their bounds in the rect
     */
    void mapRect(GrRect* rect) const {
        this->mapRect(rect, *rect);
    }

    /**
     * Checks if matrix is a perspective matrix.
     * @return true if third row is not (0, 0, 1)
     */
    bool hasPerspective() const;
    
    /**
     * Checks whether matrix is identity
     * @return true if matrix is idenity
     */
    bool isIdentity() const;
    
    /**
     * Calculates the maximum stretching factor of the matrix. Only defined if
     * the matrix does not have perspective.
     *
     * @return maximum strecthing factor or negative if matrix has perspective.
     */
    GrScalar getMaxStretch() const;

    /**
     * Checks for matrix equality. Test is element-by-element equality,
     * not a homogeneous test.
     * @return true if matrices are equal, false otherwise
     */
    bool operator == (const GrMatrix& m) const;

    /**
     * Checks for matrix inequality. Test is element-by-element inequality,
     * not a homogeneous test.
     * @return true if matrices are not equal, false otherwise
     */
    bool operator != (const GrMatrix& m) const;
    
    static void UnitTest();

private:
    static const GrScalar gRESCALE;

    void computeTypeMask() {
        fTypeMask = 0;
        if (0 != fM[kPersp0] || 0 != fM[kPersp1] || gRESCALE != fM[kPersp2]) {
            fTypeMask |= kPerspective_TypeBit;
        }
        if (GR_Scalar1 != fM[kScaleX] || GR_Scalar1 != fM[kScaleY]) {
            fTypeMask |= kScale_TypeBit;
            if (0 == fM[kScaleX] && 0 == fM[kScaleY]) {
                fTypeMask |= kZeroScale_TypeBit;
            }
        }
        if (0 != fM[kSkewX] || 0 != fM[kSkewY]) {
            fTypeMask |= kSkew_TypeBit;
        }
        if (0 != fM[kTransX] || 0 != fM[kTransY]) {
            fTypeMask |= kTranslate_TypeBit;
        }
    }

    
    double determinant() const;
    
    enum TypeBits {
        kScale_TypeBit       = 1 << 0, // set if scales are not both 1
        kTranslate_TypeBit   = 1 << 1, // set if translates are not both 0
        kSkew_TypeBit        = 1 << 2, // set if skews are not both 0
        kPerspective_TypeBit = 1 << 3, // set if perspective
        kZeroScale_TypeBit   = 1 << 4, // set if scales are both zero
    };

    void mapIdentity(GrPoint* dst, const GrPoint* src, uint32_t count) const;
    void mapScale(GrPoint* dst, const GrPoint* src, uint32_t count) const;
    void mapTranslate(GrPoint* dst, const GrPoint* src, uint32_t count) const;
    void mapScaleAndTranslate(GrPoint* dst, const GrPoint* src, uint32_t count) const;
    void mapSkew(GrPoint* dst, const GrPoint* src, uint32_t count) const;
    void mapScaleAndSkew(GrPoint* dst, const GrPoint* src, uint32_t count) const;
    void mapSkewAndTranslate(GrPoint* dst, const GrPoint* src, uint32_t count) const;
    void mapNonPerspective(GrPoint* dst, const GrPoint* src, uint32_t count) const;
    void mapPerspective(GrPoint* dst, const GrPoint* src, uint32_t count) const;
    void mapZero(GrPoint* dst, const GrPoint* src, uint32_t count) const;
    void mapSetToTranslate(GrPoint* dst, const GrPoint* src, uint32_t count) const;
    void mapSwappedScale(GrPoint* dst, const GrPoint* src, uint32_t count) const;
    void mapSwappedScaleAndTranslate(GrPoint* dst, const GrPoint* src, uint32_t count) const;
    
    void mapInvalid(GrPoint* dst, const GrPoint* src, uint32_t count) const;
    
    typedef void (GrMatrix::*MapProc) (GrPoint* dst, const GrPoint* src, uint32_t count) const;
    static const MapProc gMapProcs[];

    int      fTypeMask;
    
    GrScalar fM[9];
};

void GrMatrix::set(int idx, GrScalar value) {
    GrAssert((unsigned)idx < 9);
    fM[idx] = value;
    if (idx > 5) {
        if (0 != fM[kPersp0] || 0 != fM[kPersp1] ||
            gRESCALE != fM[kPersp2]) {
            fTypeMask |= kPerspective_TypeBit;
        } else {
            fTypeMask &= ~kPerspective_TypeBit;
        }
    } else if (!(idx % 4)) {
        if ((GR_Scalar1 == fM[kScaleX] && GR_Scalar1 == fM[kScaleY])) {
            fTypeMask &= ~kScale_TypeBit;
            fTypeMask &= ~kZeroScale_TypeBit;
        } else {
            fTypeMask |= kScale_TypeBit;
            if ((0 == fM[kScaleX] && 0 == fM[kScaleY])) {
                fTypeMask |= kZeroScale_TypeBit;
            } else {
                fTypeMask &= ~kZeroScale_TypeBit;
            }
        }
    } else if (2 == (idx % 3)) {
        if (0 != fM[kTransX] || 0 != fM[kTransY]) {
            fTypeMask |= kTranslate_TypeBit;
        } else {
            fTypeMask &= ~kTranslate_TypeBit;
        }
    } else {
        if (0 != fM[kSkewX] || 0 != fM[kSkewY]) {
            fTypeMask |= kSkew_TypeBit;
        } else {
            fTypeMask &= ~kSkew_TypeBit;
        }
    }
}

#endif
