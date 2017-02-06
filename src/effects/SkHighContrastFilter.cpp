/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SkHighContrastFilter.h"

#include "SkRasterPipeline.h"
#include "SkReadBuffer.h"
#include "SkString.h"
#include "SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrInvariantOutput.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#endif

namespace {

SkScalar Hue2RGB(SkScalar p, SkScalar q, SkScalar t) {
    if (t < 0) {
        t += 1;
    } else if (t > 1) {
        t -= 1;
    }

    if (t < 1/6.f) {
        return p + (q - p) * 6 * t;
    }

    if (t < 1/2.f) {
        return q;
    }

    if (t < 2/3.f) {
        return p + (q - p) * (2/3.f - t) * 6;
    }

    return p;
}

unsigned char SkScalarToCharClamp(SkScalar f) {
    if (f <= 0) {
        return 0;
    } else if (f >= 1) {
        return 255;
    }
    return static_cast<unsigned char>(255 * f);
}

SkScalar IncreaseContrast(SkScalar f, SkScalar contrast) {
    SkScalar m = (1 + contrast) / (1 - contrast);
    SkScalar b = (-0.5f * m + 0.5f);
    return m * f + b;
}

}  // namespace

void SkHighContrastFilter::Apply(const SkHighContrastConfig& config,
                                 unsigned char* r,
                                 unsigned char* g,
                                 unsigned char* b) {
    // Convert to floats.
    SkScalar rf = *r / 255.f;
    SkScalar gf = *g / 255.f;
    SkScalar bf = *b / 255.f;

    // Apply a gamma of 2.0 so that the rest of the calculations
    // happen roughly in linear space.
    rf *= rf;
    gf *= gf;
    bf *= bf;

    // Convert to grayscale using luminance coefficients.
    if (config.fGrayscale) {
        SkScalar lum =
            rf * SK_LUM_COEFF_R + gf * SK_LUM_COEFF_G + bf * SK_LUM_COEFF_B;
        rf = lum;
        gf = lum;
        bf = lum;
    }

    // Now invert.
    if (config.fInvertStyle == HighContrastInvertStyle::kInvertBrightness) {
        rf = 1 - rf;
        gf = 1 - gf;
        bf = 1 - bf;
    } else if (config.fInvertStyle ==
               HighContrastInvertStyle::kInvertLightness) {
        // Convert to HSL
        SkScalar max = SkTMax(SkTMax(rf, gf), bf);
        SkScalar min = SkTMin(SkTMin(rf, gf), bf);
        SkScalar l = (max + min) / 2;
        SkScalar h, s;

        if (max == min) {
            h = 0;
            s = 0;
        } else {
            SkScalar d = max - min;
            s = l > 0.5f ? d / (2 - max - min) : d / (max + min);
            if (max == rf) {
                h = (gf - bf) / d + (gf < bf ? 6 : 0);
            } else if (max == gf) {
                h = (bf - rf) / d + 2;
            } else {
                h = (rf - gf) / d + 4;
            }
            h /= 6;
        }

        // Invert lightness.
        l = 1 - l;

        // Now convert back to RGB.
        if (s == 0) {
            // Grayscale
            rf = l;
            gf = l;
            bf = l;
        } else {
            SkScalar q = l < 0.5f ? l * (1 + s) : l + s - l * s;
            SkScalar p = 2 * l - q;
            rf = Hue2RGB(p, q, h + 1/3.f);
            gf = Hue2RGB(p, q, h);
            bf = Hue2RGB(p, q, h - 1/3.f);
        }
    }

    // Increase contrast.
    if (config.fContrast != 0.0f) {
        rf = IncreaseContrast(rf, config.fContrast);
        gf = IncreaseContrast(gf, config.fContrast);
        bf = IncreaseContrast(bf, config.fContrast);
    }

    // Convert back from linear to a color space with a gamma of ~2.0.
    rf = SkScalarSqrt(rf);
    gf = SkScalarSqrt(gf);
    bf = SkScalarSqrt(bf);

    *r = SkScalarToCharClamp(rf);
    *g = SkScalarToCharClamp(gf);
    *b = SkScalarToCharClamp(bf);
}

SkColor SkHighContrastFilter::Apply(const SkHighContrastConfig& config,
                                    SkColor srcColor) {
    unsigned char a = SkColorGetA(srcColor);
    unsigned char r = SkColorGetR(srcColor);
    unsigned char g = SkColorGetG(srcColor);
    unsigned char b = SkColorGetB(srcColor);
    SkHighContrastFilter::Apply(config, &r, &g, &b);
    return SkColorSetARGB(a, r, g, b);
}

