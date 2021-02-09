/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/core/SkString.h"
#include "include/effects/SkHighContrastFilter.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkColorData.h"
#include "include/private/SkTPin.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkColorFilterBase.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkVM.h"
#include "src/core/SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "src/gpu/GrColorInfo.h"
#include "src/gpu/effects/generated/GrHighContrastFilterEffect.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#endif

using InvertStyle = SkHighContrastConfig::InvertStyle;

class SkHighContrast_Filter : public SkColorFilterBase {
public:
    SkHighContrast_Filter(const SkHighContrastConfig& config) {
        fConfig = config;
        // Clamp contrast to just inside -1 to 1 to avoid division by zero.
        fConfig.fContrast = SkTPin(fConfig.fContrast,
                                   -1.0f + FLT_EPSILON,
                                   1.0f - FLT_EPSILON);
    }

    ~SkHighContrast_Filter() override {}

#if SK_SUPPORT_GPU
    GrFPResult asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
                                   GrRecordingContext*, const GrColorInfo&) const override;
#endif

    bool onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const override;
    skvm::Color onProgram(skvm::Builder*, skvm::Color, SkColorSpace*, skvm::Uniforms*,
                          SkArenaAlloc*) const override;

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    SK_FLATTENABLE_HOOKS(SkHighContrast_Filter)

    SkHighContrastConfig fConfig;

    friend class SkHighContrastFilter;

    using INHERITED = SkColorFilter;
};

bool SkHighContrast_Filter::onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const {
    SkRasterPipeline* p = rec.fPipeline;
    SkArenaAlloc* alloc = rec.fAlloc;

    if (!shaderIsOpaque) {
        p->append(SkRasterPipeline::unpremul);
    }

    // Linearize before applying high-contrast filter.
    auto tf = alloc->make<skcms_TransferFunction>();
    if (rec.fDstCS) {
        rec.fDstCS->transferFn(tf);
    } else {
        // Historically we approximate untagged destinations as gamma 2.
        // TODO: sRGB?
        *tf = {2,1, 0,0,0,0,0};
    }
    p->append_transfer_function(*tf);

    if (fConfig.fGrayscale) {
        float r = SK_LUM_COEFF_R;
        float g = SK_LUM_COEFF_G;
        float b = SK_LUM_COEFF_B;
        float* matrix = alloc->makeArray<float>(12);
        matrix[0] = matrix[1] = matrix[2] = r;
        matrix[3] = matrix[4] = matrix[5] = g;
        matrix[6] = matrix[7] = matrix[8] = b;
        p->append(SkRasterPipeline::matrix_3x4, matrix);
    }

    if (fConfig.fInvertStyle == InvertStyle::kInvertBrightness) {
        float* matrix = alloc->makeArray<float>(12);
        matrix[0] = matrix[4] = matrix[8] = -1;
        matrix[9] = matrix[10] = matrix[11] = 1;
        p->append(SkRasterPipeline::matrix_3x4, matrix);
    } else if (fConfig.fInvertStyle == InvertStyle::kInvertLightness) {
        p->append(SkRasterPipeline::rgb_to_hsl);
        float* matrix = alloc->makeArray<float>(12);
        matrix[0] = matrix[4] = matrix[11] = 1;
        matrix[8] = -1;
        p->append(SkRasterPipeline::matrix_3x4, matrix);
        p->append(SkRasterPipeline::hsl_to_rgb);
    }

    if (fConfig.fContrast != 0.0) {
        float* matrix = alloc->makeArray<float>(12);
        float c = fConfig.fContrast;
        float m = (1 + c) / (1 - c);
        float b = (-0.5f * m + 0.5f);
        matrix[0] = matrix[4] = matrix[8] = m;
        matrix[9] = matrix[10] = matrix[11] = b;
        p->append(SkRasterPipeline::matrix_3x4, matrix);
    }

    p->append(SkRasterPipeline::clamp_0);
    p->append(SkRasterPipeline::clamp_1);

    // Re-encode back from linear.
    auto invTF = alloc->make<skcms_TransferFunction>();
    if (rec.fDstCS) {
        rec.fDstCS->invTransferFn(invTF);
    } else {
        // See above... historically untagged == gamma 2 in this filter.
        *invTF ={0.5f,1, 0,0,0,0,0};
    }
    p->append_transfer_function(*invTF);

    if (!shaderIsOpaque) {
        p->append(SkRasterPipeline::premul);
    }
    return true;
}

