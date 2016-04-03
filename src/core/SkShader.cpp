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

const GrFragmentProcessor* SkShader::asFragmentProcessor(GrContext*, const SkMatrix&,
                                                         const SkMatrix*, SkFilterQuality)  const {
    return nullptr;
}

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

//////////////////////////////////////////////////////////////////////////////

#include "SkUtils.h"

SkColorShader::SkColorShader(SkColor c)
    : fColor(c) {
}

bool SkColorShader::isOpaque() const {
    return SkColorGetA(fColor) == 255;
}

sk_sp<SkFlattenable> SkColorShader::CreateProc(SkReadBuffer& buffer) {
    return sk_make_sp<SkColorShader>(buffer.readColor());
}

void SkColorShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeColor(fColor);
}

uint32_t SkColorShader::ColorShaderContext::getFlags() const {
    return fFlags;
}

SkShader::Context* SkColorShader::onCreateContext(const ContextRec& rec, void* storage) const {
    return new (storage) ColorShaderContext(*this, rec);
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

    if (a != 255) {
        r = SkMulDiv255Round(r, a);
        g = SkMulDiv255Round(g, a);
        b = SkMulDiv255Round(b, a);
    }
    fPMColor = SkPackARGB32(a, r, g, b);

    SkColor4f c4 = SkColor4f::FromColor(shader.fColor);
    c4.fA *= rec.fPaint->getAlpha() / 255.0f;
    fPM4f = c4.premul();

    fFlags = kConstInY32_Flag;
    if (255 == a) {
        fFlags |= kOpaqueAlpha_Flag;
    }
}

void SkColorShader::ColorShaderContext::shadeSpan(int x, int y, SkPMColor span[], int count) {
    sk_memset32(span, fPMColor, count);
}

void SkColorShader::ColorShaderContext::shadeSpanAlpha(int x, int y, uint8_t alpha[], int count) {
    memset(alpha, SkGetPackedA32(fPMColor), count);
}

void SkColorShader::ColorShaderContext::shadeSpan4f(int x, int y, SkPM4f span[], int count) {
    for (int i = 0; i < count; ++i) {
        span[i] = fPM4f;
    }
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
#include "effects/GrConstColorProcessor.h"
const GrFragmentProcessor* SkColorShader::asFragmentProcessor(GrContext*, const SkMatrix&,
                                                              const SkMatrix*,
                                                              SkFilterQuality) const {
    GrColor color = SkColorToPremulGrColor(fColor);
    return GrConstColorProcessor::Create(color, GrConstColorProcessor::kModulateA_InputMode);
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

static void D32_BlitBW(SkShader::Context::BlitState* state, int x, int y, const SkPixmap& dst,
                       int count) {
    SkXfermode::D32Proc proc = (SkXfermode::D32Proc)state->fStorage[0];
    const SkPM4f* src = (const SkPM4f*)state->fStorage[1];
    proc(state->fXfer, dst.writable_addr32(x, y), src, count, nullptr);
}

static void D32_BlitAA(SkShader::Context::BlitState* state, int x, int y, const SkPixmap& dst,
                       int count, const SkAlpha aa[]) {
    SkXfermode::D32Proc proc = (SkXfermode::D32Proc)state->fStorage[0];
    const SkPM4f* src = (const SkPM4f*)state->fStorage[1];
    proc(state->fXfer, dst.writable_addr32(x, y), src, count, aa);
}

static void D64_BlitBW(SkShader::Context::BlitState* state, int x, int y, const SkPixmap& dst,
                       int count) {
    SkXfermode::D64Proc proc = (SkXfermode::D64Proc)state->fStorage[0];
    const SkPM4f* src = (const SkPM4f*)state->fStorage[1];
    proc(state->fXfer, dst.writable_addr64(x, y), src, count, nullptr);
}

static void D64_BlitAA(SkShader::Context::BlitState* state, int x, int y, const SkPixmap& dst,
                       int count, const SkAlpha aa[]) {
    SkXfermode::D64Proc proc = (SkXfermode::D64Proc)state->fStorage[0];
    const SkPM4f* src = (const SkPM4f*)state->fStorage[1];
    proc(state->fXfer, dst.writable_addr64(x, y), src, count, aa);
}

bool SkColorShader::ColorShaderContext::onChooseBlitProcs(const SkImageInfo& info,
                                                          BlitState* state) {
    uint32_t flags = SkXfermode::kSrcIsSingle_D32Flag;
    if (fPM4f.a() == 1) {
        flags |= SkXfermode::kSrcIsOpaque_D32Flag;
    }
    switch (info.colorType()) {
        case kN32_SkColorType:
            if (info.isSRGB()) {
                flags |= SkXfermode::kDstIsSRGB_D32Flag;
            }
            state->fStorage[0] = (void*)SkXfermode::GetD32Proc(state->fXfer, flags);
            state->fStorage[1] = &fPM4f;
            state->fBlitBW = D32_BlitBW;
            state->fBlitAA = D32_BlitAA;
            return true;
        case kRGBA_F16_SkColorType:
            flags |= SkXfermode::kDstIsFloat16_D64Flag;
            state->fStorage[0] = (void*)SkXfermode::GetD64Proc(state->fXfer, flags);
            state->fStorage[1] = &fPM4f;
            state->fBlitBW = D64_BlitBW;
            state->fBlitAA = D64_BlitAA;
            return true;
        default:
            return false;
    }
}

///////////////////////////////////////////////////////////////////////////////

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
