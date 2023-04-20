/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorSpace.h"
#include "include/core/SkFlattenable.h"
#include "src/base/SkArenaAlloc.h"
#include "src/base/SkUtils.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkVM.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkShaderBase.h"

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#endif

/** \class SkColorShader
    A Shader that represents a single color. In general, this effect can be
    accomplished by just using the color field on the paint, but if an
    actual shader object is needed, this provides that feature.
*/
class SkColorShader : public SkShaderBase {
public:
    /** Create a ColorShader that ignores the color in the paint, and uses the
        specified color. Note: like all shaders, at draw time the paint's alpha
        will be respected, and is applied to the specified color.
    */
    explicit SkColorShader(SkColor c);

    bool isOpaque() const override;
    bool isConstant() const override { return true; }

    GradientType asGradient(GradientInfo* info, SkMatrix* localMatrix) const override;

#if defined(SK_GANESH)
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&,
                                                             const MatrixRec&) const override;
#endif

#if defined(SK_GRAPHITE)
    void addToKey(const skgpu::graphite::KeyContext&,
                  skgpu::graphite::PaintParamsKeyBuilder*,
                  skgpu::graphite::PipelineDataGatherer*) const override;
#endif

private:
    friend void ::SkRegisterColorShaderFlattenable();
    SK_FLATTENABLE_HOOKS(SkColorShader)

    void flatten(SkWriteBuffer&) const override;

    bool onAsLuminanceColor(SkColor* lum) const override {
        *lum = fColor;
        return true;
    }

    bool appendStages(const SkStageRec&, const MatrixRec&) const override;

    skvm::Color program(skvm::Builder*,
                        skvm::Coord device,
                        skvm::Coord local,
                        skvm::Color paint,
                        const MatrixRec&,
                        const SkColorInfo& dst,
                        skvm::Uniforms* uniforms,
                        SkArenaAlloc*) const override;

    SkColor fColor;
};

class SkColor4Shader : public SkShaderBase {
public:
    SkColor4Shader(const SkColor4f&, sk_sp<SkColorSpace>);

    bool isOpaque()   const override { return fColor.isOpaque(); }
    bool isConstant() const override { return true; }

#if defined(SK_GANESH)
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&,
                                                             const MatrixRec&) const override;
#endif
#if defined(SK_GRAPHITE)
    void addToKey(const skgpu::graphite::KeyContext&,
                  skgpu::graphite::PaintParamsKeyBuilder*,
                  skgpu::graphite::PipelineDataGatherer*) const override;
#endif

private:
    friend void ::SkRegisterColor4ShaderFlattenable();
    SK_FLATTENABLE_HOOKS(SkColor4Shader)

    void flatten(SkWriteBuffer&) const override;
    bool appendStages(const SkStageRec&, const MatrixRec&) const override;

    skvm::Color program(skvm::Builder*,
                        skvm::Coord device,
                        skvm::Coord local,
                        skvm::Color paint,
                        const MatrixRec&,
                        const SkColorInfo& dst,
                        skvm::Uniforms* uniforms,
                        SkArenaAlloc*) const override;

    sk_sp<SkColorSpace> fColorSpace;
    const SkColor4f     fColor;
};

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

SkShaderBase::GradientType SkColorShader::asGradient(GradientInfo* info,
                                                     SkMatrix* localMatrix) const {
    if (info) {
        if (info->fColors && info->fColorCount >= 1) {
            info->fColors[0] = fColor;
        }
        info->fColorCount = 1;
        info->fTileMode = SkTileMode::kRepeat;
    }
    if (localMatrix) {
        *localMatrix = SkMatrix::I();
    }
    return GradientType::kColor;
}

SkColor4Shader::SkColor4Shader(const SkColor4f& color, sk_sp<SkColorSpace> space)
    : fColorSpace(std::move(space))
    , fColor({color.fR, color.fG, color.fB, SkTPin(color.fA, 0.0f, 1.0f)})
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

bool SkColorShader::appendStages(const SkStageRec& rec, const MatrixRec&) const {
    SkColor4f color = SkColor4f::FromColor(fColor);
    SkColorSpaceXformSteps(sk_srgb_singleton(), kUnpremul_SkAlphaType,
                           rec.fDstCS,          kUnpremul_SkAlphaType).apply(color.vec());
    rec.fPipeline->append_constant_color(rec.fAlloc, color.premul().vec());
    return true;
}

