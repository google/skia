/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/gl/GrGLUniformHandler.h"

#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/gl/GrGLCaps.h"
#include "src/gpu/gl/GrGLGpu.h"
#include "src/gpu/gl/builders/GrGLProgramBuilder.h"
#include "src/sksl/SkSLCompiler.h"

#define GL_CALL(X) GR_GL_CALL(this->glGpu()->glInterface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(this->glGpu()->glInterface(), R, X)

bool valid_name(const char* name) {
    // disallow unknown names that start with "sk_"
    if (!strncmp(name, GR_NO_MANGLE_PREFIX, strlen(GR_NO_MANGLE_PREFIX))) {
        return !strcmp(name, SkSL::Compiler::RTADJUST_NAME);
    }
    return true;
}

GrGLSLUniformHandler::UniformHandle GrGLUniformHandler::internalAddUniformArray(
                                                                            uint32_t visibility,
                                                                            GrSLType type,
                                                                            const char* name,
                                                                            bool mangleName,
                                                                            int arrayCount,
                                                                            const char** outName) {
    SkASSERT(name && strlen(name));
    SkASSERT(valid_name(name));
    SkASSERT(0 != visibility);

    UniformInfo& uni = fUniforms.push_back();
    uni.fVariable.setType(type);
    uni.fVariable.setTypeModifier(GrShaderVar::kUniform_TypeModifier);
    // TODO this is a bit hacky, lets think of a better way.  Basically we need to be able to use
    // the uniform view matrix name in the GP, and the GP is immutable so it has to tell the PB
    // exactly what name it wants to use for the uniform view matrix.  If we prefix anythings, then
    // the names will mismatch.  I think the correct solution is to have all GPs which need the
    // uniform view matrix, they should upload the view matrix in their setData along with regular
    // uniforms.
    char prefix = 'u';
    if ('u' == name[0] || !strncmp(name, GR_NO_MANGLE_PREFIX, strlen(GR_NO_MANGLE_PREFIX))) {
        prefix = '\0';
    }
    fProgramBuilder->nameVariable(uni.fVariable.accessName(), prefix, name, mangleName);
    uni.fVariable.setArrayCount(arrayCount);
    uni.fVisibility = visibility;
    uni.fLocation = -1;

    if (outName) {
        *outName = uni.fVariable.c_str();
    }
    return GrGLSLUniformHandler::UniformHandle(fUniforms.count() - 1);
}

GrGLSLUniformHandler::SamplerHandle GrGLUniformHandler::addSampler(const GrTextureProxy* texture,
                                                                   const GrSamplerState&,
                                                                   const GrSwizzle& swizzle,
                                                                   const char* name,
                                                                   const GrShaderCaps* shaderCaps) {
    SkASSERT(name && strlen(name));

    SkString mangleName;
    char prefix = 'u';
    fProgramBuilder->nameVariable(&mangleName, prefix, name, true);

    GrTextureType type = texture->textureType();

    UniformInfo& sampler = fSamplers.push_back();
    sampler.fVariable.setType(GrSLCombinedSamplerTypeForTextureType(type));
    sampler.fVariable.setTypeModifier(GrShaderVar::kUniform_TypeModifier);
    sampler.fVariable.setName(mangleName);
    sampler.fLocation = -1;
    sampler.fVisibility = kFragment_GrShaderFlag;
    if (shaderCaps->textureSwizzleAppliedInShader()) {
        fSamplerSwizzles.push_back(swizzle);
        SkASSERT(fSamplers.count() == fSamplerSwizzles.count());
    }
    return GrGLSLUniformHandler::SamplerHandle(fSamplers.count() - 1);
}

void GrGLUniformHandler::appendUniformDecls(GrShaderFlags visibility, SkString* out) const {
    for (int i = 0; i < fUniforms.count(); ++i) {
        if (fUniforms[i].fVisibility & visibility) {
            fUniforms[i].fVariable.appendDecl(fProgramBuilder->shaderCaps(), out);
            out->append(";");
        }
    }
    for (int i = 0; i < fSamplers.count(); ++i) {
        if (fSamplers[i].fVisibility & visibility) {
            fSamplers[i].fVariable.appendDecl(fProgramBuilder->shaderCaps(), out);
            out->append(";\n");
        }
    }
}

void GrGLUniformHandler::bindUniformLocations(GrGLuint programID, const GrGLCaps& caps) {
    if (caps.bindUniformLocationSupport()) {
        int currUniform = 0;
        for (int i = 0; i < fUniforms.count(); ++i, ++currUniform) {
            GL_CALL(BindUniformLocation(programID, currUniform, fUniforms[i].fVariable.c_str()));
            fUniforms[i].fLocation = currUniform;
        }
        for (int i = 0; i < fSamplers.count(); ++i, ++currUniform) {
            GL_CALL(BindUniformLocation(programID, currUniform, fSamplers[i].fVariable.c_str()));
            fSamplers[i].fLocation = currUniform;
        }
    }
}

void GrGLUniformHandler::getUniformLocations(GrGLuint programID, const GrGLCaps& caps, bool force) {
    if (!caps.bindUniformLocationSupport() || force) {
        int count = fUniforms.count();
        for (int i = 0; i < count; ++i) {
            GrGLint location;
            GL_CALL_RET(location, GetUniformLocation(programID, fUniforms[i].fVariable.c_str()));
            fUniforms[i].fLocation = location;
        }
        for (int i = 0; i < fSamplers.count(); ++i) {
            GrGLint location;
            GL_CALL_RET(location, GetUniformLocation(programID, fSamplers[i].fVariable.c_str()));
            fSamplers[i].fLocation = location;
        }
    }
}

const GrGLGpu* GrGLUniformHandler::glGpu() const {
    GrGLProgramBuilder* glPB = (GrGLProgramBuilder*) fProgramBuilder;
    return glPB->gpu();
}
