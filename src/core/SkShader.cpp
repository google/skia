/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapProcShader.h"
#include "SkEmptyShader.h"
#include "SkReadBuffer.h"
#include "SkMallocPixelRef.h"
#include "SkPaint.h"
#include "SkPicture.h"
#include "SkPictureShader.h"
#include "SkScalar.h"
#include "SkShader.h"
#include "SkThread.h"
#include "SkWriteBuffer.h"

//#define SK_TRACK_SHADER_LIFETIME

#ifdef SK_TRACK_SHADER_LIFETIME
    static int32_t gShaderCounter;
#endif

static inline void inc_shader_counter() {
#ifdef SK_TRACK_SHADER_LIFETIME
    int32_t prev = sk_atomic_inc(&gShaderCounter);
    SkDebugf("+++ shader counter %d\n", prev + 1);
#endif
}
static inline void dec_shader_counter() {
#ifdef SK_TRACK_SHADER_LIFETIME
    int32_t prev = sk_atomic_dec(&gShaderCounter);
    SkDebugf("--- shader counter %d\n", prev - 1);
#endif
}

SkShader::SkShader(const SkMatrix* localMatrix) {
    inc_shader_counter();
    if (localMatrix) {
        fLocalMatrix = *localMatrix;
    } else {
        fLocalMatrix.reset();
    }
}

SkShader::SkShader(SkReadBuffer& buffer) : INHERITED(buffer) {
    inc_shader_counter();
    if (buffer.readBool()) {
        buffer.readMatrix(&fLocalMatrix);
    } else {
        fLocalMatrix.reset();
    }
}

SkShader::~SkShader() {
    dec_shader_counter();
}

void SkShader::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    bool hasLocalM = !fLocalMatrix.isIdentity();
    buffer.writeBool(hasLocalM);
    if (hasLocalM) {
        buffer.writeMatrix(fLocalMatrix);
    }
}

bool SkShader::computeTotalInverse(const ContextRec& rec, SkMatrix* totalInverse) const {
    SkMatrix total;
    total.setConcat(*rec.fMatrix, fLocalMatrix);

    const SkMatrix* m = &total;
    if (rec.fLocalMatrix) {
        total.setConcat(*m, *rec.fLocalMatrix);
        m = &total;
    }
    return m->invert(totalInverse);
}

SkShader::Context* SkShader::createContext(const ContextRec& rec, void* storage) const {
    if (!this->computeTotalInverse(rec, NULL)) {
        return NULL;
    }
    return this->onCreateContext(rec, storage);
}

SkShader::Context* SkShader::onCreateContext(const ContextRec& rec, void*) const {
    return NULL;
}

size_t SkShader::contextSize() const {
    return 0;
}

SkShader::Context::Context(const SkShader& shader, const ContextRec& rec)
    : fShader(shader), fCTM(*rec.fMatrix)
{
    // Because the context parameters must be valid at this point, we know that the matrix is
    // invertible.
    SkAssertResult(fShader.computeTotalInverse(rec, &fTotalInverse));
    fTotalInverseClass = (uint8_t)ComputeMatrixClass(fTotalInverse);

    fPaintAlpha = rec.fPaint->getAlpha();
}

SkShader::Context::~Context() {}

SkShader::Context::ShadeProc SkShader::Context::asAShadeProc(void** ctx) {
    return NULL;
}

#include "SkColorPriv.h"

