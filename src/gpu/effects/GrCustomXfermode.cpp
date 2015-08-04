/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "effects/GrCustomXfermode.h"
#include "effects/GrCustomXfermodePriv.h"

#include "GrCoordTransform.h"
#include "GrContext.h"
#include "GrFragmentProcessor.h"
#include "GrInvariantOutput.h"
#include "GrProcessor.h"
#include "GrTexture.h"
#include "GrTextureAccess.h"
#include "SkXfermode.h"
#include "gl/GrGLCaps.h"
#include "gl/GrGLGpu.h"
#include "gl/GrGLFragmentProcessor.h"
#include "gl/GrGLProgramDataManager.h"
#include "gl/builders/GrGLProgramBuilder.h"
#include "glsl/GrGLSLCaps.h"

bool GrCustomXfermode::IsSupportedMode(SkXfermode::Mode mode) {
    return mode > SkXfermode::kLastCoeffMode && mode <= SkXfermode::kLastMode;
}

///////////////////////////////////////////////////////////////////////////////
// Static helpers
///////////////////////////////////////////////////////////////////////////////

static GrBlendEquation hw_blend_equation(SkXfermode::Mode mode) {
    enum { kOffset = kOverlay_GrBlendEquation - SkXfermode::kOverlay_Mode };
    return static_cast<GrBlendEquation>(mode + kOffset);

    GR_STATIC_ASSERT(kOverlay_GrBlendEquation == SkXfermode::kOverlay_Mode + kOffset);
    GR_STATIC_ASSERT(kDarken_GrBlendEquation == SkXfermode::kDarken_Mode + kOffset);
    GR_STATIC_ASSERT(kLighten_GrBlendEquation == SkXfermode::kLighten_Mode + kOffset);
    GR_STATIC_ASSERT(kColorDodge_GrBlendEquation == SkXfermode::kColorDodge_Mode + kOffset);
    GR_STATIC_ASSERT(kColorBurn_GrBlendEquation == SkXfermode::kColorBurn_Mode + kOffset);
    GR_STATIC_ASSERT(kHardLight_GrBlendEquation == SkXfermode::kHardLight_Mode + kOffset);
    GR_STATIC_ASSERT(kSoftLight_GrBlendEquation == SkXfermode::kSoftLight_Mode + kOffset);
    GR_STATIC_ASSERT(kDifference_GrBlendEquation == SkXfermode::kDifference_Mode + kOffset);
    GR_STATIC_ASSERT(kExclusion_GrBlendEquation == SkXfermode::kExclusion_Mode + kOffset);
    GR_STATIC_ASSERT(kMultiply_GrBlendEquation == SkXfermode::kMultiply_Mode + kOffset);
    GR_STATIC_ASSERT(kHSLHue_GrBlendEquation == SkXfermode::kHue_Mode + kOffset);
    GR_STATIC_ASSERT(kHSLSaturation_GrBlendEquation == SkXfermode::kSaturation_Mode + kOffset);
    GR_STATIC_ASSERT(kHSLColor_GrBlendEquation == SkXfermode::kColor_Mode + kOffset);
    GR_STATIC_ASSERT(kHSLLuminosity_GrBlendEquation == SkXfermode::kLuminosity_Mode + kOffset);
    GR_STATIC_ASSERT(kGrBlendEquationCnt == SkXfermode::kLastMode + 1 + kOffset);
}

static bool can_use_hw_blend_equation(GrBlendEquation equation,
                                      const GrProcOptInfo& coveragePOI,
                                      const GrCaps& caps) {
    if (!caps.advancedBlendEquationSupport()) {
        return false;
    }
    if (coveragePOI.isFourChannelOutput()) {
        return false; // LCD coverage must be applied after the blend equation.
    }
    if (caps.canUseAdvancedBlendEquation(equation)) {
        return false;
    }
    return true;
}

static void hard_light(GrGLFragmentBuilder* fsBuilder,
                       const char* final,
                       const char* src,
                       const char* dst) {
    static const char kComponents[] = {'r', 'g', 'b'};
    for (size_t i = 0; i < SK_ARRAY_COUNT(kComponents); ++i) {
        char component = kComponents[i];
        fsBuilder->codeAppendf("if (2.0 * %s.%c <= %s.a) {", src, component, src);
        fsBuilder->codeAppendf("%s.%c = 2.0 * %s.%c * %s.%c;",
                               final, component, src, component, dst, component);
        fsBuilder->codeAppend("} else {");
        fsBuilder->codeAppendf("%s.%c = %s.a * %s.a - 2.0 * (%s.a - %s.%c) * (%s.a - %s.%c);",
                               final, component, src, dst, dst, dst, component, src, src,
                               component);
        fsBuilder->codeAppend("}");
    }
    fsBuilder->codeAppendf("%s.rgb += %s.rgb * (1.0 - %s.a) + %s.rgb * (1.0 - %s.a);",
                           final, src, dst, dst, src);
}

