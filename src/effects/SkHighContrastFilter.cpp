/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/core/SkString.h"
#include "include/effects/SkHighContrastFilter.h"
#include "include/private/SkColorData.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "include/gpu/GrContext.h"
#include "src/gpu/GrColorSpaceInfo.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#endif

using InvertStyle = SkHighContrastConfig::InvertStyle;

class SkHighContrast_Filter : public SkColorFilter {
public:
    SkHighContrast_Filter(const SkHighContrastConfig& config) {
        fConfig = config;
        // Clamp contrast to just inside -1 to 1 to avoid division by zero.
        fConfig.fContrast = SkScalarPin(fConfig.fContrast,
                                        -1.0f + FLT_EPSILON,
                                        1.0f - FLT_EPSILON);
    }

    ~SkHighContrast_Filter() override {}

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(
            GrRecordingContext*, const GrColorSpaceInfo&) const override;
 #endif

    bool onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const override;

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    SK_FLATTENABLE_HOOKS(SkHighContrast_Filter)

    SkHighContrastConfig fConfig;

    friend class SkHighContrastFilter;

    typedef SkColorFilter INHERITED;
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
        rec.fDstCS->transferFn(&tf->g);
    } else {
        // Historically we approximate untagged destinations as gamma 2.
        // TODO: sRGB?
        *tf = {2,1, 0,0,0,0,0};
    }
    p->append(SkRasterPipeline::parametric, tf);

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
        rec.fDstCS->invTransferFn(&invTF->g);
    } else {
        // See above... historically untagged == gamma 2 in this filter.
        *invTF ={0.5f,1, 0,0,0,0,0};
    }
    p->append(SkRasterPipeline::parametric, invTF);

    if (!shaderIsOpaque) {
        p->append(SkRasterPipeline::premul);
    }
    return true;
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

sk_sp<SkColorFilter> SkHighContrastFilter::Make(
    const SkHighContrastConfig& config) {
    if (!config.isValid()) {
        return nullptr;
    }
    return sk_make_sp<SkHighContrast_Filter>(config);
}

void SkHighContrastFilter::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkHighContrast_Filter);
}

#if SK_SUPPORT_GPU
class HighContrastFilterEffect : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(const SkHighContrastConfig& config,
                                                     bool linearize) {
        return std::unique_ptr<GrFragmentProcessor>(new HighContrastFilterEffect(config,
                                                                                 linearize));
    }

    const char* name() const override { return "HighContrastFilter"; }

    const SkHighContrastConfig& config() const { return fConfig; }
    bool linearize() const { return fLinearize; }

    std::unique_ptr<GrFragmentProcessor> clone() const override {
        return Make(fConfig, fLinearize);
    }

private:
    HighContrastFilterEffect(const SkHighContrastConfig& config, bool linearize)
        : INHERITED(kHighContrastFilterEffect_ClassID, kNone_OptimizationFlags)
        , fConfig(config)
        , fLinearize(linearize) {}

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    virtual void onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                       GrProcessorKeyBuilder* b) const override;

    bool onIsEqual(const GrFragmentProcessor& other) const override {
        const HighContrastFilterEffect& that = other.cast<HighContrastFilterEffect>();
        return fConfig.fGrayscale == that.fConfig.fGrayscale &&
            fConfig.fInvertStyle == that.fConfig.fInvertStyle &&
            fConfig.fContrast == that.fConfig.fContrast &&
            fLinearize == that.fLinearize;
    }

    SkHighContrastConfig fConfig;
    bool fLinearize;

    typedef GrFragmentProcessor INHERITED;
};

class GLHighContrastFilterEffect : public GrGLSLFragmentProcessor {
public:
    static void GenKey(const GrProcessor&, const GrShaderCaps&, GrProcessorKeyBuilder*);

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) override;
    void emitCode(EmitArgs& args) override;

private:
    UniformHandle fContrastUni;

    typedef GrGLSLFragmentProcessor INHERITED;
};

GrGLSLFragmentProcessor* HighContrastFilterEffect::onCreateGLSLInstance() const {
    return new GLHighContrastFilterEffect();
}

void HighContrastFilterEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                                     GrProcessorKeyBuilder* b) const {
    GLHighContrastFilterEffect::GenKey(*this, caps, b);
}

void GLHighContrastFilterEffect::onSetData(const GrGLSLProgramDataManager& pdm,
                                           const GrFragmentProcessor& proc) {
    const HighContrastFilterEffect& hcfe = proc.cast<HighContrastFilterEffect>();
    pdm.set1f(fContrastUni, hcfe.config().fContrast);
}

void GLHighContrastFilterEffect::GenKey(
    const GrProcessor& proc, const GrShaderCaps&, GrProcessorKeyBuilder* b) {
  const HighContrastFilterEffect& hcfe = proc.cast<HighContrastFilterEffect>();
  b->add32(static_cast<uint32_t>(hcfe.config().fGrayscale));
  b->add32(static_cast<uint32_t>(hcfe.config().fInvertStyle));
  b->add32(hcfe.linearize() ? 1 : 0);
}

