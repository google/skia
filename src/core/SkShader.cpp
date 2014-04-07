/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapProcShader.h"
#include "SkReadBuffer.h"
#include "SkMallocPixelRef.h"
#include "SkPaint.h"
#include "SkScalar.h"
#include "SkShader.h"
#include "SkWriteBuffer.h"

SkShader::SkShader() {
    fLocalMatrix.reset();
    SkDEBUGCODE(fInSetContext = false;)
}

SkShader::SkShader(SkReadBuffer& buffer)
        : INHERITED(buffer) {
    if (buffer.readBool()) {
        buffer.readMatrix(&fLocalMatrix);
    } else {
        fLocalMatrix.reset();
    }

    SkDEBUGCODE(fInSetContext = false;)
}

SkShader::~SkShader() {
    SkASSERT(!fInSetContext);
}

void SkShader::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    bool hasLocalM = this->hasLocalMatrix();
    buffer.writeBool(hasLocalM);
    if (hasLocalM) {
        buffer.writeMatrix(fLocalMatrix);
    }
}

bool SkShader::setContext(const SkBitmap& device,
                          const SkPaint& paint,
                          const SkMatrix& matrix) {
    SkASSERT(!this->setContextHasBeenCalled());

    const SkMatrix* m = &matrix;
    SkMatrix        total;

    fPaintAlpha = paint.getAlpha();
    if (this->hasLocalMatrix()) {
        total.setConcat(matrix, this->getLocalMatrix());
        m = &total;
    }
    if (m->invert(&fTotalInverse)) {
        fTotalInverseClass = (uint8_t)ComputeMatrixClass(fTotalInverse);
        SkDEBUGCODE(fInSetContext = true;)
        return true;
    }
    return false;
}

void SkShader::endContext() {
    SkASSERT(fInSetContext);
    SkDEBUGCODE(fInSetContext = false;)
}

SkShader::ShadeProc SkShader::asAShadeProc(void** ctx) {
    return NULL;
}

#include "SkColorPriv.h"

void SkShader::shadeSpan16(int x, int y, uint16_t span16[], int count) {
    SkASSERT(span16);
    SkASSERT(count > 0);
    SkASSERT(this->canCallShadeSpan16());

    // basically, if we get here, the subclass screwed up
    SkDEBUGFAIL("kHasSpan16 flag is set, but shadeSpan16() not implemented");
}

#define kTempColorQuadCount 6   // balance between speed (larger) and saving stack-space
#define kTempColorCount     (kTempColorQuadCount << 2)

#ifdef SK_CPU_BENDIAN
    #define SkU32BitShiftToByteOffset(shift)    (3 - ((shift) >> 3))
#else
    #define SkU32BitShiftToByteOffset(shift)    ((shift) >> 3)
#endif

void SkShader::shadeSpanAlpha(int x, int y, uint8_t alpha[], int count) {
    SkASSERT(count > 0);

    SkPMColor   colors[kTempColorCount];

    while ((count -= kTempColorCount) >= 0) {
        this->shadeSpan(x, y, colors, kTempColorCount);
        x += kTempColorCount;

        const uint8_t* srcA = (const uint8_t*)colors + SkU32BitShiftToByteOffset(SK_A32_SHIFT);
        int quads = kTempColorQuadCount;
        do {
            U8CPU a0 = srcA[0];
            U8CPU a1 = srcA[4];
            U8CPU a2 = srcA[8];
            U8CPU a3 = srcA[12];
            srcA += 4*4;
            *alpha++ = SkToU8(a0);
            *alpha++ = SkToU8(a1);
            *alpha++ = SkToU8(a2);
            *alpha++ = SkToU8(a3);
        } while (--quads != 0);
    }
    SkASSERT(count < 0);
    SkASSERT(count + kTempColorCount >= 0);
    if (count += kTempColorCount) {
        this->shadeSpan(x, y, colors, count);

        const uint8_t* srcA = (const uint8_t*)colors + SkU32BitShiftToByteOffset(SK_A32_SHIFT);
        do {
            *alpha++ = *srcA;
            srcA += 4;
        } while (--count != 0);
    }
#if 0
    do {
        int n = count;
        if (n > kTempColorCount)
            n = kTempColorCount;
        SkASSERT(n > 0);

        this->shadeSpan(x, y, colors, n);
        x += n;
        count -= n;

        const uint8_t* srcA = (const uint8_t*)colors + SkU32BitShiftToByteOffset(SK_A32_SHIFT);
        do {
            *alpha++ = *srcA;
            srcA += 4;
        } while (--n != 0);
    } while (count > 0);
#endif
}

SkShader::MatrixClass SkShader::ComputeMatrixClass(const SkMatrix& mat) {
    MatrixClass mc = kLinear_MatrixClass;

    if (mat.hasPerspective()) {
        if (mat.fixedStepInX(0, NULL, NULL)) {
            mc = kFixedStepInX_MatrixClass;
        } else {
            mc = kPerspective_MatrixClass;
        }
    }
    return mc;
}

//////////////////////////////////////////////////////////////////////////////

SkShader::BitmapType SkShader::asABitmap(SkBitmap*, SkMatrix*,
                                         TileMode*) const {
    return kNone_BitmapType;
}

SkShader::GradientType SkShader::asAGradient(GradientInfo* info) const {
    return kNone_GradientType;
}

