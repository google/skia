/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SkHighContrastFilter.h"
#include "SkPM4f.h"
#include "SkArenaAlloc.h"
#include "SkRasterPipeline.h"
#include "SkReadBuffer.h"
#include "SkString.h"
#include "SkWriteBuffer.h"
#include "../jumper/SkJumper.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
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
    sk_sp<GrFragmentProcessor> asFragmentProcessor(GrContext*, SkColorSpace*) const override;
 #endif

    void onAppendStages(SkRasterPipeline* p,
                        SkColorSpace* dst,
                        SkArenaAlloc* scratch,
                        bool shaderIsOpaque) const override;

    SK_TO_STRING_OVERRIDE()

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkHighContrast_Filter)

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    SkHighContrastConfig fConfig;

    friend class SkHighContrastFilter;

    typedef SkColorFilter INHERITED;
};

void SkHighContrast_Filter::onAppendStages(SkRasterPipeline* p,
                                           SkColorSpace* dstCS,
                                           SkArenaAlloc* alloc,
                                           bool shaderIsOpaque) const {
    if (!shaderIsOpaque) {
        p->append(SkRasterPipeline::unpremul);
    }

    if (!dstCS) {
        // In legacy draws this effect approximately linearizes by squaring.
        // When non-legacy, we're already (better) linearized.
        auto square = alloc->make<SkJumper_ParametricTransferFunction>();
        square->G = 2.0f; square->A = 1.0f;
        square->B = square->C = square->D = square->E = square->F = 0;

        p->append(SkRasterPipeline::parametric_r, square);
        p->append(SkRasterPipeline::parametric_g, square);
        p->append(SkRasterPipeline::parametric_b, square);
    }

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

    if (!dstCS) {
        // See the previous if(!dstCS) { ... }
        auto sqrt = alloc->make<SkJumper_ParametricTransferFunction>();
        sqrt->G = 0.5f; sqrt->A = 1.0f;
        sqrt->B = sqrt->C = sqrt->D = sqrt->E = sqrt->F = 0;

        p->append(SkRasterPipeline::parametric_r, sqrt);
        p->append(SkRasterPipeline::parametric_g, sqrt);
        p->append(SkRasterPipeline::parametric_b, sqrt);
    }

    if (!shaderIsOpaque) {
        p->append(SkRasterPipeline::premul);
    }
}

void SkHighContrast_Filter::flatten(SkWriteBuffer& buffer) const {
    buffer.writeBool(fConfig.fGrayscale);
    buffer.writeInt(static_cast<int>(fConfig.fInvertStyle));
    buffer.writeScalar(fConfig.fContrast);
}

sk_sp<SkFlattenable> SkHighContrast_Filter::CreateProc(SkReadBuffer& buffer) {
    SkHighContrastConfig config;
    config.fGrayscale = buffer.readBool();
    config.fInvertStyle = static_cast<InvertStyle>(buffer.readInt());
    config.fContrast = buffer.readScalar();
    return SkHighContrastFilter::Make(config);
}

sk_sp<SkColorFilter> SkHighContrastFilter::Make(
    const SkHighContrastConfig& config) {
    if (!config.isValid())
        return nullptr;
    return sk_make_sp<SkHighContrast_Filter>(config);
}

#ifndef SK_IGNORE_TO_STRING
void SkHighContrast_Filter::toString(SkString* str) const {
    str->append("SkHighContrastColorFilter ");
}
#endif

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkHighContrastFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkHighContrast_Filter)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END

#if SK_SUPPORT_GPU
class HighContrastFilterEffect : public GrFragmentProcessor {
public:
    static sk_sp<GrFragmentProcessor> Make(const SkHighContrastConfig& config) {
        return sk_sp<GrFragmentProcessor>(new HighContrastFilterEffect(config));
    }

    const char* name() const override { return "HighContrastFilter"; }

    const SkHighContrastConfig& config() const { return fConfig; }

private:
    HighContrastFilterEffect(const SkHighContrastConfig& config)
        : INHERITED(kNone_OptimizationFlags)
        , fConfig(config) {
        this->initClassID<HighContrastFilterEffect>();
    }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    virtual void onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                       GrProcessorKeyBuilder* b) const override;

    bool onIsEqual(const GrFragmentProcessor& other) const override {
        const HighContrastFilterEffect& that = other.cast<HighContrastFilterEffect>();
        return fConfig.fGrayscale == that.fConfig.fGrayscale &&
            fConfig.fInvertStyle == that.fConfig.fInvertStyle &&
            fConfig.fContrast == that.fConfig.fContrast;
    }

    SkHighContrastConfig fConfig;

    typedef GrFragmentProcessor INHERITED;
};

class GLHighContrastFilterEffect : public GrGLSLFragmentProcessor {
public:
    static void GenKey(const GrProcessor&, const GrShaderCaps&, GrProcessorKeyBuilder*);

    GLHighContrastFilterEffect(const SkHighContrastConfig& config);

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) override;
    void emitCode(EmitArgs& args) override;

private:
    UniformHandle fContrastUni;
    SkHighContrastConfig fConfig;

    typedef GrGLSLFragmentProcessor INHERITED;
};