void GLHighContrastFilterEffect::emitCode(EmitArgs& args) {
    const HighContrastFilterEffect& hcfe = args.fFp.cast<HighContrastFilterEffect>();
    const SkHighContrastConfig& config = hcfe.config();

    const char* contrast;
    fContrastUni = args.fUniformHandler->addUniform(kFragment_GrShaderFlag, kHalf_GrSLType,
                                                    "contrast", &contrast);

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

    fragBuilder->codeAppendf("half4 color = unpremul(%s);", args.fInputColor);

    if (hcfe.linearize()) {
        fragBuilder->codeAppend("color.rgb = color.rgb * color.rgb;");
    }

    // Grayscale.
    if (config.fGrayscale) {
        fragBuilder->codeAppendf("half luma = dot(color, half4(%f, %f, %f, 0));",
                                 SK_LUM_COEFF_R, SK_LUM_COEFF_G, SK_LUM_COEFF_B);
        fragBuilder->codeAppendf("color = half4(luma, luma, luma, 0);");
    }

    if (config.fInvertStyle == InvertStyle::kInvertBrightness) {
        fragBuilder->codeAppendf("color = half4(1, 1, 1, 1) - color;");
    }

    if (config.fInvertStyle == InvertStyle::kInvertLightness) {
        // Convert from RGB to HSL.
        fragBuilder->codeAppendf("half fmax = max(color.r, max(color.g, color.b));");
        fragBuilder->codeAppendf("half fmin = min(color.r, min(color.g, color.b));");
        fragBuilder->codeAppendf("half l = (fmax + fmin) / 2;");

        fragBuilder->codeAppendf("half h;");
        fragBuilder->codeAppendf("half s;");

        fragBuilder->codeAppendf("if (fmax == fmin) {");
        fragBuilder->codeAppendf("  h = 0;");
        fragBuilder->codeAppendf("  s = 0;");
        fragBuilder->codeAppendf("} else {");
        fragBuilder->codeAppendf("  half d = fmax - fmin;");
        fragBuilder->codeAppendf("  s = l > 0.5 ?");
        fragBuilder->codeAppendf("      d / (2 - fmax - fmin) :");
        fragBuilder->codeAppendf("      d / (fmax + fmin);");
        // We'd like to just write "if (color.r == fmax) { ... }". On many GPUs, running the
        // angle_d3d9_es2 config, that failed. It seems that max(x, y) is not necessarily equal
        // to either x or y. Tried several ways to fix it, but this was the only reasonable fix.
        fragBuilder->codeAppendf("  if (color.r >= color.g && color.r >= color.b) {");
        fragBuilder->codeAppendf("    h = (color.g - color.b) / d + ");
        fragBuilder->codeAppendf("        (color.g < color.b ? 6 : 0);");
        fragBuilder->codeAppendf("  } else if (color.g >= color.b) {");
        fragBuilder->codeAppendf("    h = (color.b - color.r) / d + 2;");
        fragBuilder->codeAppendf("  } else {");
        fragBuilder->codeAppendf("    h = (color.r - color.g) / d + 4;");
        fragBuilder->codeAppendf("  }");
        fragBuilder->codeAppendf("}");
        fragBuilder->codeAppendf("h /= 6;");
        fragBuilder->codeAppendf("l = 1.0 - l;");
        // Convert back from HSL to RGB.
        SkString hue2rgbFuncName;
        const GrShaderVar gHue2rgbArgs[] = {
            GrShaderVar("p", kHalf_GrSLType),
            GrShaderVar("q", kHalf_GrSLType),
            GrShaderVar("t", kHalf_GrSLType),
        };
        fragBuilder->emitFunction(kHalf_GrSLType,
                                  "hue2rgb",
                                  SK_ARRAY_COUNT(gHue2rgbArgs),
                                  gHue2rgbArgs,
                                  "if (t < 0)"
                                  "  t += 1;"
                                  "if (t > 1)"
                                  "  t -= 1;"
                                  "if (t < 1/6.)"
                                  "  return p + (q - p) * 6 * t;"
                                  "if (t < 1/2.)"
                                  "  return q;"
                                  "if (t < 2/3.)"
                                  "  return p + (q - p) * (2/3. - t) * 6;"
                                  "return p;",
                                  &hue2rgbFuncName);
        fragBuilder->codeAppendf("if (s == 0) {");
        fragBuilder->codeAppendf("  color = half4(l, l, l, 0);");
        fragBuilder->codeAppendf("} else {");
        fragBuilder->codeAppendf("  half q = l < 0.5 ? l * (1 + s) : l + s - l * s;");
        fragBuilder->codeAppendf("  half p = 2 * l - q;");
        fragBuilder->codeAppendf("  color.r = %s(p, q, h + 1/3.);", hue2rgbFuncName.c_str());
        fragBuilder->codeAppendf("  color.g = %s(p, q, h);", hue2rgbFuncName.c_str());
        fragBuilder->codeAppendf("  color.b = %s(p, q, h - 1/3.);", hue2rgbFuncName.c_str());
        fragBuilder->codeAppendf("}");
    }

    // Contrast.
    fragBuilder->codeAppendf("if (%s != 0) {", contrast);
    fragBuilder->codeAppendf("  half m = (1 + %s) / (1 - %s);", contrast, contrast);
    fragBuilder->codeAppendf("  half off = (-0.5 * m + 0.5);");
    fragBuilder->codeAppendf("  color = m * color + off;");
    fragBuilder->codeAppendf("}");

    // Clamp.
    fragBuilder->codeAppendf("color = saturate(color);");

    if (hcfe.linearize()) {
        fragBuilder->codeAppend("color.rgb = sqrt(color.rgb);");
    }

    // Restore the original alpha and premultiply.
    fragBuilder->codeAppendf("color.a = %s.a;", args.fInputColor);
    fragBuilder->codeAppendf("color.rgb *= color.a;");

    // Copy to the output color.
    fragBuilder->codeAppendf("%s = color;", args.fOutputColor);
}

std::unique_ptr<GrFragmentProcessor> SkHighContrast_Filter::asFragmentProcessor(
        GrRecordingContext*, const GrColorSpaceInfo& csi) const {
    bool linearize = !csi.isLinearlyBlended();
    return HighContrastFilterEffect::Make(fConfig, linearize);
}
#endif