GrEffectRef* SkShader::asNewEffect(GrContext*, const SkPaint&) const {
    return NULL;
}

SkShader* SkShader::CreateBitmapShader(const SkBitmap& src,
                                       TileMode tmx, TileMode tmy) {
    return ::CreateBitmapShader(src, tmx, tmy, NULL);
}

#ifndef SK_IGNORE_TO_STRING
void SkShader::toString(SkString* str) const {
    if (this->hasLocalMatrix()) {
        str->append(" ");
        this->getLocalMatrix().toString(str);
    }
}
#endif

//////////////////////////////////////////////////////////////////////////////

#include "SkColorShader.h"
#include "SkUtils.h"

SkColorShader::SkColorShader() {
    fFlags = 0;
    fInheritColor = true;
}

SkColorShader::SkColorShader(SkColor c) {
    fFlags = 0;
    fColor = c;
    fInheritColor = false;
}

SkColorShader::~SkColorShader() {}

bool SkColorShader::isOpaque() const {
    if (fInheritColor) {
        return true; // using paint's alpha
    }
    return SkColorGetA(fColor) == 255;
}

SkColorShader::SkColorShader(SkReadBuffer& b) : INHERITED(b) {
    fFlags = 0; // computed in setContext

    fInheritColor = b.readBool();
    if (fInheritColor) {
        return;
    }
    fColor = b.readColor();
}

void SkColorShader::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeBool(fInheritColor);
    if (fInheritColor) {
        return;
    }
    buffer.writeColor(fColor);
}

uint32_t SkColorShader::getFlags() {
    return fFlags;
}

uint8_t SkColorShader::getSpan16Alpha() const {
    return SkGetPackedA32(fPMColor);
}

bool SkColorShader::setContext(const SkBitmap& device, const SkPaint& paint,
                               const SkMatrix& matrix) {
    if (!this->INHERITED::setContext(device, paint, matrix)) {
        return false;
    }

    unsigned a;

    if (fInheritColor) {
        fColor = paint.getColor();
        a = SkColorGetA(fColor);
    } else {
        a = SkAlphaMul(SkColorGetA(fColor), SkAlpha255To256(paint.getAlpha()));
    }

    unsigned r = SkColorGetR(fColor);
    unsigned g = SkColorGetG(fColor);
    unsigned b = SkColorGetB(fColor);

    // we want this before we apply any alpha
    fColor16 = SkPack888ToRGB16(r, g, b);

    if (a != 255) {
        r = SkMulDiv255Round(r, a);
        g = SkMulDiv255Round(g, a);
        b = SkMulDiv255Round(b, a);
    }
    fPMColor = SkPackARGB32(a, r, g, b);

    fFlags = kConstInY32_Flag;
    if (255 == a) {
        fFlags |= kOpaqueAlpha_Flag;
        if (paint.isDither() == false) {
            fFlags |= kHasSpan16_Flag;
        }
    }

    return true;
}

void SkColorShader::shadeSpan(int x, int y, SkPMColor span[], int count) {
    sk_memset32(span, fPMColor, count);
}

void SkColorShader::shadeSpan16(int x, int y, uint16_t span[], int count) {
    sk_memset16(span, fColor16, count);
}

void SkColorShader::shadeSpanAlpha(int x, int y, uint8_t alpha[], int count) {
    memset(alpha, SkGetPackedA32(fPMColor), count);
}

// if we had a asAColor method, that would be more efficient...
SkShader::BitmapType SkColorShader::asABitmap(SkBitmap* bitmap, SkMatrix* matrix,
                                              TileMode modes[]) const {
    return kNone_BitmapType;
}

SkShader::GradientType SkColorShader::asAGradient(GradientInfo* info) const {
    if (info) {
        if (info->fColors && info->fColorCount >= 1) {
            info->fColors[0] = fColor;
        }
        info->fColorCount = 1;
        info->fTileMode = SkShader::kRepeat_TileMode;
    }
    return kColor_GradientType;
}

#ifndef SK_IGNORE_TO_STRING
void SkColorShader::toString(SkString* str) const {
    str->append("SkColorShader: (");

    if (fInheritColor) {
        str->append("Color: inherited from paint");
    } else {
        str->append("Color: ");
        str->appendHex(fColor);
    }

    this->INHERITED::toString(str);

    str->append(")");
}
#endif

///////////////////////////////////////////////////////////////////////////////

#include "SkEmptyShader.h"

uint32_t SkEmptyShader::getFlags() { return 0; }
uint8_t SkEmptyShader::getSpan16Alpha() const { return 0; }

bool SkEmptyShader::setContext(const SkBitmap&, const SkPaint&,
                               const SkMatrix&) { return false; }

void SkEmptyShader::shadeSpan(int x, int y, SkPMColor span[], int count) {
    SkDEBUGFAIL("should never get called, since setContext() returned false");
}

void SkEmptyShader::shadeSpan16(int x, int y, uint16_t span[], int count) {
    SkDEBUGFAIL("should never get called, since setContext() returned false");
}

void SkEmptyShader::shadeSpanAlpha(int x, int y, uint8_t alpha[], int count) {
    SkDEBUGFAIL("should never get called, since setContext() returned false");
}

#ifndef SK_IGNORE_TO_STRING
void SkEmptyShader::toString(SkString* str) const {
    str->append("SkEmptyShader: (");

    this->INHERITED::toString(str);

    str->append(")");
}
#endif