GrGLSLFragmentProcessor* HighContrastFilterEffect::onCreateGLSLInstance() const {
    return new GLHighContrastFilterEffect(fConfig);
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

GLHighContrastFilterEffect::GLHighContrastFilterEffect(const SkHighContrastConfig& config)
    : INHERITED()
    , fConfig(config) {
}

void GLHighContrastFilterEffect::GenKey(
    const GrProcessor& proc, const GrShaderCaps&, GrProcessorKeyBuilder* b) {
  const HighContrastFilterEffect& hcfe = proc.cast<HighContrastFilterEffect>();
  b->add32(static_cast<uint32_t>(hcfe.config().fGrayscale));
  b->add32(static_cast<uint32_t>(hcfe.config().fInvertStyle));
}

void GLHighContrastFilterEffect::emitCode(EmitArgs& args) {
    const char* contrast;
    fContrastUni = args.fUniformHandler->addUniform(kFragment_GrShaderFlag,
                                                    kFloat_GrSLType, kDefault_GrSLPrecision,
                                                    "contrast", &contrast);

    if (nullptr == args.fInputColor) {
        args.fInputColor = "vec4(1)";
    }

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

    fragBuilder->codeAppendf("vec4 color = %s;", args.fInputColor);

    // Unpremultiply. The max() is to guard against 0 / 0.
    fragBuilder->codeAppendf("float nonZeroAlpha = max(color.a, 0.00001);");
    fragBuilder->codeAppendf("color = vec4(color.rgb / nonZeroAlpha, nonZeroAlpha);");

    // Grayscale.
    if (fConfig.fGrayscale) {
        fragBuilder->codeAppendf("float luma = dot(color, vec4(%f, %f, %f, 0));",
                                 SK_LUM_COEFF_R, SK_LUM_COEFF_G, SK_LUM_COEFF_B);
        fragBuilder->codeAppendf("color = vec4(luma, luma, luma, 0);");
    }

    if (fConfig.fInvertStyle == InvertStyle::kInvertBrightness) {
        fragBuilder->codeAppendf("color = vec4(1, 1, 1, 1) - color;");
    }

    if (fConfig.fInvertStyle == InvertStyle::kInvertLightness) {
        // Convert from RGB to HSL.
        fragBuilder->codeAppendf("float fmax = max(color.r, max(color.g, color.b));");
        fragBuilder->codeAppendf("float fmin = min(color.r, min(color.g, color.b));");
        fragBuilder->codeAppendf("float l = (fmax + fmin) / 2;");

        fragBuilder->codeAppendf("float h;");
        fragBuilder->codeAppendf("float s;");

        fragBuilder->codeAppendf("if (fmax == fmin) {");
        fragBuilder->codeAppendf("  h = 0;");
        fragBuilder->codeAppendf("  s = 0;");
        fragBuilder->codeAppendf("} else {");
        fragBuilder->codeAppendf("  float d = fmax - fmin;");
        fragBuilder->codeAppendf("  s = l > 0.5 ?");
        fragBuilder->codeAppendf("      d / (2 - fmax - fmin) :");
        fragBuilder->codeAppendf("      d / (fmax + fmin);");
        fragBuilder->codeAppendf("  if (fmax == color.r) {");
        fragBuilder->codeAppendf("    h = (color.g - color.b) / d + ");
        fragBuilder->codeAppendf("        (color.g < color.b ? 6 : 0);");
        fragBuilder->codeAppendf("  } else if (fmax == color.g) {");
        fragBuilder->codeAppendf("    h = (color.b - color.r) / d + 2;");
        fragBuilder->codeAppendf("  } else {");
        fragBuilder->codeAppendf("    h = (color.r - color.g) / d + 4;");
        fragBuilder->codeAppendf("  }");
        fragBuilder->codeAppendf("}");
        fragBuilder->codeAppendf("h /= 6;");
        fragBuilder->codeAppendf("l = 1.0 - l;");
        // Convert back from HSL to RGB.
        SkString hue2rgbFuncName;
        static const GrShaderVar gHue2rgbArgs[] = {
            GrShaderVar("p", kFloat_GrSLType),
            GrShaderVar("q", kFloat_GrSLType),
            GrShaderVar("t", kFloat_GrSLType),
        };
        fragBuilder->emitFunction(kFloat_GrSLType,
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
        fragBuilder->codeAppendf("  color = vec4(l, l, l, 0);");
        fragBuilder->codeAppendf("} else {");
        fragBuilder->codeAppendf("  float q = l < 0.5 ? l * (1 + s) : l + s - l * s;");
        fragBuilder->codeAppendf("  float p = 2 * l - q;");
        fragBuilder->codeAppendf("  color.r = %s(p, q, h + 1/3.);", hue2rgbFuncName.c_str());
        fragBuilder->codeAppendf("  color.g = %s(p, q, h);", hue2rgbFuncName.c_str());
        fragBuilder->codeAppendf("  color.b = %s(p, q, h - 1/3.);", hue2rgbFuncName.c_str());
        fragBuilder->codeAppendf("}");
    }

    // Contrast.
    fragBuilder->codeAppendf("if (%s != 0) {", contrast);
    fragBuilder->codeAppendf("  float m = (1 + %s) / (1 - %s);", contrast, contrast);
    fragBuilder->codeAppendf("  float off = (-0.5 * m + 0.5);");
    fragBuilder->codeAppendf("  color = m * color + off;");
    fragBuilder->codeAppendf("}");

    // Clamp.
    fragBuilder->codeAppendf("color = clamp(color, 0, 1);");

    // Restore the original alpha and premultiply.
    fragBuilder->codeAppendf("color.a = %s.a;", args.fInputColor);
    fragBuilder->codeAppendf("color.rgb *= color.a;");

    // Copy to the output color.
    fragBuilder->codeAppendf("%s = color;", args.fOutputColor);
}

sk_sp<GrFragmentProcessor> SkHighContrast_Filter::asFragmentProcessor(GrContext*, SkColorSpace*) const {
    return HighContrastFilterEffect::Make(fConfig);
}
#endif