SkPaint SkHighContrastFilter::Apply(const SkHighContrastConfig& config,
                                    const SkPaint& src) {
  if (src.getShader()) {
    fprintf(stderr, "Paint has shader");
  }

  SkPaint dst = src;
  dst.setColor(SkHighContrastFilter::Apply(config, dst.getColor()));
  return dst;
}

class SkHighContrast_Filter : public SkColorFilter {
public:
    SkHighContrast_Filter(const SkHighContrastConfig& config) {
        fConfig = config;
    }

    virtual ~SkHighContrast_Filter() { }

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(GrContext*, SkColorSpace*) const override;
 #endif

    void filterSpan(const SkPMColor src[], int count, SkPMColor dst[]) const
          override;
    bool onAppendStages(SkRasterPipeline* p,
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

void SkHighContrast_Filter::filterSpan(const SkPMColor src[], int count,
                                       SkPMColor dst[]) const {
    for (int i = 0; i < count; ++i) {
        dst[i] = src[i];
        SkPMColor c = src[i];
        unsigned char a = SkGetPackedA32(c);
        unsigned char r = SkGetPackedR32(c);
        unsigned char g = SkGetPackedG32(c);
        unsigned char b = SkGetPackedB32(c);
        SkHighContrastFilter::Apply(fConfig, &r, &g, &b);
        dst[i] = SkPremultiplyARGBInline(a, r, g, b);
    }
}

bool SkHighContrast_Filter::onAppendStages(SkRasterPipeline* p,
                                           SkColorSpace* dst,
                                           SkArenaAlloc* scratch,
                                           bool shaderIsOpaque) const {
    if (fConfig.fGrayscale) {
        float r = SK_LUM_COEFF_R;
        float g = SK_LUM_COEFF_G;
        float b = SK_LUM_COEFF_B;
        static float matrix[12] = {0};
        matrix[0] = matrix[1] = matrix[2] = r;
        matrix[3] = matrix[4] = matrix[5] = g;
        matrix[6] = matrix[7] = matrix[8] = b;
        p->append(SkRasterPipeline::matrix_3x4, matrix);
    }

    if (fConfig.fInvertStyle == HighContrastInvertStyle::kInvertBrightness) {
        static float matrix[12] = {0};
        matrix[0] = matrix[4] = matrix[8] = -1;
        matrix[9] = matrix[10] = matrix[11] = 1;
        p->append(SkRasterPipeline::matrix_3x4, matrix);
    } else if (fConfig.fInvertStyle == HighContrastInvertStyle::kInvertLightness) {
        p->append(SkRasterPipeline::rgb_to_hsl);
        static float matrix[12] = {0};
        matrix[0] = matrix[4] = matrix[11] = 1;
        matrix[8] = -1;
        p->append(SkRasterPipeline::matrix_3x4, matrix);
        p->append(SkRasterPipeline::hsl_to_rgb);
    }

    if (fConfig.fContrast != 0.0) {
        static float matrix[12] = {0};
        float c = fConfig.fContrast;
        float m = (1 + c) / (1 - c);
        float b = (-0.5f * m + 0.5f);
        matrix[0] = matrix[4] = matrix[8] = m;
        matrix[9] = matrix[10] = matrix[11] = b;
        p->append(SkRasterPipeline::matrix_3x4, matrix);
    }

    p->append(SkRasterPipeline::clamp_0);
    p->append(SkRasterPipeline::clamp_1);

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
    config.fInvertStyle = static_cast<HighContrastInvertStyle>(
        buffer.readInt());
    config.fContrast = buffer.readScalar();
    return SkHighContrastFilter::Make(config);
}

sk_sp<SkColorFilter> SkHighContrastFilter::Make(
    const SkHighContrastConfig& config) {
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

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
        inout->invalidateComponents(kRGBA_GrColorComponentFlags);
    }

    SkHighContrastConfig fConfig;

    typedef GrFragmentProcessor INHERITED;
};

class GLHighContrastFilterEffect : public GrGLSLFragmentProcessor {
public:
    static void GenKey(const GrProcessor&, const GrShaderCaps&, GrProcessorKeyBuilder*) {}

