
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#include "GrMatrix.h"
#include "GrRect.h"
#include <stddef.h>

#if 0
#if GR_SCALAR_IS_FLOAT
    const GrScalar GrMatrix::gRESCALE(GR_Scalar1);
#else
    GR_STATIC_ASSERT(GR_SCALAR_IS_FIXED);
    // fixed point isn't supported right now
    GR_STATIC_ASSERT(false);
const GrScalar GrMatrix::gRESCALE(1 << 30);
#endif

const GrMatrix::MapProc GrMatrix::gMapProcs[] = {
// Scales are not both zero
    &GrMatrix::mapIdentity,
    &GrMatrix::mapScale,
    &GrMatrix::mapTranslate,
    &GrMatrix::mapScaleAndTranslate,
    &GrMatrix::mapSkew,
    &GrMatrix::mapScaleAndSkew,
    &GrMatrix::mapSkewAndTranslate,
    &GrMatrix::mapNonPerspective,
    // no optimizations for perspective matrices
    &GrMatrix::mapPerspective,
    &GrMatrix::mapPerspective,
    &GrMatrix::mapPerspective,
    &GrMatrix::mapPerspective,
    &GrMatrix::mapPerspective,
    &GrMatrix::mapPerspective,
    &GrMatrix::mapPerspective,
    &GrMatrix::mapPerspective,

// Scales are zero (every other is invalid because kScale_TypeBit must be set if
// kZeroScale_TypeBit is set)
    &GrMatrix::mapInvalid,
    &GrMatrix::mapZero,
    &GrMatrix::mapInvalid,
    &GrMatrix::mapSetToTranslate,
    &GrMatrix::mapInvalid,
    &GrMatrix::mapSwappedScale,
    &GrMatrix::mapInvalid,
    &GrMatrix::mapSwappedScaleAndTranslate,

    // no optimizations for perspective matrices
    &GrMatrix::mapInvalid,
    &GrMatrix::mapZero,
    &GrMatrix::mapInvalid,
    &GrMatrix::mapPerspective,
    &GrMatrix::mapInvalid,
    &GrMatrix::mapPerspective,
    &GrMatrix::mapInvalid,
    &GrMatrix::mapPerspective,
};

void GrMatrix::setIdentity() {
    fM[0] = GR_Scalar1; fM[1] = 0;          fM[2] = 0;
    fM[3] = 0;          fM[4] = GR_Scalar1; fM[5] = 0;
    fM[6] = 0;          fM[7] = 0;          fM[8] = gRESCALE;
    fTypeMask = 0;
}

void GrMatrix::setTranslate(GrScalar dx, GrScalar dy) {
    fM[0] = GR_Scalar1; fM[1] = 0;          fM[2] = dx;
    fM[3] = 0;          fM[4] = GR_Scalar1; fM[5] = dy;
    fM[6] = 0;          fM[7] = 0;          fM[8] = gRESCALE;
    fTypeMask = (0 != dx || 0 != dy) ? kTranslate_TypeBit : 0;
}

void GrMatrix::setScale(GrScalar sx, GrScalar sy) {
    fM[0] = sx; fM[1] = 0;  fM[2] = 0;
    fM[3] = 0;  fM[4] = sy; fM[5] = 0;
    fM[6] = 0;  fM[7] = 0;  fM[8] = gRESCALE;
    fTypeMask = (GR_Scalar1 != sx || GR_Scalar1 != sy) ? kScale_TypeBit : 0;
}

void GrMatrix::setSkew(GrScalar skx, GrScalar sky) {
    fM[0] = GR_Scalar1; fM[1] = skx;        fM[2] = 0;
    fM[3] = sky;        fM[4] = GR_Scalar1; fM[5] = 0;
    fM[6] = 0;          fM[7] = 0;          fM[8] = gRESCALE;
    fTypeMask = (0 != skx || 0 != sky) ? kSkew_TypeBit : 0;
}

