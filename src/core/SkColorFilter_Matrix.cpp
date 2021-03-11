/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/core/SkUnPreMultiply.h"
#include "include/effects/SkColorMatrix.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkColorData.h"
#include "include/private/SkNx.h"
#include "src/core/SkColorFilter_Matrix.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkVM.h"
#include "src/core/SkWriteBuffer.h"

static bool is_alpha_unchanged(const float matrix[20]) {
    const float* srcA = matrix + 15;

    return SkScalarNearlyZero (srcA[0])
        && SkScalarNearlyZero (srcA[1])
        && SkScalarNearlyZero (srcA[2])
        && SkScalarNearlyEqual(srcA[3], 1)
        && SkScalarNearlyZero (srcA[4]);
}

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

bool SkColorFilter_Matrix::onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const {
    const bool willStayOpaque = shaderIsOpaque && fAlphaIsUnchanged,
                         hsla = fDomain == Domain::kHSLA;

    SkRasterPipeline* p = rec.fPipeline;
    if (!shaderIsOpaque) { p->append(SkRasterPipeline::unpremul); }
    if (           hsla) { p->append(SkRasterPipeline::rgb_to_hsl); }
    if (           true) { p->append(SkRasterPipeline::matrix_4x5, fMatrix); }
    if (           hsla) { p->append(SkRasterPipeline::hsl_to_rgb); }
    if (           true) { p->append(SkRasterPipeline::clamp_0); }
    if (           true) { p->append(SkRasterPipeline::clamp_1); }
    if (!willStayOpaque) { p->append(SkRasterPipeline::premul); }
    return true;
}


skvm::Color SkColorFilter_Matrix::onProgram(skvm::Builder* p, skvm::Color c,
                                            SkColorSpace* /*dstCS*/,
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
#include "src/gpu/effects/generated/GrColorMatrixFragmentProcessor.h"
#include "src/gpu/effects/generated/GrHSLToRGBFilterEffect.h"
#include "src/gpu/effects/generated/GrRGBToHSLFilterEffect.h"
GrFPResult SkColorFilter_Matrix::asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> fp,
                                                     GrRecordingContext*,
                                                     const GrColorInfo&) const {
    switch (fDomain) {
        case Domain::kRGBA:
            fp = GrColorMatrixFragmentProcessor::Make(std::move(fp), fMatrix,
                                                      /* unpremulInput = */  true,
                                                      /* clampRGBOutput = */ true,
                                                      /* premulOutput = */   true);
            break;

        case Domain::kHSLA:
            fp = GrRGBToHSLFilterEffect::Make(std::move(fp));
            fp = GrColorMatrixFragmentProcessor::Make(std::move(fp), fMatrix,
                                                      /* unpremulInput = */  false,
                                                      /* clampRGBOutput = */ false,
                                                      /* premulOutput = */   false);
            fp = GrHSLToRGBFilterEffect::Make(std::move(fp));
            break;
    }

    return GrFPSuccess(std::move(fp));
}

#endif

///////////////////////////////////////////////////////////////////////////////

static sk_sp<SkColorFilter> MakeMatrix(const float array[20],
                                       SkColorFilter_Matrix::Domain domain) {
    if (!sk_floats_are_finite(array, 20)) {
        return nullptr;
    }
#if defined(SK_SUPPORT_LEGACY_RUNTIME_EFFECTS)
    return sk_make_sp<SkColorFilter_Matrix>(array, domain);
#else
    const bool alphaUnchanged = SkScalarNearlyEqual(array[15], 0)
                             && SkScalarNearlyEqual(array[16], 0)
                             && SkScalarNearlyEqual(array[17], 0)
                             && SkScalarNearlyEqual(array[18], 1)
                             && SkScalarNearlyEqual(array[19], 0);

    struct { SkM44 m; SkV4 b; } uniforms;
    SkString code {
        "uniform shader  input;"
        "uniform half4x4 m;"
        "uniform half4   b;"
    };
    if (domain == SkColorFilter_Matrix::Domain::kHSLA) {
        code += kRGB_to_HSL_sksl;
        code += kHSL_to_RGB_sksl;
    }

    code += "half4 main() {";
    if (true) {
        code += "half4 c = sample(input);";  // unpremul
    }
    if (alphaUnchanged) {
        code += "half a = c.a;";
    }
    if (domain == SkColorFilter_Matrix::Domain::kHSLA) {
        code += "c.rgb = rgb_to_hsl(c.rgb);";
    }
    if (true) {
        uniforms.m = SkM44{array[ 0], array[ 1], array[ 2], array[ 3],
                           array[ 5], array[ 6], array[ 7], array[ 8],
                           array[10], array[11], array[12], array[13],
                           array[15], array[16], array[17], array[18]};
        uniforms.b = SkV4{array[4], array[9], array[14], array[19]};
        code += "c = m*c + b;";
    }
    if (domain == SkColorFilter_Matrix::Domain::kHSLA) {
        code += "c.rgb = hsl_to_rgb(c.rgb);";
    }
    if (alphaUnchanged) {
        code += "return half4(saturate(c.rgb), a);";
    } else {
        code += "return saturate(c);";
    }
    code += "}";

    sk_sp<SkRuntimeEffect> effect = SkMakeCachedRuntimeEffect(std::move(code));
    SkASSERT(effect);

    sk_sp<SkColorFilter> input = nullptr;
    SkAlphaType       unpremul = kUnpremul_SkAlphaType;
    return SkColorFilters::WithWorkingFormat(
            effect->makeColorFilter(SkData::MakeWithCopy(&uniforms,sizeof(uniforms)), &input, 1),
            nullptr/*keep dst TF encoding*/,
            nullptr/*stay in dst gamut*/,
            &unpremul);

#endif
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

void SkColorFilter_Matrix::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkColorFilter_Matrix);

    // This subclass was removed 4/2019
    SkFlattenable::Register("SkColorMatrixFilterRowMajor255",
                            [](SkReadBuffer& buffer) -> sk_sp<SkFlattenable> {
        float matrix[20];
        if (buffer.readScalarArray(matrix, 20)) {
            matrix[ 4] *= (1.0f/255);
            matrix[ 9] *= (1.0f/255);
            matrix[14] *= (1.0f/255);
            matrix[19] *= (1.0f/255);
            return SkColorFilters::Matrix(matrix);
        }
        return nullptr;
    });
}
