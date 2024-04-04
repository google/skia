/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/ganesh/glsl/GrGLSLShaderBuilder.h"

#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "modules/skcms/skcms.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/Blend.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/gpu/ganesh/GrShaderVar.h"
#include "src/gpu/ganesh/glsl/GrGLSLColorSpaceXformHelper.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramDataManager.h"
#include "src/sksl/SkSLGLSL.h"

using namespace skia_private;

GrGLSLShaderBuilder::GrGLSLShaderBuilder(GrGLSLProgramBuilder* program)
    : fProgramBuilder(program)
    , fInputs(GrGLSLProgramBuilder::kVarsPerBlock)
    , fOutputs(GrGLSLProgramBuilder::kVarsPerBlock)
    , fFeaturesAddedMask(0)
    , fCodeIndex(kCode)
    , fFinalized(false)
    , fTmpVariableCounter(0) {
    // We push back some placeholder pointers which will later become our header
    for (int i = 0; i <= kCode; i++) {
        fShaderStrings.push_back();
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

SkString GrGLSLShaderBuilder::getMangledFunctionName(const char* baseName) {
    return fProgramBuilder->nameVariable(/*prefix=*/'\0', baseName);
}

void GrGLSLShaderBuilder::appendFunctionDecl(SkSLType returnType,
                                             const char* mangledName,
                                             SkSpan<const GrShaderVar> args) {
    this->functions().appendf("%s %s(", SkSLTypeString(returnType), mangledName);
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) {
            this->functions().append(", ");
        }
        args[i].appendDecl(fProgramBuilder->shaderCaps(), &this->functions());
    }

    this->functions().append(")");
}

void GrGLSLShaderBuilder::emitFunction(SkSLType returnType,
                                       const char* mangledName,
                                       SkSpan<const GrShaderVar> args,
                                       const char* body) {
    this->appendFunctionDecl(returnType, mangledName, args);
    this->functions().appendf(" {\n"
                              "%s"
                              "}\n\n", body);
}

void GrGLSLShaderBuilder::emitFunction(const char* declaration, const char* body) {
    this->functions().appendf("%s {\n"
                              "%s"
                              "}\n\n", declaration, body);
}

void GrGLSLShaderBuilder::emitFunctionPrototype(SkSLType returnType,
                                                const char* mangledName,
                                                SkSpan<const GrShaderVar> args) {
    this->appendFunctionDecl(returnType, mangledName, args);
    this->functions().append(";\n");
}

void GrGLSLShaderBuilder::emitFunctionPrototype(const char* declaration) {
    this->functions().appendf("%s\n", declaration);
}

static inline void append_texture_swizzle(SkString* out, skgpu::Swizzle swizzle) {
    if (swizzle != skgpu::Swizzle::RGBA()) {
        out->appendf(".%s", swizzle.asString().c_str());
    }
}

void GrGLSLShaderBuilder::appendTextureLookup(SkString* out,
                                              SamplerHandle samplerHandle,
                                              const char* coordName) const {
    const char* sampler = fProgramBuilder->samplerVariable(samplerHandle);
    out->appendf("sample(%s, %s)", sampler, coordName);
    append_texture_swizzle(out, fProgramBuilder->samplerSwizzle(samplerHandle));
}

void GrGLSLShaderBuilder::appendTextureLookup(SamplerHandle samplerHandle,
                                              const char* coordName,
                                              GrGLSLColorSpaceXformHelper* colorXformHelper) {
    SkString lookup;
    this->appendTextureLookup(&lookup, samplerHandle, coordName);
    this->appendColorGamutXform(lookup.c_str(), colorXformHelper);
}

void GrGLSLShaderBuilder::appendTextureLookupAndBlend(
        const char* dst,
        SkBlendMode mode,
        SamplerHandle samplerHandle,
        const char* coordName,
        GrGLSLColorSpaceXformHelper* colorXformHelper) {
    if (!dst) {
        dst = "half4(1)";
    }
    SkString lookup;
    this->codeAppendf("%s(", skgpu::BlendFuncName(mode));
    this->appendTextureLookup(&lookup, samplerHandle, coordName);
    this->appendColorGamutXform(lookup.c_str(), colorXformHelper);
    this->codeAppendf(", %s)", dst);
}