// Does one component of color-dodge
static void color_dodge_component(GrGLFragmentBuilder* fsBuilder,
                                  const char* final,
                                  const char* src,
                                  const char* dst,
                                  const char component) {
    fsBuilder->codeAppendf("if (0.0 == %s.%c) {", dst, component);
    fsBuilder->codeAppendf("%s.%c = %s.%c * (1.0 - %s.a);",
                           final, component, src, component, dst);
    fsBuilder->codeAppend("} else {");
    fsBuilder->codeAppendf("float d = %s.a - %s.%c;", src, src, component);
    fsBuilder->codeAppend("if (0.0 == d) {");
    fsBuilder->codeAppendf("%s.%c = %s.a * %s.a + %s.%c * (1.0 - %s.a) + %s.%c * (1.0 - %s.a);",
                           final, component, src, dst, src, component, dst, dst, component,
                           src);
    fsBuilder->codeAppend("} else {");
    fsBuilder->codeAppendf("d = min(%s.a, %s.%c * %s.a / d);",
                           dst, dst, component, src);
    fsBuilder->codeAppendf("%s.%c = d * %s.a + %s.%c * (1.0 - %s.a) + %s.%c * (1.0 - %s.a);",
                           final, component, src, src, component, dst, dst, component, src);
    fsBuilder->codeAppend("}");
    fsBuilder->codeAppend("}");
}

// Does one component of color-burn
static void color_burn_component(GrGLFragmentBuilder* fsBuilder,
                                 const char* final,
                                 const char* src,
                                 const char* dst,
                                 const char component) {
    fsBuilder->codeAppendf("if (%s.a == %s.%c) {", dst, dst, component);
    fsBuilder->codeAppendf("%s.%c = %s.a * %s.a + %s.%c * (1.0 - %s.a) + %s.%c * (1.0 - %s.a);",
                           final, component, src, dst, src, component, dst, dst, component,
                           src);
    fsBuilder->codeAppendf("} else if (0.0 == %s.%c) {", src, component);
    fsBuilder->codeAppendf("%s.%c = %s.%c * (1.0 - %s.a);",
                           final, component, dst, component, src);
    fsBuilder->codeAppend("} else {");
    fsBuilder->codeAppendf("float d = max(0.0, %s.a - (%s.a - %s.%c) * %s.a / %s.%c);",
                           dst, dst, dst, component, src, src, component);
    fsBuilder->codeAppendf("%s.%c = %s.a * d + %s.%c * (1.0 - %s.a) + %s.%c * (1.0 - %s.a);",
                           final, component, src, src, component, dst, dst, component, src);
    fsBuilder->codeAppend("}");
}

// Does one component of soft-light. Caller should have already checked that dst alpha > 0.
static void soft_light_component_pos_dst_alpha(GrGLFragmentBuilder* fsBuilder,
                                               const char* final,
                                               const char* src,
                                               const char* dst,
                                               const char component) {
    // if (2S < Sa)
    fsBuilder->codeAppendf("if (2.0 * %s.%c <= %s.a) {", src, component, src);
    // (D^2 (Sa-2 S))/Da+(1-Da) S+D (-Sa+2 S+1)
    fsBuilder->codeAppendf("%s.%c = (%s.%c*%s.%c*(%s.a - 2.0*%s.%c)) / %s.a +"
                                   "(1.0 - %s.a) * %s.%c + %s.%c*(-%s.a + 2.0*%s.%c + 1.0);",
                           final, component, dst, component, dst, component, src, src,
                           component, dst, dst, src, component, dst, component, src, src,
                           component);
    // else if (4D < Da)
    fsBuilder->codeAppendf("} else if (4.0 * %s.%c <= %s.a) {",
                           dst, component, dst);
    fsBuilder->codeAppendf("float DSqd = %s.%c * %s.%c;",
                           dst, component, dst, component);
    fsBuilder->codeAppendf("float DCub = DSqd * %s.%c;", dst, component);
    fsBuilder->codeAppendf("float DaSqd = %s.a * %s.a;", dst, dst);
    fsBuilder->codeAppendf("float DaCub = DaSqd * %s.a;", dst);
    // (Da^3 (-S)+Da^2 (S-D (3 Sa-6 S-1))+12 Da D^2 (Sa-2 S)-16 D^3 (Sa-2 S))/Da^2
    fsBuilder->codeAppendf("%s.%c ="
                           "(DaSqd*(%s.%c - %s.%c * (3.0*%s.a - 6.0*%s.%c - 1.0)) +"
                           " 12.0*%s.a*DSqd*(%s.a - 2.0*%s.%c) - 16.0*DCub * (%s.a - 2.0*%s.%c) -"
                           " DaCub*%s.%c) / DaSqd;",
                           final, component, src, component, dst, component,
                           src, src, component, dst, src, src, component, src, src,
                           component, src, component);
    fsBuilder->codeAppendf("} else {");
    // -sqrt(Da * D) (Sa-2 S)-Da S+D (Sa-2 S+1)+S
    fsBuilder->codeAppendf("%s.%c = %s.%c*(%s.a - 2.0*%s.%c + 1.0) + %s.%c -"
                           " sqrt(%s.a*%s.%c)*(%s.a - 2.0*%s.%c) - %s.a*%s.%c;",
                           final, component, dst, component, src, src, component, src, component,
                           dst, dst, component, src, src, component, dst, src, component);
    fsBuilder->codeAppendf("}");
}