void GrMatrix::setConcat(const GrMatrix& a, const GrMatrix& b) {
    if (a.isIdentity()) {
        if (this != &b) {
            for (int i = 0; i < 9; ++i) {
                fM[i] = b.fM[i];
            }
            fTypeMask = b.fTypeMask;
        }
        return;
    }

    if (b.isIdentity()) {
        GrAssert(!a.isIdentity());
        if (this != &a) {
            for (int i = 0; i < 9; ++i) {
                    fM[i] = a.fM[i];
            }
            fTypeMask = a.fTypeMask;
        }
        return;
    }

    // a and/or b could be this
    GrMatrix tmp;

    // could do more optimizations based on type bits. Hopefully this call is
    // low frequency.
    // TODO: make this work for fixed point
    if (!((b.fTypeMask | a.fTypeMask) & kPerspective_TypeBit)) {
        tmp.fM[0] = a.fM[0] * b.fM[0] + a.fM[1] * b.fM[3];
        tmp.fM[1] = a.fM[0] * b.fM[1] + a.fM[1] * b.fM[4];
        tmp.fM[2] = a.fM[0] * b.fM[2] + a.fM[1] * b.fM[5] + a.fM[2] * gRESCALE;

        tmp.fM[3] = a.fM[3] * b.fM[0] + a.fM[4] * b.fM[3];
        tmp.fM[4] = a.fM[3] * b.fM[1] + a.fM[4] * b.fM[4];
        tmp.fM[5] = a.fM[3] * b.fM[2] + a.fM[4] * b.fM[5] + a.fM[5] * gRESCALE;

        tmp.fM[6] = 0;
        tmp.fM[7] = 0;
        tmp.fM[8] = gRESCALE * gRESCALE;
    } else {
        tmp.fM[0] = a.fM[0] * b.fM[0] + a.fM[1] * b.fM[3] + a.fM[2] * b.fM[6];
        tmp.fM[1] = a.fM[0] * b.fM[1] + a.fM[1] * b.fM[4] + a.fM[2] * b.fM[7];
        tmp.fM[2] = a.fM[0] * b.fM[2] + a.fM[1] * b.fM[5] + a.fM[2] * b.fM[8];

        tmp.fM[3] = a.fM[3] * b.fM[0] + a.fM[4] * b.fM[3] + a.fM[5] * b.fM[6];
        tmp.fM[4] = a.fM[3] * b.fM[1] + a.fM[4] * b.fM[4] + a.fM[5] * b.fM[7];
        tmp.fM[5] = a.fM[3] * b.fM[2] + a.fM[4] * b.fM[5] + a.fM[5] * b.fM[8];

        tmp.fM[6] = a.fM[6] * b.fM[0] + a.fM[7] * b.fM[3] + a.fM[8] * b.fM[6];
        tmp.fM[7] = a.fM[6] * b.fM[1] + a.fM[7] * b.fM[4] + a.fM[8] * b.fM[7];
        tmp.fM[8] = a.fM[6] * b.fM[2] + a.fM[7] * b.fM[5] + a.fM[8] * b.fM[8];
    }
    *this = tmp;
    this->computeTypeMask();
}

void GrMatrix::preConcat(const GrMatrix& m) {
    setConcat(*this, m);
}

void GrMatrix::postConcat(const GrMatrix& m) {
    setConcat(m, *this);
}

double GrMatrix::determinant() const {
    if (fTypeMask & kPerspective_TypeBit) {
        return  fM[0]*((double)fM[4]*fM[8] - (double)fM[5]*fM[7]) +
                fM[1]*((double)fM[5]*fM[6] - (double)fM[3]*fM[8]) +
                fM[2]*((double)fM[3]*fM[7] - (double)fM[4]*fM[6]);
    } else {
        return (double)fM[0]*fM[4]*gRESCALE -
               (double)fM[1]*fM[3]*gRESCALE;
    }
}

