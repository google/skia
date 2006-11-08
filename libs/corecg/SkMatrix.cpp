/* libs/corecg/SkMatrix.cpp
**
** Copyright 2006, Google Inc.
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

#ifdef SK_SCALAR_IS_FLOAT
    #define kMatrix22Elem   SK_Scalar1
#else
    #define kMatrix22Elem   SK_Fract1
#endif

/*      [scale-x    skew-x      trans-x]   [X]   [X']
        [skew-y     scale-y     trans-y] * [Y] = [Y']
        [persp-0    persp-1     persp-2]   [1]   [1 ]
*/

void SkMatrix::reset()
{
    fMat[kMScaleX] = fMat[kMScaleY] = SK_Scalar1;
    fMat[kMSkewX]  = fMat[kMSkewY] = 
    fMat[kMTransX] = fMat[kMTransY] =
    fMat[kMPersp0] = fMat[kMPersp1] = 0;
    fMat[kMPersp2] = kMatrix22Elem;
}

static inline int has_perspective(const SkScalar mat[9])
{
#ifdef SK_SCALAR_IS_FLOAT
    return mat[SkMatrix::kMPersp0] || mat[SkMatrix::kMPersp1] || mat[SkMatrix::kMPersp2] != kMatrix22Elem;
#else
    return mat[SkMatrix::kMPersp0] | mat[SkMatrix::kMPersp1] | (mat[SkMatrix::kMPersp2] - kMatrix22Elem);
#endif
}

// this guy aligns with the masks, so we can compute a mask from a varaible 0/1
enum {
    kTranslate_Shift,
    kScale_Shift,
    kAffine_Shift,
    kPerspective_Shift
};

SkMatrix::TypeMask SkMatrix::getType() const
{
    unsigned type = 0;

    type |= (fMat[kMPersp0] || fMat[kMPersp1] || fMat[kMPersp2] != kMatrix22Elem) << kPerspective_Shift;
    type |= (fMat[kMSkewX] || fMat[kMSkewY]) << kAffine_Shift;
    type |= (fMat[kMScaleX] != SK_Scalar1 || fMat[kMScaleY] != SK_Scalar1) << kScale_Shift;
    type |= (fMat[kMTransX] || fMat[kMTransY]) << kTranslate_Shift;

    return (TypeMask)type;
}

static inline bool is_identity(const SkScalar fMat[9])
{
#ifdef SK_SCALAR_IS_FLOAT
    return  fMat[SkMatrix::kMPersp0] == 0 && fMat[SkMatrix::kMPersp1] == 0 && fMat[SkMatrix::kMPersp2] == kMatrix22Elem &&
            fMat[SkMatrix::kMSkewX] == 0 && fMat[SkMatrix::kMSkewY] == 0 && fMat[SkMatrix::kMTransX] == 0 && fMat[SkMatrix::kMTransY] == 0 &&
            fMat[SkMatrix::kMScaleX] == SK_Scalar1 && fMat[SkMatrix::kMScaleY] == SK_Scalar1;
#else
    return  !(fMat[SkMatrix::kMPersp0] | fMat[SkMatrix::kMPersp1] | (fMat[SkMatrix::kMPersp2] - kMatrix22Elem) |
              fMat[SkMatrix::kMSkewX] | fMat[SkMatrix::kMSkewY] | fMat[SkMatrix::kMTransX] | fMat[SkMatrix::kMTransY] |
              (fMat[SkMatrix::kMScaleX] - SK_Scalar1) | (fMat[SkMatrix::kMScaleY] - SK_Scalar1));
#endif
}

bool SkMatrix::isIdentity() const
{
    return is_identity(fMat);
}

////////////////////////////////////////////////////////////////////////////////////

void SkMatrix::setTranslate(SkScalar dx, SkScalar dy)
{
    fMat[kMTransX] = dx;
    fMat[kMTransY] = dy;

    fMat[kMScaleX] = fMat[kMScaleY] = SK_Scalar1;
    fMat[kMSkewX]  = fMat[kMSkewY] = 
    fMat[kMPersp0] = fMat[kMPersp1] = 0;
    fMat[kMPersp2] = kMatrix22Elem;
}

bool SkMatrix::preTranslate(SkScalar dx, SkScalar dy)
{
    if (has_perspective(fMat))
    {
        SkMatrix    m;
        m.setTranslate(dx, dy);
        return this->preConcat(m);
    }
    else
    {
        fMat[kMTransX] += SkScalarMul(fMat[kMScaleX], dx) + SkScalarMul(fMat[kMSkewX], dy);
        fMat[kMTransY] += SkScalarMul(fMat[kMSkewY], dx) + SkScalarMul(fMat[kMScaleY], dy);
        return true;
    }
}

bool SkMatrix::postTranslate(SkScalar dx, SkScalar dy)
{
    if (has_perspective(fMat))
    {
        SkMatrix    m;
        m.setTranslate(dx, dy);
        return this->postConcat(m);
    }
    else
    {
        fMat[kMTransX] += dx;
        fMat[kMTransY] += dy;
        return true;
    }
}

////////////////////////////////////////////////////////////////////////////////////

void SkMatrix::setScale(SkScalar sx, SkScalar sy, SkScalar px, SkScalar py)
{
    fMat[kMScaleX] = sx;
    fMat[kMScaleY] = sy;
    fMat[kMTransX] = px - SkScalarMul(sx, px);
    fMat[kMTransY] = py - SkScalarMul(sy, py);
    fMat[kMPersp2] = kMatrix22Elem;

    fMat[kMSkewX]  = fMat[kMSkewY] = 
    fMat[kMPersp0] = fMat[kMPersp1] = 0;
}

void SkMatrix::setScale(SkScalar sx, SkScalar sy)
{
    fMat[kMScaleX] = sx;
    fMat[kMScaleY] = sy;
    fMat[kMPersp2] = kMatrix22Elem;

    fMat[kMTransX] = fMat[kMTransY] =
    fMat[kMSkewX]  = fMat[kMSkewY] = 
    fMat[kMPersp0] = fMat[kMPersp1] = 0;
}

bool SkMatrix::preScale(SkScalar sx, SkScalar sy, SkScalar px, SkScalar py)
{
    SkMatrix    m;
    m.setScale(sx, sy, px, py);
    return this->preConcat(m);
}

bool SkMatrix::preScale(SkScalar sx, SkScalar sy)
{
    SkMatrix    m;
    m.setScale(sx, sy);
    return this->preConcat(m);
}

bool SkMatrix::postScale(SkScalar sx, SkScalar sy, SkScalar px, SkScalar py)
{
    SkMatrix    m;
    m.setScale(sx, sy, px, py);
    return this->postConcat(m);
}

bool SkMatrix::postScale(SkScalar sx, SkScalar sy)
{
    SkMatrix    m;
    m.setScale(sx, sy);
    return this->postConcat(m);
}