bool SkColor4Shader::appendStages(const SkStageRec& rec, const MatrixRec&) const {
    SkColor4f color = fColor;
    SkColorSpaceXformSteps(fColorSpace.get(), kUnpremul_SkAlphaType,
                           rec.fDstCS,        kUnpremul_SkAlphaType).apply(color.vec());
    rec.fPipeline->append_constant_color(rec.fAlloc, color.premul().vec());
    return true;
}

skvm::Color SkColorShader::program(skvm::Builder* p,
                                   skvm::Coord /*device*/,
                                   skvm::Coord /*local*/,
                                   skvm::Color /*paint*/,
                                   const MatrixRec&,
                                   const SkColorInfo& dst,
                                   skvm::Uniforms* uniforms,
                                   SkArenaAlloc*) const {
    SkColor4f color = SkColor4f::FromColor(fColor);
    SkColorSpaceXformSteps(sk_srgb_singleton(), kUnpremul_SkAlphaType,
                              dst.colorSpace(),   kPremul_SkAlphaType).apply(color.vec());
    return p->uniformColor(color, uniforms);
}
skvm::Color SkColor4Shader::program(skvm::Builder* p,
                                    skvm::Coord /*device*/,
                                    skvm::Coord /*local*/,
                                    skvm::Color /*paint*/,
                                    const MatrixRec&,
                                    const SkColorInfo& dst,
                                    skvm::Uniforms* uniforms,
                                    SkArenaAlloc*) const {
    SkColor4f color = fColor;
    SkColorSpaceXformSteps(fColorSpace.get(), kUnpremul_SkAlphaType,
                            dst.colorSpace(),   kPremul_SkAlphaType).apply(color.vec());
    return p->uniformColor(color, uniforms);
}

#if defined(SK_GANESH)

#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrColorSpaceXform.h"
#include "src/gpu/ganesh/GrFPArgs.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/SkGr.h"

std::unique_ptr<GrFragmentProcessor> SkColorShader::asFragmentProcessor(const GrFPArgs& args,
                                                                        const MatrixRec&) const {
    return GrFragmentProcessor::MakeColor(SkColorToPMColor4f(fColor, *args.fDstColorInfo));
}

std::unique_ptr<GrFragmentProcessor> SkColor4Shader::asFragmentProcessor(const GrFPArgs& args,
                                                                         const MatrixRec&) const {
    SkColorSpaceXformSteps steps{ fColorSpace.get(),                kUnpremul_SkAlphaType,
                                  args.fDstColorInfo->colorSpace(), kUnpremul_SkAlphaType };
    SkColor4f color = fColor;
    steps.apply(color.vec());
    return GrFragmentProcessor::MakeColor(color.premul());
}

#endif

#if defined(SK_GRAPHITE)
void SkColorShader::addToKey(const skgpu::graphite::KeyContext& keyContext,
                             skgpu::graphite::PaintParamsKeyBuilder* builder,
                             skgpu::graphite::PipelineDataGatherer* gatherer) const {
    using namespace skgpu::graphite;

    SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer,
                                      SkColor4f::FromColor(fColor).premul());
    builder->endBlock();
}

void SkColor4Shader::addToKey(const skgpu::graphite::KeyContext& keyContext,
                              skgpu::graphite::PaintParamsKeyBuilder* builder,
                              skgpu::graphite::PipelineDataGatherer* gatherer) const {
    using namespace skgpu::graphite;

    SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer, fColor.premul());
    builder->endBlock();
}
#endif

sk_sp<SkShader> SkShaders::Color(SkColor color) { return sk_make_sp<SkColorShader>(color); }

sk_sp<SkShader> SkShaders::Color(const SkColor4f& color, sk_sp<SkColorSpace> space) {
    if (!SkScalarsAreFinite(color.vec(), 4)) {
        return nullptr;
    }
    return sk_make_sp<SkColor4Shader>(color, std::move(space));
}

void SkRegisterColor4ShaderFlattenable() {
    SK_REGISTER_FLATTENABLE(SkColor4Shader);
}

void SkRegisterColorShaderFlattenable() {
    SK_REGISTER_FLATTENABLE(SkColorShader);
}