bool GrMatrix::invert(GrMatrix* inverted) const {

    if (isIdentity()) {
        if (inverted != this) {
            inverted->setIdentity();
        }
        return true;
    }
    static const double MIN_DETERMINANT_SQUARED = 1.e-16;

    // could do more optimizations based on type bits. Hopefully this call is
    // low frequency.

    double det = determinant();

    // check if we can't be inverted
    if (det*det <= MIN_DETERMINANT_SQUARED) {
        return false;
    } else if (NULL == inverted) {
        return true;
    }

    double t[9];

    if (fTypeMask & kPerspective_TypeBit) {
        t[0] = ((double)fM[4]*fM[8] - (double)fM[5]*fM[7]);
        t[1] = ((double)fM[2]*fM[7] - (double)fM[1]*fM[8]);
        t[2] = ((double)fM[1]*fM[5] - (double)fM[2]*fM[4]);
        t[3] = ((double)fM[5]*fM[6] - (double)fM[3]*fM[8]);
        t[4] = ((double)fM[0]*fM[8] - (double)fM[2]*fM[6]);
        t[5] = ((double)fM[2]*fM[3] - (double)fM[0]*fM[5]);
        t[6] = ((double)fM[3]*fM[7] - (double)fM[4]*fM[6]);
        t[7] = ((double)fM[1]*fM[6] - (double)fM[0]*fM[7]);
        t[8] = ((double)fM[0]*fM[4] - (double)fM[1]*fM[3]);
        det = 1.0 / det;
        for (int i = 0; i < 9; ++i) {
            inverted->fM[i] = (GrScalar)(t[i] * det);
        }
    } else {
        t[0] =  (double)fM[4]*gRESCALE;
        t[1] = -(double)fM[1]*gRESCALE;
        t[2] =  (double)fM[1]*fM[5] - (double)fM[2]*fM[4];
        t[3] = -(double)fM[3]*gRESCALE;
        t[4] =  (double)fM[0]*gRESCALE;
        t[5] =  (double)fM[2]*fM[3] - (double)fM[0]*fM[5];
        //t[6] = 0.0;
        //t[7] = 0.0;
        t[8] = (double)fM[0]*fM[4] - (double)fM[1]*fM[3];
        det = 1.0 / det;
        for (int i = 0; i < 6; ++i) {
            inverted->fM[i] = (GrScalar)(t[i] * det);
        }
        inverted->fM[6] = 0;
        inverted->fM[7] = 0;
        inverted->fM[8] = (GrScalar)(t[8] * det);
    }
    inverted->computeTypeMask();
    return true;
}

void GrMatrix::mapRect(GrRect* dst, const GrRect& src) const {
    GrPoint srcPts[4], dstPts[4];
    srcPts[0].set(src.fLeft, src.fTop);
    srcPts[1].set(src.fRight, src.fTop);
    srcPts[2].set(src.fRight, src.fBottom);
    srcPts[3].set(src.fLeft, src.fBottom);
    this->mapPoints(dstPts, srcPts, 4);
    dst->setBounds(dstPts, 4);
}

bool GrMatrix::hasPerspective() const {
    GrAssert(!!(kPerspective_TypeBit & fTypeMask) ==
             (fM[kPersp0] != 0 || fM[kPersp1] != 0 || fM[kPersp2] != gRESCALE));
    return 0 != (kPerspective_TypeBit & fTypeMask);
}

bool GrMatrix::isIdentity() const {
    GrAssert((0 == fTypeMask) ==
             (GR_Scalar1 == fM[kScaleX] && 0          == fM[kSkewX]  && 0          == fM[kTransX] &&
              0          == fM[kSkewY]  && GR_Scalar1 == fM[kScaleY] && 0          == fM[kTransY] &&
              0          == fM[kPersp0] && 0          == fM[kPersp1] && gRESCALE == fM[kPersp2]));
    return (0 == fTypeMask);
}


bool GrMatrix::preservesAxisAlignment() const {

    // check if matrix is trans and scale only
    static const int gAllowedMask1 = kScale_TypeBit | kTranslate_TypeBit;

    if (!(~gAllowedMask1 & fTypeMask)) {
        return true;
    }

    // check matrix is trans and skew only (0 scale)
    static const int gAllowedMask2 = kScale_TypeBit | kSkew_TypeBit |
                                     kTranslate_TypeBit | kZeroScale_TypeBit;

    if (!(~gAllowedMask2 & fTypeMask) && (kZeroScale_TypeBit & fTypeMask)) {
        return true;
    }

    return false;
}