#ifdef SK_SCALAR_IS_FIXED
    static inline SkFixed roundidiv(SkFixed numer, int denom)
    {
        int ns = numer >> 31;
        int ds = denom >> 31;
        numer = (numer ^ ns) - ns;
        denom = (denom ^ ds) - ds;
        
        SkFixed answer = (numer + (denom >> 1)) / denom;
        int as = ns ^ ds;
        return (answer ^ as) - as;
    }
#else
    static inline float roundidiv(float numer, int denom)
    {
        return numer / denom;
    }
#endif

// this guy perhaps can go away, if we have a fract/high-precision way to scale matrices
bool SkMatrix::postIDiv(int divx, int divy)
{
    if (divx == 0 || divy == 0)
        return false;

    fMat[kMScaleX] = roundidiv(fMat[kMScaleX], divx);
    fMat[kMSkewX]  = roundidiv(fMat[kMSkewX],  divx);
    fMat[kMTransX] = roundidiv(fMat[kMTransX], divx);

    fMat[kMScaleY] = roundidiv(fMat[kMScaleY], divy);
    fMat[kMSkewY]  = roundidiv(fMat[kMSkewY],  divy);
    fMat[kMTransY] = roundidiv(fMat[kMTransY], divy);

    return true;
}

////////////////////////////////////////////////////////////////////////////////////

void SkMatrix::setSinCos(SkScalar sinV, SkScalar cosV, SkScalar px, SkScalar py)
{
    fMat[kMScaleX]  = cosV;
    fMat[kMSkewX]   = -sinV;
    fMat[kMTransX]  = SkScalarMul(sinV, py) + SkScalarMul(SK_Scalar1 - cosV, px);

    fMat[kMSkewY]   = sinV;
    fMat[kMScaleY]  = cosV;
    fMat[kMTransY]  = SkScalarMul(-sinV, px) + SkScalarMul(SK_Scalar1 - cosV, py);

    fMat[kMPersp0] = fMat[kMPersp1] = 0;
    fMat[kMPersp2] = kMatrix22Elem;
}

void SkMatrix::setSinCos(SkScalar sinV, SkScalar cosV)
{
    fMat[kMScaleX]  = cosV;
    fMat[kMSkewX]   = -sinV;
    fMat[kMTransX]  = 0;

    fMat[kMSkewY]   = sinV;
    fMat[kMScaleY]  = cosV;
    fMat[kMTransY]  = 0;

    fMat[kMPersp0] = fMat[kMPersp1] = 0;
    fMat[kMPersp2] = kMatrix22Elem;
}

void SkMatrix::setRotate(SkScalar degrees, SkScalar px, SkScalar py)
{
    SkScalar sinV, cosV;
    sinV = SkScalarSinCos(SkDegreesToRadians(degrees), &cosV);
    this->setSinCos(sinV, cosV, px, py);
}

void SkMatrix::setRotate(SkScalar degrees)
{
    SkScalar sinV, cosV;
    sinV = SkScalarSinCos(SkDegreesToRadians(degrees), &cosV);
    this->setSinCos(sinV, cosV);
}

bool SkMatrix::preRotate(SkScalar degrees, SkScalar px, SkScalar py)
{
    SkMatrix    m;
    m.setRotate(degrees, px, py);
    return this->preConcat(m);
}

bool SkMatrix::preRotate(SkScalar degrees)
{
    SkMatrix    m;
    m.setRotate(degrees);
    return this->preConcat(m);
}

bool SkMatrix::postRotate(SkScalar degrees, SkScalar px, SkScalar py)
{
    SkMatrix    m;
    m.setRotate(degrees, px, py);
    return this->postConcat(m);
}

bool SkMatrix::postRotate(SkScalar degrees)
{
    SkMatrix    m;
    m.setRotate(degrees);
    return this->postConcat(m);
}

////////////////////////////////////////////////////////////////////////////////////

void SkMatrix::setSkew(SkScalar sx, SkScalar sy, SkScalar px, SkScalar py)
{
    fMat[kMScaleX]  = SK_Scalar1;
    fMat[kMSkewX]   = sx;
    fMat[kMTransX]  = SkScalarMul(-sx, py);

    fMat[kMSkewY]   = sy;
    fMat[kMScaleY]  = SK_Scalar1;
    fMat[kMTransY]  = SkScalarMul(-sy, px);

    fMat[kMPersp0] = fMat[kMPersp1] = 0;
    fMat[kMPersp2] = kMatrix22Elem;
}

void SkMatrix::setSkew(SkScalar sx, SkScalar sy)
{
    fMat[kMScaleX]  = SK_Scalar1;
    fMat[kMSkewX]   = sx;
    fMat[kMTransX]  = 0;

    fMat[kMSkewY]   = sy;
    fMat[kMScaleY]  = SK_Scalar1;
    fMat[kMTransY]  = 0;

    fMat[kMPersp0] = fMat[kMPersp1] = 0;
    fMat[kMPersp2] = kMatrix22Elem;
}

bool SkMatrix::preSkew(SkScalar sx, SkScalar sy, SkScalar px, SkScalar py)
{
    SkMatrix    m;
    m.setSkew(sx, sy, px, py);
    return this->preConcat(m);
}

bool SkMatrix::preSkew(SkScalar sx, SkScalar sy)
{
    SkMatrix    m;
    m.setSkew(sx, sy);
    return this->preConcat(m);
}

bool SkMatrix::postSkew(SkScalar sx, SkScalar sy, SkScalar px, SkScalar py)
{
    SkMatrix    m;
    m.setSkew(sx, sy, px, py);
    return this->postConcat(m);
}

bool SkMatrix::postSkew(SkScalar sx, SkScalar sy)
{
    SkMatrix    m;
    m.setSkew(sx, sy);
    return this->postConcat(m);
}

////////////////////////////////////////////////////////////////////////////////////

bool SkMatrix::setRectToRect(const SkRect& src, const SkRect& dst, ScaleToFit align)
{
    if (src.isEmpty())
    {
        this->reset();
        return false;
    }

    if (dst.isEmpty())
        memset(fMat, 0, 8 * sizeof(SkScalar));
    else
    {
        SkScalar    tx, sx = SkScalarDiv(dst.width(), src.width());
        SkScalar    ty, sy = SkScalarDiv(dst.height(), src.height());
        bool        xLarger = false;

        if (align != kFill_ScaleToFit)
        {
            if (sx > sy)
            {
                xLarger = true;
                sx = sy;
            }
            else
                sy = sx;
        }

        tx = dst.fLeft - SkScalarMul(src.fLeft, sx);
        ty = dst.fTop - SkScalarMul(src.fTop, sy);
        if (align == kCenter_ScaleToFit || align == kEnd_ScaleToFit)
        {
            SkScalar diff;

            if (xLarger)
                diff = dst.width() - SkScalarMul(src.width(), sy);
            else
                diff = dst.height() - SkScalarMul(src.height(), sy);

            if (align == kCenter_ScaleToFit)
                diff = SkScalarHalf(diff);

            if (xLarger)
                tx += diff;
            else
                ty += diff;
        }

        fMat[kMScaleX] = sx;
        fMat[kMScaleY] = sy;
        fMat[kMTransX] = tx;
        fMat[kMTransY] = ty;
        fMat[kMSkewX]  = fMat[kMSkewY] = 
        fMat[kMPersp0] = fMat[kMPersp1] = 0;
    }
    // shared cleanup
    fMat[kMPersp2] = kMatrix22Elem;
    return true;
}

