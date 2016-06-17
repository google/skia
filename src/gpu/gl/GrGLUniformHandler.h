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
#include "gl/GrGLSampler.h"

class GrGLCaps;

class GrGLUniformHandler : public GrGLSLUniformHandler {
public:
    static const int kUniformsPerBlock = 8;

    const GrGLSLShaderVar& getUniformVariable(UniformHandle u) const override {
        return fUniforms[u.toIndex()].fVariable;
    }

    const char* getUniformCStr(UniformHandle u) const override {
        return this->getUniformVariable(u).c_str();
    }
private:
    explicit GrGLUniformHandler(GrGLSLProgramBuilder* program)
        : INHERITED(program)
        , fUniforms(kUniformsPerBlock) {}

    UniformHandle internalAddUniformArray(uint32_t visibility,
                                          GrSLType type,
                                          GrSLPrecision precision,
                                          const char* name,
                                          bool mangleName,
                                          int arrayCount,
                                          const char** outName) override;

    SamplerHandle internalAddSampler(uint32_t visibility,
                                     GrPixelConfig config,
                                     GrSLType type,
                                     GrSLPrecision precision,
                                     const char* name) override;

    int numSamplers() const override { return fSamplers.count(); }
    const GrGLSLSampler& getSampler(SamplerHandle handle) const override {
        return fSamplers[handle.toIndex()];
    }

    void appendUniformDecls(GrShaderFlags visibility, SkString*) const override;

    // Manually set uniform locations for all our uniforms.
    void bindUniformLocations(GrGLuint programID, const GrGLCaps& caps);

    // Updates the loction of the Uniforms if we cannot bind uniform locations manually
    void getUniformLocations(GrGLuint programID, const GrGLCaps& caps);

    const GrGLGpu* glGpu() const;

    typedef GrGLProgramDataManager::UniformInfo UniformInfo;
    typedef GrGLProgramDataManager::UniformInfoArray UniformInfoArray;

    UniformInfoArray fUniforms;

    SkTArray<GrGLSampler> fSamplers;

    friend class GrGLProgramBuilder;

    typedef GrGLSLUniformHandler INHERITED;
};

#endif
