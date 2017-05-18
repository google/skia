/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLUniformHandler_DEFINED
#define GrGLUniformHandler_DEFINED

#include "glsl/GrGLSLUniformHandler.h"

#include "gl/GrGLProgramDataManager.h"

class GrGLCaps;

class GrGLUniformHandler : public GrGLSLUniformHandler {
public:
    static const int kUniformsPerBlock = 8;

    const GrShaderVar& getUniformVariable(UniformHandle u) const override {
        return fUniforms[u.toIndex()].fVariable;
    }

    const char* getUniformCStr(UniformHandle u) const override {
        return this->getUniformVariable(u).c_str();
    }
private:
    explicit GrGLUniformHandler(GrGLSLProgramBuilder* program)
        : INHERITED(program)
        , fUniforms(kUniformsPerBlock)
        , fSamplers(kUniformsPerBlock)
        , fTexelBuffers(kUniformsPerBlock)
        , fImageStorages(kUniformsPerBlock) {}

    UniformHandle internalAddUniformArray(uint32_t visibility,
                                          GrSLType type,
                                          GrSLPrecision precision,
                                          const char* name,
                                          bool mangleName,
                                          int arrayCount,
                                          const char** outName) override;

    SamplerHandle addSampler(uint32_t visibility, GrSwizzle, GrSLType, GrSLPrecision,
                             const char* name) override;

    const GrShaderVar& samplerVariable(SamplerHandle handle) const override {
        return fSamplers[handle.toIndex()].fVariable;
    }

    GrSwizzle samplerSwizzle(SamplerHandle handle) const override {
        return fSamplerSwizzles[handle.toIndex()];
    }

    TexelBufferHandle addTexelBuffer(uint32_t visibility, GrSLPrecision,
                                     const char* name) override;

    const GrShaderVar& texelBufferVariable(TexelBufferHandle handle) const override {
        return fTexelBuffers[handle.toIndex()].fVariable;
    }

    ImageStorageHandle addImageStorage(uint32_t visibility, GrSLType, GrImageStorageFormat,
                                       GrSLMemoryModel, GrSLRestrict, GrIOType,
                                       const char* name) override;

    const GrShaderVar& imageStorageVariable(ImageStorageHandle handle) const override {
        return fImageStorages[handle.toIndex()].fVariable;
    }

    void appendUniformDecls(GrShaderFlags visibility, SkString*) const override;

    // Manually set uniform locations for all our uniforms.
    void bindUniformLocations(GrGLuint programID, const GrGLCaps& caps);

    // Updates the loction of the Uniforms if we cannot bind uniform locations manually
    void getUniformLocations(GrGLuint programID, const GrGLCaps& caps);

    const GrGLGpu* glGpu() const;

    typedef GrGLProgramDataManager::UniformInfo UniformInfo;
    typedef GrGLProgramDataManager::UniformInfoArray UniformInfoArray;

    UniformInfoArray    fUniforms;
    UniformInfoArray    fSamplers;
    SkTArray<GrSwizzle> fSamplerSwizzles;
    UniformInfoArray    fTexelBuffers;
    UniformInfoArray    fImageStorages;

    friend class GrGLProgramBuilder;

    typedef GrGLSLUniformHandler INHERITED;
};

#endif
