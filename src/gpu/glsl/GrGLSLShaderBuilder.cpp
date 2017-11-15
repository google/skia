/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrShaderVar.h"
#include "GrShaderCaps.h"
#include "GrSwizzle.h"
#include "glsl/GrGLSLShaderBuilder.h"
#include "glsl/GrGLSLColorSpaceXformHelper.h"
#include "glsl/GrGLSLProgramBuilder.h"

GrGLSLShaderBuilder::GrGLSLShaderBuilder(GrGLSLProgramBuilder* program)
    : fProgramBuilder(program)
    , fInputs(GrGLSLProgramBuilder::kVarsPerBlock)
    , fOutputs(GrGLSLProgramBuilder::kVarsPerBlock)
    , fFeaturesAddedMask(0)
    , fCodeIndex(kCode)
    , fFinalized(false) {
    // We push back some dummy pointers which will later become our header
    for (int i = 0; i <= kCode; i++) {
        fShaderStrings.push_back();
        fCompilerStrings.push_back(nullptr);
        fCompilerStringLengths.push_back(0);
    }

    this->main() = "void main() {";
}

void GrGLSLShaderBuilder::declAppend(const GrShaderVar& var) {
    SkString tempDecl;
    var.appendDecl(fProgramBuilder->shaderCaps(), &tempDecl);
    this->codeAppendf("%s;", tempDecl.c_str());
}

void GrGLSLShaderBuilder::declareGlobal(const GrShaderVar& v) {
    v.appendDecl(this->getProgramBuilder()->shaderCaps(), &this->definitions());
    this->definitions().append(";");
}

void GrGLSLShaderBuilder::emitFunction(GrSLType returnType,
                                       const char* name,
                                       int argCnt,
                                       const GrShaderVar* args,
                                       const char* body,
                                       SkString* outName) {
    this->functions().append(GrGLSLTypeString(fProgramBuilder->shaderCaps(), returnType));
    fProgramBuilder->nameVariable(outName, '\0', name);
    this->functions().appendf(" %s", outName->c_str());
    this->functions().append("(");
    for (int i = 0; i < argCnt; ++i) {
        args[i].appendDecl(fProgramBuilder->shaderCaps(), &this->functions());
        if (i < argCnt - 1) {
            this->functions().append(", ");
        }
    }
    this->functions().append(") {\n");
    this->functions().append(body);
    this->functions().append("}\n\n");
}

static inline void append_texture_swizzle(SkString* out, GrSwizzle swizzle) {
    if (swizzle != GrSwizzle::RGBA()) {
        out->appendf(".%s", swizzle.c_str());
    }
}

void GrGLSLShaderBuilder::appendTextureLookup(SkString* out,
                                              SamplerHandle samplerHandle,
                                              const char* coordName,
                                              GrSLType varyingType) const {
    const GrShaderVar& sampler = fProgramBuilder->samplerVariable(samplerHandle);
    out->appendf("texture(%s, %s)", sampler.c_str(), coordName);
    append_texture_swizzle(out, fProgramBuilder->samplerSwizzle(samplerHandle));
}

void GrGLSLShaderBuilder::appendTextureLookup(SamplerHandle samplerHandle,
                                              const char* coordName,
                                              GrSLType varyingType,
                                              GrGLSLColorSpaceXformHelper* colorXformHelper) {
    if (colorXformHelper && colorXformHelper->isValid()) {
        // With a color gamut transform, we need to wrap the lookup in another function call
        SkString lookup;
        this->appendTextureLookup(&lookup, samplerHandle, coordName, varyingType);
        this->appendColorGamutXform(lookup.c_str(), colorXformHelper);
    } else {
        this->appendTextureLookup(&this->code(), samplerHandle, coordName, varyingType);
    }
}

void GrGLSLShaderBuilder::appendTextureLookupAndModulate(
                                                    const char* modulation,
                                                    SamplerHandle samplerHandle,
                                                    const char* coordName,
                                                    GrSLType varyingType,
                                                    GrGLSLColorSpaceXformHelper* colorXformHelper) {
    SkString lookup;
    this->appendTextureLookup(&lookup, samplerHandle, coordName, varyingType);
    if (colorXformHelper && colorXformHelper->isValid()) {
        SkString xform;
        this->appendColorGamutXform(&xform, lookup.c_str(), colorXformHelper);
        if (modulation) {
            this->codeAppendf("%s * %s", modulation, xform.c_str());
        } else {
            this->codeAppendf("%s", xform.c_str());
        }
    } else {
        if (modulation) {
            this->codeAppendf("%s * %s", modulation, lookup.c_str());
        } else {
            this->codeAppendf("%s", lookup.c_str());
        }
    }
}

