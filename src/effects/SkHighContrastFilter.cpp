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
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkVM.h"
#include "src/core/SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "include/gpu/GrContext.h"
#include "src/gpu/GrColorInfo.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#endif

using InvertStyle = SkHighContrastConfig::InvertStyle;

// Matrix multiplication routines.

#define R result->fMat
#define F first.fMat
#define S second.fMat

// result_4x1 = first_4x4 . second_4x1
void multiply_4x4_to_4x1(Color4x1Matrix* result,
                         const Color4x4Matrix& first,
                         const Color4x1Matrix& second) {
    R[0] = F[0] * S[0] + F[1] * S[1] + F[2] * S[2] + F[3] * S[3];
    R[1] = F[4] * S[0] + F[5] * S[1] + F[6] * S[2] + F[7] * S[3];
    R[2] = F[8] * S[0] + F[9] * S[1] + F[10] * S[2] + F[11] * S[3];

    // Skip computing R[3], as it would be 1.
    // R[3] = 1;
    R[3] = F[12] * S[0] + F[13] * S[1] + F[14] * S[2] + F[15] * S[3];
}

// result_4x4 = first_4x4 . second_4x4
void multiply_4x4_to_4x4(Color4x4Matrix* result,
                         const Color4x4Matrix& first,
                         const Color4x4Matrix& second) {
    R[0] = F[0] * S[0] + F[1] * S[4] + F[2] * S[8] + F[3] * S[12];
    R[1] = F[0] * S[1] + F[1] * S[5] + F[2] * S[9] + F[3] * S[13];
    R[2] = F[0] * S[2] + F[1] * S[6] + F[2] * S[10] + F[3] * S[14];
    R[3] = F[0] * S[3] + F[1] * S[7] + F[2] * S[11] + F[3] * S[15];

    R[4] = F[4] * S[0] + F[5] * S[4] + F[6] * S[8] + F[7] * S[12];
    R[5] = F[4] * S[1] + F[5] * S[5] + F[6] * S[9] + F[7] * S[13];
    R[6] = F[4] * S[2] + F[5] * S[6] + F[6] * S[10] + F[7] * S[14];
    R[7] = F[4] * S[3] + F[5] * S[7] + F[6] * S[11] + F[7] * S[15];

    R[8] = F[8] * S[0] + F[9] * S[4] + F[10] * S[8] + F[11] * S[12];
    R[9] = F[8] * S[1] + F[9] * S[5] + F[10] * S[9] + F[11] * S[13];
    R[10] = F[8] * S[2] + F[9] * S[6] + F[10] * S[10] + F[11] * S[14];
    R[11] = F[8] * S[3] + F[9] * S[7] + F[10] * S[11] + F[11] * S[15];

    // Skip computing R[12] to R[15], as they would be 0, 0, 0, 1.
    // R[12] = R[13] = R[14] = 0;
    // R[15] = 1;
    R[12] = F[12] * S[0] + F[13] * S[4] + F[14] * S[8] + F[15] * S[12];
    R[13] = F[12] * S[1] + F[13] * S[5] + F[14] * S[9] + F[15] * S[13];
    R[14] = F[12] * S[2] + F[13] * S[6] + F[14] * S[10] + F[15] * S[14];
    R[15] = F[12] * S[3] + F[13] * S[7] + F[14] * S[11] + F[15] * S[15];
}

#undef R
#undef F
#undef S

const Color4x4Matrix kGrayScaleMatrix{SK_LUM_COEFF_R,
                                      SK_LUM_COEFF_G,
                                      SK_LUM_COEFF_B,
                                      0,
                                      SK_LUM_COEFF_R,
                                      SK_LUM_COEFF_G,
                                      SK_LUM_COEFF_B,
                                      0,
                                      SK_LUM_COEFF_R,
                                      SK_LUM_COEFF_G,
                                      SK_LUM_COEFF_B,
                                      0,
                                      0,
                                      0,
                                      0,
                                      1};

const Color4x4Matrix kInvertBrightnessMatrix{-1, 0, 0, 1, 0, -1, 0, 1, 0, 0, -1, 1, 0, 0, 0, 1};

const Color4x4Matrix kRGBToYCbCrBT601Matrix{
        0.299f, 0.587f,     0.114f,     0, -0.1687736f, -0.331264f, 0.5f, 0,
        0.5f,   -0.418688f, -0.081312f, 0, 0,           0,          0,    1};