// Adds a function that takes two colors and an alpha as input. It produces a color with the
// hue and saturation of the first color, the luminosity of the second color, and the input
// alpha. It has this signature:
//      vec3 set_luminance(vec3 hueSatColor, float alpha, vec3 lumColor).
static void add_lum_function(GrGLFragmentBuilder* fsBuilder, SkString* setLumFunction) {
    // Emit a helper that gets the luminance of a color.
    SkString getFunction;
    GrGLShaderVar getLumArgs[] = {
        GrGLShaderVar("color", kVec3f_GrSLType),
    };
    SkString getLumBody("return dot(vec3(0.3, 0.59, 0.11), color);");
    fsBuilder->emitFunction(kFloat_GrSLType,
                            "luminance",
                            SK_ARRAY_COUNT(getLumArgs), getLumArgs,
                            getLumBody.c_str(),
                            &getFunction);

    // Emit the set luminance function.
    GrGLShaderVar setLumArgs[] = {
        GrGLShaderVar("hueSat", kVec3f_GrSLType),
        GrGLShaderVar("alpha", kFloat_GrSLType),
        GrGLShaderVar("lumColor", kVec3f_GrSLType),
    };
    SkString setLumBody;
    setLumBody.printf("float diff = %s(lumColor - hueSat);", getFunction.c_str());
    setLumBody.append("vec3 outColor = hueSat + diff;");
    setLumBody.appendf("float outLum = %s(outColor);", getFunction.c_str());
    setLumBody.append("float minComp = min(min(outColor.r, outColor.g), outColor.b);"
                      "float maxComp = max(max(outColor.r, outColor.g), outColor.b);"
                      "if (minComp < 0.0 && outLum != minComp) {"
                      "outColor = outLum + ((outColor - vec3(outLum, outLum, outLum)) * outLum) /"
                                          "(outLum - minComp);"
                      "}"
                      "if (maxComp > alpha && maxComp != outLum) {"
                      "outColor = outLum +"
                                 "((outColor - vec3(outLum, outLum, outLum)) * (alpha - outLum)) /"
                                 "(maxComp - outLum);"
                      "}"
                      "return outColor;");
    fsBuilder->emitFunction(kVec3f_GrSLType,
                            "set_luminance",
                            SK_ARRAY_COUNT(setLumArgs), setLumArgs,
                            setLumBody.c_str(),
                            setLumFunction);
}

// Adds a function that creates a color with the hue and luminosity of one input color and
// the saturation of another color. It will have this signature:
//      float set_saturation(vec3 hueLumColor, vec3 satColor)
static void add_sat_function(GrGLFragmentBuilder* fsBuilder, SkString* setSatFunction) {
    // Emit a helper that gets the saturation of a color
    SkString getFunction;
    GrGLShaderVar getSatArgs[] = { GrGLShaderVar("color", kVec3f_GrSLType) };
    SkString getSatBody;
    getSatBody.printf("return max(max(color.r, color.g), color.b) - "
                      "min(min(color.r, color.g), color.b);");
    fsBuilder->emitFunction(kFloat_GrSLType,
                            "saturation",
                            SK_ARRAY_COUNT(getSatArgs), getSatArgs,
                            getSatBody.c_str(),
                            &getFunction);

    // Emit a helper that sets the saturation given sorted input channels. This used
    // to use inout params for min, mid, and max components but that seems to cause
    // problems on PowerVR drivers. So instead it returns a vec3 where r, g ,b are the
    // adjusted min, mid, and max inputs, respectively.
    SkString helperFunction;
    GrGLShaderVar helperArgs[] = {
        GrGLShaderVar("minComp", kFloat_GrSLType),
        GrGLShaderVar("midComp", kFloat_GrSLType),
        GrGLShaderVar("maxComp", kFloat_GrSLType),
        GrGLShaderVar("sat", kFloat_GrSLType),
    };
    static const char kHelperBody[] = "if (minComp < maxComp) {"
        "vec3 result;"
        "result.r = 0.0;"
        "result.g = sat * (midComp - minComp) / (maxComp - minComp);"
        "result.b = sat;"
        "return result;"
        "} else {"
        "return vec3(0, 0, 0);"
        "}";
    fsBuilder->emitFunction(kVec3f_GrSLType,
                            "set_saturation_helper",
                            SK_ARRAY_COUNT(helperArgs), helperArgs,
                            kHelperBody,
                            &helperFunction);

    GrGLShaderVar setSatArgs[] = {
        GrGLShaderVar("hueLumColor", kVec3f_GrSLType),
        GrGLShaderVar("satColor", kVec3f_GrSLType),
    };
    const char* helpFunc = helperFunction.c_str();
    SkString setSatBody;
    setSatBody.appendf("float sat = %s(satColor);"
                       "if (hueLumColor.r <= hueLumColor.g) {"
                       "if (hueLumColor.g <= hueLumColor.b) {"
                       "hueLumColor.rgb = %s(hueLumColor.r, hueLumColor.g, hueLumColor.b, sat);"
                       "} else if (hueLumColor.r <= hueLumColor.b) {"
                       "hueLumColor.rbg = %s(hueLumColor.r, hueLumColor.b, hueLumColor.g, sat);"
                       "} else {"
                       "hueLumColor.brg = %s(hueLumColor.b, hueLumColor.r, hueLumColor.g, sat);"
                       "}"
                       "} else if (hueLumColor.r <= hueLumColor.b) {"
                       "hueLumColor.grb = %s(hueLumColor.g, hueLumColor.r, hueLumColor.b, sat);"
                       "} else if (hueLumColor.g <= hueLumColor.b) {"
                       "hueLumColor.gbr = %s(hueLumColor.g, hueLumColor.b, hueLumColor.r, sat);"
                       "} else {"
                       "hueLumColor.bgr = %s(hueLumColor.b, hueLumColor.g, hueLumColor.r, sat);"
                       "}"
                       "return hueLumColor;",
                       getFunction.c_str(), helpFunc, helpFunc, helpFunc, helpFunc,
                       helpFunc, helpFunc);
    fsBuilder->emitFunction(kVec3f_GrSLType,
                            "set_saturation",
                            SK_ARRAY_COUNT(setSatArgs), setSatArgs,
                            setSatBody.c_str(),
                            setSatFunction);

}