void GrGLSLShaderBuilder::appendColorGamutXform(SkString* out,
                                                const char* srcColor,
                                                GrGLSLColorSpaceXformHelper* colorXformHelper) {
    GrGLSLUniformHandler* uniformHandler = fProgramBuilder->uniformHandler();

    // We define up to three helper functions, to keep things clearer. One does inverse sRGB,
    // one does an arbitrary transfer function, and the last does gamut xform. Any combination of
    // these may be present, although some configurations are much more likely.

    SkString inverseSrgbFuncName;
    if (colorXformHelper->applyInverseSRGB()) {
        static const GrShaderVar gInverseSRGBArgs[] = { GrShaderVar("x", kHalf_GrSLType) };
        SkString body;
        body.append("return (x <= 0.0031308) ? (x * 12.92) : (1.055 * pow(x, 0.4166667) - 0.055);");
        this->emitFunction(kHalf_GrSLType, "inverse_srgb", SK_ARRAY_COUNT(gInverseSRGBArgs),
                           gInverseSRGBArgs, body.c_str(), &inverseSrgbFuncName);

    }

    SkString transferFnFuncName;
    if (colorXformHelper->applyTransferFn()) {
        static const GrShaderVar gTransferFnArgs[] = { GrShaderVar("x", kHalf_GrSLType) };
        const char* coeffs = uniformHandler->getUniformCStr(colorXformHelper->transferFnUniform());
        SkString body;
        // Temporaries to make evaluation line readable
        body.appendf("half G = %s[0];", coeffs);
        body.appendf("half A = %s[1];", coeffs);
        body.appendf("half B = %s[2];", coeffs);
        body.appendf("half C = %s[3];", coeffs);
        body.appendf("half D = %s[4];", coeffs);
        body.appendf("half E = %s[5];", coeffs);
        body.appendf("half F = %s[6];", coeffs);
        body.append("half s = sign(x);");
        body.append("x = abs(x);");
        body.appendf("return s * ((x < D) ? (C * x) + F : pow(A * x + B, G) + E);");
        this->emitFunction(kHalf_GrSLType, "transfer_fn", SK_ARRAY_COUNT(gTransferFnArgs),
                           gTransferFnArgs, body.c_str(), &transferFnFuncName);
    }

    SkString gamutXformFuncName;
    if (colorXformHelper->applyGamutXform()) {
        // Our color is (r, g, b, a), but we want to multiply (r, g, b, 1) by our matrix, then
        // re-insert the original alpha.
        static const GrShaderVar gGamutXformArgs[] = { GrShaderVar("color", kHalf4_GrSLType) };
        const char* xform = uniformHandler->getUniformCStr(colorXformHelper->gamutXformUniform());
        SkString body;
        body.appendf("color.rgb = clamp((%s * half4(color.rgb, 1.0)).rgb, 0.0, color.a);", xform);
        body.append("return color;");
        this->emitFunction(kHalf4_GrSLType, "gamut_xform", SK_ARRAY_COUNT(gGamutXformArgs),
                           gGamutXformArgs, body.c_str(), &gamutXformFuncName);
    }

    // Now define a wrapper function that applies all the intermediate steps
    {
        static const GrShaderVar gColorXformArgs[] = { GrShaderVar("color", kHalf4_GrSLType) };
        SkString body;
        if (colorXformHelper->applyInverseSRGB()) {
            body.appendf("color.r = %s(color.r);", inverseSrgbFuncName.c_str());
            body.appendf("color.g = %s(color.g);", inverseSrgbFuncName.c_str());
            body.appendf("color.b = %s(color.b);", inverseSrgbFuncName.c_str());
        }
        if (colorXformHelper->applyTransferFn()) {
            body.appendf("color.r = %s(color.r);", transferFnFuncName.c_str());
            body.appendf("color.g = %s(color.g);", transferFnFuncName.c_str());
            body.appendf("color.b = %s(color.b);", transferFnFuncName.c_str());
        }
        if (colorXformHelper->applyGamutXform()) {
            body.appendf("color = %s(color);", gamutXformFuncName.c_str());
        }
        body.append("return color;");
        SkString colorXformFuncName;
        this->emitFunction(kHalf4_GrSLType, "color_xform", SK_ARRAY_COUNT(gColorXformArgs),
                           gColorXformArgs, body.c_str(), &colorXformFuncName);
        out->appendf("%s(%s)", colorXformFuncName.c_str(), srcColor);
    }
}