    void emitCode(EmitArgs& args) override;

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) override;

private:
    UniformHandle fGrayscaleUni;
    UniformHandle fInvertStyleUni;
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

void GLHighContrastFilterEffect::onSetData(const GrGLSLProgramDataManager& pdm, const GrProcessor& proc) {
    const HighContrastFilterEffect& hcfe = proc.cast<HighContrastFilterEffect>();
    pdm.set1i(fGrayscaleUni, hcfe.config().fGrayscale);
    pdm.set1i(fInvertStyleUni, static_cast<int32_t>(hcfe.config().fInvertStyle));
    pdm.set1f(fContrastUni, hcfe.config().fContrast);
}

void GLHighContrastFilterEffect::emitCode(EmitArgs& args) {
    const char* grayscale;
    const char* invertStyle;
    const char* contrast;
    fGrayscaleUni = args.fUniformHandler->addUniform(kFragment_GrShaderFlag,
                                                     kInt_GrSLType, kDefault_GrSLPrecision,
                                                     "grayscale", &grayscale);
    fInvertStyleUni = args.fUniformHandler->addUniform(kFragment_GrShaderFlag,
                                                       kInt_GrSLType, kDefault_GrSLPrecision,
                                                       "invertStyle", &invertStyle);
    fContrastUni = args.fUniformHandler->addUniform(kFragment_GrShaderFlag,
                                                    kFloat_GrSLType, kDefault_GrSLPrecision,
                                                    "contrast", &contrast);


    if (nullptr == args.fInputColor) {
        args.fInputColor = "vec4(1)";
    }

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

    fragBuilder->codeAppendf("\tfloat r = %s.r;\n", args.fInputColor);
    fragBuilder->codeAppendf("\tfloat g = %s.g;\n", args.fInputColor);
    fragBuilder->codeAppendf("\tfloat b = %s.b;\n", args.fInputColor);
    fragBuilder->codeAppendf("\t\t\n");

    // Grayscale.
    fragBuilder->codeAppendf("\tif (%s != 0) {\n", grayscale);
    fragBuilder->codeAppendf("\t\tfloat luma = %.4f * r +\n", SK_LUM_COEFF_R);
    fragBuilder->codeAppendf("\t\t\t%.4f * g +\n", SK_LUM_COEFF_G);
    fragBuilder->codeAppendf("\t\t\t%.4f * b;\n", SK_LUM_COEFF_B);
    fragBuilder->codeAppendf("\t\tr = g = b = luma;\n");
    fragBuilder->codeAppendf("\t}\n");
    fragBuilder->codeAppendf("\t\t\n");

    // Invert brightness.
    fragBuilder->codeAppendf("\tif (%s == %d) {\n", invertStyle, HighContrastInvertStyle::kInvertBrightness);
    fragBuilder->codeAppendf("\t\tr = 1 - r;\n");
    fragBuilder->codeAppendf("\t\tg = 1 - g;\n");
    fragBuilder->codeAppendf("\t\tb = 1 - b;\n");
    fragBuilder->codeAppendf("\t}\n");
    fragBuilder->codeAppendf("\t\t\n");

    // Invert lightness.
    fragBuilder->codeAppendf("\tif (%s == %d) {\n", invertStyle, HighContrastInvertStyle::kInvertLightness);
    // Convert from RGB to HSL.
    fragBuilder->codeAppendf("\t\tfloat fmax = max(r, max(g, b));\n");
    fragBuilder->codeAppendf("\t\tfloat fmin = min(r, min(g, b));\n");
    fragBuilder->codeAppendf("\t\tfloat l = (fmax + fmin) / 2;\n");

    fragBuilder->codeAppendf("\t\tfloat h;\n");
    fragBuilder->codeAppendf("\t\tfloat s;\n");

    fragBuilder->codeAppendf("\t\tif (fmax == fmin) {\n");
    fragBuilder->codeAppendf("\t\t\th = 0;\n");
    fragBuilder->codeAppendf("\t\t\ts = 0;\n");
    fragBuilder->codeAppendf("\t\t} else {\n");
    fragBuilder->codeAppendf("\t\t\tfloat d = fmax - fmin;\n");
    fragBuilder->codeAppendf("\t\t\ts = l > 0.5 ? d / (2 - fmax - fmin) : d / (fmax + fmin);\n");
    fragBuilder->codeAppendf("\t\t\tif (fmax == r) {\n");
    fragBuilder->codeAppendf("\t\t\t\th = (g - b) / d + (g < b ? 6 : 0);\n");
    fragBuilder->codeAppendf("\t\t\t} else if (fmax == g) {\n");
    fragBuilder->codeAppendf("\t\t\t\th = (b - r) / d + 2;\n");
    fragBuilder->codeAppendf("\t\t\t} else {\n");
    fragBuilder->codeAppendf("\t\t\t\th = (r - g) / d + 4;\n");
    fragBuilder->codeAppendf("\t\t\t}\n");
    fragBuilder->codeAppendf("\t\t}\n");
    fragBuilder->codeAppendf("\t\th /= 6;\n");

    // Invert lightness.
    fragBuilder->codeAppendf("\t\tl = 1.0 - l;\n");

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
                              "\t\tif (t < 0)\n"
                              "\t\t\tt += 1;\n"
                              "\t\tif (t > 1)\n"
                              "\t\t\tt -= 1;\n"
                              "\t\tif (t < 1/6.)\n"
                              "\t\t\treturn p + (q - p) * 6 * t;\n"
                              "\t\tif (t < 1/2.)\n"
                              "\t\t\treturn q;\n"
                              "\t\tif (t < 2/3.)\n"
                              "\t\t\treturn p + (q - p) * (2/3. - t) * 6;\n"
                              "\t\treturn p;\n",
                              &hue2rgbFuncName);

