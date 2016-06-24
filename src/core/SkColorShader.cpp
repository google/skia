/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorShader.h"
#include "SkColorSpace.h"
#include "SkReadBuffer.h"
#include "SkUtils.h"

SkColorShader::SkColorShader(SkColor c) : fColor(c) {}

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
sk_sp<GrFragmentProcessor> SkColorShader::asFragmentProcessor(GrContext*, const SkMatrix&,
                                                              const SkMatrix*,
                                                              SkFilterQuality,
                                                              SkSourceGammaTreatment) const {
    GrColor color = SkColorToPremulGrColor(fColor);
    return GrConstColorProcessor::Make(color, GrConstColorProcessor::kModulateA_InputMode);
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

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

static unsigned unit_to_byte(float unit) {
    SkASSERT(unit >= 0 && unit <= 1);
    return (unsigned)(unit * 255 + 0.5);
}

static SkColor unit_to_skcolor(const SkColor4f& unit, SkColorSpace* cs) {
    return SkColorSetARGB(unit_to_byte(unit.fA), unit_to_byte(unit.fR),
                          unit_to_byte(unit.fG), unit_to_byte(unit.fB));
}

SkColor4Shader::SkColor4Shader(const SkColor4f& color, sk_sp<SkColorSpace> space)
    : fColorSpace(std::move(space))
    , fColor4(color)
    , fCachedByteColor(unit_to_skcolor(color.pin(), space.get()))
{}

sk_sp<SkFlattenable> SkColor4Shader::CreateProc(SkReadBuffer& buffer) {
    SkColor4f color;
    color.fR = buffer.readScalar(); // readFloat()
    color.fG = buffer.readScalar();
    color.fB = buffer.readScalar();
    color.fA = buffer.readScalar();
    if (buffer.readBool()) {
        // TODO how do we unflatten colorspaces
    }
    return SkShader::MakeColorShader(color, nullptr);
}

void SkColor4Shader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeScalar(fColor4.fR); // writeFloat()
    buffer.writeScalar(fColor4.fG);
    buffer.writeScalar(fColor4.fB);
    buffer.writeScalar(fColor4.fA);
    buffer.writeBool(false);    // TODO how do we flatten colorspaces?
}

uint32_t SkColor4Shader::Color4Context::getFlags() const {
    return fFlags;
}

SkShader::Context* SkColor4Shader::onCreateContext(const ContextRec& rec, void* storage) const {
    return new (storage) Color4Context(*this, rec);
}

SkColor4Shader::Color4Context::Color4Context(const SkColor4Shader& shader,
                                                      const ContextRec& rec)
: INHERITED(shader, rec)
{
    SkColor color = shader.fCachedByteColor;
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

    SkColor4f c4 = shader.fColor4;
    c4.fA *= rec.fPaint->getAlpha() * (1 / 255.0f);
    fPM4f = c4.premul();

    fFlags = kConstInY32_Flag;
    if (255 == a) {
        fFlags |= kOpaqueAlpha_Flag;
    }
}

void SkColor4Shader::Color4Context::shadeSpan(int x, int y, SkPMColor span[], int count) {
    sk_memset32(span, fPMColor, count);
}

void SkColor4Shader::Color4Context::shadeSpanAlpha(int x, int y, uint8_t alpha[], int count) {
    memset(alpha, SkGetPackedA32(fPMColor), count);
}

void SkColor4Shader::Color4Context::shadeSpan4f(int x, int y, SkPM4f span[], int count) {
    for (int i = 0; i < count; ++i) {
        span[i] = fPM4f;
    }
}

// TODO: do we need an updated version of this method for color4+colorspace?
SkShader::GradientType SkColor4Shader::asAGradient(GradientInfo* info) const {
    if (info) {
        if (info->fColors && info->fColorCount >= 1) {
            info->fColors[0] = fCachedByteColor;
        }
        info->fColorCount = 1;
        info->fTileMode = SkShader::kRepeat_TileMode;
    }
    return kColor_GradientType;
}

#if SK_SUPPORT_GPU

#include "SkGr.h"
#include "effects/GrConstColorProcessor.h"
sk_sp<GrFragmentProcessor> SkColor4Shader::asFragmentProcessor(GrContext*, const SkMatrix&,
                                                               const SkMatrix*,
                                                               SkFilterQuality,
                                                               SkSourceGammaTreatment) const {
    // TODO: how to communicate color4f to Gr
    GrColor color = SkColorToPremulGrColor(fCachedByteColor);
    return GrConstColorProcessor::Make(color, GrConstColorProcessor::kModulateA_InputMode);
}

#endif

#ifndef SK_IGNORE_TO_STRING
void SkColor4Shader::toString(SkString* str) const {
    str->append("SkColor4Shader: (");

    str->append("RGBA:");
    for (int i = 0; i < 4; ++i) {
        str->appendf(" %g", fColor4.vec()[i]);
    }
    str->append(" )");
}
#endif

sk_sp<SkShader> SkShader::MakeColorShader(const SkColor4f& color, sk_sp<SkColorSpace> space) {
    if (!SkScalarsAreFinite(color.vec(), 4)) {
        return nullptr;
    }
    return sk_make_sp<SkColor4Shader>(color, std::move(space));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

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

static void F16_BlitBW(SkShader::Context::BlitState* state, int x, int y, const SkPixmap& dst,
                       int count) {
    SkXfermode::F16Proc proc = (SkXfermode::F16Proc)state->fStorage[0];
    const SkPM4f* src = (const SkPM4f*)state->fStorage[1];
    proc(state->fXfer, dst.writable_addr64(x, y), src, count, nullptr);
}

static void F16_BlitAA(SkShader::Context::BlitState* state, int x, int y, const SkPixmap& dst,
                       int count, const SkAlpha aa[]) {
    SkXfermode::F16Proc proc = (SkXfermode::F16Proc)state->fStorage[0];
    const SkPM4f* src = (const SkPM4f*)state->fStorage[1];
    proc(state->fXfer, dst.writable_addr64(x, y), src, count, aa);
}

static bool choose_blitprocs(const SkPM4f* pm4, const SkImageInfo& info,
                             SkShader::Context::BlitState* state) {
    uint32_t flags = SkXfermode::kSrcIsSingle_D32Flag;
    if (pm4->a() == 1) {
        flags |= SkXfermode::kSrcIsOpaque_D32Flag;
    }
    switch (info.colorType()) {
        case kN32_SkColorType:
            if (info.gammaCloseToSRGB()) {
                flags |= SkXfermode::kDstIsSRGB_D32Flag;
            }
            state->fStorage[0] = (void*)SkXfermode::GetD32Proc(state->fXfer, flags);
            state->fStorage[1] = (void*)pm4;
            state->fBlitBW = D32_BlitBW;
            state->fBlitAA = D32_BlitAA;
            return true;
        case kRGBA_F16_SkColorType:
            state->fStorage[0] = (void*)SkXfermode::GetF16Proc(state->fXfer, flags);
            state->fStorage[1] = (void*)pm4;
            state->fBlitBW = F16_BlitBW;
            state->fBlitAA = F16_BlitAA;
            return true;
        default:
            return false;
    }
}

bool SkColorShader::ColorShaderContext::onChooseBlitProcs(const SkImageInfo& info,
                                                          BlitState* state) {
    return choose_blitprocs(&fPM4f, info, state);
}

bool SkColor4Shader::Color4Context::onChooseBlitProcs(const SkImageInfo& info, BlitState* state) {
    return choose_blitprocs(&fPM4f, info, state);
}
