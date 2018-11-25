/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArenaAlloc.h"
#include "SkColorShader.h"
#include "SkColorSpace.h"
#include "SkPM4fPriv.h"
#include "SkRasterPipeline.h"
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

SkShaderBase::Context* SkColorShader::onMakeContext(const ContextRec& rec,
                                                    SkArenaAlloc* alloc) const {
    return alloc->make<ColorShaderContext>(*this, rec);
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
std::unique_ptr<GrFragmentProcessor> SkColorShader::asFragmentProcessor(
        const GrFPArgs& args) const {
    GrColor4f color = SkColorToPremulGrColor4f(fColor, *args.fDstColorSpaceInfo);
    return GrConstColorProcessor::Make(color, GrConstColorProcessor::InputMode::kModulateA);
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
    buffer.readColor4f(&color);
    if (buffer.readBool()) {
        // TODO how do we unflatten colorspaces
    }
    return SkShader::MakeColorShader(color, nullptr);
}

void SkColor4Shader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeColor4f(fColor4);
    buffer.writeBool(false);    // TODO how do we flatten colorspaces?
}

uint32_t SkColor4Shader::Color4Context::getFlags() const {
    return fFlags;
}

SkShaderBase::Context* SkColor4Shader::onMakeContext(const ContextRec& rec,
                                                     SkArenaAlloc* alloc) const {
    return alloc->make<Color4Context>(*this, rec);
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

#include "GrColorSpaceInfo.h"
#include "GrColorSpaceXform.h"
#include "SkGr.h"
#include "effects/GrConstColorProcessor.h"

std::unique_ptr<GrFragmentProcessor> SkColor4Shader::asFragmentProcessor(
        const GrFPArgs& args) const {
    // Construct an xform assuming float inputs. The color space can have a transfer function on
    // it, which will be applied below.
    auto colorSpaceXform = GrColorSpaceXform::Make(fColorSpace.get(), kRGBA_float_GrPixelConfig,
                                                   args.fDstColorSpaceInfo->colorSpace());
    GrColor4f color = GrColor4f::FromSkColor4f(fColor4);
    if (colorSpaceXform) {
        color = colorSpaceXform->clampedXform(color);
    }
    return GrConstColorProcessor::Make(color.premul(),
                                       GrConstColorProcessor::InputMode::kModulateA);
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

sk_sp<SkShader> SkColor4Shader::onMakeColorSpace(SkColorSpaceXformer* xformer) const {
    return SkShader::MakeColorShader(xformer->apply(fCachedByteColor));
}

sk_sp<SkShader> SkShader::MakeColorShader(const SkColor4f& color, sk_sp<SkColorSpace> space) {
    if (!SkScalarsAreFinite(color.vec(), 4)) {
        return nullptr;
    }
    return sk_make_sp<SkColor4Shader>(color, std::move(space));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool SkColorShader::onAppendStages(const StageRec& rec) const {
    rec.fPipeline->append_constant_color(rec.fAlloc, SkPM4f_from_SkColor(fColor, rec.fDstCS));
    return true;
}

bool SkColor4Shader::onAppendStages(const StageRec& rec) const {
    rec.fPipeline->append_constant_color(
                     rec.fAlloc, to_colorspace(fColor4, fColorSpace.get(), rec.fDstCS).premul());
    return true;
}