void SkShader::Context::shadeSpan16(int x, int y, uint16_t span16[], int count) {
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

void SkShader::Context::shadeSpanAlpha(int x, int y, uint8_t alpha[], int count) {
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

SkShader::Context::MatrixClass SkShader::Context::ComputeMatrixClass(const SkMatrix& mat) {
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

SkShader::BitmapType SkShader::asABitmap(SkBitmap*, SkMatrix*, TileMode*) const {
    return kNone_BitmapType;
}

SkShader::GradientType SkShader::asAGradient(GradientInfo* info) const {
    return kNone_GradientType;
}

bool SkShader::asNewEffect(GrContext* context, const SkPaint& paint,
                           const SkMatrix* localMatrixOrNull, GrColor* paintColor,
                           GrEffect** effect)  const {
    return false;
}

SkShader* SkShader::refAsALocalMatrixShader(SkMatrix*) const {
    return NULL;
}

SkShader* SkShader::CreateEmptyShader() {
    return SkNEW(SkEmptyShader);
}

SkShader* SkShader::CreateBitmapShader(const SkBitmap& src, TileMode tmx, TileMode tmy,
                                       const SkMatrix* localMatrix) {
    return ::CreateBitmapShader(src, tmx, tmy, localMatrix, NULL);
}

SkShader* SkShader::CreatePictureShader(SkPicture* src, TileMode tmx, TileMode tmy,
                                       const SkMatrix* localMatrix) {
    return SkPictureShader::Create(src, tmx, tmy, localMatrix);
}

#ifndef SK_IGNORE_TO_STRING
void SkShader::toString(SkString* str) const {
    if (!fLocalMatrix.isIdentity()) {
        str->append(" ");
        fLocalMatrix.toString(str);
    }
}
#endif

//////////////////////////////////////////////////////////////////////////////

#include "SkColorShader.h"
#include "SkUtils.h"

SkColorShader::SkColorShader(SkColor c)
    : fColor(c) {
}

bool SkColorShader::isOpaque() const {
    return SkColorGetA(fColor) == 255;
}

SkColorShader::SkColorShader(SkReadBuffer& b) : INHERITED(b) {
    // V25_COMPATIBILITY_CODE We had a boolean to make the color shader inherit the paint's
    // color. We don't support that any more.
    if (b.isVersionLT(SkReadBuffer::kColorShaderNoBool_Version)) {
        if (b.readBool()) {
            SkDEBUGFAIL("We shouldn't have pictures that recorded the inherited case.");
            fColor = SK_ColorWHITE;
            return;
        }
    }
    fColor = b.readColor();
}

void SkColorShader::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeColor(fColor);
}

uint32_t SkColorShader::ColorShaderContext::getFlags() const {
    return fFlags;
}

uint8_t SkColorShader::ColorShaderContext::getSpan16Alpha() const {
    return SkGetPackedA32(fPMColor);
}

SkShader::Context* SkColorShader::onCreateContext(const ContextRec& rec, void* storage) const {
    return SkNEW_PLACEMENT_ARGS(storage, ColorShaderContext, (*this, rec));
}

SkColorShader::ColorShaderContext::ColorShaderContext(const SkColorShader& shader,
                                                      const ContextRec& rec)
    : INHERITED(shader, rec)
{
    SkColor color = shader.fColor;
    unsigned a = SkAlphaMul(SkColorGetA(color), SkAlpha255To256(rec.fPaint->getAlpha()));

    unsigned r = SkColorGetR(color);
    unsigned g = SkColorGetG(color);
    unsigned b = SkColorGetB(color);

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
        if (rec.fPaint->isDither() == false) {
            fFlags |= kHasSpan16_Flag;
        }
    }
}

void SkColorShader::ColorShaderContext::shadeSpan(int x, int y, SkPMColor span[], int count) {
    sk_memset32(span, fPMColor, count);
}

void SkColorShader::ColorShaderContext::shadeSpan16(int x, int y, uint16_t span[], int count) {
    sk_memset16(span, fColor16, count);
}

void SkColorShader::ColorShaderContext::shadeSpanAlpha(int x, int y, uint8_t alpha[], int count) {
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

#if SK_SUPPORT_GPU

#include "SkGr.h"

bool SkColorShader::asNewEffect(GrContext* context, const SkPaint& paint,
                                const SkMatrix* localMatrix, GrColor* paintColor,
                                GrEffect** effect) const {
    *effect = NULL;
    SkColor skColor = fColor;
    U8CPU newA = SkMulDiv255Round(SkColorGetA(fColor), paint.getAlpha());
    *paintColor = SkColor2GrColor(SkColorSetA(skColor, newA));
    return true;
}

#else

bool SkColorShader::asNewEffect(GrContext* context, const SkPaint& paint,
                                     const SkMatrix* localMatrix, GrColor* paintColor,
                                     GrEffect** effect) const {
    SkDEBUGFAIL("Should not call in GPU-less build");
    return false;
}

#endif

#ifndef SK_IGNORE_TO_STRING
void SkColorShader::toString(SkString* str) const {
    str->append("SkColorShader: (");

    str->append("Color: ");
    str->appendHex(fColor);

    this->INHERITED::toString(str);

    str->append(")");
}
#endif

///////////////////////////////////////////////////////////////////////////////

#ifndef SK_IGNORE_TO_STRING
#include "SkEmptyShader.h"

void SkEmptyShader::toString(SkString* str) const {
    str->append("SkEmptyShader: (");

    this->INHERITED::toString(str);

    str->append(")");
}
#endif
