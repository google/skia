/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLShaderBuilder.h"
#include "GrGLProgramBuilder.h"
#include "GrGLShaderStringBuilder.h"
#include "../GrGLGpu.h"
#include "../GrGLShaderVar.h"
#include "glsl/GrGLSLCaps.h"

namespace {
void append_texture_lookup(SkString* out,
                           GrGLGpu* gpu,
                           const char* samplerName,
                           const char* coordName,
                           uint32_t configComponentMask,
                           const char* swizzle,
                           GrSLType varyingType = kVec2f_GrSLType) {
    SkASSERT(coordName);

    out->appendf("%s(%s, %s)",
                 GrGLSLTexture2DFunctionName(varyingType, gpu->glslGeneration()),
                 samplerName,
                 coordName);

    char mangledSwizzle[5];

    // The swizzling occurs using texture params instead of shader-mangling if ARB_texture_swizzle
    // is available.
    if (!gpu->glCaps().textureSwizzleSupport() &&
        (kA_GrColorComponentFlag == configComponentMask)) {
        char alphaChar = gpu->glCaps().textureRedSupport() ? 'r' : 'a';
        int i;
        for (i = 0; '\0' != swizzle[i]; ++i) {
            mangledSwizzle[i] = alphaChar;
        }
        mangledSwizzle[i] ='\0';
        swizzle = mangledSwizzle;
    }
    // For shader prettiness we omit the swizzle rather than appending ".rgba".
    if (memcmp(swizzle, "rgba", 4)) {
        out->appendf(".%s", swizzle);
    }
}
}

GrGLShaderBuilder::GrGLShaderBuilder(GrGLProgramBuilder* program)
    : fProgramBuilder(program)
    , fInputs(GrGLProgramBuilder::kVarsPerBlock)
    , fOutputs(GrGLProgramBuilder::kVarsPerBlock)
    , fFeaturesAddedMask(0)
    , fCodeIndex(kCode)
    , fFinalized(false) {
    // We push back some dummy pointers which will later become our header
    for (int i = 0; i <= kCode; i++) {
        fShaderStrings.push_back();
        fCompilerStrings.push_back(NULL);
        fCompilerStringLengths.push_back(0);
    }

    this->main() = "void main() {";
}

void GrGLShaderBuilder::declAppend(const GrGLShaderVar& var) {
    SkString tempDecl;
    var.appendDecl(fProgramBuilder->ctxInfo(), &tempDecl);
    this->codeAppendf("%s;", tempDecl.c_str());
}

void GrGLShaderBuilder::emitFunction(GrSLType returnType,
                                     const char* name,
                                     int argCnt,
                                     const GrGLShaderVar* args,
                                     const char* body,
                                     SkString* outName) {
    this->functions().append(GrGLSLTypeString(returnType));
    fProgramBuilder->nameVariable(outName, '\0', name);
    this->functions().appendf(" %s", outName->c_str());
    this->functions().append("(");
    const GrGLContextInfo& ctxInfo = fProgramBuilder->gpu()->ctxInfo();
    for (int i = 0; i < argCnt; ++i) {
        args[i].appendDecl(ctxInfo, &this->functions());
        if (i < argCnt - 1) {
            this->functions().append(", ");
        }
    }
    this->functions().append(") {\n");
    this->functions().append(body);
    this->functions().append("}\n\n");
}

void GrGLShaderBuilder::appendTextureLookup(SkString* out,
                                            const TextureSampler& sampler,
                                            const char* coordName,
                                            GrSLType varyingType) const {
    append_texture_lookup(out,
                          fProgramBuilder->gpu(),
                          fProgramBuilder->getUniformCStr(sampler.fSamplerUniform),
                          coordName,
                          sampler.configComponentMask(),
                          sampler.swizzle(),
                          varyingType);
}

void GrGLShaderBuilder::appendTextureLookup(const TextureSampler& sampler,
                                            const char* coordName,
                                            GrSLType varyingType) {
    this->appendTextureLookup(&this->code(), sampler, coordName, varyingType);
}