void GrGLSLShaderBuilder::appendInputLoad(SamplerHandle samplerHandle) {
    const char* input = fProgramBuilder->inputSamplerVariable(samplerHandle);
    SkString load;
    load.appendf("subpassLoad(%s)", input);
    append_texture_swizzle(&load, fProgramBuilder->inputSamplerSwizzle(samplerHandle));
    this->codeAppend(load.c_str());
}

void GrGLSLShaderBuilder::appendColorGamutXform(SkString* out,
                                                const char* srcColor,
                                                GrGLSLColorSpaceXformHelper* colorXformHelper) {
    if (!colorXformHelper || colorXformHelper->isNoop()) {
        *out = srcColor;
        return;
    }

    GrGLSLUniformHandler* uniformHandler = fProgramBuilder->uniformHandler();

    // We define up to three helper functions, to keep things clearer. One for the source transfer
    // function, one for the (inverse) destination transfer function, and one for the gamut xform.
    // Any combination of these may be present, although some configurations are much more likely.

    auto emitTFFunc = [this, &uniformHandler](const char* name,
                                              GrGLSLProgramDataManager::UniformHandle uniform,
                                              skcms_TFType tfType) {
        const GrShaderVar gTFArgs[] = { GrShaderVar("x", SkSLType::kFloat) };
        const char* coeffs = uniformHandler->getUniformCStr(uniform);
        SkString body;
        // Temporaries to make evaluation line readable. We always use the sRGBish names, so the
        // PQ and HLG math is confusing.
        body.appendf("float G = %s[0];", coeffs);
        body.appendf("float A = %s[1];", coeffs);
        body.appendf("float B = %s[2];", coeffs);
        body.appendf("float C = %s[3];", coeffs);
        body.appendf("float D = %s[4];", coeffs);
        body.appendf("float E = %s[5];", coeffs);
        body.appendf("float F = %s[6];", coeffs);
        body.append("float s = sign(x);");
        body.append("x = abs(x);");
        switch (tfType) {
            case skcms_TFType_sRGBish:
                body.append("x = (x < D) ? (C * x) + F : pow(A * x + B, G) + E;");
                break;
            case skcms_TFType_PQish:
                body.append("x = pow(max(A + B * pow(x, C), 0) / (D + E * pow(x, C)), F);");
                break;
            case skcms_TFType_HLGish:
                body.append("x = (x*A <= 1) ? pow(x*A, B) : exp((x-E)*C) + D; x *= (F+1);");
                break;
            case skcms_TFType_HLGinvish:
                body.append("x /= (F+1); x = (x <= 1) ? A * pow(x, B) : C * log(x - D) + E;");
                break;
            default:
                SkASSERT(false);
                break;
        }
        body.append("return s * x;");
        SkString funcName = this->getMangledFunctionName(name);
        this->emitFunction(SkSLType::kFloat, funcName.c_str(), {gTFArgs, std::size(gTFArgs)},
                           body.c_str());
        return funcName;
    };

    SkString srcTFFuncName;
    if (colorXformHelper->applySrcTF()) {
        srcTFFuncName = emitTFFunc("src_tf", colorXformHelper->srcTFUniform(),
                                   colorXformHelper->srcTFType());
    }

    SkString dstTFFuncName;
    if (colorXformHelper->applyDstTF()) {
        dstTFFuncName = emitTFFunc("dst_tf", colorXformHelper->dstTFUniform(),
                                   colorXformHelper->dstTFType());
    }

    SkString gamutXformFuncName;
    if (colorXformHelper->applyGamutXform()) {
        const GrShaderVar gGamutXformArgs[] = { GrShaderVar("color", SkSLType::kFloat4) };
        const char* xform = uniformHandler->getUniformCStr(colorXformHelper->gamutXformUniform());
        SkString body;
        body.appendf("color.rgb = (%s * color.rgb);", xform);
        body.append("return color;");
        gamutXformFuncName = this->getMangledFunctionName("gamut_xform");
        this->emitFunction(SkSLType::kFloat4, gamutXformFuncName.c_str(),
                           {gGamutXformArgs, std::size(gGamutXformArgs)}, body.c_str());
    }

    // Now define a wrapper function that applies all the intermediate steps
    {
        const GrShaderVar gColorXformArgs[] = { GrShaderVar("color", SkSLType::kFloat4) };
        SkString body;
        if (colorXformHelper->applyUnpremul()) {
            body.append("color = unpremul(color);");
        }
        if (colorXformHelper->applySrcTF()) {
            body.appendf("color.r = %s(color.r);", srcTFFuncName.c_str());
            body.appendf("color.g = %s(color.g);", srcTFFuncName.c_str());
            body.appendf("color.b = %s(color.b);", srcTFFuncName.c_str());
        }
        if (colorXformHelper->applyGamutXform()) {
            body.appendf("color = %s(color);", gamutXformFuncName.c_str());
        }
        if (colorXformHelper->applyDstTF()) {
            body.appendf("color.r = %s(color.r);", dstTFFuncName.c_str());
            body.appendf("color.g = %s(color.g);", dstTFFuncName.c_str());
            body.appendf("color.b = %s(color.b);", dstTFFuncName.c_str());
        }
        if (colorXformHelper->applyPremul()) {
            body.append("color.rgb *= color.a;");
        }
        body.append("return half4(color);");
        SkString colorXformFuncName = this->getMangledFunctionName("color_xform");
        this->emitFunction(SkSLType::kHalf4, colorXformFuncName.c_str(),
                           {gColorXformArgs, std::size(gColorXformArgs)}, body.c_str());
        out->appendf("%s(%s)", colorXformFuncName.c_str(), srcColor);
    }
}

