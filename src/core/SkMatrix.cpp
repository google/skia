/* libs/corecg/SkMatrix.cpp
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include "SkMatrix.h"
#include "Sk64.h"
#include "SkFloatBits.h"
#include "SkScalarCompare.h"
#include "SkString.h"

#ifdef SK_SCALAR_IS_FLOAT
    #define kMatrix22Elem   SK_Scalar1

    static inline float SkDoubleToFloat(double x) {
        return static_cast<float>(x);
    }
#else
    #define kMatrix22Elem   SK_Fract1
#endif

/*      [scale-x    skew-x      trans-x]   [X]   [X']
        [skew-y     scale-y     trans-y] * [Y] = [Y']
        [persp-0    persp-1     persp-2]   [1]   [1 ]
*/

void SkMatrix::reset() {
    fMat[kMScaleX] = fMat[kMScaleY] = SK_Scalar1;
    fMat[kMSkewX]  = fMat[kMSkewY] = 
    fMat[kMTransX] = fMat[kMTransY] =
    fMat[kMPersp0] = fMat[kMPersp1] = 0;
    fMat[kMPersp2] = kMatrix22Elem;

    this->setTypeMask(kIdentity_Mask | kRectStaysRect_Mask);
}

// this guy aligns with the masks, so we can compute a mask from a varaible 0/1
enum {
    kTranslate_Shift,
    kScale_Shift,
    kAffine_Shift,
    kPerspective_Shift,
    kRectStaysRect_Shift
};

#ifdef SK_SCALAR_IS_FLOAT
    static const int32_t kScalar1Int = 0x3f800000;
    static const int32_t kPersp1Int  = 0x3f800000;
#else
    #define scalarAsInt(x)  (x)
    static const int32_t kScalar1Int = (1 << 16);
    static const int32_t kPersp1Int  = (1 << 30);
#endif