const Color4x4Matrix kInvertLumaMatrix{-1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

const Color4x4Matrix kYCbCrBT601ToRGBMatrix{1, 0,      1.402f, 0, 1, -0.344f, -0.714136f, 0,
                                            1, 1.772f, 0,      0, 0, 0,       0,          1};

Color4x4Matrix getMatrix_InvertLightness_YCbCrBT601() {
    Color4x4Matrix result;
    Color4x4Matrix temp;
    multiply_4x4_to_4x4(&temp, kYCbCrBT601ToRGBMatrix, kInvertLumaMatrix);
    multiply_4x4_to_4x4(&result, temp, kRGBToYCbCrBT601Matrix);

    return result;
}

SkHighContrastFilter::SkHighContrastFilter(const SkHighContrastConfig& config) {
    fConfig = config;
    // Clamp contrast to just inside -1 to 1 to avoid division by zero.
    fConfig.fContrast = SkTPin(fConfig.fContrast, -1.0f + FLT_EPSILON, 1.0f - FLT_EPSILON);

    buildTransformationMatrix();
}

sk_sp<SkColorFilter> SkHighContrastFilter::Make(const SkHighContrastConfig& config) {
    if (!config.isValid()) {
        return nullptr;
    }

    return sk_make_sp<SkHighContrastFilter>(config);
}

void SkHighContrastFilter::buildTransformationMatrix() {
    if (!fConfig.fUseOptimized) return;

    if (fConfig.fGrayscale) {
        fTransformationMatrix = std::make_unique<Color4x4Matrix>(kGrayScaleMatrix);
    } else {
        fTransformationMatrix = std::make_unique<Color4x4Matrix>();
    }

    if (fConfig.fInvertStyle == InvertStyle::kInvertBrightness) {
        Color4x4Matrix temp(*fTransformationMatrix);
        multiply_4x4_to_4x4(fTransformationMatrix.get(), kInvertBrightnessMatrix, temp);
    } else if (fConfig.fInvertStyle == InvertStyle::kInvertLightness_YCbCrBT601) {
        static Color4x4Matrix matrix = getMatrix_InvertLightness_YCbCrBT601();
        Color4x4Matrix temp(*fTransformationMatrix);
        multiply_4x4_to_4x4(fTransformationMatrix.get(), matrix, temp);
    }

    if (fConfig.fContrast != 0.0) {
        float c = fConfig.fContrast;
        float m = (1 + c) / (1 - c);
        float b = (-0.5f * m + 0.5f);
        Color4x4Matrix contrast(m, 0, 0, b, 0, m, 0, b, 0, 0, m, b, 0, 0, 0, 1);

        Color4x4Matrix temp(*fTransformationMatrix);
        multiply_4x4_to_4x4(fTransformationMatrix.get(), contrast, temp);
    }
}

SkColor SkHighContrastFilter::filterColorWithoutPipeline(SkColor color) const {
    SkScalar rc = SkColorGetR(color) / 255;
    SkScalar gc = SkColorGetG(color) / 255;
    SkScalar bc = SkColorGetB(color) / 255;

    // Apply a gamma of 2.0 so that the rest of the calculations
    // happen roughly in linear space.
    rc *= rc;
    gc *= gc;
    bc *= bc;

    Color4x1Matrix color_vector(rc, gc, bc, 1);
    Color4x1Matrix output;
    multiply_4x4_to_4x1(&output, *fTransformationMatrix, color_vector);

    rc = std::min(std::max(rc, 0.0f), 1.0f);
    gc = std::min(std::max(gc, 0.0f), 1.0f);
    bc = std::min(std::max(bc, 0.0f), 1.0f);

    // Convert back from linear to a color space with a gamma of ~2.0.
    rc = SkScalarSqrt(rc) * 255;
    gc = SkScalarSqrt(gc) * 255;
    bc = SkScalarSqrt(bc) * 255;

    return SkColorSetARGB(SkColorGetA(color), rc, gc, bc);
}

bool SkHighContrastFilter::onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const {
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

skvm::Color SkHighContrastFilter::onProgram(skvm::Builder* p,
                                            skvm::Color c,
                                            SkColorSpace* dstCS,
                                            skvm::Uniforms* uniforms,
                                            SkArenaAlloc* alloc) const {
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
        skvm::F32 gray = p->mad(p->splat(SK_LUM_COEFF_R),c.r,
                         p->mad(p->splat(SK_LUM_COEFF_G),c.g,
                         p->mul(p->splat(SK_LUM_COEFF_B),c.b)));
        c = {gray, gray, gray, c.a};
    }

    if (fConfig.fInvertStyle == InvertStyle::kInvertBrightness) {
        c = {p->inv(c.r), p->inv(c.g), p->inv(c.b), c.a};
    } else if (fConfig.fInvertStyle == InvertStyle::kInvertLightness) {
        auto [h, s, l, a] = p->to_hsla(c);
        c = p->to_rgba({h, s, p->inv(l), a});
    }

    if (fConfig.fContrast != 0.0) {
        const float m = (1 + fConfig.fContrast) / (1 - fConfig.fContrast);
        const float b = (-0.5f * m + 0.5f);
        skvm::F32   M = p->uniformF(uniforms->pushF(m));
        skvm::F32   B = p->uniformF(uniforms->pushF(b));
        c = {p->mad(M,c.r, B), p->mad(M,c.g, B), p->mad(M,c.b, B), c.a};
    }

    c = {p->clamp01(c.r), p->clamp01(c.g), p->clamp01(c.b), c.a};

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

void SkHighContrastFilter::flatten(SkWriteBuffer& buffer) const {
    buffer.writeBool(fConfig.fGrayscale);
    buffer.writeInt(static_cast<int>(fConfig.fInvertStyle));
    buffer.writeScalar(fConfig.fContrast);
}

sk_sp<SkFlattenable> SkHighContrastFilter::CreateProc(SkReadBuffer& buffer) {
    SkHighContrastConfig config;
    config.fGrayscale = buffer.readBool();
    config.fInvertStyle = buffer.read32LE(InvertStyle::kLast);
    config.fContrast = buffer.readScalar();

    return SkHighContrastFilter::Make(config);
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

    fragBuilder->codeAppendf("half4 color = %s;", args.fInputColor);

    // Unpremultiply. The max() is to guard against 0 / 0.
    fragBuilder->codeAppendf("half nonZeroAlpha = max(color.a, 0.0001);");
    fragBuilder->codeAppendf("color = half4(color.rgb / nonZeroAlpha, nonZeroAlpha);");

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

std::unique_ptr<GrFragmentProcessor> SkHighContrastFilter::asFragmentProcessor(
        GrRecordingContext*, const GrColorInfo& csi) const {
    bool linearize = !csi.isLinearlyBlended();
    return HighContrastFilterEffect::Make(fConfig, linearize);
}
#endif