////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_SCALAR_IS_FLOAT
    static inline int fixmuladdmul(float a, float b, float c, float d, float* result)
    {
        *result = a * b + c * d;
        return true;
    }
    static inline int fixmuladdmulshiftmul(float a, float b, float c, float d,
                                            int /*shift not used*/, float scale, float* result)
    {
        *result = (a * b + c * d) * scale;
        return true;
    }
    static inline float rowcol3(const float row[], const float col[])
    {
        return row[0] * col[0] + row[1] * col[3] + row[2] * col[6];
    }
    static inline int negifaddoverflows(float& result, float a, float b)
    {
        result = a + b;
        return 0;
    }
#else
    static inline bool fixmuladdmul(SkFixed a, SkFixed b, SkFixed c, SkFixed d, SkFixed* result)
    {
        Sk64    tmp1, tmp2;
        tmp1.setMul(a, b);
        tmp2.setMul(c, d);
        tmp1.add(tmp2);
        if (tmp1.isFixed())
        {
            *result = tmp1.getFixed();
            return true;
        }
        return false;
    }
    static inline bool fixmuladdmulshiftmul(SkFixed a, SkFixed b, SkFixed c, SkFixed d,
                                            int shift, SkFixed scale, SkFixed* result)
    {
        Sk64    tmp1, tmp2;
        tmp1.setMul(a, b);
        tmp2.setMul(c, d);
        tmp1.add(tmp2);

        S32 hi = SkAbs32(tmp1.fHi);
        int afterShift = 16;
        if (hi >> 15)
        {
            int clz = 17 - SkCLZ(hi);
            SkASSERT(clz > 0 && clz <= 16);
            afterShift -= clz;
            shift += clz;
        }

        tmp1.roundRight(shift + 16);
        SkASSERT(tmp1.is32());

        tmp1.setMul(tmp1.get32(), scale);
        tmp1.roundRight(afterShift);
        if (tmp1.is32())
        {
            *result = tmp1.get32();
            return true;
        }
        return false;
    }
    static inline SkFixed fracmuladdmul(SkFixed a, SkFract b, SkFixed c, SkFract d)
    {
        Sk64    tmp1, tmp2;
        tmp1.setMul(a, b);
        tmp2.setMul(c, d);
        tmp1.add(tmp2);
        return tmp1.getFract();
    }

    static inline SkFixed rowcol3(const SkFixed row[], const SkFixed col[])
    {
        Sk64    tmp1, tmp2;

        tmp1.setMul(row[0], col[0]);    // N * fixed
        tmp2.setMul(row[1], col[3]);    // N * fixed
        tmp1.add(tmp2);

        tmp2.setMul(row[2], col[6]);    // N * fract
        tmp2.roundRight(14);            // make it fixed
        tmp1.add(tmp2);

        return tmp1.getFixed();
    }
    static inline int negifaddoverflows(SkFixed& result, SkFixed a, SkFixed b)
    {
        SkFixed c = a + b;
        result = c;
        SkASSERT(((c ^ a) & (c ^ b)) >= 0);
        return (c ^ a) & (c ^ b);
    }
#endif

static void normalize_perspective(SkScalar mat[9])
{
    if (SkScalarAbs(mat[SkMatrix::kMPersp2]) > kMatrix22Elem)
    {
        for (int i = 0; i < 9; i++)
            mat[i] = SkScalarHalf(mat[i]);
    }
}

bool SkMatrix::setConcat(const SkMatrix& a, const SkMatrix& b)
{
    TypeMask    aType = a.getType();
    TypeMask    bType = b.getType();

    if (0 == aType)
        *this = b;
    else if (0 == bType)
        *this = a;
    else
    {
        SkMatrix    tmp;
        SkMatrix*   c = this;

        if (this == &a || this == &b)
            c = &tmp;

        if ((aType | bType) & kPerspective_Mask)
        {
            c->fMat[kMScaleX] = rowcol3(&a.fMat[0], &b.fMat[0]);
            c->fMat[kMSkewX]  = rowcol3(&a.fMat[0], &b.fMat[1]);
            c->fMat[kMTransX] = rowcol3(&a.fMat[0], &b.fMat[2]);

            c->fMat[kMSkewY]  = rowcol3(&a.fMat[3], &b.fMat[0]);
            c->fMat[kMScaleY] = rowcol3(&a.fMat[3], &b.fMat[1]);
            c->fMat[kMTransY] = rowcol3(&a.fMat[3], &b.fMat[2]);

            c->fMat[kMPersp0] = rowcol3(&a.fMat[6], &b.fMat[0]);
            c->fMat[kMPersp1] = rowcol3(&a.fMat[6], &b.fMat[1]);
            c->fMat[kMPersp2] = rowcol3(&a.fMat[6], &b.fMat[2]);

            normalize_perspective(c->fMat);
        }
        else    // not perspective
        {
            if (!fixmuladdmul(a.fMat[kMScaleX], b.fMat[kMScaleX], a.fMat[kMSkewX], b.fMat[kMSkewY], &c->fMat[kMScaleX]))
                return false;
            if (!fixmuladdmul(a.fMat[kMScaleX], b.fMat[kMSkewX], a.fMat[kMSkewX], b.fMat[kMScaleY], &c->fMat[kMSkewX]))
                return false;
            if (!fixmuladdmul(a.fMat[kMScaleX], b.fMat[kMTransX], a.fMat[kMSkewX], b.fMat[kMTransY], &c->fMat[kMTransX]))
                return false;
            if (negifaddoverflows(c->fMat[kMTransX], c->fMat[kMTransX], a.fMat[kMTransX]) < 0)
                return false;

            if (!fixmuladdmul(a.fMat[kMSkewY], b.fMat[kMScaleX], a.fMat[kMScaleY], b.fMat[kMSkewY], &c->fMat[kMSkewY]))
                return false;
            if (!fixmuladdmul(a.fMat[kMSkewY], b.fMat[kMSkewX], a.fMat[kMScaleY], b.fMat[kMScaleY], &c->fMat[kMScaleY]))
                return false;
            if (!fixmuladdmul(a.fMat[kMSkewY], b.fMat[kMTransX], a.fMat[kMScaleY], b.fMat[kMTransY], &c->fMat[kMTransY]))
                return false;
            if (negifaddoverflows(c->fMat[kMTransY], c->fMat[kMTransY], a.fMat[kMTransY]) < 0)
                return false;

            c->fMat[kMPersp0] = c->fMat[kMPersp1] = 0;
            c->fMat[kMPersp2] = kMatrix22Elem;
        }

        if (c == &tmp)
            *this = tmp;
    }
    return true;
}

