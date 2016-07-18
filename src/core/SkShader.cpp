/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAtomics.h"
#include "SkBitmapProcShader.h"
#include "SkColorShader.h"
#include "SkEmptyShader.h"
#include "SkMallocPixelRef.h"
#include "SkPaint.h"
#include "SkPicture.h"
#include "SkPictureShader.h"
#include "SkReadBuffer.h"
#include "SkScalar.h"
#include "SkShader.h"
#include "SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "GrFragmentProcessor.h"
#endif

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
    // Pre-cache so future calls to fLocalMatrix.getType() are threadsafe.
    (void)fLocalMatrix.getType();
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

bool SkShader::asLuminanceColor(SkColor* colorPtr) const {
    SkColor storage;
    if (nullptr == colorPtr) {
        colorPtr = &storage;
    }
    if (this->onAsLuminanceColor(colorPtr)) {
        *colorPtr = SkColorSetA(*colorPtr, 0xFF);   // we only return opaque
        return true;
    }
    return false;
}

SkShader::Context* SkShader::createContext(const ContextRec& rec, void* storage) const {
    if (!this->computeTotalInverse(rec, nullptr)) {
        return nullptr;
    }
    return this->onCreateContext(rec, storage);
}

SkShader::Context* SkShader::onCreateContext(const ContextRec& rec, void*) const {
    return nullptr;
}

size_t SkShader::contextSize(const ContextRec& rec) const {
    return this->onContextSize(rec);
}

size_t SkShader::onContextSize(const ContextRec&) const {
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
    return nullptr;
}

void SkShader::Context::shadeSpan4f(int x, int y, SkPM4f dst[], int count) {
    const int N = 128;
    SkPMColor tmp[N];
    while (count > 0) {
        int n = SkTMin(count, N);
        this->shadeSpan(x, y, tmp, n);
        for (int i = 0; i < n; ++i) {
            dst[i] = SkPM4f::FromPMColor(tmp[i]);
        }
        dst += n;
        x += n;
        count -= n;
    }
}

#include "SkColorPriv.h"

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
        if (mat.isFixedStepInX()) {
            mc = kFixedStepInX_MatrixClass;
        } else {
            mc = kPerspective_MatrixClass;
        }
    }
    return mc;
}

//////////////////////////////////////////////////////////////////////////////

SkShader::GradientType SkShader::asAGradient(GradientInfo* info) const {
    return kNone_GradientType;
}

#if SK_SUPPORT_GPU
sk_sp<GrFragmentProcessor> SkShader::asFragmentProcessor(GrContext*, const SkMatrix&,
                                                         const SkMatrix*, SkFilterQuality,
                                                         SkSourceGammaTreatment) const {
    return nullptr;
}
#endif

SkShader* SkShader::refAsALocalMatrixShader(SkMatrix*) const {
    return nullptr;
}

sk_sp<SkShader> SkShader::MakeEmptyShader() { return sk_make_sp<SkEmptyShader>(); }

sk_sp<SkShader> SkShader::MakeColorShader(SkColor color) { return sk_make_sp<SkColorShader>(color); }

sk_sp<SkShader> SkShader::MakeBitmapShader(const SkBitmap& src, TileMode tmx, TileMode tmy,
                                           const SkMatrix* localMatrix) {
    return SkMakeBitmapShader(src, tmx, tmy, localMatrix, nullptr);
}

sk_sp<SkShader> SkShader::MakePictureShader(sk_sp<SkPicture> src, TileMode tmx, TileMode tmy,
                                            const SkMatrix* localMatrix, const SkRect* tile) {
    return SkPictureShader::Make(std::move(src), tmx, tmy, localMatrix, tile);
}

#ifndef SK_IGNORE_TO_STRING
void SkShader::toString(SkString* str) const {
    if (!fLocalMatrix.isIdentity()) {
        str->append(" ");
        fLocalMatrix.toString(str);
    }
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkFlattenable> SkEmptyShader::CreateProc(SkReadBuffer&) {
    return SkShader::MakeEmptyShader();
}

#ifndef SK_IGNORE_TO_STRING
#include "SkEmptyShader.h"

void SkEmptyShader::toString(SkString* str) const {
    str->append("SkEmptyShader: (");

    this->INHERITED::toString(str);

    str->append(")");
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_SUPPORT_LEGACY_CREATESHADER_PTR
SkShader* SkShader::CreateComposeShader(SkShader* dst, SkShader* src, SkXfermode::Mode mode) {
    return MakeComposeShader(sk_ref_sp(dst), sk_ref_sp(src), mode).release();
}
SkShader* SkShader::CreateComposeShader(SkShader* dst, SkShader* src, SkXfermode* xfer) {
    return MakeComposeShader(sk_ref_sp(dst), sk_ref_sp(src), xfer).release();
}
SkShader* SkShader::CreatePictureShader(const SkPicture* src, TileMode tmx, TileMode tmy,
                                     const SkMatrix* localMatrix, const SkRect* tile) {
    return MakePictureShader(sk_ref_sp(const_cast<SkPicture*>(src)), tmx, tmy,
                             localMatrix, tile).release();
}
SkShader* SkShader::newWithColorFilter(SkColorFilter* filter) const {
    return this->makeWithColorFilter(sk_ref_sp(filter)).release();
}
#endif

#ifdef SK_SUPPORT_LEGACY_XFERMODE_PTR
#include "SkXfermode.h"
sk_sp<SkShader> SkShader::MakeComposeShader(sk_sp<SkShader> dst, sk_sp<SkShader> src,
                                            SkXfermode* xfer) {
    return MakeComposeShader(std::move(dst), std::move(src), sk_ref_sp(xfer));
}
#endif
