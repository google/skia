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

SkShader::GradientType SkColorShader::asAGradient(GradientInfo* info) const {
    if (info) {
        if (info->fColors && info->fColorCount >= 1) {
            info->fColors[0] = fColor;
        }
        info->fColorCount = 1;
#ifdef SK_SUPPORT_LEGACY_TILEMODE_ENUM
        info->fTileMode = SkShader::kRepeat_TileMode;
#else
        info->fTileMode = SkTileMode::kRepeat;
#endif
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
    return SkShaders::Color(color, std::move(colorSpace));
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


#ifdef SK_SUPPORT_LEGACY_SHADER_FACTORIES
sk_sp<SkShader> SkShader::MakeColorShader(const SkColor4f& color, sk_sp<SkColorSpace> space) {
    return SkShaders::Color(color, std::move(space));
}
#endif

sk_sp<SkShader> SkShaders::Color(const SkColor4f& color, sk_sp<SkColorSpace> space) {
    if (!SkScalarsAreFinite(color.vec(), 4)) {
        return nullptr;
    }
    return sk_make_sp<SkColor4Shader>(color, std::move(space));
}

bool SkColorShader::onAppendStages(const SkStageRec& rec) const {
    SkColor4f color = SkColor4f::FromColor(fColor);
    SkColorSpaceXformSteps(sk_srgb_singleton(), kUnpremul_SkAlphaType,
                           rec.fDstCS,          kUnpremul_SkAlphaType).apply(color.vec());
    rec.fPipeline->append_constant_color(rec.fAlloc, color.premul().vec());
    return true;
}

bool SkColor4Shader::onAppendStages(const SkStageRec& rec) const {
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
