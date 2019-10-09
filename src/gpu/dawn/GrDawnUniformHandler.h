/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnUniformHandler_DEFINED
#define GrDawnUniformHandler_DEFINED

#include "src/gpu/GrAllocator.h"
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
        kGeometryBinding = 0,
        kFragBinding = 1,
        kSamplerBindingBase = 2,
    };

private:
    explicit GrDawnUniformHandler(GrGLSLProgramBuilder* program);

    SamplerHandle addSampler(const GrTextureProxy*, const GrSamplerState&, const GrSwizzle&,
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

    void updateUniformVisibility(UniformHandle u, uint32_t visibility) override;

    UniformInfoArray     fUniforms;
    UniformInfoArray     fSamplers;
    UniformInfoArray     fTextures;
    SkTArray<GrSwizzle>  fSamplerSwizzles;
    SkTArray<SkString>   fSamplerReferences;

    uint32_t fCurrentGeometryUBOOffset = 0;
    uint32_t fCurrentFragmentUBOOffset = 0;

    friend class GrDawnProgramBuilder;
    typedef GrGLSLUniformHandler INHERITED;
};

#endif
