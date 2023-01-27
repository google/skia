/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "include/core/SkUnPreMultiply.h"
#include "include/effects/SkColorMatrix.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkColorData.h"
#include "src/core/SkColorFilterBase.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkVM.h"
#include "src/core/SkWriteBuffer.h"

#ifdef SK_GRAPHITE_ENABLED
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#endif // SK_GRAPHITE_ENABLED

static bool is_alpha_unchanged(const float matrix[20]) {
    const float* srcA = matrix + 15;

    return SkScalarNearlyZero (srcA[0])
        && SkScalarNearlyZero (srcA[1])
        && SkScalarNearlyZero (srcA[2])
        && SkScalarNearlyEqual(srcA[3], 1)
        && SkScalarNearlyZero (srcA[4]);
}

class SkColorFilter_Matrix final : public SkColorFilterBase {
public:
    enum class Domain : uint8_t { kRGBA, kHSLA };

    explicit SkColorFilter_Matrix(const float array[20], Domain);

    bool appendStages(const SkStageRec& rec, bool shaderIsOpaque) const override;

    bool onIsAlphaUnchanged() const override { return fAlphaIsUnchanged; }

#if SK_SUPPORT_GPU
    GrFPResult asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
                                   GrRecordingContext*,
                                   const GrColorInfo&,
                                   const SkSurfaceProps&) const override;
#endif
#ifdef SK_GRAPHITE_ENABLED
    void addToKey(const skgpu::graphite::KeyContext&,
                  skgpu::graphite::PaintParamsKeyBuilder*,
                  skgpu::graphite::PipelineDataGatherer*) const override;
#endif

private:
    friend void ::SkRegisterMatrixColorFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkColorFilter_Matrix)

    void flatten(SkWriteBuffer&) const override;
    bool onAsAColorMatrix(float matrix[20]) const override;

    skvm::Color onProgram(skvm::Builder*, skvm::Color,
                          const SkColorInfo& dst,
                          skvm::Uniforms* uniforms, SkArenaAlloc*) const override;

    float  fMatrix[20];
    bool   fAlphaIsUnchanged;
    Domain fDomain;
};

SkColorFilter_Matrix::SkColorFilter_Matrix(const float array[20], Domain domain)
        : fAlphaIsUnchanged(is_alpha_unchanged(array))
        , fDomain(domain) {
    memcpy(fMatrix, array, 20 * sizeof(float));
}

void SkColorFilter_Matrix::flatten(SkWriteBuffer& buffer) const {
    SkASSERT(sizeof(fMatrix)/sizeof(float) == 20);
    buffer.writeScalarArray(fMatrix, 20);

    // RGBA flag
    buffer.writeBool(fDomain == Domain::kRGBA);
}

sk_sp<SkFlattenable> SkColorFilter_Matrix::CreateProc(SkReadBuffer& buffer) {
    float matrix[20];
    if (!buffer.readScalarArray(matrix, 20)) {
        return nullptr;
    }

    auto   is_rgba = buffer.readBool();
    return is_rgba ? SkColorFilters::Matrix(matrix)
                   : SkColorFilters::HSLAMatrix(matrix);
}

bool SkColorFilter_Matrix::onAsAColorMatrix(float matrix[20]) const {
    if (matrix) {
        memcpy(matrix, fMatrix, 20 * sizeof(float));
    }
    return true;
}

bool SkColorFilter_Matrix::appendStages(const SkStageRec& rec, bool shaderIsOpaque) const {
    const bool willStayOpaque = shaderIsOpaque && fAlphaIsUnchanged,
                         hsla = fDomain == Domain::kHSLA;

    SkRasterPipeline* p = rec.fPipeline;
    if (!shaderIsOpaque) { p->append(SkRasterPipelineOp::unpremul); }
    if (           hsla) { p->append(SkRasterPipelineOp::rgb_to_hsl); }
    if (           true) { p->append(SkRasterPipelineOp::matrix_4x5, fMatrix); }
    if (           hsla) { p->append(SkRasterPipelineOp::hsl_to_rgb); }
    if (           true) { p->append(SkRasterPipelineOp::clamp_01); }
    if (!willStayOpaque) { p->append(SkRasterPipelineOp::premul); }
    return true;
}