uint8_t SkMatrix::computeTypeMask() const {
    unsigned mask = 0;

#ifdef SK_SCALAR_SLOW_COMPARES
    if (SkScalarAs2sCompliment(fMat[kMPersp0]) |
            SkScalarAs2sCompliment(fMat[kMPersp1]) |
            (SkScalarAs2sCompliment(fMat[kMPersp2]) - kPersp1Int)) {
        mask |= kPerspective_Mask;
    }

    if (SkScalarAs2sCompliment(fMat[kMTransX]) |
            SkScalarAs2sCompliment(fMat[kMTransY])) {
        mask |= kTranslate_Mask;
    }
#else
    // Benchmarking suggests that replacing this set of SkScalarAs2sCompliment
    // is a win, but replacing those below is not. We don't yet understand
    // that result.
    if (fMat[kMPersp0] != 0 || fMat[kMPersp1] != 0 ||
        fMat[kMPersp2] != kMatrix22Elem) {
        mask |= kPerspective_Mask;
    }

    if (fMat[kMTransX] != 0 || fMat[kMTransY] != 0) {
        mask |= kTranslate_Mask;
    }
#endif

    int m00 = SkScalarAs2sCompliment(fMat[SkMatrix::kMScaleX]);
    int m01 = SkScalarAs2sCompliment(fMat[SkMatrix::kMSkewX]);
    int m10 = SkScalarAs2sCompliment(fMat[SkMatrix::kMSkewY]);
    int m11 = SkScalarAs2sCompliment(fMat[SkMatrix::kMScaleY]);

    if (m01 | m10) {
        mask |= kAffine_Mask;
    }

    if ((m00 - kScalar1Int) | (m11 - kScalar1Int)) {
        mask |= kScale_Mask;
    }

    if ((mask & kPerspective_Mask) == 0) {
        // map non-zero to 1
        m00 = m00 != 0;
        m01 = m01 != 0;
        m10 = m10 != 0;
        m11 = m11 != 0;

        // record if the (p)rimary and (s)econdary diagonals are all 0 or
        // all non-zero (answer is 0 or 1)
        int dp0 = (m00 | m11) ^ 1;  // true if both are 0
        int dp1 = m00 & m11;        // true if both are 1
        int ds0 = (m01 | m10) ^ 1;  // true if both are 0
        int ds1 = m01 & m10;        // true if both are 1

        // return 1 if primary is 1 and secondary is 0 or
        // primary is 0 and secondary is 1
        mask |= ((dp0 & ds1) | (dp1 & ds0)) << kRectStaysRect_Shift;
    }

    return SkToU8(mask);
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_SCALAR_IS_FLOAT

bool operator==(const SkMatrix& a, const SkMatrix& b) {
    const SkScalar* SK_RESTRICT ma = a.fMat;
    const SkScalar* SK_RESTRICT mb = b.fMat;

    return  ma[0] == mb[0] && ma[1] == mb[1] && ma[2] == mb[2] &&
            ma[3] == mb[3] && ma[4] == mb[4] && ma[5] == mb[5] &&
            ma[6] == mb[6] && ma[7] == mb[7] && ma[8] == mb[8];
}

#endif

///////////////////////////////////////////////////////////////////////////////

void SkMatrix::setTranslate(SkScalar dx, SkScalar dy) {
    if (SkScalarToCompareType(dx) || SkScalarToCompareType(dy)) {
        fMat[kMTransX] = dx;
        fMat[kMTransY] = dy;

        fMat[kMScaleX] = fMat[kMScaleY] = SK_Scalar1;
        fMat[kMSkewX]  = fMat[kMSkewY] = 
        fMat[kMPersp0] = fMat[kMPersp1] = 0;
        fMat[kMPersp2] = kMatrix22Elem;

        this->setTypeMask(kTranslate_Mask | kRectStaysRect_Mask);
    } else {
        this->reset();
    }
}

bool SkMatrix::preTranslate(SkScalar dx, SkScalar dy) {
    if (this->hasPerspective()) {
        SkMatrix    m;
        m.setTranslate(dx, dy);
        return this->preConcat(m);
    }
    
    if (SkScalarToCompareType(dx) || SkScalarToCompareType(dy)) {
        fMat[kMTransX] += SkScalarMul(fMat[kMScaleX], dx) +
                          SkScalarMul(fMat[kMSkewX], dy);
        fMat[kMTransY] += SkScalarMul(fMat[kMSkewY], dx) +
                          SkScalarMul(fMat[kMScaleY], dy);

        this->setTypeMask(kUnknown_Mask);
    }
    return true;
}

bool SkMatrix::postTranslate(SkScalar dx, SkScalar dy) {
    if (this->hasPerspective()) {
        SkMatrix    m;
        m.setTranslate(dx, dy);
        return this->postConcat(m);
    }
    
    if (SkScalarToCompareType(dx) || SkScalarToCompareType(dy)) {
        fMat[kMTransX] += dx;
        fMat[kMTransY] += dy;
        this->setTypeMask(kUnknown_Mask);
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void SkMatrix::setScale(SkScalar sx, SkScalar sy, SkScalar px, SkScalar py) {
    fMat[kMScaleX] = sx;
    fMat[kMScaleY] = sy;
    fMat[kMTransX] = px - SkScalarMul(sx, px);
    fMat[kMTransY] = py - SkScalarMul(sy, py);
    fMat[kMPersp2] = kMatrix22Elem;

    fMat[kMSkewX]  = fMat[kMSkewY] = 
    fMat[kMPersp0] = fMat[kMPersp1] = 0;
    
    this->setTypeMask(kScale_Mask | kTranslate_Mask | kRectStaysRect_Mask);
}

void SkMatrix::setScale(SkScalar sx, SkScalar sy) {
    fMat[kMScaleX] = sx;
    fMat[kMScaleY] = sy;
    fMat[kMPersp2] = kMatrix22Elem;

    fMat[kMTransX] = fMat[kMTransY] =
    fMat[kMSkewX]  = fMat[kMSkewY] = 
    fMat[kMPersp0] = fMat[kMPersp1] = 0;

    this->setTypeMask(kScale_Mask | kRectStaysRect_Mask);
}

bool SkMatrix::preScale(SkScalar sx, SkScalar sy, SkScalar px, SkScalar py) {
    SkMatrix    m;
    m.setScale(sx, sy, px, py);
    return this->preConcat(m);
}

bool SkMatrix::preScale(SkScalar sx, SkScalar sy) {
#ifdef SK_SCALAR_IS_FIXED
    SkMatrix    m;
    m.setScale(sx, sy);
    return this->preConcat(m);
#else
    // the assumption is that these multiplies are very cheap, and that
    // a full concat and/or just computing the matrix type is more expensive.
    // Also, the fixed-point case checks for overflow, but the float doesn't,
    // so we can get away with these blind multiplies.

    fMat[kMScaleX] = SkScalarMul(fMat[kMScaleX], sx);
    fMat[kMSkewY] = SkScalarMul(fMat[kMSkewY],   sx);
    fMat[kMPersp0] = SkScalarMul(fMat[kMPersp0], sx);

    fMat[kMSkewX] = SkScalarMul(fMat[kMSkewX],   sy);
    fMat[kMScaleY] = SkScalarMul(fMat[kMScaleY], sy);
    fMat[kMPersp1] = SkScalarMul(fMat[kMPersp1], sy);

    this->orTypeMask(kScale_Mask);
    return true;
#endif
}

bool SkMatrix::postScale(SkScalar sx, SkScalar sy, SkScalar px, SkScalar py) {
    SkMatrix    m;
    m.setScale(sx, sy, px, py);
    return this->postConcat(m);
}

bool SkMatrix::postScale(SkScalar sx, SkScalar sy) {
    SkMatrix    m;
    m.setScale(sx, sy);
    return this->postConcat(m);
}

#ifdef SK_SCALAR_IS_FIXED
    static inline SkFixed roundidiv(SkFixed numer, int denom) {
        int ns = numer >> 31;
        int ds = denom >> 31;
        numer = (numer ^ ns) - ns;
        denom = (denom ^ ds) - ds;
        
        SkFixed answer = (numer + (denom >> 1)) / denom;
        int as = ns ^ ds;
        return (answer ^ as) - as;
    }
#endif

// this guy perhaps can go away, if we have a fract/high-precision way to
// scale matrices
bool SkMatrix::postIDiv(int divx, int divy) {
    if (divx == 0 || divy == 0) {
        return false;
    }

#ifdef SK_SCALAR_IS_FIXED
    fMat[kMScaleX] = roundidiv(fMat[kMScaleX], divx);
    fMat[kMSkewX]  = roundidiv(fMat[kMSkewX],  divx);
    fMat[kMTransX] = roundidiv(fMat[kMTransX], divx);

    fMat[kMScaleY] = roundidiv(fMat[kMScaleY], divy);
    fMat[kMSkewY]  = roundidiv(fMat[kMSkewY],  divy);
    fMat[kMTransY] = roundidiv(fMat[kMTransY], divy);
#else
    const float invX = 1.f / divx;
    const float invY = 1.f / divy;

    fMat[kMScaleX] *= invX;
    fMat[kMSkewX]  *= invX;
    fMat[kMTransX] *= invX;
    
    fMat[kMScaleY] *= invY;
    fMat[kMSkewY]  *= invY;
    fMat[kMTransY] *= invY;
#endif

    this->setTypeMask(kUnknown_Mask);
    return true;
}

////////////////////////////////////////////////////////////////////////////////////

void SkMatrix::setSinCos(SkScalar sinV, SkScalar cosV,
                         SkScalar px, SkScalar py) {
    const SkScalar oneMinusCosV = SK_Scalar1 - cosV;

    fMat[kMScaleX]  = cosV;
    fMat[kMSkewX]   = -sinV;
    fMat[kMTransX]  = SkScalarMul(sinV, py) + SkScalarMul(oneMinusCosV, px);

    fMat[kMSkewY]   = sinV;
    fMat[kMScaleY]  = cosV;
    fMat[kMTransY]  = SkScalarMul(-sinV, px) + SkScalarMul(oneMinusCosV, py);

    fMat[kMPersp0] = fMat[kMPersp1] = 0;
    fMat[kMPersp2] = kMatrix22Elem;
    
    this->setTypeMask(kUnknown_Mask);
}

void SkMatrix::setSinCos(SkScalar sinV, SkScalar cosV) {
    fMat[kMScaleX]  = cosV;
    fMat[kMSkewX]   = -sinV;
    fMat[kMTransX]  = 0;

    fMat[kMSkewY]   = sinV;
    fMat[kMScaleY]  = cosV;
    fMat[kMTransY]  = 0;

    fMat[kMPersp0] = fMat[kMPersp1] = 0;
    fMat[kMPersp2] = kMatrix22Elem;

    this->setTypeMask(kUnknown_Mask);
}

void SkMatrix::setRotate(SkScalar degrees, SkScalar px, SkScalar py) {
    SkScalar sinV, cosV;
    sinV = SkScalarSinCos(SkDegreesToRadians(degrees), &cosV);
    this->setSinCos(sinV, cosV, px, py);
}

void SkMatrix::setRotate(SkScalar degrees) {
    SkScalar sinV, cosV;
    sinV = SkScalarSinCos(SkDegreesToRadians(degrees), &cosV);
    this->setSinCos(sinV, cosV);
}

bool SkMatrix::preRotate(SkScalar degrees, SkScalar px, SkScalar py) {
    SkMatrix    m;
    m.setRotate(degrees, px, py);
    return this->preConcat(m);
}

bool SkMatrix::preRotate(SkScalar degrees) {
    SkMatrix    m;
    m.setRotate(degrees);
    return this->preConcat(m);
}

bool SkMatrix::postRotate(SkScalar degrees, SkScalar px, SkScalar py) {
    SkMatrix    m;
    m.setRotate(degrees, px, py);
    return this->postConcat(m);
}

bool SkMatrix::postRotate(SkScalar degrees) {
    SkMatrix    m;
    m.setRotate(degrees);
    return this->postConcat(m);
}

////////////////////////////////////////////////////////////////////////////////////

void SkMatrix::setSkew(SkScalar sx, SkScalar sy, SkScalar px, SkScalar py) {
    fMat[kMScaleX]  = SK_Scalar1;
    fMat[kMSkewX]   = sx;
    fMat[kMTransX]  = SkScalarMul(-sx, py);

    fMat[kMSkewY]   = sy;
    fMat[kMScaleY]  = SK_Scalar1;
    fMat[kMTransY]  = SkScalarMul(-sy, px);

    fMat[kMPersp0] = fMat[kMPersp1] = 0;
    fMat[kMPersp2] = kMatrix22Elem;

    this->setTypeMask(kUnknown_Mask);
}

void SkMatrix::setSkew(SkScalar sx, SkScalar sy) {
    fMat[kMScaleX]  = SK_Scalar1;
    fMat[kMSkewX]   = sx;
    fMat[kMTransX]  = 0;

    fMat[kMSkewY]   = sy;
    fMat[kMScaleY]  = SK_Scalar1;
    fMat[kMTransY]  = 0;

    fMat[kMPersp0] = fMat[kMPersp1] = 0;
    fMat[kMPersp2] = kMatrix22Elem;

    this->setTypeMask(kUnknown_Mask);
}

bool SkMatrix::preSkew(SkScalar sx, SkScalar sy, SkScalar px, SkScalar py) {
    SkMatrix    m;
    m.setSkew(sx, sy, px, py);
    return this->preConcat(m);
}

bool SkMatrix::preSkew(SkScalar sx, SkScalar sy) {
    SkMatrix    m;
    m.setSkew(sx, sy);
    return this->preConcat(m);
}

bool SkMatrix::postSkew(SkScalar sx, SkScalar sy, SkScalar px, SkScalar py) {
    SkMatrix    m;
    m.setSkew(sx, sy, px, py);
    return this->postConcat(m);
}

bool SkMatrix::postSkew(SkScalar sx, SkScalar sy) {
    SkMatrix    m;
    m.setSkew(sx, sy);
    return this->postConcat(m);
}

///////////////////////////////////////////////////////////////////////////////

bool SkMatrix::setRectToRect(const SkRect& src, const SkRect& dst,
                             ScaleToFit align)
{
    if (src.isEmpty()) {
        this->reset();
        return false;
    }

    if (dst.isEmpty()) {
        sk_bzero(fMat, 8 * sizeof(SkScalar));
        this->setTypeMask(kScale_Mask | kRectStaysRect_Mask);
    } else {
        SkScalar    tx, sx = SkScalarDiv(dst.width(), src.width());
        SkScalar    ty, sy = SkScalarDiv(dst.height(), src.height());
        bool        xLarger = false;

        if (align != kFill_ScaleToFit) {
            if (sx > sy) {
                xLarger = true;
                sx = sy;
            } else {
                sy = sx;
            }
        }

        tx = dst.fLeft - SkScalarMul(src.fLeft, sx);
        ty = dst.fTop - SkScalarMul(src.fTop, sy);
        if (align == kCenter_ScaleToFit || align == kEnd_ScaleToFit) {
            SkScalar diff;

            if (xLarger) {
                diff = dst.width() - SkScalarMul(src.width(), sy);
            } else {
                diff = dst.height() - SkScalarMul(src.height(), sy);
            }
            
            if (align == kCenter_ScaleToFit) {
                diff = SkScalarHalf(diff);
            }

            if (xLarger) {
                tx += diff;
            } else {
                ty += diff;
            }
        }

        fMat[kMScaleX] = sx;
        fMat[kMScaleY] = sy;
        fMat[kMTransX] = tx;
        fMat[kMTransY] = ty;
        fMat[kMSkewX]  = fMat[kMSkewY] = 
        fMat[kMPersp0] = fMat[kMPersp1] = 0;

        this->setTypeMask(kScale_Mask | kTranslate_Mask | kRectStaysRect_Mask);
    }
    // shared cleanup
    fMat[kMPersp2] = kMatrix22Elem;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_SCALAR_IS_FLOAT
    static inline int fixmuladdmul(float a, float b, float c, float d,
                                   float* result) {
        *result = SkDoubleToFloat((double)a * b + (double)c * d);
        return true;
    }

    static inline bool rowcol3(const float row[], const float col[],
                               float* result) {
        *result = row[0] * col[0] + row[1] * col[3] + row[2] * col[6];
        return true;
    }

    static inline int negifaddoverflows(float& result, float a, float b) {
        result = a + b;
        return 0;
    }
#else
    static inline bool fixmuladdmul(SkFixed a, SkFixed b, SkFixed c, SkFixed d,
                                    SkFixed* result) {
        Sk64    tmp1, tmp2;
        tmp1.setMul(a, b);
        tmp2.setMul(c, d);
        tmp1.add(tmp2);
        if (tmp1.isFixed()) {
            *result = tmp1.getFixed();
            return true;
        }
        return false;
    }

    static inline SkFixed fracmuladdmul(SkFixed a, SkFract b, SkFixed c,
                                        SkFract d) {
        Sk64 tmp1, tmp2;
        tmp1.setMul(a, b);
        tmp2.setMul(c, d);
        tmp1.add(tmp2);
        return tmp1.getFract();
    }

    static inline bool rowcol3(const SkFixed row[], const SkFixed col[],
                               SkFixed* result) {
        Sk64 tmp1, tmp2;

        tmp1.setMul(row[0], col[0]);    // N * fixed
        tmp2.setMul(row[1], col[3]);    // N * fixed
        tmp1.add(tmp2);

        tmp2.setMul(row[2], col[6]);    // N * fract
        tmp2.roundRight(14);            // make it fixed
        tmp1.add(tmp2);

        if (tmp1.isFixed()) {
            *result = tmp1.getFixed();
            return true;
        }
        return false;
    }

    static inline int negifaddoverflows(SkFixed& result, SkFixed a, SkFixed b) {
        SkFixed c = a + b;
        result = c;
        return (c ^ a) & (c ^ b);
    }
#endif

static void normalize_perspective(SkScalar mat[9]) {
    if (SkScalarAbs(mat[SkMatrix::kMPersp2]) > kMatrix22Elem) {
        for (int i = 0; i < 9; i++)
            mat[i] = SkScalarHalf(mat[i]);
    }
}

bool SkMatrix::setConcat(const SkMatrix& a, const SkMatrix& b) {
    TypeMask aType = a.getType();
    TypeMask bType = b.getType();

    if (0 == aType) {
        *this = b;
    } else if (0 == bType) {
        *this = a;
    } else {
        SkMatrix tmp;

        if ((aType | bType) & kPerspective_Mask) {
            if (!rowcol3(&a.fMat[0], &b.fMat[0], &tmp.fMat[kMScaleX])) {
                return false;
            }
            if (!rowcol3(&a.fMat[0], &b.fMat[1], &tmp.fMat[kMSkewX])) {
                return false;
            }
            if (!rowcol3(&a.fMat[0], &b.fMat[2], &tmp.fMat[kMTransX])) {
                return false;
            }

            if (!rowcol3(&a.fMat[3], &b.fMat[0], &tmp.fMat[kMSkewY])) {
                return false;
            }
            if (!rowcol3(&a.fMat[3], &b.fMat[1], &tmp.fMat[kMScaleY])) {
                return false;
            }
            if (!rowcol3(&a.fMat[3], &b.fMat[2], &tmp.fMat[kMTransY])) {
                return false;
            }

            if (!rowcol3(&a.fMat[6], &b.fMat[0], &tmp.fMat[kMPersp0])) {
                return false;
            }
            if (!rowcol3(&a.fMat[6], &b.fMat[1], &tmp.fMat[kMPersp1])) {
                return false;
            }
            if (!rowcol3(&a.fMat[6], &b.fMat[2], &tmp.fMat[kMPersp2])) {
                return false;
            }

            normalize_perspective(tmp.fMat);
        } else {    // not perspective
            if (!fixmuladdmul(a.fMat[kMScaleX], b.fMat[kMScaleX],
                    a.fMat[kMSkewX], b.fMat[kMSkewY], &tmp.fMat[kMScaleX])) {
                return false;
            }
            if (!fixmuladdmul(a.fMat[kMScaleX], b.fMat[kMSkewX],
                      a.fMat[kMSkewX], b.fMat[kMScaleY], &tmp.fMat[kMSkewX])) {
                return false;
            }
            if (!fixmuladdmul(a.fMat[kMScaleX], b.fMat[kMTransX],
                      a.fMat[kMSkewX], b.fMat[kMTransY], &tmp.fMat[kMTransX])) {
                return false;
            }
            if (negifaddoverflows(tmp.fMat[kMTransX], tmp.fMat[kMTransX],
                                  a.fMat[kMTransX]) < 0) {
                return false;
            }

            if (!fixmuladdmul(a.fMat[kMSkewY], b.fMat[kMScaleX],
                      a.fMat[kMScaleY], b.fMat[kMSkewY], &tmp.fMat[kMSkewY])) {
                return false;
            }
            if (!fixmuladdmul(a.fMat[kMSkewY], b.fMat[kMSkewX],
                    a.fMat[kMScaleY], b.fMat[kMScaleY], &tmp.fMat[kMScaleY])) {
                return false;
            }
            if (!fixmuladdmul(a.fMat[kMSkewY], b.fMat[kMTransX],
                     a.fMat[kMScaleY], b.fMat[kMTransY], &tmp.fMat[kMTransY])) {
                return false;
            }
            if (negifaddoverflows(tmp.fMat[kMTransY], tmp.fMat[kMTransY],
                                  a.fMat[kMTransY]) < 0) {
                return false;
            }

            tmp.fMat[kMPersp0] = tmp.fMat[kMPersp1] = 0;
            tmp.fMat[kMPersp2] = kMatrix22Elem;
        }
        *this = tmp;
    }
    this->setTypeMask(kUnknown_Mask);
    return true;
}

bool SkMatrix::preConcat(const SkMatrix& mat) {
    // check for identity first, so we don't do a needless copy of ourselves
    // to ourselves inside setConcat()
    return mat.isIdentity() || this->setConcat(*this, mat);
}

bool SkMatrix::postConcat(const SkMatrix& mat) {
    // check for identity first, so we don't do a needless copy of ourselves
    // to ourselves inside setConcat()
    return mat.isIdentity() || this->setConcat(mat, *this);
}

///////////////////////////////////////////////////////////////////////////////

/*  Matrix inversion is very expensive, but also the place where keeping
    precision may be most important (here and matrix concat). Hence to avoid
    bitmap blitting artifacts when walking the inverse, we use doubles for
    the intermediate math, even though we know that is more expensive.
    The fixed counter part is us using Sk64 for temp calculations.
 */

#ifdef SK_SCALAR_IS_FLOAT
    typedef double SkDetScalar;
    #define SkPerspMul(a, b)            SkScalarMul(a, b)
    #define SkScalarMulShift(a, b, s)   SkDoubleToFloat((a) * (b))
    static double sk_inv_determinant(const float mat[9], int isPerspective,
                                    int* /* (only used in Fixed case) */) {
        double det;

        if (isPerspective) {
            det =   mat[SkMatrix::kMScaleX] * ((double)mat[SkMatrix::kMScaleY] * mat[SkMatrix::kMPersp2] - (double)mat[SkMatrix::kMTransY] * mat[SkMatrix::kMPersp1]) +
                    mat[SkMatrix::kMSkewX] * ((double)mat[SkMatrix::kMTransY] * mat[SkMatrix::kMPersp0] - (double)mat[SkMatrix::kMSkewY] * mat[SkMatrix::kMPersp2]) +
                    mat[SkMatrix::kMTransX] * ((double)mat[SkMatrix::kMSkewY] * mat[SkMatrix::kMPersp1] - (double)mat[SkMatrix::kMScaleY] * mat[SkMatrix::kMPersp0]);
        } else {
            det =   (double)mat[SkMatrix::kMScaleX] * mat[SkMatrix::kMScaleY] - (double)mat[SkMatrix::kMSkewX] * mat[SkMatrix::kMSkewY];
        }

        // Since the determinant is on the order of the cube of the matrix members,
        // compare to the cube of the default nearly-zero constant (although an
        // estimate of the condition number would be better if it wasn't so expensive).
        if (SkScalarNearlyZero((float)det, SK_ScalarNearlyZero * SK_ScalarNearlyZero * SK_ScalarNearlyZero)) {
            return 0;
        }
        return 1.0 / det;
    }
    // we declar a,b,c,d to all be doubles, because we want to perform
    // double-precision muls and subtract, even though the original values are
    // from the matrix, which are floats.
    static float inline mul_diff_scale(double a, double b, double c, double d,
                                       double scale) {
        return SkDoubleToFloat((a * b - c * d) * scale);
    }
#else
    typedef SkFixed SkDetScalar;
    #define SkPerspMul(a, b)            SkFractMul(a, b)
    #define SkScalarMulShift(a, b, s)   SkMulShift(a, b, s)
    static void set_muladdmul(Sk64* dst, int32_t a, int32_t b, int32_t c,
                              int32_t d) {
        Sk64 tmp;
        dst->setMul(a, b);
        tmp.setMul(c, d);
        dst->add(tmp);
    }

    static SkFixed sk_inv_determinant(const SkFixed mat[9], int isPerspective,
                                      int* shift) {
        Sk64    tmp1, tmp2;

        if (isPerspective) {
            tmp1.setMul(mat[SkMatrix::kMScaleX], fracmuladdmul(mat[SkMatrix::kMScaleY], mat[SkMatrix::kMPersp2], -mat[SkMatrix::kMTransY], mat[SkMatrix::kMPersp1]));
            tmp2.setMul(mat[SkMatrix::kMSkewX], fracmuladdmul(mat[SkMatrix::kMTransY], mat[SkMatrix::kMPersp0], -mat[SkMatrix::kMSkewY], mat[SkMatrix::kMPersp2]));
            tmp1.add(tmp2);
            tmp2.setMul(mat[SkMatrix::kMTransX], fracmuladdmul(mat[SkMatrix::kMSkewY], mat[SkMatrix::kMPersp1], -mat[SkMatrix::kMScaleY], mat[SkMatrix::kMPersp0]));
            tmp1.add(tmp2);
        } else {
            tmp1.setMul(mat[SkMatrix::kMScaleX], mat[SkMatrix::kMScaleY]);
            tmp2.setMul(mat[SkMatrix::kMSkewX], mat[SkMatrix::kMSkewY]);
            tmp1.sub(tmp2);
        }

        int s = tmp1.getClzAbs();
        *shift = s;

        SkFixed denom;
        if (s <= 32) {
            denom = tmp1.getShiftRight(33 - s);
        } else {
            denom = (int32_t)tmp1.fLo << (s - 33);
        }

        if (denom == 0) {
            return 0;
        }
        /** This could perhaps be a special fractdiv function, since both of its
            arguments are known to have bit 31 clear and bit 30 set (when they
            are made positive), thus eliminating the need for calling clz()
        */
        return SkFractDiv(SK_Fract1, denom);
    }
#endif

bool SkMatrix::pdfTransform(SkScalar transform[6]) const {
    SkMatrix identity;
    const SkMatrix* use = this;
    bool ret = true;
    if (this->hasPerspective()) {
        identity.reset();
        use = &identity;
        ret = false;
    }
    transform[0] = use->fMat[kMScaleX];
    transform[1] = use->fMat[kMSkewY];
    transform[2] = use->fMat[kMSkewX];
    transform[3] = use->fMat[kMScaleY];
    transform[4] = use->fMat[kMTransX];
    transform[5] = use->fMat[kMTransY];
    return true;
}

bool SkMatrix::invert(SkMatrix* inv) const {
    int         isPersp = this->hasPerspective();
    int         shift;
    SkDetScalar scale = sk_inv_determinant(fMat, isPersp, &shift);

    if (scale == 0) { // underflow
        return false;
    }

    if (inv) {
        SkMatrix tmp;
        if (inv == this)
            inv = &tmp;

        if (isPersp) {
            shift = 61 - shift;
            inv->fMat[kMScaleX] = SkScalarMulShift(SkPerspMul(fMat[kMScaleY], fMat[kMPersp2]) - SkPerspMul(fMat[kMTransY], fMat[kMPersp1]), scale, shift);
            inv->fMat[kMSkewX]  = SkScalarMulShift(SkPerspMul(fMat[kMTransX], fMat[kMPersp1]) - SkPerspMul(fMat[kMSkewX],  fMat[kMPersp2]), scale, shift);
            inv->fMat[kMTransX] = SkScalarMulShift(SkScalarMul(fMat[kMSkewX], fMat[kMTransY]) - SkScalarMul(fMat[kMTransX], fMat[kMScaleY]), scale, shift);

            inv->fMat[kMSkewY]  = SkScalarMulShift(SkPerspMul(fMat[kMTransY], fMat[kMPersp0]) - SkPerspMul(fMat[kMSkewY],   fMat[kMPersp2]), scale, shift);
            inv->fMat[kMScaleY] = SkScalarMulShift(SkPerspMul(fMat[kMScaleX], fMat[kMPersp2]) - SkPerspMul(fMat[kMTransX],  fMat[kMPersp0]), scale, shift);
            inv->fMat[kMTransY] = SkScalarMulShift(SkScalarMul(fMat[kMTransX], fMat[kMSkewY]) - SkScalarMul(fMat[kMScaleX], fMat[kMTransY]), scale, shift);

            inv->fMat[kMPersp0] = SkScalarMulShift(SkScalarMul(fMat[kMSkewY], fMat[kMPersp1]) - SkScalarMul(fMat[kMScaleY], fMat[kMPersp0]), scale, shift);             
            inv->fMat[kMPersp1] = SkScalarMulShift(SkScalarMul(fMat[kMSkewX], fMat[kMPersp0]) - SkScalarMul(fMat[kMScaleX], fMat[kMPersp1]), scale, shift);
            inv->fMat[kMPersp2] = SkScalarMulShift(SkScalarMul(fMat[kMScaleX], fMat[kMScaleY]) - SkScalarMul(fMat[kMSkewX], fMat[kMSkewY]), scale, shift);
#ifdef SK_SCALAR_IS_FIXED
            if (SkAbs32(inv->fMat[kMPersp2]) > SK_Fixed1) {
                Sk64    tmp;

                tmp.set(SK_Fract1);
                tmp.shiftLeft(16);
                tmp.div(inv->fMat[kMPersp2], Sk64::kRound_DivOption);

                SkFract scale = tmp.get32();

                for (int i = 0; i < 9; i++) {
                    inv->fMat[i] = SkFractMul(inv->fMat[i], scale);
                }
            }
            inv->fMat[kMPersp2] = SkFixedToFract(inv->fMat[kMPersp2]);
#endif
        } else {   // not perspective
#ifdef SK_SCALAR_IS_FIXED
            Sk64    tx, ty;
            int     clzNumer;

            // check the 2x2 for overflow
            {
                int32_t value = SkAbs32(fMat[kMScaleY]);
                value |= SkAbs32(fMat[kMSkewX]);
                value |= SkAbs32(fMat[kMScaleX]);
                value |= SkAbs32(fMat[kMSkewY]);
                clzNumer = SkCLZ(value);
                if (shift - clzNumer > 31)
                    return false;   // overflow
            }

            set_muladdmul(&tx, fMat[kMSkewX], fMat[kMTransY], -fMat[kMScaleY], fMat[kMTransX]);
            set_muladdmul(&ty, fMat[kMSkewY], fMat[kMTransX], -fMat[kMScaleX], fMat[kMTransY]);
            // check tx,ty for overflow
            clzNumer = SkCLZ(SkAbs32(tx.fHi) | SkAbs32(ty.fHi));
            if (shift - clzNumer > 14) {
                return false;   // overflow
            }

            int fixedShift = 61 - shift;
            int sk64shift = 44 - shift + clzNumer;

            inv->fMat[kMScaleX] = SkMulShift(fMat[kMScaleY], scale, fixedShift);
            inv->fMat[kMSkewX]  = SkMulShift(-fMat[kMSkewX], scale, fixedShift);
            inv->fMat[kMTransX] = SkMulShift(tx.getShiftRight(33 - clzNumer), scale, sk64shift);
                
            inv->fMat[kMSkewY]  = SkMulShift(-fMat[kMSkewY], scale, fixedShift);
            inv->fMat[kMScaleY] = SkMulShift(fMat[kMScaleX], scale, fixedShift);
            inv->fMat[kMTransY] = SkMulShift(ty.getShiftRight(33 - clzNumer), scale, sk64shift);
#else
            inv->fMat[kMScaleX] = SkDoubleToFloat(fMat[kMScaleY] * scale);
            inv->fMat[kMSkewX] = SkDoubleToFloat(-fMat[kMSkewX] * scale);
            inv->fMat[kMTransX] = mul_diff_scale(fMat[kMSkewX], fMat[kMTransY],
                                     fMat[kMScaleY], fMat[kMTransX], scale);
                
            inv->fMat[kMSkewY] = SkDoubleToFloat(-fMat[kMSkewY] * scale);
            inv->fMat[kMScaleY] = SkDoubleToFloat(fMat[kMScaleX] * scale);
            inv->fMat[kMTransY] = mul_diff_scale(fMat[kMSkewY], fMat[kMTransX],
                                        fMat[kMScaleX], fMat[kMTransY], scale);
#endif
            inv->fMat[kMPersp0] = 0;
            inv->fMat[kMPersp1] = 0;
            inv->fMat[kMPersp2] = kMatrix22Elem;
        }

        if (inv == &tmp) {
            *(SkMatrix*)this = tmp;
        }
        inv->setTypeMask(kUnknown_Mask);
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void SkMatrix::Identity_pts(const SkMatrix& m, SkPoint dst[],
                            const SkPoint src[], int count) {
    SkASSERT(m.getType() == 0);

    if (dst != src && count > 0)
        memcpy(dst, src, count * sizeof(SkPoint));
}

void SkMatrix::Trans_pts(const SkMatrix& m, SkPoint dst[],
                         const SkPoint src[], int count) {
    SkASSERT(m.getType() == kTranslate_Mask);

    if (count > 0) {
        SkScalar tx = m.fMat[kMTransX];
        SkScalar ty = m.fMat[kMTransY];
        do {
            dst->fY = src->fY + ty;
            dst->fX = src->fX + tx;
            src += 1;
            dst += 1;
        } while (--count);
    }
}

void SkMatrix::Scale_pts(const SkMatrix& m, SkPoint dst[],
                         const SkPoint src[], int count) {
    SkASSERT(m.getType() == kScale_Mask);

    if (count > 0) {
        SkScalar mx = m.fMat[kMScaleX];
        SkScalar my = m.fMat[kMScaleY];
        do {
            dst->fY = SkScalarMul(src->fY, my);
            dst->fX = SkScalarMul(src->fX, mx);
            src += 1;
            dst += 1;
        } while (--count);
    }
}

void SkMatrix::ScaleTrans_pts(const SkMatrix& m, SkPoint dst[],
                              const SkPoint src[], int count) {
    SkASSERT(m.getType() == (kScale_Mask | kTranslate_Mask));

    if (count > 0) {
        SkScalar mx = m.fMat[kMScaleX];
        SkScalar my = m.fMat[kMScaleY];
        SkScalar tx = m.fMat[kMTransX];
        SkScalar ty = m.fMat[kMTransY];
        do {
            dst->fY = SkScalarMulAdd(src->fY, my, ty);
            dst->fX = SkScalarMulAdd(src->fX, mx, tx);
            src += 1;
            dst += 1;
        } while (--count);
    }
}

void SkMatrix::Rot_pts(const SkMatrix& m, SkPoint dst[],
                       const SkPoint src[], int count) {
    SkASSERT((m.getType() & (kPerspective_Mask | kTranslate_Mask)) == 0);

    if (count > 0) {
        SkScalar mx = m.fMat[kMScaleX];
        SkScalar my = m.fMat[kMScaleY];
        SkScalar kx = m.fMat[kMSkewX];
        SkScalar ky = m.fMat[kMSkewY];
        do {
            SkScalar sy = src->fY;
            SkScalar sx = src->fX;
            src += 1;
            dst->fY = SkScalarMul(sx, ky) + SkScalarMul(sy, my);
            dst->fX = SkScalarMul(sx, mx) + SkScalarMul(sy, kx);
            dst += 1;
        } while (--count);
    }
}

void SkMatrix::RotTrans_pts(const SkMatrix& m, SkPoint dst[],
                            const SkPoint src[], int count) {
    SkASSERT(!m.hasPerspective());

    if (count > 0) {
        SkScalar mx = m.fMat[kMScaleX];
        SkScalar my = m.fMat[kMScaleY];
        SkScalar kx = m.fMat[kMSkewX];
        SkScalar ky = m.fMat[kMSkewY];
        SkScalar tx = m.fMat[kMTransX];
        SkScalar ty = m.fMat[kMTransY];
        do {
            SkScalar sy = src->fY;
            SkScalar sx = src->fX;
            src += 1;
            dst->fY = SkScalarMul(sx, ky) + SkScalarMulAdd(sy, my, ty);
            dst->fX = SkScalarMul(sx, mx) + SkScalarMulAdd(sy, kx, tx);
            dst += 1;
        } while (--count);
    }
}

void SkMatrix::Persp_pts(const SkMatrix& m, SkPoint dst[],
                         const SkPoint src[], int count) {
    SkASSERT(m.hasPerspective());

#ifdef SK_SCALAR_IS_FIXED
    SkFixed persp2 = SkFractToFixed(m.fMat[kMPersp2]);
#endif

    if (count > 0) {
        do {
            SkScalar sy = src->fY;
            SkScalar sx = src->fX;
            src += 1;

            SkScalar x = SkScalarMul(sx, m.fMat[kMScaleX]) +
                         SkScalarMul(sy, m.fMat[kMSkewX]) + m.fMat[kMTransX];
            SkScalar y = SkScalarMul(sx, m.fMat[kMSkewY]) +
                         SkScalarMul(sy, m.fMat[kMScaleY]) + m.fMat[kMTransY];
#ifdef SK_SCALAR_IS_FIXED
            SkFixed z = SkFractMul(sx, m.fMat[kMPersp0]) +
                        SkFractMul(sy, m.fMat[kMPersp1]) + persp2;
#else
            float z = SkScalarMul(sx, m.fMat[kMPersp0]) +
                      SkScalarMulAdd(sy, m.fMat[kMPersp1], m.fMat[kMPersp2]);
#endif
            if (z) {
                z = SkScalarFastInvert(z);
            }

            dst->fY = SkScalarMul(y, z);
            dst->fX = SkScalarMul(x, z);
            dst += 1;
        } while (--count);
    }
}

const SkMatrix::MapPtsProc SkMatrix::gMapPtsProcs[] = {
    SkMatrix::Identity_pts, SkMatrix::Trans_pts,
    SkMatrix::Scale_pts,    SkMatrix::ScaleTrans_pts,
    SkMatrix::Rot_pts,      SkMatrix::RotTrans_pts,
    SkMatrix::Rot_pts,      SkMatrix::RotTrans_pts,
    // repeat the persp proc 8 times
    SkMatrix::Persp_pts,    SkMatrix::Persp_pts,
    SkMatrix::Persp_pts,    SkMatrix::Persp_pts,
    SkMatrix::Persp_pts,    SkMatrix::Persp_pts,
    SkMatrix::Persp_pts,    SkMatrix::Persp_pts
};

void SkMatrix::mapPoints(SkPoint dst[], const SkPoint src[], int count) const {
    SkASSERT((dst && src && count > 0) || count == 0);
    // no partial overlap
    SkASSERT(src == dst || SkAbs32((int32_t)(src - dst)) >= count);

    this->getMapPtsProc()(*this, dst, src, count);
}

///////////////////////////////////////////////////////////////////////////////

void SkMatrix::mapVectors(SkPoint dst[], const SkPoint src[], int count) const {
    if (this->hasPerspective()) {
        SkPoint origin;

        MapXYProc proc = this->getMapXYProc();
        proc(*this, 0, 0, &origin);

        for (int i = count - 1; i >= 0; --i) {
            SkPoint tmp;

            proc(*this, src[i].fX, src[i].fY, &tmp);
            dst[i].set(tmp.fX - origin.fX, tmp.fY - origin.fY);
        }
    } else {
        SkMatrix tmp = *this;

        tmp.fMat[kMTransX] = tmp.fMat[kMTransY] = 0;
        tmp.clearTypeMask(kTranslate_Mask);
        tmp.mapPoints(dst, src, count);
    }
}

bool SkMatrix::mapRect(SkRect* dst, const SkRect& src) const {
    SkASSERT(dst && &src);

    if (this->rectStaysRect()) {
        this->mapPoints((SkPoint*)dst, (const SkPoint*)&src, 2);
        dst->sort();
        return true;
    } else {
        SkPoint quad[4];

        src.toQuad(quad);
        this->mapPoints(quad, quad, 4);
        dst->set(quad, 4);
        return false;
    }
}

SkScalar SkMatrix::mapRadius(SkScalar radius) const {
    SkVector    vec[2];

    vec[0].set(radius, 0);
    vec[1].set(0, radius);
    this->mapVectors(vec, 2);

    SkScalar d0 = vec[0].length();
    SkScalar d1 = vec[1].length();

    return SkScalarMean(d0, d1);
}

///////////////////////////////////////////////////////////////////////////////

void SkMatrix::Persp_xy(const SkMatrix& m, SkScalar sx, SkScalar sy,
                        SkPoint* pt) {
    SkASSERT(m.hasPerspective());

    SkScalar x = SkScalarMul(sx, m.fMat[kMScaleX]) +
                 SkScalarMul(sy, m.fMat[kMSkewX]) + m.fMat[kMTransX];
    SkScalar y = SkScalarMul(sx, m.fMat[kMSkewY]) +
                 SkScalarMul(sy, m.fMat[kMScaleY]) + m.fMat[kMTransY];
#ifdef SK_SCALAR_IS_FIXED
    SkFixed z = SkFractMul(sx, m.fMat[kMPersp0]) +
                SkFractMul(sy, m.fMat[kMPersp1]) +
                SkFractToFixed(m.fMat[kMPersp2]);
#else
    float z = SkScalarMul(sx, m.fMat[kMPersp0]) +
              SkScalarMul(sy, m.fMat[kMPersp1]) + m.fMat[kMPersp2];
#endif
    if (z) {
        z = SkScalarFastInvert(z);
    }
    pt->fX = SkScalarMul(x, z);
    pt->fY = SkScalarMul(y, z);
}

#ifdef SK_SCALAR_IS_FIXED
static SkFixed fixmuladdmul(SkFixed a, SkFixed b, SkFixed c, SkFixed d) {
    Sk64    tmp, tmp1;

    tmp.setMul(a, b);
    tmp1.setMul(c, d);
    return tmp.addGetFixed(tmp1);
//  tmp.add(tmp1);
//  return tmp.getFixed();
}
#endif

void SkMatrix::RotTrans_xy(const SkMatrix& m, SkScalar sx, SkScalar sy,
                           SkPoint* pt) {
    SkASSERT((m.getType() & (kAffine_Mask | kPerspective_Mask)) == kAffine_Mask);
    
#ifdef SK_SCALAR_IS_FIXED
    pt->fX = fixmuladdmul(sx, m.fMat[kMScaleX], sy, m.fMat[kMSkewX]) +
             m.fMat[kMTransX];
    pt->fY = fixmuladdmul(sx, m.fMat[kMSkewY], sy, m.fMat[kMScaleY]) +
             m.fMat[kMTransY];
#else
    pt->fX = SkScalarMul(sx, m.fMat[kMScaleX]) +
             SkScalarMulAdd(sy, m.fMat[kMSkewX], m.fMat[kMTransX]);
    pt->fY = SkScalarMul(sx, m.fMat[kMSkewY]) +
             SkScalarMulAdd(sy, m.fMat[kMScaleY], m.fMat[kMTransY]);
#endif
}

void SkMatrix::Rot_xy(const SkMatrix& m, SkScalar sx, SkScalar sy,
                      SkPoint* pt) {
    SkASSERT((m.getType() & (kAffine_Mask | kPerspective_Mask))== kAffine_Mask);
    SkASSERT(0 == m.fMat[kMTransX]);
    SkASSERT(0 == m.fMat[kMTransY]);

#ifdef SK_SCALAR_IS_FIXED
    pt->fX = fixmuladdmul(sx, m.fMat[kMScaleX], sy, m.fMat[kMSkewX]);
    pt->fY = fixmuladdmul(sx, m.fMat[kMSkewY], sy, m.fMat[kMScaleY]);
#else
    pt->fX = SkScalarMul(sx, m.fMat[kMScaleX]) +
             SkScalarMulAdd(sy, m.fMat[kMSkewX], m.fMat[kMTransX]);
    pt->fY = SkScalarMul(sx, m.fMat[kMSkewY]) +
             SkScalarMulAdd(sy, m.fMat[kMScaleY], m.fMat[kMTransY]);
#endif
}

void SkMatrix::ScaleTrans_xy(const SkMatrix& m, SkScalar sx, SkScalar sy,
                             SkPoint* pt) {
    SkASSERT((m.getType() & (kScale_Mask | kAffine_Mask | kPerspective_Mask))
             == kScale_Mask);
    
    pt->fX = SkScalarMulAdd(sx, m.fMat[kMScaleX], m.fMat[kMTransX]);
    pt->fY = SkScalarMulAdd(sy, m.fMat[kMScaleY], m.fMat[kMTransY]);
}

void SkMatrix::Scale_xy(const SkMatrix& m, SkScalar sx, SkScalar sy,
                        SkPoint* pt) {
    SkASSERT((m.getType() & (kScale_Mask | kAffine_Mask | kPerspective_Mask))
             == kScale_Mask);
    SkASSERT(0 == m.fMat[kMTransX]);
    SkASSERT(0 == m.fMat[kMTransY]);

    pt->fX = SkScalarMul(sx, m.fMat[kMScaleX]);
    pt->fY = SkScalarMul(sy, m.fMat[kMScaleY]);
}

void SkMatrix::Trans_xy(const SkMatrix& m, SkScalar sx, SkScalar sy,
                        SkPoint* pt) {
    SkASSERT(m.getType() == kTranslate_Mask);

    pt->fX = sx + m.fMat[kMTransX];
    pt->fY = sy + m.fMat[kMTransY];
}

void SkMatrix::Identity_xy(const SkMatrix& m, SkScalar sx, SkScalar sy,
                           SkPoint* pt) {
    SkASSERT(0 == m.getType());

    pt->fX = sx;
    pt->fY = sy;
}

const SkMatrix::MapXYProc SkMatrix::gMapXYProcs[] = {
    SkMatrix::Identity_xy, SkMatrix::Trans_xy,
    SkMatrix::Scale_xy,    SkMatrix::ScaleTrans_xy,
    SkMatrix::Rot_xy,      SkMatrix::RotTrans_xy,
    SkMatrix::Rot_xy,      SkMatrix::RotTrans_xy,
    // repeat the persp proc 8 times
    SkMatrix::Persp_xy,    SkMatrix::Persp_xy,
    SkMatrix::Persp_xy,    SkMatrix::Persp_xy,
    SkMatrix::Persp_xy,    SkMatrix::Persp_xy,
    SkMatrix::Persp_xy,    SkMatrix::Persp_xy
};

///////////////////////////////////////////////////////////////////////////////

// if its nearly zero (just made up 26, perhaps it should be bigger or smaller)
#ifdef SK_SCALAR_IS_FIXED
    typedef SkFract             SkPerspElemType;
    #define PerspNearlyZero(x)  (SkAbs32(x) < (SK_Fract1 >> 26))
#else
    typedef float               SkPerspElemType;
    #define PerspNearlyZero(x)  SkScalarNearlyZero(x, (1.0f / (1 << 26)))
#endif

bool SkMatrix::fixedStepInX(SkScalar y, SkFixed* stepX, SkFixed* stepY) const {
    if (PerspNearlyZero(fMat[kMPersp0])) {
        if (stepX || stepY) {
            if (PerspNearlyZero(fMat[kMPersp1]) &&
                    PerspNearlyZero(fMat[kMPersp2] - kMatrix22Elem)) {
                if (stepX) {
                    *stepX = SkScalarToFixed(fMat[kMScaleX]);
                }
                if (stepY) {
                    *stepY = SkScalarToFixed(fMat[kMSkewY]);
                }
            } else {
#ifdef SK_SCALAR_IS_FIXED
                SkFixed z = SkFractMul(y, fMat[kMPersp1]) +
                            SkFractToFixed(fMat[kMPersp2]);
#else
                float z = y * fMat[kMPersp1] + fMat[kMPersp2];
#endif
                if (stepX) {
                    *stepX = SkScalarToFixed(SkScalarDiv(fMat[kMScaleX], z));
                }
                if (stepY) {
                    *stepY = SkScalarToFixed(SkScalarDiv(fMat[kMSkewY], z));
                }
            }
        }
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

#include "SkPerspIter.h"

SkPerspIter::SkPerspIter(const SkMatrix& m, SkScalar x0, SkScalar y0, int count)
        : fMatrix(m), fSX(x0), fSY(y0), fCount(count) {
    SkPoint pt;

    SkMatrix::Persp_xy(m, x0, y0, &pt);
    fX = SkScalarToFixed(pt.fX);
    fY = SkScalarToFixed(pt.fY);
}

int SkPerspIter::next() {
    int n = fCount;
    
    if (0 == n) {
        return 0;
    }
    SkPoint pt;
    SkFixed x = fX;
    SkFixed y = fY;
    SkFixed dx, dy;

    if (n >= kCount) {
        n = kCount;
        fSX += SkIntToScalar(kCount);
        SkMatrix::Persp_xy(fMatrix, fSX, fSY, &pt);
        fX = SkScalarToFixed(pt.fX);
        fY = SkScalarToFixed(pt.fY);
        dx = (fX - x) >> kShift;
        dy = (fY - y) >> kShift;
    } else {
        fSX += SkIntToScalar(n);
        SkMatrix::Persp_xy(fMatrix, fSX, fSY, &pt);
        fX = SkScalarToFixed(pt.fX);
        fY = SkScalarToFixed(pt.fY);
        dx = (fX - x) / n;
        dy = (fY - y) / n;
    }

    SkFixed* p = fStorage;
    for (int i = 0; i < n; i++) {
        *p++ = x; x += dx;
        *p++ = y; y += dy;
    }
    
    fCount -= n;
    return n;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_SCALAR_IS_FIXED

static inline bool poly_to_point(SkPoint* pt, const SkPoint poly[], int count) {
    SkFixed x = SK_Fixed1, y = SK_Fixed1;
    SkPoint pt1, pt2;
    Sk64    w1, w2;

    if (count > 1) {
        pt1.fX = poly[1].fX - poly[0].fX;
        pt1.fY = poly[1].fY - poly[0].fY;
        y = SkPoint::Length(pt1.fX, pt1.fY);
        if (y == 0) {
            return false;
        }
        switch (count) {
            case 2:
                break;
            case 3:
                pt2.fX = poly[0].fY - poly[2].fY;
                pt2.fY = poly[2].fX - poly[0].fX;
                goto CALC_X;
            default:
                pt2.fX = poly[0].fY - poly[3].fY;
                pt2.fY = poly[3].fX - poly[0].fX;
            CALC_X:
                w1.setMul(pt1.fX, pt2.fX);
                w2.setMul(pt1.fY, pt2.fY);
                w1.add(w2);
                w1.div(y, Sk64::kRound_DivOption);
                if (!w1.is32()) {
                    return false;
                }
                x = w1.get32();
                break;
        }
    }
    pt->set(x, y);
    return true;
}

bool SkMatrix::Poly2Proc(const SkPoint srcPt[], SkMatrix* dst,
                         const SkPoint& scalePt) {
    // need to check if SkFixedDiv overflows...

    const SkFixed scale = scalePt.fY;
    dst->fMat[kMScaleX] = SkFixedDiv(srcPt[1].fY - srcPt[0].fY, scale);
    dst->fMat[kMSkewY]  = SkFixedDiv(srcPt[0].fX - srcPt[1].fX, scale);
    dst->fMat[kMPersp0] = 0;
    dst->fMat[kMSkewX]  = SkFixedDiv(srcPt[1].fX - srcPt[0].fX, scale);
    dst->fMat[kMScaleY] = SkFixedDiv(srcPt[1].fY - srcPt[0].fY, scale);
    dst->fMat[kMPersp1] = 0;
    dst->fMat[kMTransX] = srcPt[0].fX;
    dst->fMat[kMTransY] = srcPt[0].fY;
    dst->fMat[kMPersp2] = SK_Fract1;
    dst->setTypeMask(kUnknown_Mask);
    return true;
}

bool SkMatrix::Poly3Proc(const SkPoint srcPt[], SkMatrix* dst,
                         const SkPoint& scale) {
    // really, need to check if SkFixedDiv overflow'd

    dst->fMat[kMScaleX] = SkFixedDiv(srcPt[2].fX - srcPt[0].fX, scale.fX);
    dst->fMat[kMSkewY]  = SkFixedDiv(srcPt[2].fY - srcPt[0].fY, scale.fX);
    dst->fMat[kMPersp0] = 0;
    dst->fMat[kMSkewX]  = SkFixedDiv(srcPt[1].fX - srcPt[0].fX, scale.fY);
    dst->fMat[kMScaleY] = SkFixedDiv(srcPt[1].fY - srcPt[0].fY, scale.fY);
    dst->fMat[kMPersp1] = 0;
    dst->fMat[kMTransX] = srcPt[0].fX;
    dst->fMat[kMTransY] = srcPt[0].fY;
    dst->fMat[kMPersp2] = SK_Fract1;
    dst->setTypeMask(kUnknown_Mask);
    return true;
}

bool SkMatrix::Poly4Proc(const SkPoint srcPt[], SkMatrix* dst,
                         const SkPoint& scale) {
    SkFract a1, a2;
    SkFixed x0, y0, x1, y1, x2, y2;

    x0 = srcPt[2].fX - srcPt[0].fX;
    y0 = srcPt[2].fY - srcPt[0].fY;
    x1 = srcPt[2].fX - srcPt[1].fX;
    y1 = srcPt[2].fY - srcPt[1].fY;
    x2 = srcPt[2].fX - srcPt[3].fX;
    y2 = srcPt[2].fY - srcPt[3].fY;

    /* check if abs(x2) > abs(y2) */
    if ( x2 > 0 ? y2 > 0 ? x2 > y2 : x2 > -y2 : y2 > 0 ? -x2 > y2 : x2 < y2) {
        SkFixed denom = SkMulDiv(x1, y2, x2) - y1;
        if (0 == denom) {
            return false;
        }
        a1 = SkFractDiv(SkMulDiv(x0 - x1, y2, x2) - y0 + y1, denom);
    } else {
        SkFixed denom = x1 - SkMulDiv(y1, x2, y2);
        if (0 == denom) {
            return false;
        }
        a1 = SkFractDiv(x0 - x1 - SkMulDiv(y0 - y1, x2, y2), denom);
    }

    /* check if abs(x1) > abs(y1) */
    if ( x1 > 0 ? y1 > 0 ? x1 > y1 : x1 > -y1 : y1 > 0 ? -x1 > y1 : x1 < y1) {
        SkFixed denom = y2 - SkMulDiv(x2, y1, x1);
        if (0 == denom) {
            return false;
        }
        a2 = SkFractDiv(y0 - y2 - SkMulDiv(x0 - x2, y1, x1), denom);
    } else {
        SkFixed denom = SkMulDiv(y2, x1, y1) - x2;
        if (0 == denom) {
            return false;
        }
        a2 = SkFractDiv(SkMulDiv(y0 - y2, x1, y1) - x0 + x2, denom);
    }

    // need to check if SkFixedDiv overflows...
    dst->fMat[kMScaleX] = SkFixedDiv(SkFractMul(a2, srcPt[3].fX) +
                                     srcPt[3].fX - srcPt[0].fX, scale.fX);
    dst->fMat[kMSkewY]  = SkFixedDiv(SkFractMul(a2, srcPt[3].fY) +
                                     srcPt[3].fY - srcPt[0].fY, scale.fX);
    dst->fMat[kMPersp0] = SkFixedDiv(a2, scale.fX);
    dst->fMat[kMSkewX]  = SkFixedDiv(SkFractMul(a1, srcPt[1].fX) +
                                     srcPt[1].fX - srcPt[0].fX, scale.fY);
    dst->fMat[kMScaleY] = SkFixedDiv(SkFractMul(a1, srcPt[1].fY) +
                                     srcPt[1].fY - srcPt[0].fY, scale.fY);
    dst->fMat[kMPersp1] = SkFixedDiv(a1, scale.fY);
    dst->fMat[kMTransX] = srcPt[0].fX;
    dst->fMat[kMTransY] = srcPt[0].fY;
    dst->fMat[kMPersp2] = SK_Fract1;
    dst->setTypeMask(kUnknown_Mask);
    return true;
}

#else   /* Scalar is float */

static inline bool checkForZero(float x) {
    return x*x == 0;
}

static inline bool poly_to_point(SkPoint* pt, const SkPoint poly[], int count) {
    float   x = 1, y = 1;
    SkPoint pt1, pt2;

    if (count > 1) {
        pt1.fX = poly[1].fX - poly[0].fX;
        pt1.fY = poly[1].fY - poly[0].fY;
        y = SkPoint::Length(pt1.fX, pt1.fY);
        if (checkForZero(y)) {
            return false;
        }
        switch (count) {
            case 2:
                break;
            case 3:
                pt2.fX = poly[0].fY - poly[2].fY;
                pt2.fY = poly[2].fX - poly[0].fX;
                goto CALC_X;
            default:
                pt2.fX = poly[0].fY - poly[3].fY;
                pt2.fY = poly[3].fX - poly[0].fX;
            CALC_X:
                x = SkScalarDiv(SkScalarMul(pt1.fX, pt2.fX) +
                                SkScalarMul(pt1.fY, pt2.fY), y);
                break;
        }
    }
    pt->set(x, y);
    return true;
}

bool SkMatrix::Poly2Proc(const SkPoint srcPt[], SkMatrix* dst,
                         const SkPoint& scale) {
    float invScale = 1 / scale.fY;

    dst->fMat[kMScaleX] = (srcPt[1].fY - srcPt[0].fY) * invScale;
    dst->fMat[kMSkewY] = (srcPt[0].fX - srcPt[1].fX) * invScale;
    dst->fMat[kMPersp0] = 0;
    dst->fMat[kMSkewX] = (srcPt[1].fX - srcPt[0].fX) * invScale;
    dst->fMat[kMScaleY] = (srcPt[1].fY - srcPt[0].fY) * invScale;
    dst->fMat[kMPersp1] = 0;
    dst->fMat[kMTransX] = srcPt[0].fX;
    dst->fMat[kMTransY] = srcPt[0].fY;
    dst->fMat[kMPersp2] = 1;
    dst->setTypeMask(kUnknown_Mask);
    return true;
}

bool SkMatrix::Poly3Proc(const SkPoint srcPt[], SkMatrix* dst,
                         const SkPoint& scale) {
    float invScale = 1 / scale.fX;
    dst->fMat[kMScaleX] = (srcPt[2].fX - srcPt[0].fX) * invScale;
    dst->fMat[kMSkewY] = (srcPt[2].fY - srcPt[0].fY) * invScale;
    dst->fMat[kMPersp0] = 0;

    invScale = 1 / scale.fY;
    dst->fMat[kMSkewX] = (srcPt[1].fX - srcPt[0].fX) * invScale;
    dst->fMat[kMScaleY] = (srcPt[1].fY - srcPt[0].fY) * invScale;
    dst->fMat[kMPersp1] = 0;

    dst->fMat[kMTransX] = srcPt[0].fX;
    dst->fMat[kMTransY] = srcPt[0].fY;
    dst->fMat[kMPersp2] = 1;
    dst->setTypeMask(kUnknown_Mask);
    return true;
}

bool SkMatrix::Poly4Proc(const SkPoint srcPt[], SkMatrix* dst,
                         const SkPoint& scale) {
    float   a1, a2;
    float   x0, y0, x1, y1, x2, y2;

    x0 = srcPt[2].fX - srcPt[0].fX;
    y0 = srcPt[2].fY - srcPt[0].fY;
    x1 = srcPt[2].fX - srcPt[1].fX;
    y1 = srcPt[2].fY - srcPt[1].fY;
    x2 = srcPt[2].fX - srcPt[3].fX;
    y2 = srcPt[2].fY - srcPt[3].fY;

    /* check if abs(x2) > abs(y2) */
    if ( x2 > 0 ? y2 > 0 ? x2 > y2 : x2 > -y2 : y2 > 0 ? -x2 > y2 : x2 < y2) {
        float denom = SkScalarMulDiv(x1, y2, x2) - y1;
        if (checkForZero(denom)) {
            return false;
        }
        a1 = SkScalarDiv(SkScalarMulDiv(x0 - x1, y2, x2) - y0 + y1, denom);
    } else {
        float denom = x1 - SkScalarMulDiv(y1, x2, y2);
        if (checkForZero(denom)) {
            return false;
        }
        a1 = SkScalarDiv(x0 - x1 - SkScalarMulDiv(y0 - y1, x2, y2), denom);
    }

    /* check if abs(x1) > abs(y1) */
    if ( x1 > 0 ? y1 > 0 ? x1 > y1 : x1 > -y1 : y1 > 0 ? -x1 > y1 : x1 < y1) {
        float denom = y2 - SkScalarMulDiv(x2, y1, x1);
        if (checkForZero(denom)) {
            return false;
        }
        a2 = SkScalarDiv(y0 - y2 - SkScalarMulDiv(x0 - x2, y1, x1), denom);
    } else {
        float denom = SkScalarMulDiv(y2, x1, y1) - x2;
        if (checkForZero(denom)) {
            return false;
        }
        a2 = SkScalarDiv(SkScalarMulDiv(y0 - y2, x1, y1) - x0 + x2, denom);
    }

    float invScale = 1 / scale.fX;
    dst->fMat[kMScaleX] = SkScalarMul(SkScalarMul(a2, srcPt[3].fX) +
                                      srcPt[3].fX - srcPt[0].fX, invScale);
    dst->fMat[kMSkewY] = SkScalarMul(SkScalarMul(a2, srcPt[3].fY) +
                                     srcPt[3].fY - srcPt[0].fY, invScale);
    dst->fMat[kMPersp0] = SkScalarMul(a2, invScale);
    invScale = 1 / scale.fY;
    dst->fMat[kMSkewX] = SkScalarMul(SkScalarMul(a1, srcPt[1].fX) +
                                     srcPt[1].fX - srcPt[0].fX, invScale);
    dst->fMat[kMScaleY] = SkScalarMul(SkScalarMul(a1, srcPt[1].fY) +
                                      srcPt[1].fY - srcPt[0].fY, invScale);
    dst->fMat[kMPersp1] = SkScalarMul(a1, invScale);
    dst->fMat[kMTransX] = srcPt[0].fX;
    dst->fMat[kMTransY] = srcPt[0].fY;
    dst->fMat[kMPersp2] = 1;
    dst->setTypeMask(kUnknown_Mask);
    return true;
}

#endif

typedef bool (*PolyMapProc)(const SkPoint[], SkMatrix*, const SkPoint&);

/*  Taken from Rob Johnson's original sample code in QuickDraw GX
*/
bool SkMatrix::setPolyToPoly(const SkPoint src[], const SkPoint dst[],
                             int count) {
    if ((unsigned)count > 4) {
        SkDebugf("--- SkMatrix::setPolyToPoly count out of range %d\n", count);
        return false;
    }

    if (0 == count) {
        this->reset();
        return true;
    }
    if (1 == count) {
        this->setTranslate(dst[0].fX - src[0].fX, dst[0].fY - src[0].fY);
        return true;
    }

    SkPoint scale;
    if (!poly_to_point(&scale, src, count) ||
            SkScalarNearlyZero(scale.fX) ||
            SkScalarNearlyZero(scale.fY)) {
        return false;
    }

    static const PolyMapProc gPolyMapProcs[] = {
        SkMatrix::Poly2Proc, SkMatrix::Poly3Proc, SkMatrix::Poly4Proc
    };
    PolyMapProc proc = gPolyMapProcs[count - 2];

    SkMatrix tempMap, result;
    tempMap.setTypeMask(kUnknown_Mask);

    if (!proc(src, &tempMap, scale)) {
        return false;
    }
    if (!tempMap.invert(&result)) {
        return false;
    }
    if (!proc(dst, &tempMap, scale)) {
        return false;
    }
    if (!result.setConcat(tempMap, result)) {
        return false;
    }
    *this = result;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

SkScalar SkMatrix::getMaxStretch() const {
    TypeMask mask = this->getType();

    if (mask & kPerspective_Mask) {
        return -SK_Scalar1;
    }
    
    SkScalar stretch;
    
    if (this->isIdentity()) {
        stretch = SK_Scalar1;
    } else if (!(mask & kAffine_Mask)) {
        stretch = SkMaxScalar(SkScalarAbs(fMat[kMScaleX]), SkScalarAbs(fMat[kMScaleY]));
#if 0   // don't have this bit
    } else if (mask & kZeroScale_TypeBit) {
        stretch = SkMaxScalar(SkScalarAbs(fM[kSkewX]), SkScalarAbs(fM[kSkewY]));
#endif
    } else {
        // ignore the translation part of the matrix, just look at 2x2 portion.
        // compute singular values, take largest abs value.
        // [a b; b c] = A^T*A
        SkScalar a = SkScalarMul(fMat[kMScaleX], fMat[kMScaleX]) + SkScalarMul(fMat[kMSkewY],  fMat[kMSkewY]);
        SkScalar b = SkScalarMul(fMat[kMScaleX], fMat[kMSkewX]) +  SkScalarMul(fMat[kMScaleY], fMat[kMSkewY]);
        SkScalar c = SkScalarMul(fMat[kMSkewX],  fMat[kMSkewX]) +  SkScalarMul(fMat[kMScaleY], fMat[kMScaleY]);
        // eigenvalues of A^T*A are the squared singular values of A.
        // characteristic equation is det((A^T*A) - l*I) = 0
        // l^2 - (a + c)l + (ac-b^2)
        // solve using quadratic equation (divisor is non-zero since l^2 has 1 coeff
        // and roots are guaraunteed to be pos and real).
        SkScalar largerRoot;
        SkScalar bSqd = SkScalarMul(b,b);
        if (bSqd <= SkFloatToScalar(1e-10)) { // will be true if upper left 2x2 is orthogonal, which is common, so save some math
            largerRoot = SkMaxScalar(a, c);
        } else {
            SkScalar aminusc = a - c;
            SkScalar apluscdiv2 = (a + c) / 2;
            SkScalar x = SkScalarSqrt(SkScalarMul(aminusc, aminusc) + 4 * bSqd) / 2;
            largerRoot = apluscdiv2 + x;
        }
        
        stretch = SkScalarSqrt(largerRoot);
    }
#if defined(SK_DEBUG) && 0
    // test a bunch of vectors. None should be scaled by more than stretch
    // (modulo some error) and we should find a vector that is scaled by almost
    // stretch.
    SkPoint pt;
    SkScalar max = 0;
    for (int i = 0; i < 1000; ++i) {
        SkScalar x = (float)rand() / RAND_MAX;
        SkScalar y = sqrtf(1 - (x*x));
        pt.fX = fMat[kMScaleX]*x + fMat[kMSkewX]*y;
        pt.fY = fMat[kMSkewY]*x + fMat[kMScaleY]*y;
        SkScalar d = pt.distanceToOrigin();
        SkASSERT(d <= (1.0001 * stretch));
        if (max < pt.distanceToOrigin()) {
            max = pt.distanceToOrigin();
        }
    }
    SkASSERT((stretch - max) < .05*stretch);
#endif
    return stretch;
}

const SkMatrix& SkMatrix::I() {
    static SkMatrix gIdentity;
    static bool gOnce;
    if (!gOnce) {
        gIdentity.reset();
        gOnce = true;
    }
    return gIdentity;
};

const SkMatrix& SkMatrix::InvalidMatrix() {
    static SkMatrix gInvalid;
    static bool gOnce;
    if (!gOnce) {
        gInvalid.setAll(SK_ScalarMax, SK_ScalarMax, SK_ScalarMax,
                        SK_ScalarMax, SK_ScalarMax, SK_ScalarMax,
                        SK_ScalarMax, SK_ScalarMax, SK_ScalarMax);
        gInvalid.getType(); // force the type to be computed
        gOnce = true;
    }
    return gInvalid;
}

///////////////////////////////////////////////////////////////////////////////

uint32_t SkMatrix::flatten(void* buffer) const {
    // TODO write less for simple matrices
    if (buffer) {
        memcpy(buffer, fMat, 9 * sizeof(SkScalar));
    }
    return 9 * sizeof(SkScalar);
}

uint32_t SkMatrix::unflatten(const void* buffer) {
    if (buffer) {
        memcpy(fMat, buffer, 9 * sizeof(SkScalar));
        this->setTypeMask(kUnknown_Mask);
    }
    return 9 * sizeof(SkScalar);
}

void SkMatrix::dump() const {
    SkString str;
    this->toDumpString(&str);
    SkDebugf("%s\n", str.c_str());
}

void SkMatrix::toDumpString(SkString* str) const {
#ifdef SK_CAN_USE_FLOAT
    str->printf("[%8.4f %8.4f %8.4f][%8.4f %8.4f %8.4f][%8.4f %8.4f %8.4f]",
#ifdef SK_SCALAR_IS_FLOAT
             fMat[0], fMat[1], fMat[2], fMat[3], fMat[4], fMat[5],
             fMat[6], fMat[7], fMat[8]);
#else
    SkFixedToFloat(fMat[0]), SkFixedToFloat(fMat[1]), SkFixedToFloat(fMat[2]),
    SkFixedToFloat(fMat[3]), SkFixedToFloat(fMat[4]), SkFixedToFloat(fMat[5]),
    SkFractToFloat(fMat[6]), SkFractToFloat(fMat[7]), SkFractToFloat(fMat[8]));
#endif
#else   // can't use float
    str->printf("[%x %x %x][%x %x %x][%x %x %x]",
                fMat[0], fMat[1], fMat[2], fMat[3], fMat[4], fMat[5],
                fMat[6], fMat[7], fMat[8]);
#endif
}