void GrGLSLShaderBuilder::appendColorGamutXform(const char* srcColor,
                                                GrGLSLColorSpaceXformHelper* colorXformHelper) {
    SkString xform;
    this->appendColorGamutXform(&xform, srcColor, colorXformHelper);
    this->codeAppend(xform.c_str());
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
    for (const auto& v : vars.items()) {
        v.appendDecl(fProgramBuilder->shaderCaps(), out);
        out->append(";\n");
    }
}

void GrGLSLShaderBuilder::addLayoutQualifier(const char* param, InterfaceQualifier interface) {
    SkASSERT(fProgramBuilder->shaderCaps()->fGLSLGeneration >= SkSL::GLSLGeneration::k330 ||
             fProgramBuilder->shaderCaps()->mustEnableAdvBlendEqs());
    fLayoutParams[interface].push_back() = param;
}

void GrGLSLShaderBuilder::compileAndAppendLayoutQualifiers() {
    static const char* interfaceQualifierNames[] = {
        "in",
        "out"
    };

    for (int interface = 0; interface <= kLastInterfaceQualifier; ++interface) {
        const TArray<SkString>& params = fLayoutParams[interface];
        if (params.empty()) {
            continue;
        }
        this->layoutQualifiers().appendf("layout(%s", params[0].c_str());
        for (int i = 1; i < params.size(); ++i) {
            this->layoutQualifiers().appendf(", %s", params[i].c_str());
        }
        this->layoutQualifiers().appendf(") %s;\n", interfaceQualifierNames[interface]);
    }

    static_assert(0 == GrGLSLShaderBuilder::kIn_InterfaceQualifier);
    static_assert(1 == GrGLSLShaderBuilder::kOut_InterfaceQualifier);
    static_assert(std::size(interfaceQualifierNames) == kLastInterfaceQualifier + 1);
}

void GrGLSLShaderBuilder::finalize(uint32_t visibility) {
    SkASSERT(!fFinalized);
    this->compileAndAppendLayoutQualifiers();
    SkASSERT(visibility);
    fProgramBuilder->appendUniformDecls((GrShaderFlags) visibility, &this->uniforms());
    this->appendDecls(fInputs, &this->inputs());
    this->appendDecls(fOutputs, &this->outputs());
    this->onFinalize();
    // append the 'footer' to code
    this->code().append("}");

    for (int i = 0; i <= fCodeIndex; i++) {
        fCompilerString.append(fShaderStrings[i].c_str(), fShaderStrings[i].size());
    }

    fFinalized = true;
}
