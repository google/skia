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
#include "gl/GrGLProcessor.h"
#include "gl/GrGLProgramDataManager.h"
#include "gl/builders/GrGLProgramBuilder.h"

bool GrCustomXfermode::IsSupportedMode(SkXfermode::Mode mode) {
    return mode > SkXfermode::kLastCoeffMode && mode <= SkXfermode::kLastMode;
}

///////////////////////////////////////////////////////////////////////////////
// Static helpers
///////////////////////////////////////////////////////////////////////////////

static void hard_light(GrGLFPFragmentBuilder* fsBuilder,
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
static void color_dodge_component(GrGLFPFragmentBuilder* fsBuilder,
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
static void color_burn_component(GrGLFPFragmentBuilder* fsBuilder,
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
static void soft_light_component_pos_dst_alpha(GrGLFPFragmentBuilder* fsBuilder,
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
                           "(-DaCub*%s.%c + DaSqd*(%s.%c - %s.%c * (3.0*%s.a - 6.0*%s.%c - 1.0)) +"
                           " 12.0*%s.a*DSqd*(%s.a - 2.0*%s.%c) - 16.0*DCub * (%s.a - 2.0*%s.%c)) /"
                           "DaSqd;",
                           final, component, src, component, src, component, dst, component,
                           src, src, component, dst, src, src, component, src, src,
                           component);
    fsBuilder->codeAppendf("} else {");
    // -sqrt(Da * D) (Sa-2 S)-Da S+D (Sa-2 S+1)+S
    fsBuilder->codeAppendf("%s.%c = -sqrt(%s.a*%s.%c)*(%s.a - 2.0*%s.%c) - %s.a*%s.%c +"
                                   "%s.%c*(%s.a - 2.0*%s.%c + 1.0) + %s.%c;",
                           final, component, dst, dst, component, src, src, component, dst,
                           src, component, dst, component, src, src, component, src,
                           component);
    fsBuilder->codeAppendf("}");
}

// Adds a function that takes two colors and an alpha as input. It produces a color with the
// hue and saturation of the first color, the luminosity of the second color, and the input
// alpha. It has this signature:
//      vec3 set_luminance(vec3 hueSatColor, float alpha, vec3 lumColor).
static void add_lum_function(GrGLFPFragmentBuilder* fsBuilder, SkString* setLumFunction) {
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
static void add_sat_function(GrGLFPFragmentBuilder* fsBuilder, SkString* setSatFunction) {
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
                                      GrGLFPFragmentBuilder* fsBuilder,
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

GrFragmentProcessor* GrCustomXfermode::CreateFP(SkXfermode::Mode mode, GrTexture* background) {
    if (!GrCustomXfermode::IsSupportedMode(mode)) {
        return NULL;
    } else {
        return SkNEW_ARGS(GrCustomXferFP, (mode, background));
    }
}

///////////////////////////////////////////////////////////////////////////////

class GLCustomXferFP : public GrGLFragmentProcessor {
public:
    GLCustomXferFP(const GrFragmentProcessor&) {}
    ~GLCustomXferFP() SK_OVERRIDE {};

    void emitCode(GrGLFPBuilder* builder,
                  const GrFragmentProcessor& fp,
                  const char* outputColor,
                  const char* inputColor,
                  const TransformedCoordsArray& coords,
                  const TextureSamplerArray& samplers) SK_OVERRIDE {
        SkXfermode::Mode mode = fp.cast<GrCustomXferFP>().mode();
        GrGLFPFragmentBuilder* fsBuilder = builder->getFragmentShaderBuilder();
        const char* dstColor = "bgColor";
        fsBuilder->codeAppendf("vec4 %s = ", dstColor);
        fsBuilder->appendTextureLookup(samplers[0], coords[0].c_str(), coords[0].getType());
        fsBuilder->codeAppendf(";");

        emit_custom_xfermode_code(mode, fsBuilder, outputColor, inputColor, dstColor); 
    }

    void setData(const GrGLProgramDataManager&, const GrProcessor&) SK_OVERRIDE {}

    static void GenKey(const GrFragmentProcessor& proc, const GrGLCaps&, GrProcessorKeyBuilder* b) {
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

GrCustomXferFP::GrCustomXferFP(SkXfermode::Mode mode, GrTexture* background)
    : fMode(mode) {
    this->initClassID<GrCustomXferFP>();

    SkASSERT(background);
    fBackgroundTransform.reset(kLocal_GrCoordSet, background, 
                               GrTextureParams::kNone_FilterMode);
    this->addCoordTransform(&fBackgroundTransform);
    fBackgroundAccess.reset(background);
    this->addTextureAccess(&fBackgroundAccess);
}

void GrCustomXferFP::getGLProcessorKey(const GrGLCaps& caps, GrProcessorKeyBuilder* b) const {
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
GrFragmentProcessor* GrCustomXferFP::TestCreate(SkRandom* rand,
                                                GrContext*,
                                                const GrDrawTargetCaps&,
                                                GrTexture* textures[]) {
    int mode = rand->nextRangeU(SkXfermode::kLastCoeffMode + 1, SkXfermode::kLastSeparableMode);

    return SkNEW_ARGS(GrCustomXferFP, (static_cast<SkXfermode::Mode>(mode), textures[0]));
}

///////////////////////////////////////////////////////////////////////////////
// Xfer Processor
///////////////////////////////////////////////////////////////////////////////

class CustomXP : public GrXferProcessor {
public:
    static GrXferProcessor* Create(SkXfermode::Mode mode, const GrDeviceCoordTexture* dstCopy,
                                   bool willReadDstColor) {
        if (!GrCustomXfermode::IsSupportedMode(mode)) {
            return NULL;
        } else {
            return SkNEW_ARGS(CustomXP, (mode, dstCopy, willReadDstColor));
        }
    }

    ~CustomXP() SK_OVERRIDE {};

    const char* name() const SK_OVERRIDE { return "Custom Xfermode"; }

    GrGLXferProcessor* createGLInstance() const SK_OVERRIDE;

    bool hasSecondaryOutput() const SK_OVERRIDE { return false; }

    GrXferProcessor::OptFlags getOptimizations(const GrProcOptInfo& colorPOI,
                                               const GrProcOptInfo& coveragePOI,
                                               bool doesStencilWrite,
                                               GrColor* overrideColor,
                                               const GrDrawTargetCaps& caps) SK_OVERRIDE;

    void getBlendInfo(GrXferProcessor::BlendInfo* blendInfo) const SK_OVERRIDE {
        blendInfo->fSrcBlend = kOne_GrBlendCoeff;
        blendInfo->fDstBlend = kZero_GrBlendCoeff;
        blendInfo->fBlendConstant = 0;
    }

    SkXfermode::Mode mode() const { return fMode; }

private:
    CustomXP(SkXfermode::Mode mode, const GrDeviceCoordTexture* dstCopy, bool willReadDstColor);

    void onGetGLProcessorKey(const GrGLCaps& caps, GrProcessorKeyBuilder* b) const SK_OVERRIDE;

    bool onIsEqual(const GrXferProcessor& xpBase) const SK_OVERRIDE;

    SkXfermode::Mode fMode;

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
    ~GLCustomXP() SK_OVERRIDE {}

    static void GenKey(const GrXferProcessor& proc, const GrGLCaps&, GrProcessorKeyBuilder* b) {
        uint32_t key = proc.numTextures();
        SkASSERT(key <= 1);
        key |= proc.cast<CustomXP>().mode() << 1;
        b->add32(key);
    }

private:
    void onEmitCode(const EmitArgs& args) SK_OVERRIDE {
        SkXfermode::Mode mode = args.fXP.cast<CustomXP>().mode();
        GrGLFPFragmentBuilder* fsBuilder = args.fPB->getFragmentShaderBuilder();
        const char* dstColor = fsBuilder->dstColor();

        emit_custom_xfermode_code(mode, fsBuilder, args.fOutputPrimary, args.fInputColor, dstColor);

        fsBuilder->codeAppendf("%s = %s * %s + (vec4(1.0) - %s) * %s;",
                               args.fOutputPrimary, args.fOutputPrimary, args.fInputCoverage,
                               args.fInputCoverage, dstColor);
    }

    void onSetData(const GrGLProgramDataManager&, const GrXferProcessor&) SK_OVERRIDE {}

    typedef GrGLFragmentProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

CustomXP::CustomXP(SkXfermode::Mode mode, const GrDeviceCoordTexture* dstCopy,
                   bool willReadDstColor)
    : INHERITED(dstCopy, willReadDstColor), fMode(mode) {
    this->initClassID<CustomXP>();
}

void CustomXP::onGetGLProcessorKey(const GrGLCaps& caps, GrProcessorKeyBuilder* b) const {
    GLCustomXP::GenKey(*this, caps, b);
}

GrGLXferProcessor* CustomXP::createGLInstance() const {
    return SkNEW_ARGS(GLCustomXP, (*this));
}

bool CustomXP::onIsEqual(const GrXferProcessor& other) const {
    const CustomXP& s = other.cast<CustomXP>();
    return fMode == s.fMode;
}

GrXferProcessor::OptFlags CustomXP::getOptimizations(const GrProcOptInfo& colorPOI,
                                                       const GrProcOptInfo& coveragePOI,
                                                       bool doesStencilWrite,
                                                       GrColor* overrideColor,
                                                       const GrDrawTargetCaps& caps) {
   return GrXferProcessor::kNone_Opt;
}

///////////////////////////////////////////////////////////////////////////////

GrCustomXPFactory::GrCustomXPFactory(SkXfermode::Mode mode)
    : fMode(mode) {
    this->initClassID<GrCustomXPFactory>();
}

GrXferProcessor*
GrCustomXPFactory::onCreateXferProcessor(const GrDrawTargetCaps& caps,
                                         const GrProcOptInfo& colorPOI,
                                         const GrProcOptInfo& coveragePOI,
                                         const GrDeviceCoordTexture* dstCopy) const {
    return CustomXP::Create(fMode, dstCopy, this->willReadDstColor(caps, colorPOI, coveragePOI));
}


void GrCustomXPFactory::getInvariantOutput(const GrProcOptInfo& colorPOI,
                                               const GrProcOptInfo& coveragePOI,
                                               GrXPFactory::InvariantOutput* output) const {
    output->fWillBlendWithDst = true;
    output->fBlendedColorFlags = 0;
}

GR_DEFINE_XP_FACTORY_TEST(GrCustomXPFactory);
GrXPFactory* GrCustomXPFactory::TestCreate(SkRandom* rand,
                                           GrContext*,
                                           const GrDrawTargetCaps&,
                                           GrTexture*[]) {
    int mode = rand->nextRangeU(SkXfermode::kLastCoeffMode + 1, SkXfermode::kLastSeparableMode);

    return SkNEW_ARGS(GrCustomXPFactory, (static_cast<SkXfermode::Mode>(mode)));
}

