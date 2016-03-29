/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrSwizzle.h"
#include "glsl/GrGLSLShaderBuilder.h"
#include "glsl/GrGLSLCaps.h"
#include "glsl/GrGLSLShaderVar.h"
#include "glsl/GrGLSLTextureSampler.h"
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

void GrGLSLShaderBuilder::declAppend(const GrGLSLShaderVar& var) {
    SkString tempDecl;
    var.appendDecl(fProgramBuilder->glslCaps(), &tempDecl);
    this->codeAppendf("%s;", tempDecl.c_str());
}

void GrGLSLShaderBuilder::emitFunction(GrSLType returnType,
                                       const char* name,
                                       int argCnt,
                                       const GrGLSLShaderVar* args,
                                       const char* body,
                                       SkString* outName) {
    this->functions().append(GrGLSLTypeString(returnType));
    fProgramBuilder->nameVariable(outName, '\0', name);
    this->functions().appendf(" %s", outName->c_str());
    this->functions().append("(");
    for (int i = 0; i < argCnt; ++i) {
        args[i].appendDecl(fProgramBuilder->glslCaps(), &this->functions());
        if (i < argCnt - 1) {
            this->functions().append(", ");
        }
    }
    this->functions().append(") {\n");
    this->functions().append(body);
    this->functions().append("}\n\n");
}

void GrGLSLShaderBuilder::appendTextureLookup(SkString* out,
                                              const GrGLSLTextureSampler& sampler,
                                              const char* coordName,
                                              GrSLType varyingType) const {
    const GrGLSLCaps* glslCaps = fProgramBuilder->glslCaps();
    GrGLSLUniformHandler* uniformHandler = fProgramBuilder->uniformHandler();
    GrSLType samplerType = uniformHandler->getUniformVariable(sampler.fSamplerUniform).getType();
    if (samplerType == kSampler2DRect_GrSLType) {
        if (varyingType == kVec2f_GrSLType) {
            out->appendf("%s(%s, textureSize(%s) * %s)",
                         GrGLSLTexture2DFunctionName(varyingType, samplerType,
                                                     glslCaps->generation()),
                         uniformHandler->getUniformCStr(sampler.fSamplerUniform),
                         uniformHandler->getUniformCStr(sampler.fSamplerUniform),
                         coordName);
        } else {
            out->appendf("%s(%s, vec3(textureSize(%s) * %s.xy, %s.z))",
                         GrGLSLTexture2DFunctionName(varyingType, samplerType,
                                                     glslCaps->generation()),
                         uniformHandler->getUniformCStr(sampler.fSamplerUniform),
                         uniformHandler->getUniformCStr(sampler.fSamplerUniform),
                         coordName,
                         coordName);
        }
    } else {
        out->appendf("%s(%s, %s)",
                     GrGLSLTexture2DFunctionName(varyingType, samplerType, glslCaps->generation()),
                     uniformHandler->getUniformCStr(sampler.fSamplerUniform),
                     coordName);
    }

    // This refers to any swizzling we may need to get from some backend internal format to the
    // format used in GrPixelConfig. If this is implemented by the GrGpu object, then swizzle will
    // be rgba. For shader prettiness we omit the swizzle rather than appending ".rgba".
    const GrSwizzle& configSwizzle = glslCaps->configTextureSwizzle(sampler.config());

    if (configSwizzle != GrSwizzle::RGBA()) {
        out->appendf(".%s", configSwizzle.c_str());
    }
}

void GrGLSLShaderBuilder::appendTextureLookup(const GrGLSLTextureSampler& sampler,
                                              const char* coordName,
                                              GrSLType varyingType) {
    this->appendTextureLookup(&this->code(), sampler, coordName, varyingType);
}

void GrGLSLShaderBuilder::appendTextureLookupAndModulate(const char* modulation,
                                                         const GrGLSLTextureSampler& sampler,
                                                         const char* coordName,
                                                         GrSLType varyingType) {
    SkString lookup;
    this->appendTextureLookup(&lookup, sampler, coordName, varyingType);
    this->codeAppend((GrGLSLExpr4(modulation) * GrGLSLExpr4(lookup)).c_str());
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
        vars[i].appendDecl(fProgramBuilder->glslCaps(), out);
        out->append(";\n");
    }
}

void GrGLSLShaderBuilder::addLayoutQualifier(const char* param, InterfaceQualifier interface) {
    SkASSERT(fProgramBuilder->glslCaps()->generation() >= k330_GrGLSLGeneration ||
             fProgramBuilder->glslCaps()->mustEnableAdvBlendEqs());
    fLayoutParams[interface].push_back() = param;
}

void GrGLSLShaderBuilder::compileAndAppendLayoutQualifiers() {
    static const char* interfaceQualifierNames[] = {
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

    GR_STATIC_ASSERT(0 == GrGLSLShaderBuilder::kOut_InterfaceQualifier);
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(interfaceQualifierNames) == kLastInterfaceQualifier + 1);
}

void GrGLSLShaderBuilder::finalize(uint32_t visibility) {
    SkASSERT(!fFinalized);
    this->versionDecl() = fProgramBuilder->glslCaps()->versionDeclString();
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