void GrGLShaderBuilder::appendTextureLookupAndModulate(const char* modulation,
                                                       const TextureSampler& sampler,
                                                       const char* coordName,
                                                       GrSLType varyingType) {
    SkString lookup;
    this->appendTextureLookup(&lookup, sampler, coordName, varyingType);
    this->codeAppend((GrGLSLExpr4(modulation) * GrGLSLExpr4(lookup)).c_str());
}


const GrGLenum* GrGLShaderBuilder::GetTexParamSwizzle(GrPixelConfig config, const GrGLCaps& caps) {
    if (caps.textureSwizzleSupport() && GrPixelConfigIsAlphaOnly(config)) {
        if (caps.textureRedSupport()) {
            static const GrGLenum gRedSmear[] = { GR_GL_RED, GR_GL_RED, GR_GL_RED, GR_GL_RED };
            return gRedSmear;
        } else {
            static const GrGLenum gAlphaSmear[] = { GR_GL_ALPHA, GR_GL_ALPHA,
                                                    GR_GL_ALPHA, GR_GL_ALPHA };
            return gAlphaSmear;
        }
    } else {
        static const GrGLenum gStraight[] = { GR_GL_RED, GR_GL_GREEN, GR_GL_BLUE, GR_GL_ALPHA };
        return gStraight;
    }
}

void GrGLShaderBuilder::addFeature(uint32_t featureBit, const char* extensionName) {
    if (!(featureBit & fFeaturesAddedMask)) {
        this->extensions().appendf("#extension %s: require\n", extensionName);
        fFeaturesAddedMask |= featureBit;
    }
}

void GrGLShaderBuilder::appendDecls(const VarArray& vars, SkString* out) const {
    for (int i = 0; i < vars.count(); ++i) {
        vars[i].appendDecl(fProgramBuilder->ctxInfo(), out);
        out->append(";\n");
    }
}

void GrGLShaderBuilder::appendTextureLookup(const char* samplerName,
                                            const char* coordName,
                                            uint32_t configComponentMask,
                                            const char* swizzle) {
    append_texture_lookup(&this->code(),
                          fProgramBuilder->gpu(),
                          samplerName,
                          coordName,
                          configComponentMask,
                          swizzle,
                          kVec2f_GrSLType);
}

void GrGLShaderBuilder::addLayoutQualifier(const char* param, InterfaceQualifier interface) {
    SkASSERT(fProgramBuilder->gpu()->glslGeneration() >= k330_GrGLSLGeneration ||
             fProgramBuilder->gpu()->glCaps().glslCaps()->mustEnableAdvBlendEqs());
    fLayoutParams[interface].push_back() = param;
}

void GrGLShaderBuilder::compileAndAppendLayoutQualifiers() {
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

    GR_STATIC_ASSERT(0 == GrGLShaderBuilder::kOut_InterfaceQualifier);
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(interfaceQualifierNames) == kLastInterfaceQualifier + 1);
}

bool
GrGLShaderBuilder::finalize(GrGLuint programId, GrGLenum type, SkTDArray<GrGLuint>* shaderIds) {
    SkASSERT(!fFinalized);
    // append the 'footer' to code
    this->code().append("}");

    for (int i = 0; i <= fCodeIndex; i++) {
        fCompilerStrings[i] = fShaderStrings[i].c_str();
        fCompilerStringLengths[i] = (int)fShaderStrings[i].size();
    }

    GrGLGpu* gpu = fProgramBuilder->gpu();
    GrGLuint shaderId = GrGLCompileAndAttachShader(gpu->glContext(),
                                                   programId,
                                                   type,
                                                   fCompilerStrings.begin(),
                                                   fCompilerStringLengths.begin(),
                                                   fCompilerStrings.count(),
                                                   gpu->stats());

    fFinalized = true;

    if (!shaderId) {
        return false;
    }

    *shaderIds->append() = shaderId;

    return true;
}
