/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/glsl/GrGLSLBlend.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"

//////////////////////////////////////////////////////////////////////////////
//  Advanced (non-coeff) blend helpers
//////////////////////////////////////////////////////////////////////////////

static void hard_light(GrGLSLFragmentBuilder* fsBuilder,
                       const char* final,
                       const char* src,
                       const char* dst) {
    static const char kComponents[] = { 'r', 'g', 'b' };
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
static void color_dodge_component(GrGLSLFragmentBuilder* fsBuilder,
                                  const char* final,
                                  const char* src,
                                  const char* dst,
                                  const char component) {
    const char* divisorGuard = "";
    const GrShaderCaps* shaderCaps = fsBuilder->getProgramBuilder()->shaderCaps();
    if (shaderCaps->mustGuardDivisionEvenAfterExplicitZeroCheck()) {
        divisorGuard = "+ 0.00000001";
    }

    fsBuilder->codeAppendf("if (0.0 == %s.%c) {", dst, component);
    fsBuilder->codeAppendf("%s.%c = %s.%c * (1.0 - %s.a);",
                           final, component, src, component, dst);
    fsBuilder->codeAppend("} else {");
    fsBuilder->codeAppendf("half d = %s.a - %s.%c;", src, src, component);
    fsBuilder->codeAppend("if (0.0 == d) {");
    fsBuilder->codeAppendf("%s.%c = %s.a * %s.a + %s.%c * (1.0 - %s.a) + %s.%c * (1.0 - %s.a);",
                           final, component, src, dst, src, component, dst, dst, component,
                           src);
    fsBuilder->codeAppend("} else {");
    fsBuilder->codeAppendf("d = min(%s.a, %s.%c * %s.a / (d %s));",
                           dst, dst, component, src, divisorGuard);
    fsBuilder->codeAppendf("%s.%c = d * %s.a + %s.%c * (1.0 - %s.a) + %s.%c * (1.0 - %s.a);",
                           final, component, src, src, component, dst, dst, component, src);
    fsBuilder->codeAppend("}");
    fsBuilder->codeAppend("}");
}

// Does one component of color-burn
static void color_burn_component(GrGLSLFragmentBuilder* fsBuilder,
                                 const char* final,
                                 const char* src,
                                 const char* dst,
                                 const char component) {
    const char* divisorGuard = "";
    const GrShaderCaps* shaderCaps = fsBuilder->getProgramBuilder()->shaderCaps();
    if (shaderCaps->mustGuardDivisionEvenAfterExplicitZeroCheck()) {
        divisorGuard = "+ 0.00000001";
    }

    fsBuilder->codeAppendf("if (%s.a == %s.%c) {", dst, dst, component);
    fsBuilder->codeAppendf("%s.%c = %s.a * %s.a + %s.%c * (1.0 - %s.a) + %s.%c * (1.0 - %s.a);",
                           final, component, src, dst, src, component, dst, dst, component,
                           src);
    fsBuilder->codeAppendf("} else if (0.0 == %s.%c) {", src, component);
    fsBuilder->codeAppendf("%s.%c = %s.%c * (1.0 - %s.a);",
                           final, component, dst, component, src);
    fsBuilder->codeAppend("} else {");
    fsBuilder->codeAppendf("half d = max(0.0, %s.a - (%s.a - %s.%c) * %s.a / (%s.%c %s));",
                           dst, dst, dst, component, src, src, component, divisorGuard);
    fsBuilder->codeAppendf("%s.%c = %s.a * d + %s.%c * (1.0 - %s.a) + %s.%c * (1.0 - %s.a);",
                           final, component, src, src, component, dst, dst, component, src);
    fsBuilder->codeAppend("}");
}

// Does one component of soft-light. Caller should have already checked that dst alpha > 0.
static void soft_light_component_pos_dst_alpha(GrGLSLFragmentBuilder* fsBuilder,
                                               const char* final,
                                               const char* src,
                                               const char* dst,
                                               const char component) {
    const char* divisorGuard = "";
    const GrShaderCaps* shaderCaps = fsBuilder->getProgramBuilder()->shaderCaps();
    if (shaderCaps->mustGuardDivisionEvenAfterExplicitZeroCheck()) {
        divisorGuard = "+ 0.00000001";
    }

    // if (2S < Sa)
    fsBuilder->codeAppendf("if (2.0 * %s.%c <= %s.a) {", src, component, src);
    // (D^2 (Sa-2 S))/Da+(1-Da) S+D (-Sa+2 S+1)
    fsBuilder->codeAppendf("%s.%c = (%s.%c*%s.%c*(%s.a - 2.0*%s.%c)) / (%s.a %s) +"
                           "(1.0 - %s.a) * %s.%c + %s.%c*(-%s.a + 2.0*%s.%c + 1.0);",
                           final, component, dst, component, dst, component, src, src,
                           component, dst, divisorGuard, dst, src, component, dst, component, src,
                           src, component);
    // else if (4D < Da)
    fsBuilder->codeAppendf("} else if (4.0 * %s.%c <= %s.a) {",
                           dst, component, dst);
    fsBuilder->codeAppendf("half DSqd = %s.%c * %s.%c;",
                           dst, component, dst, component);
    fsBuilder->codeAppendf("half DCub = DSqd * %s.%c;", dst, component);
    fsBuilder->codeAppendf("half DaSqd = %s.a * %s.a;", dst, dst);
    fsBuilder->codeAppendf("half DaCub = DaSqd * %s.a;", dst);
    // (Da^3 (-S)+Da^2 (S-D (3 Sa-6 S-1))+12 Da D^2 (Sa-2 S)-16 D^3 (Sa-2 S))/Da^2
    fsBuilder->codeAppendf("%s.%c ="
                           "(DaSqd*(%s.%c - %s.%c * (3.0*%s.a - 6.0*%s.%c - 1.0)) +"
                           " 12.0*%s.a*DSqd*(%s.a - 2.0*%s.%c) - 16.0*DCub * (%s.a - 2.0*%s.%c) -"
                           " DaCub*%s.%c) / (DaSqd %s);",
                           final, component, src, component, dst, component,
                           src, src, component, dst, src, src, component, src, src,
                           component, src, component, divisorGuard);
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
//      float3 set_luminance(float3 hueSatColor, float alpha, float3 lumColor).
static void add_lum_function(GrGLSLFragmentBuilder* fsBuilder, SkString* setLumFunction) {
    // Emit a helper that gets the luminance of a color.
    SkString getFunction;
    GrShaderVar getLumArgs[] = {
        GrShaderVar("color", kHalf3_GrSLType),
    };
    SkString getLumBody("return dot(half3(0.3, 0.59, 0.11), color);");
    fsBuilder->emitFunction(kHalf_GrSLType,
                            "luminance",
                            SK_ARRAY_COUNT(getLumArgs), getLumArgs,
                            getLumBody.c_str(),
                            &getFunction);

    // Emit the set luminance function.
    GrShaderVar setLumArgs[] = {
        GrShaderVar("hueSat", kHalf3_GrSLType),
        GrShaderVar("alpha", kHalf_GrSLType),
        GrShaderVar("lumColor", kHalf3_GrSLType),
    };
    SkString setLumBody;
    setLumBody.printf("half outLum = %s(lumColor);", getFunction.c_str());
    setLumBody.appendf("half3 outColor = outLum - %s(hueSat) + hueSat;", getFunction.c_str());
    setLumBody.append("half minComp = min(min(outColor.r, outColor.g), outColor.b);"
                      "half maxComp = max(max(outColor.r, outColor.g), outColor.b);"
                      "if (minComp < 0.0 && outLum != minComp) {"
                      "outColor = outLum + ((outColor - half3(outLum, outLum, outLum)) * outLum) /"
                      "(outLum - minComp);"
                      "}"
                      "if (maxComp > alpha && maxComp != outLum) {"
                      "outColor = outLum +"
                      "((outColor - half3(outLum, outLum, outLum)) * (alpha - outLum)) /"
                      "(maxComp - outLum);"
                      "}"
                      "return outColor;");
    fsBuilder->emitFunction(kHalf3_GrSLType,
                            "set_luminance",
                            SK_ARRAY_COUNT(setLumArgs), setLumArgs,
                            setLumBody.c_str(),
                            setLumFunction);
}

// Adds a function that creates a color with the hue and luminosity of one input color and
// the saturation of another color. It will have this signature:
//      float set_saturation(float3 hueLumColor, float3 satColor)
static void add_sat_function(GrGLSLFragmentBuilder* fsBuilder, SkString* setSatFunction) {
    // Emit a helper that gets the saturation of a color
    SkString getFunction;
    GrShaderVar getSatArgs[] = { GrShaderVar("color", kHalf3_GrSLType) };
    SkString getSatBody;
    getSatBody.printf("return max(max(color.r, color.g), color.b) - "
                      "min(min(color.r, color.g), color.b);");
    fsBuilder->emitFunction(kHalf_GrSLType,
                            "saturation",
                            SK_ARRAY_COUNT(getSatArgs), getSatArgs,
                            getSatBody.c_str(),
                            &getFunction);

    // Emit a helper that sets the saturation given sorted input channels. This used
    // to use inout params for min, mid, and max components but that seems to cause
    // problems on PowerVR drivers. So instead it returns a float3 where r, g ,b are the
    // adjusted min, mid, and max inputs, respectively.
    SkString helperFunction;
    GrShaderVar helperArgs[] = {
        GrShaderVar("minComp", kHalf_GrSLType),
        GrShaderVar("midComp", kHalf_GrSLType),
        GrShaderVar("maxComp", kHalf_GrSLType),
        GrShaderVar("sat", kHalf_GrSLType),
    };
    static const char kHelperBody[] = "if (minComp < maxComp) {"
        "half3 result;"
        "result.r = 0.0;"
        "result.g = sat * (midComp - minComp) / (maxComp - minComp);"
        "result.b = sat;"
        "return result;"
        "} else {"
        "return half3(0, 0, 0);"
        "}";
    fsBuilder->emitFunction(kHalf3_GrSLType,
                            "set_saturation_helper",
                            SK_ARRAY_COUNT(helperArgs), helperArgs,
                            kHelperBody,
                            &helperFunction);

    GrShaderVar setSatArgs[] = {
        GrShaderVar("hueLumColor", kHalf3_GrSLType),
        GrShaderVar("satColor", kHalf3_GrSLType),
    };
    const char* helpFunc = helperFunction.c_str();
    SkString setSatBody;
    setSatBody.appendf("half sat = %s(satColor);"
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
    fsBuilder->emitFunction(kHalf3_GrSLType,
                            "set_saturation",
                            SK_ARRAY_COUNT(setSatArgs), setSatArgs,
                            setSatBody.c_str(),
                            setSatFunction);
}

static void emit_advanced_xfermode_code(GrGLSLFragmentBuilder* fsBuilder, const char* srcColor,
                                        const char* dstColor, const char* outputColor,
                                        SkBlendMode mode) {
    SkASSERT(srcColor);
    SkASSERT(dstColor);
    SkASSERT(outputColor);
    // These all perform src-over on the alpha channel.
    fsBuilder->codeAppendf("%s.a = %s.a + (1.0 - %s.a) * %s.a;",
                           outputColor, srcColor, srcColor, dstColor);

    switch (mode) {
        case SkBlendMode::kOverlay:
            // Overlay is Hard-Light with the src and dst reversed
            hard_light(fsBuilder, outputColor, dstColor, srcColor);
            break;
        case SkBlendMode::kDarken:
            fsBuilder->codeAppendf("%s.rgb = min((1.0 - %s.a) * %s.rgb + %s.rgb, "
                                   "(1.0 - %s.a) * %s.rgb + %s.rgb);",
                                   outputColor,
                                   srcColor, dstColor, srcColor,
                                   dstColor, srcColor, dstColor);
            break;
        case SkBlendMode::kLighten:
            fsBuilder->codeAppendf("%s.rgb = max((1.0 - %s.a) * %s.rgb + %s.rgb, "
                                   "(1.0 - %s.a) * %s.rgb + %s.rgb);",
                                   outputColor,
                                   srcColor, dstColor, srcColor,
                                   dstColor, srcColor, dstColor);
            break;
        case SkBlendMode::kColorDodge:
            color_dodge_component(fsBuilder, outputColor, srcColor, dstColor, 'r');
            color_dodge_component(fsBuilder, outputColor, srcColor, dstColor, 'g');
            color_dodge_component(fsBuilder, outputColor, srcColor, dstColor, 'b');
            break;
        case SkBlendMode::kColorBurn:
            color_burn_component(fsBuilder, outputColor, srcColor, dstColor, 'r');
            color_burn_component(fsBuilder, outputColor, srcColor, dstColor, 'g');
            color_burn_component(fsBuilder, outputColor, srcColor, dstColor, 'b');
            break;
        case SkBlendMode::kHardLight:
            hard_light(fsBuilder, outputColor, srcColor, dstColor);
            break;
        case SkBlendMode::kSoftLight:
            fsBuilder->codeAppendf("if (0.0 == %s.a) {", dstColor);
            fsBuilder->codeAppendf("%s.rgba = %s;", outputColor, srcColor);
            fsBuilder->codeAppendf("} else {");
            soft_light_component_pos_dst_alpha(fsBuilder, outputColor, srcColor, dstColor, 'r');
            soft_light_component_pos_dst_alpha(fsBuilder, outputColor, srcColor, dstColor, 'g');
            soft_light_component_pos_dst_alpha(fsBuilder, outputColor, srcColor, dstColor, 'b');
            fsBuilder->codeAppendf("}");
            break;
        case SkBlendMode::kDifference:
            fsBuilder->codeAppendf("%s.rgb = %s.rgb + %s.rgb -"
                                   "2.0 * min(%s.rgb * %s.a, %s.rgb * %s.a);",
                                   outputColor, srcColor, dstColor, srcColor, dstColor,
                                   dstColor, srcColor);
            break;
        case SkBlendMode::kExclusion:
            fsBuilder->codeAppendf("%s.rgb = %s.rgb + %s.rgb - "
                                   "2.0 * %s.rgb * %s.rgb;",
                                   outputColor, dstColor, srcColor, dstColor, srcColor);
            break;
        case SkBlendMode::kMultiply:
            fsBuilder->codeAppendf("%s.rgb = (1.0 - %s.a) * %s.rgb + "
                                   "(1.0 - %s.a) * %s.rgb + "
                                   "%s.rgb * %s.rgb;",
                                   outputColor, srcColor, dstColor, dstColor, srcColor,
                                   srcColor, dstColor);
            break;
        case SkBlendMode::kHue: {
            //  SetLum(SetSat(S * Da, Sat(D * Sa)), Sa*Da, D*Sa) + (1 - Sa) * D + (1 - Da) * S
            SkString setSat, setLum;
            add_sat_function(fsBuilder, &setSat);
            add_lum_function(fsBuilder, &setLum);
            fsBuilder->codeAppendf("half4 dstSrcAlpha = %s * %s.a;",
                                   dstColor, srcColor);
            fsBuilder->codeAppendf("%s.rgb = %s(%s(%s.rgb * %s.a, dstSrcAlpha.rgb),"
                                   "dstSrcAlpha.a, dstSrcAlpha.rgb);",
                                   outputColor, setLum.c_str(), setSat.c_str(), srcColor,
                                   dstColor);
            fsBuilder->codeAppendf("%s.rgb += (1.0 - %s.a) * %s.rgb + (1.0 - %s.a) * %s.rgb;",
                                   outputColor, srcColor, dstColor, dstColor, srcColor);
            break;
        }
        case SkBlendMode::kSaturation: {
            // SetLum(SetSat(D * Sa, Sat(S * Da)), Sa*Da, D*Sa)) + (1 - Sa) * D + (1 - Da) * S
            SkString setSat, setLum;
            add_sat_function(fsBuilder, &setSat);
            add_lum_function(fsBuilder, &setLum);
            fsBuilder->codeAppendf("half4 dstSrcAlpha = %s * %s.a;",
                                   dstColor, srcColor);
            fsBuilder->codeAppendf("%s.rgb = %s(%s(dstSrcAlpha.rgb, %s.rgb * %s.a),"
                                   "dstSrcAlpha.a, dstSrcAlpha.rgb);",
                                   outputColor, setLum.c_str(), setSat.c_str(), srcColor,
                                   dstColor);
            fsBuilder->codeAppendf("%s.rgb += (1.0 - %s.a) * %s.rgb + (1.0 - %s.a) * %s.rgb;",
                                   outputColor, srcColor, dstColor, dstColor, srcColor);
            break;
        }
        case SkBlendMode::kColor: {
            //  SetLum(S * Da, Sa* Da, D * Sa) + (1 - Sa) * D + (1 - Da) * S
            SkString setLum;
            add_lum_function(fsBuilder, &setLum);
            fsBuilder->codeAppendf("half4 srcDstAlpha = %s * %s.a;",
                                   srcColor, dstColor);
            fsBuilder->codeAppendf("%s.rgb = %s(srcDstAlpha.rgb, srcDstAlpha.a, %s.rgb * %s.a);",
                                   outputColor, setLum.c_str(), dstColor, srcColor);
            fsBuilder->codeAppendf("%s.rgb += (1.0 - %s.a) * %s.rgb + (1.0 - %s.a) * %s.rgb;",
                                   outputColor, srcColor, dstColor, dstColor, srcColor);
            break;
        }
        case SkBlendMode::kLuminosity: {
            //  SetLum(D * Sa, Sa* Da, S * Da) + (1 - Sa) * D + (1 - Da) * S
            SkString setLum;
            add_lum_function(fsBuilder, &setLum);
            fsBuilder->codeAppendf("half4 srcDstAlpha = %s * %s.a;",
                                   srcColor, dstColor);
            fsBuilder->codeAppendf("%s.rgb = %s(%s.rgb * %s.a, srcDstAlpha.a, srcDstAlpha.rgb);",
                                   outputColor, setLum.c_str(), dstColor, srcColor);
            fsBuilder->codeAppendf("%s.rgb += (1.0 - %s.a) * %s.rgb + (1.0 - %s.a) * %s.rgb;",
                                   outputColor, srcColor, dstColor, dstColor, srcColor);
            break;
        }
        default:
            SK_ABORT("Unknown Custom Xfer mode.");
            break;
    }
}

//////////////////////////////////////////////////////////////////////////////
//  Porter-Duff blend helper
//////////////////////////////////////////////////////////////////////////////

static bool append_porterduff_term(GrGLSLFragmentBuilder* fsBuilder, SkBlendModeCoeff coeff,
                                   const char* colorName, const char* srcColorName,
                                   const char* dstColorName, bool hasPrevious) {
    if (SkBlendModeCoeff::kZero == coeff) {
        return hasPrevious;
    } else {
        if (hasPrevious) {
            fsBuilder->codeAppend(" + ");
        }
        fsBuilder->codeAppendf("%s", colorName);
        switch (coeff) {
            case SkBlendModeCoeff::kOne:
                break;
            case SkBlendModeCoeff::kSC:
                fsBuilder->codeAppendf(" * %s", srcColorName);
                break;
            case SkBlendModeCoeff::kISC:
                fsBuilder->codeAppendf(" * (half4(1.0) - %s)", srcColorName);
                break;
            case SkBlendModeCoeff::kDC:
                fsBuilder->codeAppendf(" * %s", dstColorName);
                break;
            case SkBlendModeCoeff::kIDC:
                fsBuilder->codeAppendf(" * (half4(1.0) - %s)", dstColorName);
                break;
            case SkBlendModeCoeff::kSA:
                fsBuilder->codeAppendf(" * %s.a", srcColorName);
                break;
            case SkBlendModeCoeff::kISA:
                fsBuilder->codeAppendf(" * (1.0 - %s.a)", srcColorName);
                break;
            case SkBlendModeCoeff::kDA:
                fsBuilder->codeAppendf(" * %s.a", dstColorName);
                break;
            case SkBlendModeCoeff::kIDA:
                fsBuilder->codeAppendf(" * (1.0 - %s.a)", dstColorName);
                break;
            default:
                SK_ABORT("Unsupported Blend Coeff");
        }
        return true;
    }
}

//////////////////////////////////////////////////////////////////////////////

void GrGLSLBlend::AppendMode(GrGLSLFragmentBuilder* fsBuilder, const char* srcColor,
                             const char* dstColor, const char* outColor,
                             SkBlendMode mode) {

    SkBlendModeCoeff srcCoeff, dstCoeff;
    if (SkBlendMode_AsCoeff(mode, &srcCoeff, &dstCoeff)) {
        // The only coeff mode that can go out of range is plus.
        bool clamp = mode == SkBlendMode::kPlus;

        fsBuilder->codeAppendf("%s = ", outColor);
        if (clamp) {
            fsBuilder->codeAppend("clamp(");
        }
        // append src blend
        bool didAppend = append_porterduff_term(fsBuilder, srcCoeff, srcColor, srcColor, dstColor,
                                                false);
        // append dst blend
        if(!append_porterduff_term(fsBuilder, dstCoeff, dstColor, srcColor, dstColor, didAppend)) {
            fsBuilder->codeAppend("half4(0, 0, 0, 0)");
        }
        if (clamp) {
            fsBuilder->codeAppend(", 0, 1);");
        }
        fsBuilder->codeAppend(";");
    } else {
        emit_advanced_xfermode_code(fsBuilder, srcColor, dstColor, outColor, mode);
    }
}

void GrGLSLBlend::AppendRegionOp(GrGLSLFragmentBuilder* fsBuilder, const char* srcColor,
                                 const char* dstColor, const char* outColor,
                                 SkRegion::Op regionOp) {
    SkBlendModeCoeff srcCoeff, dstCoeff;
    switch (regionOp) {
        case SkRegion::kReplace_Op:
            srcCoeff = SkBlendModeCoeff::kOne;
            dstCoeff = SkBlendModeCoeff::kZero;
            break;
        case SkRegion::kIntersect_Op:
            srcCoeff = SkBlendModeCoeff::kDC;
            dstCoeff = SkBlendModeCoeff::kZero;
            break;
        case SkRegion::kUnion_Op:
            srcCoeff = SkBlendModeCoeff::kOne;
            dstCoeff = SkBlendModeCoeff::kISC;
            break;
        case SkRegion::kXOR_Op:
            srcCoeff = SkBlendModeCoeff::kIDC;
            dstCoeff = SkBlendModeCoeff::kISC;
            break;
        case SkRegion::kDifference_Op:
            srcCoeff = SkBlendModeCoeff::kZero;
            dstCoeff = SkBlendModeCoeff::kISC;
            break;
        case SkRegion::kReverseDifference_Op:
            srcCoeff = SkBlendModeCoeff::kIDC;
            dstCoeff = SkBlendModeCoeff::kZero;
            break;
        default:
            SK_ABORT("Unsupported Op");
            // We should never get here but to make compiler happy
            srcCoeff = SkBlendModeCoeff::kZero;
            dstCoeff = SkBlendModeCoeff::kZero;
    }
    fsBuilder->codeAppendf("%s = ", outColor);
    // append src blend
    bool didAppend = append_porterduff_term(fsBuilder, srcCoeff, srcColor, srcColor, dstColor,
                                            false);
    // append dst blend
    if(!append_porterduff_term(fsBuilder, dstCoeff, dstColor, srcColor, dstColor, didAppend)) {
        fsBuilder->codeAppend("half4(0, 0, 0, 0)");
    }
    fsBuilder->codeAppend(";");
}