GrScalar GrMatrix::getMaxStretch() const {

    if (fTypeMask & kPerspective_TypeBit) {
        return -GR_Scalar1;
    }

    GrScalar stretch;

    if (isIdentity()) {
        stretch = GR_Scalar1;
    } else if (!(fTypeMask & kSkew_TypeBit)) {
        stretch = GrMax(GrScalarAbs(fM[kScaleX]), GrScalarAbs(fM[kScaleY]));
    } else if (fTypeMask & kZeroScale_TypeBit) {
        stretch = GrMax(GrScalarAbs(fM[kSkewX]), GrScalarAbs(fM[kSkewY]));
    } else {
        // ignore the translation part of the matrix, just look at 2x2 portion.
        // compute singular values, take largest abs value.
        // [a b; b c] = A^T*A
        GrScalar a = GrMul(fM[kScaleX], fM[kScaleX]) + GrMul(fM[kSkewY],  fM[kSkewY]);
        GrScalar b = GrMul(fM[kScaleX], fM[kSkewX]) +  GrMul(fM[kScaleY], fM[kSkewY]);
        GrScalar c = GrMul(fM[kSkewX],  fM[kSkewX]) +  GrMul(fM[kScaleY], fM[kScaleY]);
        // eigenvalues of A^T*A are the squared singular values of A.
        // characteristic equation is det((A^T*A) - l*I) = 0
        // l^2 - (a + c)l + (ac-b^2)
        // solve using quadratic equation (divisor is non-zero since l^2 has 1 coeff
        // and roots are guaraunteed to be pos and real).
        GrScalar largerRoot;
        GrScalar bSqd = GrMul(b,b);
        // TODO: fixed point tolerance value.
        if (bSqd < 1e-10) { // will be true if upper left 2x2 is orthogonal, which is common, so save some math
            largerRoot = GrMax(a, c);
        } else {
            GrScalar aminusc = a - c;
            GrScalar apluscdiv2 = (a + c) / 2;
            GrScalar x = sqrtf(GrMul(aminusc,aminusc) + GrMul(4,(bSqd))) / 2;
            largerRoot = apluscdiv2 + x;
        }

        stretch = sqrtf(largerRoot);
    }
#if GR_DEBUG && 0
    // test a bunch of vectors. None should be scaled by more than stretch
    // (modulo some error) and we should find a vector that is scaled by almost
    // stretch.
    GrPoint pt;
    GrScalar max = 0;
    for (int i = 0; i < 1000; ++i) {
        GrScalar x = (float)rand() / RAND_MAX;
        GrScalar y = sqrtf(1 - (x*x));
        pt.fX = fM[kScaleX]*x + fM[kSkewX]*y;
        pt.fY = fM[kSkewY]*x + fM[kScaleY]*y;
        GrScalar d = pt.distanceToOrigin();
        GrAssert(d <= (1.0001 * stretch));
        max = GrMax(max, pt.distanceToOrigin());
    }
    GrAssert((stretch - max) < .05*stretch);
#endif
    return stretch;
}

bool GrMatrix::operator == (const GrMatrix& m) const {
    if (fTypeMask != m.fTypeMask) {
        return false;
    }
    if (!fTypeMask) {
        return true;
    }
    for (int i = 0; i < 9; ++i) {
        if (m.fM[i] != fM[i]) {
            return false;
        }
    }
    return true;
}

bool GrMatrix::operator != (const GrMatrix& m) const {
    return !(*this == m);
}

////////////////////////////////////////////////////////////////////////////////
// Matrix transformation procs
//////

void GrMatrix::mapIdentity(GrPoint* dst, const GrPoint* src, uint32_t count) const {
    if (src != dst) {
        for (uint32_t i = 0; i < count; ++i) {
            dst[i] = src[i];
        }
    }
}

void GrMatrix::mapScale(GrPoint* dst, const GrPoint* src, uint32_t count) const {
    for (uint32_t i = 0; i < count; ++i) {
        dst[i].fX = GrMul(src[i].fX, fM[kScaleX]);
        dst[i].fY = GrMul(src[i].fY, fM[kScaleY]);
    }
}


void GrMatrix::mapTranslate(GrPoint* dst, const GrPoint* src, uint32_t count) const {
    for (uint32_t i = 0; i < count; ++i) {
        dst[i].fX = src[i].fX + fM[kTransX];
        dst[i].fY = src[i].fY + fM[kTransY];
    }
}

void GrMatrix::mapScaleAndTranslate(GrPoint* dst, const GrPoint* src, uint32_t count) const {
    for (uint32_t i = 0; i < count; ++i) {
        dst[i].fX = GrMul(src[i].fX, fM[kScaleX]) + fM[kTransX];
        dst[i].fY = GrMul(src[i].fY, fM[kScaleY]) + fM[kTransY];
    }
}

void GrMatrix::mapSkew(GrPoint* dst, const GrPoint* src, uint32_t count) const {
    if (src != dst) {
        for (uint32_t i = 0; i < count; ++i) {
            dst[i].fX = src[i].fX + GrMul(src[i].fY, fM[kSkewX]);
            dst[i].fY = src[i].fY + GrMul(src[i].fX, fM[kSkewY]);
        }
    } else {
        for (uint32_t i = 0; i < count; ++i) {
            GrScalar newX = src[i].fX + GrMul(src[i].fY, fM[kSkewX]);
            dst[i].fY = src[i].fY + GrMul(src[i].fX, fM[kSkewY]);
            dst[i].fX = newX;
        }
    }
}

