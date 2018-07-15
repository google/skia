/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrNXTUniformHandler_DEFINED
#define GrNXTUniformHandler_DEFINED

#include "GrAllocator.h"
#include "glsl/GrGLSLUniformHandler.h"

class GrNXTGpu;

class GrNXTUniformHandler : public GrGLSLUniformHandler {
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
    explicit GrNXTUniformHandler(GrGLSLProgramBuilder* program);

    SamplerHandle addSampler(uint32_t visibility, GrSwizzle, GrSLType, GrSLPrecision,
                             const char* name) override;
    const char* samplerVariable(SamplerHandle handle) const override;
    GrSwizzle samplerSwizzle(SamplerHandle handle) const override;
    TexelBufferHandle addTexelBuffer(uint32_t visibility, GrSLPrecision, const char* name) override;
    const GrShaderVar& texelBufferVariable(TexelBufferHandle handle) const override;
    void appendUniformDecls(GrShaderFlags visibility, SkString*) const override;
    UniformHandle internalAddUniformArray(uint32_t visibility,
                                          GrSLType type,
                                          GrSLPrecision precision,
                                          const char* name,
                                          bool mangleName,
                                          int arrayCount,
                                          const char** outName) override;

    GrShaderVar fDummyVar;
    UniformInfoArray fUniforms;
    UniformInfoArray fSamplers;
    UniformInfoArray fTextures;
    SkTArray<GrSwizzle> fSamplerSwizzles;
    SkTArray<SkString> fSamplerReferences;

    uint32_t fCurrentGeometryUBOOffset = 0;
    uint32_t fCurrentFragmentUBOOffset = 0;

    friend class GrNXTProgramBuilder;
    typedef GrGLSLUniformHandler INHERITED;
};

#endif
