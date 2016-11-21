/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/GrGLUniformHandler.h"

#include "gl/GrGLCaps.h"
#include "gl/GrGLGpu.h"
#include "gl/builders/GrGLProgramBuilder.h"

#define GL_CALL(X) GR_GL_CALL(this->glGpu()->glInterface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(this->glGpu()->glInterface(), R, X)

GrGLSLUniformHandler::UniformHandle GrGLUniformHandler::internalAddUniformArray(
                                                                            uint32_t visibility,
                                                                            GrSLType type,
                                                                            GrSLPrecision precision,
                                                                            const char* name,
                                                                            bool mangleName,
                                                                            int arrayCount,
                                                                            const char** outName) {
    SkASSERT(name && strlen(name));
    SkDEBUGCODE(static const uint32_t kVisMask = kVertex_GrShaderFlag | kFragment_GrShaderFlag);
    SkASSERT(0 == (~kVisMask & visibility));
    SkASSERT(0 != visibility);
    SkASSERT(kDefault_GrSLPrecision == precision || GrSLTypeAcceptsPrecision(type));

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
    if ('u' == name[0]) {
        prefix = '\0';
    }
    fProgramBuilder->nameVariable(uni.fVariable.accessName(), prefix, name, mangleName);
    uni.fVariable.setArrayCount(arrayCount);
    uni.fVisibility = visibility;
    uni.fVariable.setPrecision(precision);
    uni.fLocation = -1;

    if (outName) {
        *outName = uni.fVariable.c_str();
    }
    return GrGLSLUniformHandler::UniformHandle(fUniforms.count() - 1);
}

GrGLSLUniformHandler::SamplerHandle GrGLUniformHandler::addSampler(uint32_t visibility,
                                                                   GrSwizzle swizzle,
                                                                   GrSLType type,
                                                                   GrSLPrecision precision,
                                                                   const char* name) {
    SkASSERT(name && strlen(name));
    SkDEBUGCODE(static const uint32_t kVisMask = kVertex_GrShaderFlag | kFragment_GrShaderFlag);
    SkASSERT(0 == (~kVisMask & visibility));
    SkASSERT(0 != visibility);

    SkString mangleName;
    char prefix = 'u';
    fProgramBuilder->nameVariable(&mangleName, prefix, name, true);

    UniformInfo& sampler = fSamplers.push_back();
    SkASSERT(GrSLTypeIsCombinedSamplerType(type));
    sampler.fVariable.setType(type);
    sampler.fVariable.setTypeModifier(GrShaderVar::kUniform_TypeModifier);
    sampler.fVariable.setPrecision(precision);
    sampler.fVariable.setName(mangleName);
    sampler.fLocation = -1;
    sampler.fVisibility = visibility;
    fSamplerSwizzles.push_back(swizzle);
    SkASSERT(fSamplers.count() == fSamplerSwizzles.count());
    return GrGLSLUniformHandler::SamplerHandle(fSamplers.count() - 1);
}

GrGLSLUniformHandler::StorageImageHandle GrGLUniformHandler::addStorageImage(uint32_t visibility,
                                                                             GrPixelConfig config,
                                                                             const char* name) {
    SkASSERT(name && strlen(name));
    SkDEBUGCODE(static const uint32_t kVisMask = kVertex_GrShaderFlag | kFragment_GrShaderFlag);
    SkASSERT(0 == (~kVisMask & visibility));
    SkASSERT(0 != visibility);
    SkString mangleName;
    char prefix = 'u';
    fProgramBuilder->nameVariable(&mangleName, prefix, name, true);

    UniformInfo& storageImage = fStorageImages.push_back();
    storageImage.fVariable.setName(mangleName);
    if (GrPixelConfigIsSint(config)) {
        storageImage.fVariable.setType(GrSLType::kIImage2D_GrSLType);
    } else {
        storageImage.fVariable.setType(GrSLType::kImage2D_GrSLType);
    }
    storageImage.fVariable.setTypeModifier(GrShaderVar::kUniform_TypeModifier);
    const char* layout;
    switch (config) {
        case kUnknown_GrPixelConfig:
            layout = "";
            break;
        case kRGBA_8888_GrPixelConfig:
            layout = "rgba8";
            break;
        case kRGBA_float_GrPixelConfig:
            layout = "rgba32";
            break;
        case kRGBA_half_GrPixelConfig:
            layout = "rgba16f";
            break;
        case kRGBA_8888_sint_GrPixelConfig:
            layout = "rgba8i";
            break;
        default:
            SkFAIL("Unexpected config for 2D image.");
            layout = "--invalid--";
            break;
    }
    storageImage.fVariable.setLayoutQualifier(layout);
    storageImage.fLocation = -1;
    storageImage.fVisibility = visibility;
    return GrGLSLUniformHandler::StorageImageHandle(fStorageImages.count() - 1);
}

void GrGLUniformHandler::appendUniformDecls(GrShaderFlags visibility, SkString* out) const {
    for (int i = 0; i < fUniforms.count(); ++i) {
        if (fUniforms[i].fVisibility & visibility) {
            fUniforms[i].fVariable.appendDecl(fProgramBuilder->glslCaps(), out);
            out->append(";");
        }
    }
    for (int i = 0; i < fSamplers.count(); ++i) {
        if (fSamplers[i].fVisibility & visibility) {
            fSamplers[i].fVariable.appendDecl(fProgramBuilder->glslCaps(), out);
            out->append(";\n");
        }
    }
    for (int i = 0; i < fStorageImages.count(); ++i) {
        if (fStorageImages[i].fVisibility & visibility) {
            fStorageImages[i].fVariable.appendDecl(fProgramBuilder->glslCaps(), out);
            out->append(";");
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
        for (int i = 0; i < fStorageImages.count(); ++i) {
            GL_CALL(BindUniformLocation(programID, currUniform,
                                        fStorageImages[i].fVariable.c_str()));
            fStorageImages[i].fLocation = currUniform;
        }
    }
}

void GrGLUniformHandler::getUniformLocations(GrGLuint programID, const GrGLCaps& caps) {
    if (!caps.bindUniformLocationSupport()) {
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
        for (int i = 0; i < fStorageImages.count(); ++i) {
            GrGLint location;
            GL_CALL_RET(location, GetUniformLocation(programID,
                                                     fStorageImages[i].fVariable.c_str()));
            fStorageImages[i].fLocation = location;
        }
    }
}

const GrGLGpu* GrGLUniformHandler::glGpu() const {
    GrGLProgramBuilder* glPB = (GrGLProgramBuilder*) fProgramBuilder;
    return glPB->gpu();
}