static void emit_custom_xfermode_code(SkXfermode::Mode mode,
                                      GrGLFragmentBuilder* fsBuilder,
                                      const char* outputColor,
                                      const char* inputColor,
                                      const char* dstColor) {
    // We don't try to optimize for this case at all
    if (NULL == inputColor) {
        fsBuilder->codeAppendf("const vec4 ones = vec4(1);");
        inputColor = "ones";
    }
    fsBuilder->codeAppendf("// SkXfermode::Mode: %s\n", SkXfermode::ModeName(mode));

    // These all perform src-over on the alpha channel.
    fsBuilder->codeAppendf("%s.a = %s.a + (1.0 - %s.a) * %s.a;",
                           outputColor, inputColor, inputColor, dstColor);

    switch (mode) {
        case SkXfermode::kOverlay_Mode:
            // Overlay is Hard-Light with the src and dst reversed
            hard_light(fsBuilder, outputColor, dstColor, inputColor);
            break;
        case SkXfermode::kDarken_Mode:
            fsBuilder->codeAppendf("%s.rgb = min((1.0 - %s.a) * %s.rgb + %s.rgb, "
                                   "(1.0 - %s.a) * %s.rgb + %s.rgb);",
                                   outputColor,
                                   inputColor, dstColor, inputColor,
                                   dstColor, inputColor, dstColor);
            break;
        case SkXfermode::kLighten_Mode:
            fsBuilder->codeAppendf("%s.rgb = max((1.0 - %s.a) * %s.rgb + %s.rgb, "
                                   "(1.0 - %s.a) * %s.rgb + %s.rgb);",
                                   outputColor,
                                   inputColor, dstColor, inputColor,
                                   dstColor, inputColor, dstColor);
            break;
        case SkXfermode::kColorDodge_Mode:
            color_dodge_component(fsBuilder, outputColor, inputColor, dstColor, 'r');
            color_dodge_component(fsBuilder, outputColor, inputColor, dstColor, 'g');
            color_dodge_component(fsBuilder, outputColor, inputColor, dstColor, 'b');
            break;
        case SkXfermode::kColorBurn_Mode:
            color_burn_component(fsBuilder, outputColor, inputColor, dstColor, 'r');
            color_burn_component(fsBuilder, outputColor, inputColor, dstColor, 'g');
            color_burn_component(fsBuilder, outputColor, inputColor, dstColor, 'b');
            break;
        case SkXfermode::kHardLight_Mode:
            hard_light(fsBuilder, outputColor, inputColor, dstColor);
            break;
        case SkXfermode::kSoftLight_Mode:
            fsBuilder->codeAppendf("if (0.0 == %s.a) {", dstColor);
            fsBuilder->codeAppendf("%s.rgba = %s;", outputColor, inputColor);
            fsBuilder->codeAppendf("} else {");
            soft_light_component_pos_dst_alpha(fsBuilder, outputColor, inputColor, dstColor, 'r');
            soft_light_component_pos_dst_alpha(fsBuilder, outputColor, inputColor, dstColor, 'g');
            soft_light_component_pos_dst_alpha(fsBuilder, outputColor, inputColor, dstColor, 'b');
            fsBuilder->codeAppendf("}");
            break;
        case SkXfermode::kDifference_Mode:
            fsBuilder->codeAppendf("%s.rgb = %s.rgb + %s.rgb -"
                                   "2.0 * min(%s.rgb * %s.a, %s.rgb * %s.a);",
                                   outputColor, inputColor, dstColor, inputColor, dstColor,
                                   dstColor, inputColor);
            break;
        case SkXfermode::kExclusion_Mode:
            fsBuilder->codeAppendf("%s.rgb = %s.rgb + %s.rgb - "
                                   "2.0 * %s.rgb * %s.rgb;",
                                   outputColor, dstColor, inputColor, dstColor, inputColor);
            break;
        case SkXfermode::kMultiply_Mode:
            fsBuilder->codeAppendf("%s.rgb = (1.0 - %s.a) * %s.rgb + "
                                   "(1.0 - %s.a) * %s.rgb + "
                                   "%s.rgb * %s.rgb;",
                                   outputColor, inputColor, dstColor, dstColor, inputColor,
                                   inputColor, dstColor);
            break;
        case SkXfermode::kHue_Mode: {
            //  SetLum(SetSat(S * Da, Sat(D * Sa)), Sa*Da, D*Sa) + (1 - Sa) * D + (1 - Da) * S
            SkString setSat, setLum;
            add_sat_function(fsBuilder, &setSat);
            add_lum_function(fsBuilder, &setLum);
            fsBuilder->codeAppendf("vec4 dstSrcAlpha = %s * %s.a;",
                                   dstColor, inputColor);
            fsBuilder->codeAppendf("%s.rgb = %s(%s(%s.rgb * %s.a, dstSrcAlpha.rgb),"
                                               "dstSrcAlpha.a, dstSrcAlpha.rgb);",
                                   outputColor, setLum.c_str(), setSat.c_str(), inputColor,
                                   dstColor);
            fsBuilder->codeAppendf("%s.rgb += (1.0 - %s.a) * %s.rgb + (1.0 - %s.a) * %s.rgb;",
                                   outputColor, inputColor, dstColor, dstColor, inputColor);
            break;
        }
        case SkXfermode::kSaturation_Mode: {
            // SetLum(SetSat(D * Sa, Sat(S * Da)), Sa*Da, D*Sa)) + (1 - Sa) * D + (1 - Da) * S
            SkString setSat, setLum;
            add_sat_function(fsBuilder, &setSat);
            add_lum_function(fsBuilder, &setLum);
            fsBuilder->codeAppendf("vec4 dstSrcAlpha = %s * %s.a;",
                                   dstColor, inputColor);
            fsBuilder->codeAppendf("%s.rgb = %s(%s(dstSrcAlpha.rgb, %s.rgb * %s.a),"
                                               "dstSrcAlpha.a, dstSrcAlpha.rgb);",
                                   outputColor, setLum.c_str(), setSat.c_str(), inputColor,
                                   dstColor);
            fsBuilder->codeAppendf("%s.rgb += (1.0 - %s.a) * %s.rgb + (1.0 - %s.a) * %s.rgb;",
                                   outputColor, inputColor, dstColor, dstColor, inputColor);
            break;
        }
        case SkXfermode::kColor_Mode: {
            //  SetLum(S * Da, Sa* Da, D * Sa) + (1 - Sa) * D + (1 - Da) * S
            SkString setLum;
            add_lum_function(fsBuilder, &setLum);
            fsBuilder->codeAppendf("vec4 srcDstAlpha = %s * %s.a;",
                                   inputColor, dstColor);
            fsBuilder->codeAppendf("%s.rgb = %s(srcDstAlpha.rgb, srcDstAlpha.a, %s.rgb * %s.a);",
                                   outputColor, setLum.c_str(), dstColor, inputColor);
            fsBuilder->codeAppendf("%s.rgb += (1.0 - %s.a) * %s.rgb + (1.0 - %s.a) * %s.rgb;",
                                   outputColor, inputColor, dstColor, dstColor, inputColor);
            break;
        }
        case SkXfermode::kLuminosity_Mode: {
            //  SetLum(D * Sa, Sa* Da, S * Da) + (1 - Sa) * D + (1 - Da) * S
            SkString setLum;
            add_lum_function(fsBuilder, &setLum);
            fsBuilder->codeAppendf("vec4 srcDstAlpha = %s * %s.a;",
                                   inputColor, dstColor);
            fsBuilder->codeAppendf("%s.rgb = %s(%s.rgb * %s.a, srcDstAlpha.a, srcDstAlpha.rgb);",
                                   outputColor, setLum.c_str(), dstColor, inputColor);
            fsBuilder->codeAppendf("%s.rgb += (1.0 - %s.a) * %s.rgb + (1.0 - %s.a) * %s.rgb;",
                                   outputColor, inputColor, dstColor, dstColor, inputColor);
            break;
        }
        default:
            SkFAIL("Unknown Custom Xfer mode.");
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Fragment Processor
///////////////////////////////////////////////////////////////////////////////

GrFragmentProcessor* GrCustomXfermode::CreateFP(GrProcessorDataManager* procDataManager,
                                                SkXfermode::Mode mode, GrTexture* background) {
    if (!GrCustomXfermode::IsSupportedMode(mode)) {
        return NULL;
    } else {
        return SkNEW_ARGS(GrCustomXferFP, (procDataManager, mode, background));
    }
}

///////////////////////////////////////////////////////////////////////////////

class GLCustomXferFP : public GrGLFragmentProcessor {
public:
    GLCustomXferFP(const GrFragmentProcessor&) {}
    ~GLCustomXferFP() override {};

    void emitCode(EmitArgs& args) override {
        SkXfermode::Mode mode = args.fFp.cast<GrCustomXferFP>().mode();
        GrGLFragmentBuilder* fsBuilder = args.fBuilder->getFragmentShaderBuilder();
        const char* dstColor = "bgColor";
        fsBuilder->codeAppendf("vec4 %s = ", dstColor);
        fsBuilder->appendTextureLookup(args.fSamplers[0], args.fCoords[0].c_str(),
                                       args.fCoords[0].getType());
        fsBuilder->codeAppendf(";");

        emit_custom_xfermode_code(mode, fsBuilder, args.fOutputColor, args.fInputColor, dstColor);
    }

    void setData(const GrGLProgramDataManager&, const GrProcessor&) override {}

    static void GenKey(const GrFragmentProcessor& proc, const GrGLSLCaps&, GrProcessorKeyBuilder* b) {
        // The background may come from the dst or from a texture.
        uint32_t key = proc.numTextures();
        SkASSERT(key <= 1);
        key |= proc.cast<GrCustomXferFP>().mode() << 1;
        b->add32(key);
    }

private:
    typedef GrGLFragmentProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrCustomXferFP::GrCustomXferFP(GrProcessorDataManager*, SkXfermode::Mode mode, GrTexture* background)
    : fMode(mode) {
    this->initClassID<GrCustomXferFP>();

    SkASSERT(background);
    fBackgroundTransform.reset(kLocal_GrCoordSet, background, 
                               GrTextureParams::kNone_FilterMode);
    this->addCoordTransform(&fBackgroundTransform);
    fBackgroundAccess.reset(background);
    this->addTextureAccess(&fBackgroundAccess);
}

void GrCustomXferFP::onGetGLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const {
    GLCustomXferFP::GenKey(*this, caps, b);
}

GrGLFragmentProcessor* GrCustomXferFP::createGLInstance() const {
    return SkNEW_ARGS(GLCustomXferFP, (*this));
}

bool GrCustomXferFP::onIsEqual(const GrFragmentProcessor& other) const {
    const GrCustomXferFP& s = other.cast<GrCustomXferFP>();
    return fMode == s.fMode;
}

void GrCustomXferFP::onComputeInvariantOutput(GrInvariantOutput* inout) const {
    inout->setToUnknown(GrInvariantOutput::kWill_ReadInput);
}

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrCustomXferFP);
GrFragmentProcessor* GrCustomXferFP::TestCreate(GrProcessorTestData* d) {
    int mode = d->fRandom->nextRangeU(SkXfermode::kLastCoeffMode + 1, SkXfermode::kLastSeparableMode);

    return SkNEW_ARGS(GrCustomXferFP, (d->fProcDataManager, static_cast<SkXfermode::Mode>(mode),
                                       d->fTextures[0]));
}

///////////////////////////////////////////////////////////////////////////////
// Xfer Processor
///////////////////////////////////////////////////////////////////////////////

class CustomXP : public GrXferProcessor {
public:
    CustomXP(SkXfermode::Mode mode, GrBlendEquation hwBlendEquation)
        : fMode(mode),
          fHWBlendEquation(hwBlendEquation) {
        this->initClassID<CustomXP>();
    }

    CustomXP(const DstTexture* dstTexture, bool hasMixedSamples, SkXfermode::Mode mode)
        : INHERITED(dstTexture, true, hasMixedSamples),
          fMode(mode),
          fHWBlendEquation(static_cast<GrBlendEquation>(-1)) {
        this->initClassID<CustomXP>();
    }

    const char* name() const override { return "Custom Xfermode"; }

    GrGLXferProcessor* createGLInstance() const override;

    SkXfermode::Mode mode() const { return fMode; }
    bool hasHWBlendEquation() const { return -1 != static_cast<int>(fHWBlendEquation); }

    GrBlendEquation hwBlendEquation() const {
        SkASSERT(this->hasHWBlendEquation());
        return fHWBlendEquation;
    }

private:
    GrXferProcessor::OptFlags onGetOptimizations(const GrProcOptInfo& colorPOI,
                                                 const GrProcOptInfo& coveragePOI,
                                                 bool doesStencilWrite,
                                                 GrColor* overrideColor,
                                                 const GrCaps& caps) override;

    void onGetGLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override;

    bool onWillNeedXferBarrier(const GrRenderTarget* rt,
                               const GrCaps& caps,
                               GrXferBarrierType* outBarrierType) const override;

    void onGetBlendInfo(BlendInfo*) const override;

    bool onIsEqual(const GrXferProcessor& xpBase) const override;

    const SkXfermode::Mode fMode;
    const GrBlendEquation  fHWBlendEquation;

    typedef GrXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrXPFactory* GrCustomXfermode::CreateXPFactory(SkXfermode::Mode mode) {
    if (!GrCustomXfermode::IsSupportedMode(mode)) {
        return NULL;
    } else {
        return SkNEW_ARGS(GrCustomXPFactory, (mode));
    }
}

///////////////////////////////////////////////////////////////////////////////

class GLCustomXP : public GrGLXferProcessor {
public:
    GLCustomXP(const GrXferProcessor&) {}
    ~GLCustomXP() override {}

    static void GenKey(const GrXferProcessor& p, const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) {
        const CustomXP& xp = p.cast<CustomXP>();
        uint32_t key = 0;
        if (xp.hasHWBlendEquation()) {
            SkASSERT(caps.advBlendEqInteraction() > 0);  // 0 will mean !xp.hasHWBlendEquation().
            key |= caps.advBlendEqInteraction();
            key |= xp.readsCoverage() << 2;
            GR_STATIC_ASSERT(GrGLSLCaps::kLast_AdvBlendEqInteraction < 4);
        }
        if (!xp.hasHWBlendEquation() || caps.mustEnableSpecificAdvBlendEqs()) {
            key |= xp.mode() << 3;
        }
        b->add32(key);
    }

private:
    void emitOutputsForBlendState(const EmitArgs& args) override {
        const CustomXP& xp = args.fXP.cast<CustomXP>();
        SkASSERT(xp.hasHWBlendEquation());

        GrGLXPFragmentBuilder* fsBuilder = args.fPB->getFragmentShaderBuilder();
        fsBuilder->enableAdvancedBlendEquationIfNeeded(xp.hwBlendEquation());

        // Apply coverage by multiplying it into the src color before blending. Mixed samples will
        // "just work" automatically. (See onGetOptimizations())
        if (xp.readsCoverage()) {
            fsBuilder->codeAppendf("%s = %s * %s;",
                                   args.fOutputPrimary, args.fInputCoverage, args.fInputColor);
        } else {
            fsBuilder->codeAppendf("%s = %s;", args.fOutputPrimary, args.fInputColor);
        }
    }

    void emitBlendCodeForDstRead(GrGLXPBuilder* pb, const char* srcColor, const char* dstColor,
                                 const char* outColor, const GrXferProcessor& proc) override {
        const CustomXP& xp = proc.cast<CustomXP>();
        SkASSERT(!xp.hasHWBlendEquation());

        GrGLXPFragmentBuilder* fsBuilder = pb->getFragmentShaderBuilder();
        emit_custom_xfermode_code(xp.mode(), fsBuilder, outColor, srcColor, dstColor);
    }

    void onSetData(const GrGLProgramDataManager&, const GrXferProcessor&) override {}

    typedef GrGLFragmentProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

void CustomXP::onGetGLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const {
    GLCustomXP::GenKey(*this, caps, b);
}

GrGLXferProcessor* CustomXP::createGLInstance() const {
    SkASSERT(this->willReadDstColor() != this->hasHWBlendEquation());
    return SkNEW_ARGS(GLCustomXP, (*this));
}

bool CustomXP::onIsEqual(const GrXferProcessor& other) const {
    const CustomXP& s = other.cast<CustomXP>();
    return fMode == s.fMode && fHWBlendEquation == s.fHWBlendEquation;
}

GrXferProcessor::OptFlags CustomXP::onGetOptimizations(const GrProcOptInfo& colorPOI,
                                                       const GrProcOptInfo& coveragePOI,
                                                       bool doesStencilWrite,
                                                       GrColor* overrideColor,
                                                       const GrCaps& caps) {
  /*
    Most the optimizations we do here are based on tweaking alpha for coverage.

    The general SVG blend equation is defined in the spec as follows:

      Dca' = B(Sc, Dc) * Sa * Da + Y * Sca * (1-Da) + Z * Dca * (1-Sa)
      Da'  = X * Sa * Da + Y * Sa * (1-Da) + Z * Da * (1-Sa)

    (Note that Sca, Dca indicate RGB vectors that are premultiplied by alpha,
     and that B(Sc, Dc) is a mode-specific function that accepts non-multiplied
     RGB colors.)

    For every blend mode supported by this class, i.e. the "advanced" blend
    modes, X=Y=Z=1 and this equation reduces to the PDF blend equation.

    It can be shown that when X=Y=Z=1, these equations can modulate alpha for
    coverage.


    == Color ==

    We substitute Y=Z=1 and define a blend() function that calculates Dca' in
    terms of premultiplied alpha only:

      blend(Sca, Dca, Sa, Da) = {Dca : if Sa == 0,
                                 Sca : if Da == 0,
                                 B(Sca/Sa, Dca/Da) * Sa * Da + Sca * (1-Da) + Dca * (1-Sa) : if Sa,Da != 0}

    And for coverage modulation, we use a post blend src-over model:

      Dca'' = f * blend(Sca, Dca, Sa, Da) + (1-f) * Dca

    (Where f is the fractional coverage.)

    Next we show that canTweakAlphaForCoverage() is true by proving the
    following relationship:

      blend(f*Sca, Dca, f*Sa, Da) == f * blend(Sca, Dca, Sa, Da) + (1-f) * Dca

    General case (f,Sa,Da != 0):

      f * blend(Sca, Dca, Sa, Da) + (1-f) * Dca
        = f * (B(Sca/Sa, Dca/Da) * Sa * Da + Sca * (1-Da) + Dca * (1-Sa)) + (1-f) * Dca  [Sa,Da != 0, definition of blend()]
        = B(Sca/Sa, Dca/Da) * f*Sa * Da + f*Sca * (1-Da) + f*Dca * (1-Sa) + Dca - f*Dca
        = B(Sca/Sa, Dca/Da) * f*Sa * Da + f*Sca - f*Sca * Da + f*Dca - f*Dca * Sa + Dca - f*Dca
        = B(Sca/Sa, Dca/Da) * f*Sa * Da + f*Sca - f*Sca * Da - f*Dca * Sa + Dca
        = B(Sca/Sa, Dca/Da) * f*Sa * Da + f*Sca * (1-Da) - f*Dca * Sa + Dca
        = B(Sca/Sa, Dca/Da) * f*Sa * Da + f*Sca * (1-Da) + Dca * (1 - f*Sa)
        = B(f*Sca/f*Sa, Dca/Da) * f*Sa * Da + f*Sca * (1-Da) + Dca * (1 - f*Sa)  [f!=0]
        = blend(f*Sca, Dca, f*Sa, Da)  [definition of blend()]

    Corner cases (Sa=0, Da=0, and f=0):

      Sa=0: f * blend(Sca, Dca, Sa, Da) + (1-f) * Dca
              = f * Dca + (1-f) * Dca  [Sa=0, definition of blend()]
              = Dca
              = blend(0, Dca, 0, Da)  [definition of blend()]
              = blend(f*Sca, Dca, f*Sa, Da)  [Sa=0]

      Da=0: f * blend(Sca, Dca, Sa, Da) + (1-f) * Dca
              = f * Sca + (1-f) * Dca  [Da=0, definition of blend()]
              = f * Sca  [Da=0]
              = blend(f*Sca, 0, f*Sa, 0)  [definition of blend()]
              = blend(f*Sca, Dca, f*Sa, Da)  [Da=0]

      f=0: f * blend(Sca, Dca, Sa, Da) + (1-f) * Dca
             = Dca  [f=0]
             = blend(0, Dca, 0, Da)  [definition of blend()]
             = blend(f*Sca, Dca, f*Sa, Da)  [f=0]

    == Alpha ==

    We substitute X=Y=Z=1 and define a blend() function that calculates Da':

      blend(Sa, Da) = Sa * Da + Sa * (1-Da) + Da * (1-Sa)
                    = Sa * Da + Sa - Sa * Da + Da - Da * Sa
                    = Sa + Da - Sa * Da

    We use the same model for coverage modulation as we did with color:

      Da'' = f * blend(Sa, Da) + (1-f) * Da

    And show that canTweakAlphaForCoverage() is true by proving the following
    relationship:

      blend(f*Sa, Da) == f * blend(Sa, Da) + (1-f) * Da


      f * blend(Sa, Da) + (1-f) * Da
        = f * (Sa + Da - Sa * Da) + (1-f) * Da
        = f*Sa + f*Da - f*Sa * Da + Da - f*Da
        = f*Sa - f*Sa * Da + Da
        = f*Sa + Da - f*Sa * Da
        = blend(f*Sa, Da)
   */

    OptFlags flags = kNone_OptFlags;
    if (colorPOI.allStagesMultiplyInput()) {
        flags |= kCanTweakAlphaForCoverage_OptFlag;
    }
    if (this->hasHWBlendEquation() && coveragePOI.isSolidWhite()) {
        flags |= kIgnoreCoverage_OptFlag;
    }
    return flags;
}

bool CustomXP::onWillNeedXferBarrier(const GrRenderTarget* rt,
                                     const GrCaps& caps,
                                     GrXferBarrierType* outBarrierType) const {
    if (this->hasHWBlendEquation() && !caps.advancedCoherentBlendEquationSupport()) {
        *outBarrierType = kBlend_GrXferBarrierType;
        return true;
    }
    return false;
}

void CustomXP::onGetBlendInfo(BlendInfo* blendInfo) const {
    if (this->hasHWBlendEquation()) {
        blendInfo->fEquation = this->hwBlendEquation();
    }
}

///////////////////////////////////////////////////////////////////////////////

GrCustomXPFactory::GrCustomXPFactory(SkXfermode::Mode mode)
    : fMode(mode),
      fHWBlendEquation(hw_blend_equation(mode)) {
    SkASSERT(GrCustomXfermode::IsSupportedMode(fMode));
    this->initClassID<GrCustomXPFactory>();
}

GrXferProcessor*
GrCustomXPFactory::onCreateXferProcessor(const GrCaps& caps,
                                         const GrProcOptInfo& colorPOI,
                                         const GrProcOptInfo& coveragePOI,
                                         bool hasMixedSamples,
                                         const DstTexture* dstTexture) const {
    if (can_use_hw_blend_equation(fHWBlendEquation, coveragePOI, caps)) {
        SkASSERT(!dstTexture || !dstTexture->texture());
        return SkNEW_ARGS(CustomXP, (fMode, fHWBlendEquation));
    }
    return SkNEW_ARGS(CustomXP, (dstTexture, hasMixedSamples, fMode));
}

bool GrCustomXPFactory::willReadDstColor(const GrCaps& caps,
                                         const GrProcOptInfo& colorPOI,
                                         const GrProcOptInfo& coveragePOI,
                                         bool hasMixedSamples) const {
    return !can_use_hw_blend_equation(fHWBlendEquation, coveragePOI, caps);
}

void GrCustomXPFactory::getInvariantBlendedColor(const GrProcOptInfo& colorPOI,
                                                 InvariantBlendedColor* blendedColor) const {
    blendedColor->fWillBlendWithDst = true;
    blendedColor->fKnownColorFlags = kNone_GrColorComponentFlags;
}

GR_DEFINE_XP_FACTORY_TEST(GrCustomXPFactory);
GrXPFactory* GrCustomXPFactory::TestCreate(GrProcessorTestData* d) {
    int mode = d->fRandom->nextRangeU(SkXfermode::kLastCoeffMode + 1,
                                      SkXfermode::kLastSeparableMode);

    return SkNEW_ARGS(GrCustomXPFactory, (static_cast<SkXfermode::Mode>(mode)));
}

