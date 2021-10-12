/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/gl/GrGLUniformHandler.h"

#include "src/gpu/GrTexture.h"
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
                                                                   const GrFragmentProcessor* owner,
                                                                   uint32_t visibility,
                                                                   GrSLType type,
                                                                   const char* name,
                                                                   bool mangleName,
                                                                   int arrayCount,
                                                                   const char** outName) {
    SkASSERT(name && strlen(name));
    SkASSERT(valid_name(name));
    SkASSERT(0 != visibility);

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
    SkString resolvedName = fProgramBuilder->nameVariable(prefix, name, mangleName);

    GLUniformInfo tempInfo;
    tempInfo.fVariable = GrShaderVar{std::move(resolvedName),
                                     type,
                                     GrShaderVar::TypeModifier::Uniform,
                                     arrayCount};

    tempInfo.fVisibility = visibility;
    tempInfo.fOwner      = owner;
    tempInfo.fRawName    = SkString(name);
    tempInfo.fLocation   = -1;

    fUniforms.push_back(tempInfo);

    if (outName) {
        *outName = fUniforms.back().fVariable.c_str();
    }
    return GrGLSLUniformHandler::UniformHandle(fUniforms.count() - 1);
}

GrGLSLUniformHandler::SamplerHandle GrGLUniformHandler::addSampler(
        const GrBackendFormat& backendFormat, GrSamplerState, const GrSwizzle& swizzle,
        const char* name, const GrShaderCaps* shaderCaps) {
    SkASSERT(name && strlen(name));

    constexpr char prefix = 'u';
    SkString mangleName = fProgramBuilder->nameVariable(prefix, name, true);

    GrTextureType type = backendFormat.textureType();

    GLUniformInfo tempInfo;
    tempInfo.fVariable = GrShaderVar{std::move(mangleName),
                                     GrSLCombinedSamplerTypeForTextureType(type),
                                     GrShaderVar::TypeModifier::Uniform};

    tempInfo.fVisibility = kFragment_GrShaderFlag;
    tempInfo.fOwner      = nullptr;
    tempInfo.fRawName    = SkString(name);
    tempInfo.fLocation   = -1;

    fSamplers.push_back(tempInfo);
    fSamplerSwizzles.push_back(swizzle);
    SkASSERT(fSamplers.count() == fSamplerSwizzles.count());

    return GrGLSLUniformHandler::SamplerHandle(fSamplers.count() - 1);
}

void GrGLUniformHandler::appendUniformDecls(GrShaderFlags visibility, SkString* out) const {
    for (const UniformInfo& uniform : fUniforms.items()) {
        if (uniform.fVisibility & visibility) {
            uniform.fVariable.appendDecl(fProgramBuilder->shaderCaps(), out);
            out->append(";");
        }
    }
    for (const UniformInfo& sampler : fSamplers.items()) {
        if (sampler.fVisibility & visibility) {
            sampler.fVariable.appendDecl(fProgramBuilder->shaderCaps(), out);
            out->append(";\n");
        }
    }
}

void GrGLUniformHandler::bindUniformLocations(GrGLuint programID, const GrGLCaps& caps) {
    if (caps.bindUniformLocationSupport()) {
        int currUniform = 0;
        for (GLUniformInfo& uniform : fUniforms.items()) {
            GL_CALL(BindUniformLocation(programID, currUniform, uniform.fVariable.c_str()));
            uniform.fLocation = currUniform;
            ++currUniform;
        }
        for (GLUniformInfo& sampler : fSamplers.items()) {
            GL_CALL(BindUniformLocation(programID, currUniform, sampler.fVariable.c_str()));
            sampler.fLocation = currUniform;
            ++currUniform;
        }
    }
}

void GrGLUniformHandler::getUniformLocations(GrGLuint programID, const GrGLCaps& caps, bool force) {
    if (!caps.bindUniformLocationSupport() || force) {
        for (GLUniformInfo& uniform : fUniforms.items()) {
            GrGLint location;
            GL_CALL_RET(location, GetUniformLocation(programID, uniform.fVariable.c_str()));
            uniform.fLocation = location;
        }
        for (GLUniformInfo& sampler : fSamplers.items()) {
            GrGLint location;
            GL_CALL_RET(location, GetUniformLocation(programID, sampler.fVariable.c_str()));
            sampler.fLocation = location;
        }
    }
}

const GrGLGpu* GrGLUniformHandler::glGpu() const {
    GrGLProgramBuilder* glPB = (GrGLProgramBuilder*) fProgramBuilder;
    return glPB->gpu();
}
