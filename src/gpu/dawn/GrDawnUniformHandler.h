/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnUniformHandler_DEFINED
#define GrDawnUniformHandler_DEFINED

#include "src/gpu/GrTAllocator.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"

class GrDawnGpu;

class GrDawnUniformHandler : public GrGLSLUniformHandler {
public:
    static const int kUniformsPerBlock = 8;

    const GrShaderVar& getUniformVariable(UniformHandle u) const override;
    const char* getUniformCStr(UniformHandle u) const override;

    struct UniformInfo {
        GrShaderVar    fVar;
        int            fUBOOffset;
        int            fVisibility;
    };
    typedef GrTAllocator<UniformInfo> UniformInfoArray;
    enum {
        kUniformBinding = 0,
    };
    uint32_t getRTHeightOffset() const;

private:
    explicit GrDawnUniformHandler(GrGLSLProgramBuilder* program);

    SamplerHandle addSampler(const GrBackendFormat&, GrSamplerState, const GrSwizzle&,
                             const char* name, const GrShaderCaps*) override;
    const char* samplerVariable(SamplerHandle handle) const override;
    GrSwizzle samplerSwizzle(SamplerHandle handle) const override;
    void appendUniformDecls(GrShaderFlags visibility, SkString*) const override;
    UniformHandle internalAddUniformArray(uint32_t visibility,
                                          GrSLType type,
                                          const char* name,
                                          bool mangleName,
                                          int arrayCount,
                                          const char** outName) override;

    UniformInfoArray     fUniforms;
    UniformInfoArray     fSamplers;
    UniformInfoArray     fTextures;
    SkTArray<GrSwizzle>  fSamplerSwizzles;
    SkTArray<SkString>   fSamplerReferences;

    uint32_t fCurrentUBOOffset = 0;
    uint32_t fRTHeightOffset = 0;

    friend class GrDawnProgramBuilder;
    typedef GrGLSLUniformHandler INHERITED;
};

#endif