skvm::Color SkHighContrast_Filter::onProgram(skvm::Builder* p, skvm::Color c, SkColorSpace* dstCS,
                                             skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const {
    c = p->unpremul(c);

    // Linearize before applying high-contrast filter.
    skcms_TransferFunction tf;
    if (dstCS) {
        dstCS->transferFn(&tf);
    } else {
        //sk_srgb_singleton()->transferFn(&tf);
        tf = {2,1, 0,0,0,0,0};
    }
    c = sk_program_transfer_fn(p, uniforms, tf, c);

    if (fConfig.fGrayscale) {
        skvm::F32 gray = c.r * SK_LUM_COEFF_R
                       + c.g * SK_LUM_COEFF_G
                       + c.b * SK_LUM_COEFF_B;
        c = {gray, gray, gray, c.a};
    }

    if (fConfig.fInvertStyle == InvertStyle::kInvertBrightness) {
        c = {1-c.r, 1-c.g, 1-c.b, c.a};
    } else if (fConfig.fInvertStyle == InvertStyle::kInvertLightness) {
        auto [h, s, l, a] = p->to_hsla(c);
        c = p->to_rgba({h, s, 1-l, a});
    }

    if (fConfig.fContrast != 0.0) {
        const float m = (1 + fConfig.fContrast) / (1 - fConfig.fContrast);
        const float b = (-0.5f * m + 0.5f);
        skvm::F32   M = p->uniformF(uniforms->pushF(m));
        skvm::F32   B = p->uniformF(uniforms->pushF(b));
        c.r = c.r * M + B;
        c.g = c.g * M + B;
        c.b = c.b * M + B;
    }

    c.r = clamp01(c.r);
    c.g = clamp01(c.g);
    c.b = clamp01(c.b);

    // Re-encode back from linear.
    if (dstCS) {
        dstCS->invTransferFn(&tf);
    } else {
        //sk_srgb_singleton()->invTransferFn(&tf);
        tf = {0.5f,1, 0,0,0,0,0};
    }
    c = sk_program_transfer_fn(p, uniforms, tf, c);

    return p->premul(c);
}

void SkHighContrast_Filter::flatten(SkWriteBuffer& buffer) const {
    buffer.writeBool(fConfig.fGrayscale);
    buffer.writeInt(static_cast<int>(fConfig.fInvertStyle));
    buffer.writeScalar(fConfig.fContrast);
}

sk_sp<SkFlattenable> SkHighContrast_Filter::CreateProc(SkReadBuffer& buffer) {
    SkHighContrastConfig config;
    config.fGrayscale = buffer.readBool();
    config.fInvertStyle = buffer.read32LE(InvertStyle::kLast);
    config.fContrast = buffer.readScalar();

    return SkHighContrastFilter::Make(config);
}

sk_sp<SkColorFilter> SkHighContrastFilter::Make(const SkHighContrastConfig& config) {
    if (!config.isValid()) {
        return nullptr;
    }
#if defined(SK_SUPPORT_LEGACY_RUNTIME_EFFECTS)
    return sk_make_sp<SkHighContrast_Filter>(config);
#else
    struct { float M; } uniforms;
    SkString code{
        "uniform shader input;"
        "uniform half M;"

        //This is straight out of GrHSLToRGBFilterEffect.fp.
        "half3 hsl_to_rgb(half3 hsl) {"
            "half  C = (1 - abs(2 * hsl.z - 1)) * hsl.y;"
            "half3 p = hsl.xxx + half3(0, 2/3.0, 1/3.0);"
            "half3 q = saturate(abs(fract(p) * 6 - 3) - 1);"
            "return (q - 0.5) * C + hsl.z;"
        "}"

        // This is mostly from skvm's rgb->hsl code, with some GPU-related finesse pulled from
        // GrHighContrastFilterEffect.fp, see next comment.
        "half3 rgb_to_hsl(half3 c) {"
            "half mx = max(max(c.r,c.g),c.b),"
            "     mn = min(min(c.r,c.g),c.b),"
            "      d = mx-mn,                "
            "   invd = 1.0 / d,              "
            " g_lt_b = c.g < c.b ? 6.0 : 0.0;"

            // We'd prefer to write these tests like `mx == c.r`, but on some GPUs max(x,y) is
            // not always equal to either x or y.  So we use long form, c.r >= c.g && c.r >= c.b.
            "half h = (1/6.0) * (mx == mn                 ? 0.0 :"
            "     /*mx==c.r*/    c.r >= c.g && c.r >= c.b ? invd * (c.g - c.b) + g_lt_b :"
            "     /*mx==c.g*/    c.g >= c.b               ? invd * (c.b - c.r) + 2.0  "
            "     /*mx==c.b*/                             : invd * (c.r - c.g) + 4.0);"

            "half sum = mx+mn,"
            "       l = sum * 0.5,"
            "       s = mx == mn ? 0.0"
            "                    : d / (l > 0.5 ? 2.0 - sum : sum);"
            "return half3(h,s,l);"
        "}"
    };

    code += "half4 main() {";
    if (true) {
        code += "half4 c = sample(input);"; // c is linear unpremul RGBA in the dst gamut.
    }
    if (config.fGrayscale) {
        code += "c.rgb = dot(half3(0.2126, 0.7152, 0.0722), c.rgb).rrr;";
    }
    if (config.fInvertStyle == InvertStyle::kInvertBrightness) {
        code += "c.rgb = 1 - c.rgb;";
    }
    if (config.fInvertStyle == InvertStyle::kInvertLightness) {
        code += "c.rgb = rgb_to_hsl(c.rgb);";
        code += "c.b = 1 - c.b;";
        code += "c.rgb = hsl_to_rgb(c.rgb);";
    }
    if (float c = config.fContrast) {
        uniforms.M = (1+c)/(1-c);
        code += "c.rgb = (c.rgb - 0.5)*M + 0.5;";
    }
    if (true) {
        code += "return saturate(c);";
    }
    code += "}";

    auto [effect, err] = SkRuntimeEffect::Make(code);
    if (!err.isEmpty()) {
        SkDebugf("%s\n%s\n", code.c_str(), err.c_str());
    }
    SkASSERT(effect && err.isEmpty());

    sk_sp<SkColorFilter>    input = nullptr;
    skcms_TransferFunction linear = SkNamedTransferFn::kLinear;
    SkAlphaType          unpremul = kUnpremul_SkAlphaType;
    return SkColorFilters::WithWorkingFormat(
            effect->makeColorFilter(SkData::MakeWithCopy(&uniforms,sizeof(uniforms)), &input, 1),
            &linear, nullptr/*use dst gamut*/, &unpremul);
#endif
}

void SkHighContrastFilter::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkHighContrast_Filter);
}

#if SK_SUPPORT_GPU
GrFPResult SkHighContrast_Filter::asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
                                                      GrRecordingContext*,
                                                      const GrColorInfo& csi) const {
    bool linearize = !csi.isLinearlyBlended();
    return GrFPSuccess(GrHighContrastFilterEffect::Make(std::move(inputFP), fConfig, linearize));
}
#endif
