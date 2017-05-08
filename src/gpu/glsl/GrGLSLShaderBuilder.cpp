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
    this->functions().append(GrGLSLTypeString(returnType));
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
    // Our color is (r, g, b, a), but we want to multiply (r, g, b, 1) by our matrix, then
    // re-insert the original alpha. The supplied srcColor is likely to be of the form
    // "texture(...)", and we don't want to evaluate that twice, so wrap everything in a function.
    static const GrShaderVar gColorGamutXformArgs[] = {
        GrShaderVar("color", kVec4f_GrSLType),
        GrShaderVar("xform", kMat44f_GrSLType),
    };
    SkString functionBody;
    // Gamut xform, clamp to destination gamut. We only support/have premultiplied textures, so we
    // always just clamp to alpha.
    functionBody.append("\tcolor.rgb = clamp((xform * vec4(color.rgb, 1.0)).rgb, 0.0, color.a);\n");
    functionBody.append("\treturn color;");
    SkString colorGamutXformFuncName;
    this->emitFunction(kVec4f_GrSLType,
                       "colorGamutXform",
                       SK_ARRAY_COUNT(gColorGamutXformArgs),
                       gColorGamutXformArgs,
                       functionBody.c_str(),
                       &colorGamutXformFuncName);

    GrGLSLUniformHandler* uniformHandler = fProgramBuilder->uniformHandler();
    out->appendf("%s(%s, %s)", colorGamutXformFuncName.c_str(), srcColor,
                 uniformHandler->getUniformCStr(colorXformHelper->gamutXformUniform()));
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

void GrGLSLShaderBuilder::appendImageStorageLoad(SkString* out, ImageStorageHandle handle,
                                                 const char* coordExpr) {
    const GrShaderVar& imageStorage = fProgramBuilder->imageStorageVariable(handle);
    out->appendf("imageLoad(%s, %s)", imageStorage.c_str(), coordExpr);
}

void GrGLSLShaderBuilder::appendImageStorageLoad(ImageStorageHandle handle, const char* coordExpr) {
    this->appendImageStorageLoad(&this->code(), handle, coordExpr);
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
