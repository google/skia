/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArenaAlloc.h"
#include "SkColorShader.h"
#include "SkColorSpace.h"
#include "SkColorSpacePriv.h"
#include "SkColorSpaceXformSteps.h"
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

#ifdef SK_ENABLE_LEGACY_SHADERCONTEXT
SkShaderBase::Context* SkColorShader::onMakeContext(const ContextRec& rec,
                                                    SkArenaAlloc* alloc) const {
    return alloc->make<ColorShaderContext>(*this, rec);
}
#endif

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
    fPMColor4f = c4.premul();

    fFlags = kConstInY32_Flag;
    if (255 == a) {
        fFlags |= kOpaqueAlpha_Flag;
    }
}

void SkColorShader::ColorShaderContext::shadeSpan(int x, int y, SkPMColor span[], int count) {
    sk_memset32(span, fPMColor, count);
}

void SkColorShader::ColorShaderContext::shadeSpan4f(int x, int y, SkPMColor4f span[], int count) {
    for (int i = 0; i < count; ++i) {
        span[i] = fPMColor4f;
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

SkColor4Shader::SkColor4Shader(const SkColor4f& color, sk_sp<SkColorSpace> space)
    : fColorSpace(std::move(space))
    , fColor(color)
{}

sk_sp<SkFlattenable> SkColor4Shader::CreateProc(SkReadBuffer& buffer) {
    SkColor4f color;
    sk_sp<SkColorSpace> colorSpace;
    buffer.readColor4f(&color);
    if (buffer.readBool()) {
        sk_sp<SkData> data = buffer.readByteArrayAsData();
        colorSpace = data ? SkColorSpace::Deserialize(data->data(), data->size()) : nullptr;
    }
    return SkShader::MakeColorShader(color, std::move(colorSpace));
}

void SkColor4Shader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeColor4f(fColor);
    sk_sp<SkData> colorSpaceData = fColorSpace ? fColorSpace->serialize() : nullptr;
    if (colorSpaceData) {
        buffer.writeBool(true);
        buffer.writeDataAsByteArray(colorSpaceData.get());
    } else {
        buffer.writeBool(false);
    }
}


sk_sp<SkShader> SkColor4Shader::onMakeColorSpace(SkColorSpaceXformer* xformer) const {
    SkColor4f color = fColor;
    SkColorSpaceXformSteps(fColorSpace.get(),    kUnpremul_SkAlphaType,
                           xformer->dst().get(), kUnpremul_SkAlphaType).apply(color.vec());
    return SkShader::MakeColorShader(color.toSkColor());
}

sk_sp<SkShader> SkShader::MakeColorShader(const SkColor4f& color, sk_sp<SkColorSpace> space) {
    if (!SkScalarsAreFinite(color.vec(), 4)) {
        return nullptr;
    }
    return sk_make_sp<SkColor4Shader>(color, std::move(space));
}

bool SkColorShader::onAppendStages(const StageRec& rec) const {
    SkColor4f color = SkColor4f::FromColor(fColor);
    SkColorSpaceXformSteps(sk_srgb_singleton(), kUnpremul_SkAlphaType,
                           rec.fDstCS,          kUnpremul_SkAlphaType).apply(color.vec());
    rec.fPipeline->append_constant_color(rec.fAlloc, color.premul().vec());
    return true;
}

bool SkColor4Shader::onAppendStages(const StageRec& rec) const {
    SkColor4f color = fColor;
    SkColorSpaceXformSteps(fColorSpace.get(), kUnpremul_SkAlphaType,
                           rec.fDstCS,        kUnpremul_SkAlphaType).apply(color.vec());
    rec.fPipeline->append_constant_color(rec.fAlloc, color.premul().vec());
    return true;
}

#if SK_SUPPORT_GPU

#include "GrColorSpaceInfo.h"
#include "GrColorSpaceXform.h"
#include "SkGr.h"
#include "effects/GrConstColorProcessor.h"

std::unique_ptr<GrFragmentProcessor> SkColorShader::asFragmentProcessor(
        const GrFPArgs& args) const {
    SkPMColor4f color = SkColorToPMColor4f(fColor, *args.fDstColorSpaceInfo);
    return GrConstColorProcessor::Make(color, GrConstColorProcessor::InputMode::kModulateA);
}

std::unique_ptr<GrFragmentProcessor> SkColor4Shader::asFragmentProcessor(
        const GrFPArgs& args) const {
    SkColorSpaceXformSteps steps{ fColorSpace.get(),                     kUnpremul_SkAlphaType,
                                  args.fDstColorSpaceInfo->colorSpace(), kUnpremul_SkAlphaType };
    SkColor4f color = fColor;
    steps.apply(color.vec());
    return GrConstColorProcessor::Make(color.premul(),
                                       GrConstColorProcessor::InputMode::kModulateA);
}

#endif