skvm::Color SkColorFilter_Matrix::onProgram(skvm::Builder* p, skvm::Color c,
                                            const SkColorInfo& /*dst*/,
                                            skvm::Uniforms* uniforms, SkArenaAlloc*) const {
    auto apply_matrix = [&](auto xyzw) {
        auto dot = [&](int j) {
            auto custom_mad = [&](float f, skvm::F32 m, skvm::F32 a) {
                // skvm::Builder won't fold f*0 == 0, but we shouldn't encounter NaN here.
                // While looking, also simplify f == ±1.  Anything else becomes a uniform.
                return f ==  0.0f ? a
                     : f == +1.0f ? a + m
                     : f == -1.0f ? a - m
                     : m * p->uniformF(uniforms->pushF(f)) + a;
            };

            // Similarly, let skvm::Builder fold away the additive bias when zero.
            const float b = fMatrix[4+j*5];
            skvm::F32 bias = b == 0.0f ? p->splat(0.0f)
                                       : p->uniformF(uniforms->pushF(b));

            auto [x,y,z,w] = xyzw;
            return custom_mad(fMatrix[0+j*5], x,
                   custom_mad(fMatrix[1+j*5], y,
                   custom_mad(fMatrix[2+j*5], z,
                   custom_mad(fMatrix[3+j*5], w, bias))));
        };
        return std::make_tuple(dot(0), dot(1), dot(2), dot(3));
    };

    c = unpremul(c);

    if (fDomain == Domain::kHSLA) {
        auto [h,s,l,a] = apply_matrix(p->to_hsla(c));
        c = p->to_rgba({h,s,l,a});
    } else {
        auto [r,g,b,a] = apply_matrix(c);
        c = {r,g,b,a};
    }

    return premul(clamp01(c));
}

#if SK_SUPPORT_GPU
#include "src/gpu/ganesh/effects/GrSkSLFP.h"

static std::unique_ptr<GrFragmentProcessor> rgb_to_hsl(std::unique_ptr<GrFragmentProcessor> child) {
    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForColorFilter,
        "half4 main(half4 color) {"
            "return $rgb_to_hsl(color.rgb, color.a);"
        "}"
    );
    SkASSERT(SkRuntimeEffectPriv::SupportsConstantOutputForConstantInput(effect));
    return GrSkSLFP::Make(effect, "RgbToHsl", std::move(child),
                          GrSkSLFP::OptFlags::kPreservesOpaqueInput);
}

static std::unique_ptr<GrFragmentProcessor> hsl_to_rgb(std::unique_ptr<GrFragmentProcessor> child) {
    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForColorFilter,
        "half4 main(half4 color) {"
            "return $hsl_to_rgb(color.rgb, color.a);"
        "}"
    );
    SkASSERT(SkRuntimeEffectPriv::SupportsConstantOutputForConstantInput(effect));
    return GrSkSLFP::Make(effect, "HslToRgb", std::move(child),
                          GrSkSLFP::OptFlags::kPreservesOpaqueInput);
}

GrFPResult SkColorFilter_Matrix::asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> fp,
                                                     GrRecordingContext*,
                                                     const GrColorInfo&,
                                                     const SkSurfaceProps&) const {
    switch (fDomain) {
        case Domain::kRGBA:
            fp = GrFragmentProcessor::ColorMatrix(std::move(fp), fMatrix,
                                                  /* unpremulInput = */  true,
                                                  /* clampRGBOutput = */ true,
                                                  /* premulOutput = */   true);
            break;

        case Domain::kHSLA:
            fp = rgb_to_hsl(std::move(fp));
            fp = GrFragmentProcessor::ColorMatrix(std::move(fp), fMatrix,
                                                  /* unpremulInput = */  false,
                                                  /* clampRGBOutput = */ false,
                                                  /* premulOutput = */   false);
            fp = hsl_to_rgb(std::move(fp));
            break;
    }

    return GrFPSuccess(std::move(fp));
}

#endif // SK_SUPPORT_GPU

#ifdef SK_GRAPHITE_ENABLED
void SkColorFilter_Matrix::addToKey(const skgpu::graphite::KeyContext& keyContext,
                                    skgpu::graphite::PaintParamsKeyBuilder* builder,
                                    skgpu::graphite::PipelineDataGatherer* gatherer) const {
    using namespace skgpu::graphite;

    MatrixColorFilterBlock::MatrixColorFilterData matrixCFData(fMatrix,
                                                               fDomain == Domain::kHSLA);

    MatrixColorFilterBlock::BeginBlock(keyContext, builder, gatherer, &matrixCFData);
    builder->endBlock();
}
#endif // SK_GRAPHITE_ENABLED

///////////////////////////////////////////////////////////////////////////////

static sk_sp<SkColorFilter> MakeMatrix(const float array[20],
                                       SkColorFilter_Matrix::Domain domain) {
    if (!sk_floats_are_finite(array, 20)) {
        return nullptr;
    }
    return sk_make_sp<SkColorFilter_Matrix>(array, domain);
}

sk_sp<SkColorFilter> SkColorFilters::Matrix(const float array[20]) {
    return MakeMatrix(array, SkColorFilter_Matrix::Domain::kRGBA);
}

sk_sp<SkColorFilter> SkColorFilters::Matrix(const SkColorMatrix& cm) {
    return MakeMatrix(cm.fMat.data(), SkColorFilter_Matrix::Domain::kRGBA);
}

sk_sp<SkColorFilter> SkColorFilters::HSLAMatrix(const float array[20]) {
    return MakeMatrix(array, SkColorFilter_Matrix::Domain::kHSLA);
}

sk_sp<SkColorFilter> SkColorFilters::HSLAMatrix(const SkColorMatrix& cm) {
    return MakeMatrix(cm.fMat.data(), SkColorFilter_Matrix::Domain::kHSLA);
}

void SkRegisterMatrixColorFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkColorFilter_Matrix);
}