bool SkMatrix::preConcat(const SkMatrix& mat)
{
    return this->setConcat(*this, mat);
}

bool SkMatrix::postConcat(const SkMatrix& mat)
{
    return this->setConcat(mat, *this);
}


////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_SCALAR_IS_FLOAT
    #define SkPerspMul(a, b)            SkScalarMul(a, b)
    #define SkScalarMulShift(a, b, s)   SkScalarMul(a, b)
    static float sk_inv_determinant(const float mat[9], int isPerspective, int* /* (only used in Fixed case) */)
    {
        double det;

        if (isPerspective)
            det =   mat[SkMatrix::kMScaleX] * ((double)mat[SkMatrix::kMScaleY] * mat[SkMatrix::kMPersp2] - (double)mat[SkMatrix::kMTransY] * mat[SkMatrix::kMPersp1]) +
                    mat[SkMatrix::kMSkewX] * ((double)mat[SkMatrix::kMTransY] * mat[SkMatrix::kMPersp0] - (double)mat[SkMatrix::kMSkewY] * mat[SkMatrix::kMPersp2]) +
                    mat[SkMatrix::kMTransX] * ((double)mat[SkMatrix::kMSkewY] * mat[SkMatrix::kMPersp1] - (double)mat[SkMatrix::kMScaleY] * mat[SkMatrix::kMPersp0]);
        else
            det =   (double)mat[SkMatrix::kMScaleX] * mat[SkMatrix::kMScaleY] - (double)mat[SkMatrix::kMSkewX] * mat[SkMatrix::kMSkewY];

        if (SkScalarNearlyZero((float)det))
            return 0;
        return (float)(1.0 / det);
    }
#else
    #define SkPerspMul(a, b)    SkFractMul(a, b)
    #define SkScalarMulShift(a, b, s)   SkMulShift(a, b, s)
    static void set_muladdmul(Sk64* dst, S32 a, S32 b, S32 c, S32 d)
    {
        Sk64    tmp;

        dst->setMul(a, b);
        tmp.setMul(c, d);
        dst->add(tmp);
    }
    static SkFixed sk_inv_determinant(const SkFixed mat[9], int isPerspective, int* shift)
    {
        Sk64    tmp1, tmp2;

        if (isPerspective)
        {
            tmp1.setMul(mat[SkMatrix::kMScaleX], fracmuladdmul(mat[SkMatrix::kMScaleY], mat[SkMatrix::kMPersp2], -mat[SkMatrix::kMTransY], mat[SkMatrix::kMPersp1]));
            tmp2.setMul(mat[SkMatrix::kMSkewX], fracmuladdmul(mat[SkMatrix::kMTransY], mat[SkMatrix::kMPersp0], -mat[SkMatrix::kMSkewY], mat[SkMatrix::kMPersp2]));
            tmp1.add(tmp2);
            tmp2.setMul(mat[SkMatrix::kMTransX], fracmuladdmul(mat[SkMatrix::kMSkewY], mat[SkMatrix::kMPersp1], -mat[SkMatrix::kMScaleY], mat[SkMatrix::kMPersp0]));
            tmp1.add(tmp2);
        }
        else
        {
            tmp1.setMul(mat[SkMatrix::kMScaleX], mat[SkMatrix::kMScaleY]);
            tmp2.setMul(mat[SkMatrix::kMSkewX], mat[SkMatrix::kMSkewY]);
            tmp1.sub(tmp2);
        }

        int s = tmp1.getClzAbs();
        *shift = s;

        SkFixed denom;
        if (s <= 32)
            denom = tmp1.getShiftRight(33 - s);
        else
            denom = (S32)tmp1.fLo << (s - 33);

        if (denom == 0)
            return 0;
        /** This could perhaps be a special fractdiv function, since both of its
            arguments are known to have bit 31 clear and bit 30 set (when they
            are made positive), thus eliminating the need for calling clz()
        */
        return SkFractDiv(SK_Fract1, denom);
    }
#endif