void GrGLSLShaderBuilder::appendColorGamutXform(const char* srcColor,
                                                GrGLSLColorSpaceXformHelper* colorXformHelper) {
    SkString xform;
    this->appendColorGamutXform(&xform, srcColor, colorXformHelper);
    this->codeAppend(xform.c_str());
}

void GrGLSLShaderBuilder::appendTexelFetch(SkString* out,
                                           TexelBufferHandle texelBufferHandle,
                                           const char* coordExpr) const {
    const GrShaderVar& texelBuffer = fProgramBuilder->texelBufferVariable(texelBufferHandle);
    SkASSERT(fProgramBuilder->shaderCaps()->texelFetchSupport());

    out->appendf("texelFetch(%s, %s)", texelBuffer.c_str(), coordExpr);
}

void GrGLSLShaderBuilder::appendTexelFetch(TexelBufferHandle texelBufferHandle,
                                           const char* coordExpr) {
    this->appendTexelFetch(&this->code(), texelBufferHandle, coordExpr);
}

bool GrGLSLShaderBuilder::addFeature(uint32_t featureBit, const char* extensionName) {
    if (featureBit & fFeaturesAddedMask) {
        return false;
    }
    this->extensions().appendf("#extension %s: require\n", extensionName);
    fFeaturesAddedMask |= featureBit;
    return true;
}

void GrGLSLShaderBuilder::appendDecls(const VarArray& vars, SkString* out) const {
    for (int i = 0; i < vars.count(); ++i) {
        vars[i].appendDecl(fProgramBuilder->shaderCaps(), out);
        out->append(";\n");
    }
}

void GrGLSLShaderBuilder::addLayoutQualifier(const char* param, InterfaceQualifier interface) {
    SkASSERT(fProgramBuilder->shaderCaps()->generation() >= k330_GrGLSLGeneration ||
             fProgramBuilder->shaderCaps()->mustEnableAdvBlendEqs());
    fLayoutParams[interface].push_back() = param;
}

void GrGLSLShaderBuilder::compileAndAppendLayoutQualifiers() {
    static const char* interfaceQualifierNames[] = {
        "in",
        "out"
    };

    for (int interface = 0; interface <= kLastInterfaceQualifier; ++interface) {
        const SkTArray<SkString>& params = fLayoutParams[interface];
        if (params.empty()) {
            continue;
        }
        this->layoutQualifiers().appendf("layout(%s", params[0].c_str());
        for (int i = 1; i < params.count(); ++i) {
            this->layoutQualifiers().appendf(", %s", params[i].c_str());
        }
        this->layoutQualifiers().appendf(") %s;\n", interfaceQualifierNames[interface]);
    }

    GR_STATIC_ASSERT(0 == GrGLSLShaderBuilder::kIn_InterfaceQualifier);
    GR_STATIC_ASSERT(1 == GrGLSLShaderBuilder::kOut_InterfaceQualifier);
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(interfaceQualifierNames) == kLastInterfaceQualifier + 1);
}

void GrGLSLShaderBuilder::finalize(uint32_t visibility) {
    SkASSERT(!fFinalized);
    this->versionDecl() = fProgramBuilder->shaderCaps()->versionDeclString();
    this->compileAndAppendLayoutQualifiers();
    SkASSERT(visibility);
    fProgramBuilder->appendUniformDecls((GrShaderFlags) visibility, &this->uniforms());
    this->appendDecls(fInputs, &this->inputs());
    this->appendDecls(fOutputs, &this->outputs());
    this->onFinalize();
    // append the 'footer' to code
    this->code().append("}");

    for (int i = 0; i <= fCodeIndex; i++) {
        fCompilerStrings[i] = fShaderStrings[i].c_str();
        fCompilerStringLengths[i] = (int)fShaderStrings[i].size();
    }

    fFinalized = true;
}