void GrMatrix::mapScaleAndSkew(GrPoint* dst, const GrPoint* src, uint32_t count) const {
    if (src != dst) {
        for (uint32_t i = 0; i < count; ++i) {
            dst[i].fX = GrMul(src[i].fX, fM[kScaleX]) + GrMul(src[i].fY, fM[kSkewX]);
            dst[i].fY = GrMul(src[i].fY, fM[kScaleY]) + GrMul(src[i].fX, fM[kSkewY]);
        }
    } else {
        for (uint32_t i = 0; i < count; ++i) {
            GrScalar newX = GrMul(src[i].fX, fM[kScaleX]) + GrMul(src[i].fY, fM[kSkewX]);
            dst[i].fY = GrMul(src[i].fY, fM[kScaleY]) + GrMul(src[i].fX, fM[kSkewY]);
            dst[i].fX = newX;
        }
    }
}

void GrMatrix::mapSkewAndTranslate(GrPoint* dst, const GrPoint* src, uint32_t count) const {
    if (src != dst) {
        for (uint32_t i = 0; i < count; ++i) {
            dst[i].fX = src[i].fX + GrMul(src[i].fY, fM[kSkewX]) + fM[kTransX];
            dst[i].fY = src[i].fY + GrMul(src[i].fX, fM[kSkewY]) + fM[kTransY];
        }
    } else {
        for (uint32_t i = 0; i < count; ++i) {
            GrScalar newX = src[i].fX + GrMul(src[i].fY, fM[kSkewX]) + fM[kTransX];
            dst[i].fY = src[i].fY + GrMul(src[i].fX, fM[kSkewY]) + fM[kTransY];
            dst[i].fX = newX;
        }
    }
}

void GrMatrix::mapNonPerspective(GrPoint* dst, const GrPoint* src, uint32_t count) const {
    if (src != dst) {
        for (uint32_t i = 0; i < count; ++i) {
            dst[i].fX = GrMul(fM[kScaleX], src[i].fX) + GrMul(fM[kSkewX], src[i].fY) + fM[kTransX];
            dst[i].fY = GrMul(fM[kSkewY], src[i].fX) + GrMul(fM[kScaleY], src[i].fY) + fM[kTransY];
        }
    } else {
        for (uint32_t i = 0; i < count; ++i) {
            GrScalar newX = GrMul(fM[kScaleX], src[i].fX) + GrMul(fM[kSkewX], src[i].fY) + fM[kTransX];
            dst[i].fY = GrMul(fM[kSkewY], src[i].fX) + GrMul(fM[kScaleY], src[i].fY) + fM[kTransY];
            dst[i].fX = newX;
        }
    }
}

void GrMatrix::mapPerspective(GrPoint* dst, const GrPoint* src, uint32_t count) const {
    for (uint32_t i = 0; i < count; ++i) {
        GrScalar x, y, w;
        x = GrMul(fM[kScaleX], src[i].fX) + GrMul(fM[kSkewX], src[i].fY) + fM[kTransX];
        y = GrMul(fM[kSkewY], src[i].fX) + GrMul(fM[kScaleY], src[i].fY) + fM[kTransY];
        w = GrMul(fM[kPersp0], src[i].fX) + GrMul(fM[kPersp1], src[i].fY) + fM[kPersp2];
        // TODO need fixed point invert
        if (w) {
            w = 1 / w;
        }
        dst[i].fX = GrMul(x, w);
        dst[i].fY = GrMul(y, w);
    }
}

void GrMatrix::mapInvalid(GrPoint* dst, const GrPoint* src, uint32_t count) const {
    GrAssert(0);
}

void GrMatrix::mapZero(GrPoint* dst, const GrPoint* src, uint32_t count) const {
    memset(dst, 0, sizeof(GrPoint)*count);
}

void GrMatrix::mapSetToTranslate(GrPoint* dst, const GrPoint* src, uint32_t count) const {
    for (uint32_t i = 0; i < count; ++i) {
        dst[i].fX = fM[kTransX];
        dst[i].fY = fM[kTransY];
    }
}