bool SkMatrix::invert(SkMatrix* inv) const
{
    int         isPersp = has_perspective(fMat);
    int         shift;
    SkScalar    scale = sk_inv_determinant(fMat, isPersp, &shift);

    if (scale == 0) // underflow
        return false;

    if (inv)
    {
        SkMatrix tmp;
        if (inv == this)
            inv = &tmp;

        if (isPersp)
        {
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
            if (SkAbs32(inv->fMat[kMPersp2]) > SK_Fixed1)
            {
                Sk64    tmp;

                tmp.set(SK_Fract1);
                tmp.shiftLeft(16);
                tmp.div(inv->fMat[kMPersp2], Sk64::kRound_DivOption);

                SkFract scale = tmp.get32();

                for (int i = 0; i < 9; i++)
                    inv->fMat[i] = SkFractMul(inv->fMat[i], scale);
            }
            inv->fMat[kMPersp2] = SkFixedToFract(inv->fMat[kMPersp2]);
#endif
        }
        else    // not perspective
        {
#ifdef SK_SCALAR_IS_FIXED
            Sk64    tx, ty;
            int     clzNumer;

            // check the 2x2 for overflow
            {
                S32 value = SkAbs32(fMat[kMScaleY]);
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
            if (shift - clzNumer > 14)
                return false;   // overflow

            int fixedShift = 61 - shift;
            int sk64shift = 44 - shift + clzNumer;

            inv->fMat[kMScaleX] = SkMulShift(fMat[kMScaleY], scale, fixedShift);
            inv->fMat[kMSkewX]  = SkMulShift(-fMat[kMSkewX], scale, fixedShift);
            inv->fMat[kMTransX] = SkMulShift(tx.getShiftRight(33 - clzNumer), scale, sk64shift);
                
            inv->fMat[kMSkewY]  = SkMulShift(-fMat[kMSkewY], scale, fixedShift);
            inv->fMat[kMScaleY] = SkMulShift(fMat[kMScaleX], scale, fixedShift);
            inv->fMat[kMTransY] = SkMulShift(ty.getShiftRight(33 - clzNumer), scale, sk64shift);
#else
            inv->fMat[kMScaleX] = SkScalarMul(fMat[kMScaleY], scale);
            inv->fMat[kMSkewX] = SkScalarMul(-fMat[kMSkewX], scale);
            if (!fixmuladdmulshiftmul(fMat[kMSkewX], fMat[kMTransY], -fMat[kMScaleY], fMat[kMTransX], shift, scale, &inv->fMat[kMTransX]))
                return false;
                
            inv->fMat[kMSkewY] = SkScalarMul(-fMat[kMSkewY], scale);
            inv->fMat[kMScaleY] = SkScalarMul(fMat[kMScaleX], scale);
            if (!fixmuladdmulshiftmul(fMat[kMSkewY], fMat[kMTransX], -fMat[kMScaleX], fMat[kMTransY], shift, scale, &inv->fMat[kMTransY]))
                return false;
#endif
            inv->fMat[kMPersp0] = 0;
            inv->fMat[kMPersp1] = 0;
            inv->fMat[kMPersp2] = kMatrix22Elem;
        }

        if (inv == &tmp)
            *(SkMatrix*)this = tmp;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////////

bool SkMatrix::mapPoints(SkPoint dst[], const SkPoint src[], int count, TypeMask typeMask) const
{
    SkASSERT(dst && src && count > 0 || count == 0);
    SkASSERT(src == dst || SkAbs32((S32)(src - dst)) >= count); // no partial overlap

    if (count <= 0)
        return true;

    bool ok = true;

    if (typeMask & kPerspective_Mask)
    {
#ifdef SK_SCALAR_IS_FIXED
        SkFixed persp2 = SkFractToFixed(fMat[kMPersp2]);
#endif
        for (int i = count - 1; i >= 0; --i)
        {
            SkScalar sx = src[i].fX;
            SkScalar sy = src[i].fY;
            SkScalar x = SkScalarMul(sx, fMat[kMScaleX]) + SkScalarMul(sy, fMat[kMSkewX]) + fMat[kMTransX];
            SkScalar y = SkScalarMul(sx, fMat[kMSkewY]) + SkScalarMul(sy, fMat[kMScaleY]) + fMat[kMTransY];
#ifdef SK_SCALAR_IS_FIXED
            SkFixed z = SkFractMul(sx, fMat[kMPersp0]) + SkFractMul(sy, fMat[kMPersp1]) + persp2;
#else
            float z = SkScalarMul(sx, fMat[kMPersp0]) + SkScalarMul(sy, fMat[kMPersp1]) + fMat[kMPersp2];
#endif
            if (z)
                z = SkScalarFastInvert(z);
            dst[i].fX = SkScalarMul(x, z);
            dst[i].fY = SkScalarMul(y, z);
        }
    }
    else if (typeMask & kAffine_Mask)
    {
        for (int i = count - 1; i >= 0; --i)
        {
            SkScalar sx = src[i].fX;
            SkScalar sy = src[i].fY;
            dst[i].fX = SkScalarMul(sx, fMat[kMScaleX]) + SkScalarMul(sy, fMat[kMSkewX]) + fMat[kMTransX];
            dst[i].fY = SkScalarMul(sx, fMat[kMSkewY]) + SkScalarMul(sy, fMat[kMScaleY]) + fMat[kMTransY];
        }
    }
    else if (typeMask & kScale_Mask)
    {
        for (int i = count - 1; i >= 0; --i)
        {
            dst[i].fX = SkScalarMul(src[i].fX, fMat[kMScaleX]) + fMat[kMTransX];
            dst[i].fY = SkScalarMul(src[i].fY, fMat[kMScaleY]) + fMat[kMTransY];
        }
    }
    else if (typeMask & kTranslate_Mask)
    {
        for (int i = count - 1; i >= 0; --i)
        {
            dst[i].fX = src[i].fX + fMat[kMTransX];
            dst[i].fY = src[i].fY + fMat[kMTransY];
        }
    }
    else
    {
        SkASSERT(typeMask == 0);
        if (dst != src)
            memcpy(dst, src, count * sizeof(SkPoint));
    }
    return ok;
}

bool SkMatrix::mapVectors(SkPoint dst[], const SkPoint src[], int count, TypeMask maskType) const
{
    bool ok;

    if (maskType & kPerspective_Mask)
    {
        SkPoint origin;

        origin.set(0, 0);
        ok = this->mapPoints(&origin, &origin, 1, maskType);

        for (int i = count - 1; i >= 0; --i)
        {
            SkPoint tmp;

            ok &= this->mapPoints(&tmp, &src[i], 1, maskType);
            dst[i].set(tmp.fX - origin.fX, tmp.fY - origin.fY);
        }
    }
    else
    {
        SkMatrix tmp = *this;

        tmp.fMat[kMTransX] = tmp.fMat[kMTransY] = 0;
        ok = tmp.mapPoints(dst, src, count, maskType);
    }
    return ok;
}

bool SkMatrix::mapRect(SkRect* dst, const SkRect& src, TypeMask maskType) const
{
    SkASSERT(dst && &src);

    bool ok;

    if (RectStaysRect(maskType))
    {
        ok = this->mapPoints((SkPoint*)dst, (const SkPoint*)&src, 2, maskType);
        dst->sort();
    }
    else
    {
        SkPoint quad[4];

        src.toQuad(quad);
        ok = this->mapPoints(quad, quad, 4, maskType);
        dst->set(quad, 4);
    }
    return ok;
}

SkScalar SkMatrix::mapRadius(SkScalar radius) const
{
    SkVector    vec[2];

    vec[0].set(radius, 0);
    vec[1].set(0, radius);
    this->mapVectors(vec, 2);

    SkScalar d0 = vec[0].length();
    SkScalar d1 = vec[1].length();

    return SkScalarMean(d0, d1);
}

//////////////////////////////////////////////////////////////////////////////////////////

void SkMatrix::Perspective_ptProc(const SkMatrix& m, SkScalar sx, SkScalar sy, SkPoint* pt)
{
    SkASSERT(m.getType() & kPerspective_Mask);

    SkScalar x = SkScalarMul(sx, m.fMat[kMScaleX]) + SkScalarMul(sy, m.fMat[kMSkewX]) + m.fMat[kMTransX];
    SkScalar y = SkScalarMul(sx, m.fMat[kMSkewY]) + SkScalarMul(sy, m.fMat[kMScaleY]) + m.fMat[kMTransY];
#ifdef SK_SCALAR_IS_FIXED
    SkFixed z = SkFractMul(sx, m.fMat[kMPersp0]) + SkFractMul(sy, m.fMat[kMPersp1]) + SkFractToFixed(m.fMat[kMPersp2]);
#else
    float z = SkScalarMul(sx, m.fMat[kMPersp0]) + SkScalarMul(sy, m.fMat[kMPersp1]) + m.fMat[kMPersp2];
#endif
    if (z)
        z = SkScalarFastInvert(z);
    pt->fX = SkScalarMul(x, z);
    pt->fY = SkScalarMul(y, z);
}

#ifdef SK_SCALAR_IS_FIXED
static SkFixed fixmuladdmul(SkFixed a, SkFixed b, SkFixed c, SkFixed d)
{
    Sk64    tmp, tmp1;

    tmp.setMul(a, b);
    tmp1.setMul(c, d);
    return tmp.addGetFixed(tmp1);
//  tmp.add(tmp1);
//  return tmp.getFixed();
}
#endif

void SkMatrix::Affine_ptProc(const SkMatrix& m, SkScalar sx, SkScalar sy, SkPoint* pt)
{
    SkASSERT((m.getType() & (kAffine_Mask | kPerspective_Mask)) == kAffine_Mask);

#ifdef SK_SCALAR_IS_FIXED
    pt->fX = fixmuladdmul(sx, m.fMat[kMScaleX], sy, m.fMat[kMSkewX]) + m.fMat[kMTransX];
    pt->fY = fixmuladdmul(sx, m.fMat[kMSkewY], sy, m.fMat[kMScaleY]) + m.fMat[kMTransY];
#else
    pt->fX = SkScalarMul(sx, m.fMat[kMScaleX]) + SkScalarMul(sy, m.fMat[kMSkewX]) + m.fMat[kMTransX];
    pt->fY = SkScalarMul(sx, m.fMat[kMSkewY]) + SkScalarMul(sy, m.fMat[kMScaleY]) + m.fMat[kMTransY];
#endif
}

void SkMatrix::Scale_ptProc(const SkMatrix& m, SkScalar sx, SkScalar sy, SkPoint* pt)
{
    SkASSERT((m.getType() & (kScale_Mask | kAffine_Mask | kPerspective_Mask)) == kScale_Mask);

    pt->fX = SkScalarMul(sx, m.fMat[kMScaleX]) + m.fMat[kMTransX];
    pt->fY = SkScalarMul(sy, m.fMat[kMScaleY]) + m.fMat[kMTransY];
}

void SkMatrix::Translate_ptProc(const SkMatrix& m, SkScalar sx, SkScalar sy, SkPoint* pt)
{
    SkASSERT(m.getType() == kTranslate_Mask);

    pt->fX = sx + m.fMat[kMTransX];
    pt->fY = sy + m.fMat[kMTransY];
}

void SkMatrix::Identity_ptProc(const SkMatrix& m, SkScalar sx, SkScalar sy, SkPoint* pt)
{
    SkASSERT(0 == m.getType());

    pt->fX = sx;
    pt->fY = sy;
}

SkMatrix::MapPtProc SkMatrix::getMapPtProc() const
{
    TypeMask typeMask = this->getType();

    if (typeMask & kPerspective_Mask)
        return Perspective_ptProc;
    if (typeMask & kAffine_Mask)
        return Affine_ptProc;
    if (typeMask & kScale_Mask)
        return Scale_ptProc;
    if (typeMask & kTranslate_Mask)
        return Translate_ptProc;
    return Identity_ptProc;
}

//////////////////////////////////////////////////////////////////////////////////////////

// if its nearly zero (just made up 24 for that, perhaps it should be bigger or smaller)
#ifdef SK_SCALAR_IS_FIXED
    typedef SkFract             SkPerspElemType;
    #define PerspNearlyZero(x)  (SkAbs32(x) < (SK_Fract1 >> 26))
#else
    typedef float               SkPerspElemType;
    #define PerspNearlyZero(x)  SkScalarNearlyZero(x, (1.0f / (1 << 26)))
#endif

bool SkMatrix::fixedStepInX(SkScalar y, SkFixed* stepX, SkFixed* stepY) const
{
    if (PerspNearlyZero(fMat[kMPersp0]))
    {
        if (stepX || stepY)
        {
            if (PerspNearlyZero(fMat[kMPersp1]) && PerspNearlyZero(fMat[kMPersp2] - kMatrix22Elem))
            {
                if (stepX)
                    *stepX = SkScalarToFixed(fMat[kMScaleX]);
                if (stepY)
                    *stepY = SkScalarToFixed(fMat[kMSkewY]);
            }
            else
            {
#ifdef SK_SCALAR_IS_FIXED
                SkFixed z = SkFractMul(y, fMat[kMPersp1]) + SkFractToFixed(fMat[kMPersp2]);
#else
                float z = y * fMat[kMPersp1] + fMat[kMPersp2];
#endif
                if (stepX)
                    *stepX = SkScalarToFixed(SkScalarDiv(fMat[kMScaleX], z));
                if (stepY)
                    *stepY = SkScalarToFixed(SkScalarDiv(fMat[kMSkewY], z));
            }
        }
        return true;
    }
    return false;
}

#if 0
void SkMatrix::perspectiveLine(SkScalar x, SkScalar y, SkFixed dst[], int count) const
{
    if (count <= 0)
        return;

    SkPoint start, stop;        
    SkMatrix::Perspective_ptProc(*this, x, y, &start);

    SkFixed fx = SkScalarToFixed(start.fX);
    SkFixed fy = SkScalarToFixed(start.fY);
    
    *dst++ = fx;
    *dst++ = fy;
    if (1 == count) {
        return;
    }
        
    SkMatrix::Perspective_ptProc(*this, x + SkIntToScalar(count - 1), y, &stop);

    SkFixed dx = SkScalarToFixed(stop.fX - start.fX) / (count - 1);
    SkFixed dy = SkScalarToFixed(stop.fY - start.fY) / (count - 1);

    for (int i = 1; i < count; i++)
    {
        fx += dx;
        *dst++ = fx;
        fy += dy;
        *dst++ = fy;
    }
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////

#include "SkPerspIter.h"

SkPerspIter::SkPerspIter(const SkMatrix& m, SkScalar x0, SkScalar y0, int count)
    : fMatrix(m), fSX(x0), fSY(y0), fCount(count)
{
    SkPoint pt;

    SkMatrix::Perspective_ptProc(m, x0, y0, &pt);
    fX = SkScalarToFixed(pt.fX);
    fY = SkScalarToFixed(pt.fY);
}

int SkPerspIter::next()
{
    int n = fCount;
    
    if (0 == n)
        return 0;

    SkPoint pt;
    SkFixed x = fX;
    SkFixed y = fY;
    SkFixed dx, dy;

    if (n >= kCount)
    {
        n = kCount;
        fSX += SkIntToScalar(kCount);
        SkMatrix::Perspective_ptProc(fMatrix, fSX, fSY, &pt);
        fX = SkScalarToFixed(pt.fX);
        fY = SkScalarToFixed(pt.fY);
        dx = (fX - x) >> kShift;
        dy = (fY - y) >> kShift;
    }
    else
    {
        fSX += SkIntToScalar(n);
        SkMatrix::Perspective_ptProc(fMatrix, fSX, fSY, &pt);
        fX = SkScalarToFixed(pt.fX);
        fY = SkScalarToFixed(pt.fY);
        dx = (fX - x) / n;
        dy = (fY - y) / n;
    }

    SkFixed* p = fStorage;
    for (int i = 0; i < n; i++)
    {
        *p++ = x; x += dx;
        *p++ = y; y += dy;
    }
    
    fCount -= n;
    return n;
}

/////////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_SCALAR_IS_FIXED

static inline void poly_to_point(SkPoint* pt, const SkPoint poly[], int count)
{
    SkFixed x = SK_Fixed1, y = SK_Fixed1;
    SkPoint pt1, pt2;
    Sk64    w1, w2;

    if (count > 1)
    {   pt1.fX = poly[1].fX - poly[0].fX;
        pt1.fY = poly[1].fY - poly[0].fY;
        y = SkPoint::Length(pt1.fX, pt1.fY);
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
            x = w1.get32();
            break;
        }
    }
    pt->set(x, y);
}

static inline void Map1Pt(const SkPoint source[], SkMatrix* dst)
{
    dst->setTranslate(source[0].fX, source[0].fY);
}

void SkMatrix::Map2Pt(const SkPoint srcPt[], SkMatrix* dst, SkFixed scale)
{
    dst->fMat[kMScaleX] = SkFixedDiv(srcPt[1].fY - srcPt[0].fY, scale);
    dst->fMat[kMSkewY]  = SkFixedDiv(srcPt[0].fX - srcPt[1].fX, scale);
    dst->fMat[kMPersp0] = 0;
    dst->fMat[kMSkewX]  = SkFixedDiv(srcPt[1].fX - srcPt[0].fX, scale);
    dst->fMat[kMScaleY] = SkFixedDiv(srcPt[1].fY - srcPt[0].fY, scale);
    dst->fMat[kMPersp1] = 0;
    dst->fMat[kMTransX] = srcPt[0].fX;
    dst->fMat[kMTransY] = srcPt[0].fY;
    dst->fMat[kMPersp2] = SK_Fract1;
}

void SkMatrix::Map3Pt(const SkPoint srcPt[], SkMatrix* dst, SkFixed scaleX, SkFixed scaleY)
{
    dst->fMat[kMScaleX] = SkFixedDiv(srcPt[2].fX - srcPt[0].fX, scaleX);
    dst->fMat[kMSkewY]  = SkFixedDiv(srcPt[2].fY - srcPt[0].fY, scaleX);
    dst->fMat[kMPersp0] = 0;
    dst->fMat[kMSkewX]  = SkFixedDiv(srcPt[1].fX - srcPt[0].fX, scaleY);
    dst->fMat[kMScaleY] = SkFixedDiv(srcPt[1].fY - srcPt[0].fY, scaleY);
    dst->fMat[kMPersp1] = 0;
    dst->fMat[kMTransX] = srcPt[0].fX;
    dst->fMat[kMTransY] = srcPt[0].fY;
    dst->fMat[kMPersp2] = SK_Fract1;
}

void SkMatrix::Map4Pt(const SkPoint srcPt[], SkMatrix* dst, SkFixed scaleX, SkFixed scaleY)
{
    SkFract a1, a2;
    SkFixed x0, y0, x1, y1, x2, y2;

    x0 = srcPt[2].fX - srcPt[0].fX;
    y0 = srcPt[2].fY - srcPt[0].fY;
    x1 = srcPt[2].fX - srcPt[1].fX;
    y1 = srcPt[2].fY - srcPt[1].fY;
    x2 = srcPt[2].fX - srcPt[3].fX;
    y2 = srcPt[2].fY - srcPt[3].fY;

    /* check if abs(x2) > abs(y2) */
    if ( x2 > 0 ? y2 > 0 ? x2 > y2 : x2 > -y2 : y2 > 0 ? -x2 > y2 : x2 < y2)
        a1 = SkFractDiv(SkMulDiv(x0 - x1, y2, x2) - y0 + y1, SkMulDiv(x1, y2, x2) - y1);
    else
        a1 = SkFractDiv(x0 - x1 - SkMulDiv(y0 - y1, x2, y2), x1 - SkMulDiv(y1, x2, y2));

    /* check if abs(x1) > abs(y1) */
    if ( x1 > 0 ? y1 > 0 ? x1 > y1 : x1 > -y1 : y1 > 0 ? -x1 > y1 : x1 < y1)
        a2 = SkFractDiv(y0 - y2 - SkMulDiv(x0 - x2, y1, x1), y2 - SkMulDiv(x2, y1, x1));
    else
        a2 = SkFractDiv(SkMulDiv(y0 - y2, x1, y1) - x0 + x2, SkMulDiv(y2, x1, y1) - x2);

    dst->fMat[kMScaleX] = SkFixedDiv(SkFractMul(a2, srcPt[3].fX) + srcPt[3].fX - srcPt[0].fX, scaleX);
    dst->fMat[kMSkewY]  = SkFixedDiv(SkFractMul(a2, srcPt[3].fY) + srcPt[3].fY - srcPt[0].fY, scaleX);
    dst->fMat[kMPersp0] = SkFixedDiv(a2, scaleX);
    dst->fMat[kMSkewX]  = SkFixedDiv(SkFractMul(a1, srcPt[1].fX) + srcPt[1].fX - srcPt[0].fX, scaleY);
    dst->fMat[kMScaleY] = SkFixedDiv(SkFractMul(a1, srcPt[1].fY) + srcPt[1].fY - srcPt[0].fY, scaleY);
    dst->fMat[kMPersp1] = SkFixedDiv(a1, scaleY);
    dst->fMat[kMTransX] = srcPt[0].fX;
    dst->fMat[kMTransY] = srcPt[0].fY;
    dst->fMat[kMPersp2] = SK_Fract1;
}

#else   /* Scalar is float */

static inline void poly_to_point(SkPoint* pt, const SkPoint poly[], int count)
{
    float   x = 1, y = 1;
    SkPoint pt1, pt2;

    if (count > 1)
    {   pt1.fX = poly[1].fX - poly[0].fX;
        pt1.fY = poly[1].fY - poly[0].fY;
        y = SkPoint::Length(pt1.fX, pt1.fY);
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
            x = SkScalarDiv(SkScalarMul(pt1.fX, pt2.fX) + SkScalarMul(pt1.fY, pt2.fY), y);
            break;
        }
    }
    pt->set(x, y);
}

static inline void Map1Pt(const SkPoint source[], SkMatrix* dst)
{
    dst->setTranslate(source[0].fX, source[0].fY);
}

void SkMatrix::Map2Pt(const SkPoint srcPt[], SkMatrix* dst, float scale)
{
    float invScale = 1 / scale;

    dst->fMat[kMScaleX] = (srcPt[1].fY - srcPt[0].fY) * invScale;
    dst->fMat[kMSkewY] = (srcPt[0].fX - srcPt[1].fX) * invScale;
    dst->fMat[kMPersp0] = 0;
    dst->fMat[kMSkewX] = (srcPt[1].fX - srcPt[0].fX) * invScale;
    dst->fMat[kMScaleY] = (srcPt[1].fY - srcPt[0].fY) * invScale;
    dst->fMat[kMPersp1] = 0;
    dst->fMat[kMTransX] = srcPt[0].fX;
    dst->fMat[kMTransY] = srcPt[0].fY;
    dst->fMat[kMPersp2] = 1;
}

void SkMatrix::Map3Pt(const SkPoint srcPt[], SkMatrix* dst, float scaleX, float scaleY)
{
    float invScale = 1 / scaleX;

    dst->fMat[kMScaleX] = (srcPt[2].fX - srcPt[0].fX) * invScale;
    dst->fMat[kMSkewY] = (srcPt[2].fY - srcPt[0].fY) * invScale;
    dst->fMat[kMPersp0] = 0;
    invScale = 1 / scaleY;
    dst->fMat[kMSkewX] = (srcPt[1].fX - srcPt[0].fX) * invScale;
    dst->fMat[kMScaleY] = (srcPt[1].fY - srcPt[0].fY) * invScale;
    dst->fMat[kMPersp1] = 0;
    dst->fMat[kMTransX] = srcPt[0].fX;
    dst->fMat[kMTransY] = srcPt[0].fY;
    dst->fMat[kMPersp2] = 1;
}

void SkMatrix::Map4Pt(const SkPoint srcPt[], SkMatrix* dst, float scaleX, float scaleY)
{
    float   a1, a2;
    float   x0, y0, x1, y1, x2, y2;

    x0 = srcPt[2].fX - srcPt[0].fX;
    y0 = srcPt[2].fY - srcPt[0].fY;
    x1 = srcPt[2].fX - srcPt[1].fX;
    y1 = srcPt[2].fY - srcPt[1].fY;
    x2 = srcPt[2].fX - srcPt[3].fX;
    y2 = srcPt[2].fY - srcPt[3].fY;

    /* check if abs(x2) > abs(y2) */
    if ( x2 > 0 ? y2 > 0 ? x2 > y2 : x2 > -y2 : y2 > 0 ? -x2 > y2 : x2 < y2)
        a1 = SkScalarDiv(SkScalarMulDiv(x0 - x1, y2, x2) - y0 + y1, SkScalarMulDiv(x1, y2, x2) - y1);
    else
        a1 = SkScalarDiv(x0 - x1 - SkScalarMulDiv(y0 - y1, x2, y2), x1 - SkScalarMulDiv(y1, x2, y2));

    /* check if abs(x1) > abs(y1) */
    if ( x1 > 0 ? y1 > 0 ? x1 > y1 : x1 > -y1 : y1 > 0 ? -x1 > y1 : x1 < y1)
        a2 = SkScalarDiv(y0 - y2 - SkScalarMulDiv(x0 - x2, y1, x1), y2 - SkScalarMulDiv(x2, y1, x1));
    else
        a2 = SkScalarDiv(SkScalarMulDiv(y0 - y2, x1, y1) - x0 + x2, SkScalarMulDiv(y2, x1, y1) - x2);

    scaleX = 1 / scaleX;
    dst->fMat[kMScaleX] = SkScalarMul(SkScalarMul(a2, srcPt[3].fX) + srcPt[3].fX - srcPt[0].fX, scaleX);
    dst->fMat[kMSkewY] = SkScalarMul(SkScalarMul(a2, srcPt[3].fY) + srcPt[3].fY - srcPt[0].fY, scaleX);
    dst->fMat[kMPersp0] = SkScalarMul(a2, scaleX);
    scaleY = 1 / scaleY;
    dst->fMat[kMSkewX] = SkScalarMul(SkScalarMul(a1, srcPt[1].fX) + srcPt[1].fX - srcPt[0].fX, scaleY);
    dst->fMat[kMScaleY] = SkScalarMul(SkScalarMul(a1, srcPt[1].fY) + srcPt[1].fY - srcPt[0].fY, scaleY);
    dst->fMat[kMPersp1] = SkScalarMul(a1, scaleY);
    dst->fMat[kMTransX] = srcPt[0].fX;
    dst->fMat[kMTransY] = srcPt[0].fY;
    dst->fMat[kMPersp2] = 1;
}

#endif

/*  Taken from Rob Johnson's original sample code in QuickDraw GX
*/
bool SkMatrix::setPolyToPoly(const SkPoint dst[], const SkPoint src[], int count)
{
    SkASSERT((unsigned)count <= 4);

    SkPoint     tempPt;
    SkMatrix    tempMap;

    poly_to_point(&tempPt, src, count);
    switch (count) {
    case 0:
        this->reset();
        break;
    case 1:
        this->setTranslate(dst[0].fX - src[0].fX, dst[0].fY - src[0].fY);
        break;
    case 2:
        Map2Pt(src, &tempMap, tempPt.fY);
        if (tempMap.invert(this) == false)
            return false;
        Map2Pt(dst, &tempMap, tempPt.fY);
        goto mapMap;
    case 3:
        Map3Pt(src, &tempMap, tempPt.fX, tempPt.fY);
        if (tempMap.invert(this) == false)
            return false;
        Map3Pt(dst, &tempMap, tempPt.fX, tempPt.fY);
        goto mapMap;
    default:
        Map4Pt(src, &tempMap, tempPt.fX, tempPt.fY);
        if (tempMap.invert(this) == false)
            return false;
        Map4Pt(dst, &tempMap, tempPt.fX, tempPt.fY);
    mapMap:
        this->setConcat(tempMap, *this);
        break;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG

void SkMatrix::dump() const
{
    SkDebugf("SkMatrix mask = ");
    unsigned mask = this->getType();

#ifdef SK_CAN_USE_FLOAT
    SkDebugf("[%8.4f %8.4f %8.4f]\n[%8.4f %8.4f %8.4f]\n[%8.4f %8.4f %8.4f]\n",
#ifdef SK_SCALAR_IS_FLOAT
            fMat[0], fMat[1], fMat[2], fMat[3], fMat[4], fMat[5],
            fMat[6], fMat[7], fMat[8]);
#else
            SkFixedToFloat(fMat[0]), SkFixedToFloat(fMat[1]), SkFixedToFloat(fMat[2]),
            SkFixedToFloat(fMat[3]), SkFixedToFloat(fMat[4]), SkFixedToFloat(fMat[5]),
            SkFractToFloat(fMat[6]), SkFractToFloat(fMat[7]), SkFractToFloat(fMat[8]));
#endif
#else   // can't use float
    SkDebugf("[%x %x %x]\n[%x %x %x]\n[%x %x %x]\n",
            fMat[0], fMat[1], fMat[2], fMat[3], fMat[4], fMat[5],
            fMat[6], fMat[7], fMat[8]);
#endif
}

void SkMatrix::UnitTest()
{
#ifdef SK_SUPPORT_UNITTEST
    SkMatrix    mat, inverse, iden1, iden2;

    mat.reset();
    mat.setTranslate(SK_Scalar1, SK_Scalar1);
    mat.invert(&inverse);
    inverse.dump();
    iden1.setConcat(mat, inverse);
    iden1.dump();

    mat.setScale(SkIntToScalar(2), SkIntToScalar(2));
    mat.invert(&inverse);
    inverse.dump();
    iden1.setConcat(mat, inverse);
    iden1.dump();

    mat.setScale(SK_Scalar1/2, SK_Scalar1/2);
    mat.invert(&inverse);
    inverse.dump();
    iden1.setConcat(mat, inverse);
    iden1.dump();

    mat.setScale(SkIntToScalar(3), SkIntToScalar(5), SkIntToScalar(20), 0);
    mat.postRotate(SkIntToScalar(25));

    SkASSERT(mat.invert(nil));
    mat.invert(&inverse);

    iden1.setConcat(mat, inverse);
    iden2.setConcat(inverse, mat);

    iden1.dump();
    iden2.dump();
#endif
}

#endif