    fragBuilder->codeAppendf("\t\tif (s == 0) {\n");
    fragBuilder->codeAppendf("\t\t\tr = l;\n");
    fragBuilder->codeAppendf("\t\t\tg = l;\n");
    fragBuilder->codeAppendf("\t\t\tb = l;\n");
    fragBuilder->codeAppendf("\t\t} else {\n");
    fragBuilder->codeAppendf("\t\t\tfloat q = l < 0.5 ? l * (1 + s) : l + s - l * s;\n");
    fragBuilder->codeAppendf("\t\t\tfloat p = 2 * l - q;\n");
    fragBuilder->codeAppendf("\t\t\tr = %s(p, q, h + 1/3.);\n", hue2rgbFuncName.c_str());
    fragBuilder->codeAppendf("\t\t\tg = %s(p, q, h);\n", hue2rgbFuncName.c_str());
    fragBuilder->codeAppendf("\t\t\tb = %s(p, q, h - 1/3.);\n", hue2rgbFuncName.c_str());
    fragBuilder->codeAppendf("\t\t}\n");
    fragBuilder->codeAppendf("\t}\n");
    fragBuilder->codeAppendf("\t\t\n");

    // Contrast.
    fragBuilder->codeAppendf("\tif (%s != 0) {\n", contrast);
    fragBuilder->codeAppendf("\t\tfloat m = (1 + %s) / (1 - %s);\n", contrast, contrast);
    fragBuilder->codeAppendf("\t\tfloat off = (-0.5 * m + 0.5);\n");
    fragBuilder->codeAppendf("\t\tr = m * r + off;\n");
    fragBuilder->codeAppendf("\t\tg = m * g + off;\n");
    fragBuilder->codeAppendf("\t\tb = m * b + off;\n");
    fragBuilder->codeAppendf("\t}\n");
    fragBuilder->codeAppendf("\t\t\n");

    // Clamp.
    fragBuilder->codeAppendf("\tr = r > 1 ? 1 : r;\n");
    fragBuilder->codeAppendf("\tg = g > 1 ? 1 : g;\n");
    fragBuilder->codeAppendf("\tb = b > 1 ? 1 : b;\n");
    fragBuilder->codeAppendf("\tr = r < 0 ? 0 : r;\n");
    fragBuilder->codeAppendf("\tg = g < 0 ? 0 : g;\n");
    fragBuilder->codeAppendf("\tb = b < 0 ? 0 : b;\n");

    fragBuilder->codeAppendf("\t%s.r = r;\n", args.fOutputColor);
    fragBuilder->codeAppendf("\t%s.g = g;\n", args.fOutputColor);
    fragBuilder->codeAppendf("\t%s.b = b;\n", args.fOutputColor);
    fragBuilder->codeAppendf("\t%s.a = %s.a;\n", args.fOutputColor, args.fInputColor);
}

sk_sp<GrFragmentProcessor> SkHighContrast_Filter::asFragmentProcessor(GrContext*, SkColorSpace*) const {
    return HighContrastFilterEffect::Make(fConfig);
}
#endif
