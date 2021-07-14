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
#include "src/gpu/effects/GrSkSLFP.h"

// Convert RGBA -> HSLA (including unpremul).
//
// Based on work by Sam Hocevar, Emil Persson, and Ian Taylor [1][2][3].  High-level ideas:
//
//   - minimize the number of branches by sorting and computing the hue phase in parallel (vec4s)
//
//   - trade the third sorting branch for a potentially faster std::min and leaving 2nd/3rd
//     channels unsorted (based on the observation that swapping both the channels and the bias sign
//     has no effect under abs)
//
//   - use epsilon offsets for denominators, to avoid explicit zero-checks
//
// An additional trick we employ is deferring premul->unpremul conversion until the very end: the
// alpha factor gets naturally simplified for H and S, and only L requires a dedicated unpremul
// division (so we trade three divs for one).
//
// [1] http://lolengine.net/blog/2013/01/13/fast-rgb-to-hsv
// [2] http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
// [3] http://www.chilliant.com/rgb2hsv.html
static std::unique_ptr<GrFragmentProcessor> rgb_to_hsl(std::unique_ptr<GrFragmentProcessor> child) {
    static auto effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForColorFilter, R"(
        half4 main(half4 c) {
            half4 p = (c.g < c.b) ? half4(c.bg, -1,  2/3.0)
                                  : half4(c.gb,  0, -1/3.0);
            half4 q = (c.r < p.x) ? half4(p.x, c.r, p.yw)
                                  : half4(c.r, p.x, p.yz);

            // q.x  -> max channel value
            // q.yz -> 2nd/3rd channel values (unsorted)
            // q.w  -> bias value dependent on max channel selection

            half eps = 0.0001;
            half pmV = q.x;
            half pmC = pmV - min(q.y, q.z);
            half pmL = pmV - pmC * 0.5;
            half   H = abs(q.w + (q.y - q.z) / (pmC * 6 + eps));
            half   S = pmC / (c.a + eps - abs(pmL * 2 - c.a));
            half   L = pmL / (c.a + eps);

            return half4(H, S, L, c.a);
        }
    )");
    SkASSERT(SkRuntimeEffectPriv::SupportsConstantOutputForConstantInput(effect));
    return GrSkSLFP::Make(
            effect, "RgbToHsl", std::move(child), GrSkSLFP::OptFlags::kPreservesOpaqueInput);
}

// Convert HSLA -> RGBA (including clamp and premul).
//
// Based on work by Sam Hocevar, Emil Persson, and Ian Taylor [1][2][3].
//
// [1] http://lolengine.net/blog/2013/01/13/fast-rgb-to-hsv
// [2] http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
// [3] http://www.chilliant.com/rgb2hsv.html
static std::unique_ptr<GrFragmentProcessor> hsl_to_rgb(std::unique_ptr<GrFragmentProcessor> child) {
    static auto effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForColorFilter, R"(
        half4 main(half4 color) {
            half3   hsl = color.rgb;

            half      C = (1 - abs(2 * hsl.z - 1)) * hsl.y;
            half3     p = hsl.xxx + half3(0, 2/3.0, 1/3.0);
            half3     q = saturate(abs(fract(p) * 6 - 3) - 1);
            half3   rgb = (q - 0.5) * C + hsl.z;

            color = saturate(half4(rgb, color.a));
            color.rgb *= color.a;
            return color;
        }
    )");
    SkASSERT(SkRuntimeEffectPriv::SupportsConstantOutputForConstantInput(effect));
    return GrSkSLFP::Make(
            effect, "HslToRgb", std::move(child), GrSkSLFP::OptFlags::kPreservesOpaqueInput);
}

GrFPResult SkColorFilter_Matrix::asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> fp,
                                                     GrRecordingContext*,
                                                     const GrColorInfo&) const {
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

#endif

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

void SkColorFilter_Matrix::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkColorFilter_Matrix);
}
