/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLShaderBuilder.h"
#include "GrGLFullProgramBuilder.h"
#include "GrGLProgramBuilder.h"
#include "../GrGpuGL.h"
#include "../GrGLShaderVar.h"

namespace {
inline const char* sample_function_name(GrSLType type, GrGLSLGeneration glslGen) {
    if (kVec2f_GrSLType == type) {
        return glslGen >= k130_GrGLSLGeneration ? "texture" : "texture2D";
    } else {
        SkASSERT(kVec3f_GrSLType == type);
        return glslGen >= k130_GrGLSLGeneration ? "textureProj" : "texture2DProj";
    }
}
void append_texture_lookup(SkString* out,
                           GrGpuGL* gpu,
                           const char* samplerName,
                           const char* coordName,
                           uint32_t configComponentMask,
                           const char* swizzle,
                           GrSLType varyingType = kVec2f_GrSLType) {
    SkASSERT(coordName);

    out->appendf("%s(%s, %s)",
                 sample_function_name(varyingType, gpu->glslGeneration()),
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
static const int kVarsPerBlock = 8;
}

GrGLShaderBuilder::GrGLShaderBuilder(GrGLProgramBuilder* program)
    : fProgramBuilder(program)
    , fInputs(kVarsPerBlock)
    , fOutputs(kVarsPerBlock)
    , fFeaturesAddedMask(0) {
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
    fFunctions.append(GrGLSLTypeString(returnType));
    fProgramBuilder->nameVariable(outName, '\0', name);
    fFunctions.appendf(" %s", outName->c_str());
    fFunctions.append("(");
    const GrGLContextInfo& ctxInfo = fProgramBuilder->gpu()->ctxInfo();
    for (int i = 0; i < argCnt; ++i) {
        args[i].appendDecl(ctxInfo, &fFunctions);
        if (i < argCnt - 1) {
            fFunctions.append(", ");
        }
    }
    fFunctions.append(") {\n");
    fFunctions.append(body);
    fFunctions.append("}\n\n");
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
    this->appendTextureLookup(&fCode, sampler, coordName, varyingType);
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
        fExtensions.appendf("#extension %s: require\n", extensionName);
            fFeaturesAddedMask |= featureBit;
    }
}

void GrGLShaderBuilder::appendTextureLookup(const char* samplerName,
                                            const char* coordName,
                                            uint32_t configComponentMask,
                                            const char* swizzle) {
    append_texture_lookup(&fCode,
                          fProgramBuilder->gpu(),
                          samplerName,
                          coordName,
                          configComponentMask,
                          swizzle,
                          kVec2f_GrSLType);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
GrGLFullShaderBuilder::GrGLFullShaderBuilder(GrGLFullProgramBuilder* program)
    : INHERITED(program)
    , fFullProgramBuilder(program) {}