void GrMatrix::mapSwappedScale(GrPoint* dst, const GrPoint* src, uint32_t count) const {
    if (src != dst) {
        for (uint32_t i = 0; i < count; ++i) {
            dst[i].fX = GrMul(src[i].fY, fM[kSkewX]);
            dst[i].fY = GrMul(src[i].fX, fM[kSkewY]);
        }
    } else {
        for (uint32_t i = 0; i < count; ++i) {
            GrScalar newX = GrMul(src[i].fY, fM[kSkewX]);
            dst[i].fY = GrMul(src[i].fX, fM[kSkewY]);
            dst[i].fX = newX;
        }
    }
}

void GrMatrix::mapSwappedScaleAndTranslate(GrPoint* dst, const GrPoint* src, uint32_t count) const {
    if (src != dst) {
        for (uint32_t i = 0; i < count; ++i) {
            dst[i].fX = GrMul(src[i].fY, fM[kSkewX]) + fM[kTransX];
            dst[i].fY = GrMul(src[i].fX, fM[kSkewY]) + fM[kTransY];
        }
    } else {
        for (uint32_t i = 0; i < count; ++i) {
            GrScalar newX = GrMul(src[i].fY, fM[kSkewX]) + fM[kTransX];
            dst[i].fY = GrMul(src[i].fX, fM[kSkewY]) + fM[kTransY];
            dst[i].fX = newX;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// Unit test
//////

#include "GrRandom.h"

#if GR_DEBUG
enum MatrixType {
    kRotate_MatrixType,
    kScaleX_MatrixType,
    kScaleY_MatrixType,
    kSkewX_MatrixType,
    kSkewY_MatrixType,
    kTranslateX_MatrixType,
    kTranslateY_MatrixType,
    kSwapScaleXY_MatrixType,
    kPersp_MatrixType,

    kMatrixTypeCount
};

static void create_matrix(GrMatrix* matrix, GrRandom& rand) {
    MatrixType type = (MatrixType)(rand.nextU() % kMatrixTypeCount);
    switch (type) {
        case kRotate_MatrixType: {
            float angle = rand.nextF() * 2 *3.14159265358979323846f;
            GrScalar cosa = GrFloatToScalar(cosf(angle));
            GrScalar sina = GrFloatToScalar(sinf(angle));
            matrix->setAll(cosa,      -sina,           0,
                           sina,       cosa,           0,
                           0,          0,              GrMatrix::I()[8]);
        } break;
        case kScaleX_MatrixType: {
            GrScalar scale = GrFloatToScalar(rand.nextF(-2, 2));
            matrix->setAll(scale,      0,              0,
                           0,          GR_Scalar1,     0,
                           0,          0,              GrMatrix::I()[8]);
        } break;
        case kScaleY_MatrixType: {
            GrScalar scale = GrFloatToScalar(rand.nextF(-2, 2));
            matrix->setAll(GR_Scalar1, 0,              0,
                           0,          scale,          0,
                           0,          0,              GrMatrix::I()[8]);
        } break;
        case kSkewX_MatrixType: {
            GrScalar skew = GrFloatToScalar(rand.nextF(-2, 2));
            matrix->setAll(GR_Scalar1, skew,           0,
                           0,          GR_Scalar1,     0,
                           0,          0,              GrMatrix::I()[8]);
        } break;
        case kSkewY_MatrixType: {
            GrScalar skew = GrFloatToScalar(rand.nextF(-2, 2));
            matrix->setAll(GR_Scalar1, 0,              0,
                           skew,       GR_Scalar1,     0,
                           0,          0,              GrMatrix::I()[8]);
        } break;
        case kTranslateX_MatrixType: {
            GrScalar trans = GrFloatToScalar(rand.nextF(-10, 10));
            matrix->setAll(GR_Scalar1, 0,              trans,
                           0,          GR_Scalar1,     0,
                           0,          0,              GrMatrix::I()[8]);
        } break;
        case kTranslateY_MatrixType: {
            GrScalar trans = GrFloatToScalar(rand.nextF(-10, 10));
            matrix->setAll(GR_Scalar1, 0,              0,
                           0,          GR_Scalar1,     trans,
                           0,          0,              GrMatrix::I()[8]);
        } break;
        case kSwapScaleXY_MatrixType: {
            GrScalar xy = GrFloatToScalar(rand.nextF(-2, 2));
            GrScalar yx = GrFloatToScalar(rand.nextF(-2, 2));
            matrix->setAll(0,          xy,             0,
                           yx,         0,              0,
                           0,          0,              GrMatrix::I()[8]);
        } break;
        case kPersp_MatrixType: {
            GrScalar p0 = GrFloatToScalar(rand.nextF(-2, 2));
            GrScalar p1 = GrFloatToScalar(rand.nextF(-2, 2));
            GrScalar p2 = GrFloatToScalar(rand.nextF(-0.5f, 0.75f));
            matrix->setAll(GR_Scalar1, 0,              0,
                           0,          GR_Scalar1,     0,
                           p0,         p1,             GrMul(p2,GrMatrix::I()[8]));
        } break;
        default:
            GrAssert(0);
            break;
    }
}
#endif

void GrMatrix::UnitTest() {
    GrRandom rand;

    // Create a bunch of matrices and test point mapping, max stretch calc,
    // inversion and multiply-by-inverse.
#if GR_DEBUG
    for (int i = 0; i < 10000; ++i) {
        GrMatrix a, b;
        a.setIdentity();
        int num = rand.nextU() % 6;
        // force testing of I and swapXY
        if (0 == i) {
            num = 0;
            GrAssert(a.isIdentity());
        } else if (1 == i) {
            num = 0;
            a.setAll(0, GR_Scalar1, 0,
                     GR_Scalar1, 0, 0,
                     0, 0, I()[8]);
        }
        for (int j = 0; j < num; ++j) {
            create_matrix(&b, rand);
            a.preConcat(b);
        }

        GrScalar maxStretch = a.getMaxStretch();
        if (maxStretch > 0) {
            maxStretch = GrMul(GR_Scalar1 + GR_Scalar1 / 100, maxStretch);
        }
        GrPoint origin = a.mapPoint(GrPoint::Make(0,0));

        for (int j = 0; j < 9; ++j) {
            int mask, origMask = a.fTypeMask;
            GrScalar old = a[j];

            a.set(j, GR_Scalar1);
            mask = a.fTypeMask;
            a.computeTypeMask();
            GrAssert(mask == a.fTypeMask);

            a.set(j, 0);
            mask = a.fTypeMask;
            a.computeTypeMask();
            GrAssert(mask == a.fTypeMask);

            a.set(j, 10 * GR_Scalar1);
            mask = a.fTypeMask;
            a.computeTypeMask();
            GrAssert(mask == a.fTypeMask);

            a.set(j, old);
            GrAssert(a.fTypeMask == origMask);
        }

        for (int j = 0; j < 100; ++j) {
            GrPoint pt;
            pt.fX = GrFloatToScalar(rand.nextF(-10, 10));
            pt.fY = GrFloatToScalar(rand.nextF(-10, 10));

            GrPoint t0, t1, t2;
            t0 = a.mapPoint(pt);             // map to a new point
            t1 = pt;
            a.mapPoints(&t1, &t1, 1);        // in place
            a.mapPerspective(&t2, &pt, 1);   // full mult
            GrAssert(t0 == t1 && t1 == t2);
            if (maxStretch >= 0.f) {
                GrVec vec = origin - t0;
//                vec.setBetween(t0, origin);
                GrScalar stretch = vec.length() / pt.distanceToOrigin();
                GrAssert(stretch <= maxStretch);
            }
        }
        double det = a.determinant();
        if (fabs(det) > 1e-3 && a.invert(&b)) {
            GrMatrix c;
            c.setConcat(a,b);
            for (int i = 0; i < 9; ++i) {
                GrScalar diff = GrScalarAbs(c[i] - I()[i]);
                GrAssert(diff < (5*GR_Scalar1 / 100));
            }
        }
    }
#endif
}

///////////////////////////////////////////////////////////////////////////////
#endif

int Gr_clz(uint32_t n) {
    if (0 == n) {
        return 32;
    }

    int count = 0;
    if (0 == (n & 0xFFFF0000)) {
        count += 16;
        n <<= 16;
    }
    if (0 == (n & 0xFF000000)) {
        count += 8;
        n <<= 8;
    }
    if (0 == (n & 0xF0000000)) {
        count += 4;
        n <<= 4;
    }
    if (0 == (n & 0xC0000000)) {
        count += 2;
        n <<= 2;
    }
    if (0 == (n & 0x80000000)) {
        count += 1;
    }
    return count;
}
