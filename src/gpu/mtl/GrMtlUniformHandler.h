/*
* Copyright 2018 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrMtlUniformHandler_DEFINED
#define GrMtlUniformHandler_DEFINED

#include "src/gpu/GrAllocator.h"
#include "src/gpu/GrShaderVar.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"

// TODO: this class is basically copy and pasted from GrVkUniformHandler so that we can have
// some shaders working. The SkSL Metal code generator was written to work with GLSL generated for
// the Ganesh Vulkan backend, so it should all work. There might be better ways to do things in
// Metal and/or some Vulkan GLSLisms left in.
class GrMtlUniformHandler : public GrGLSLUniformHandler {
public:
    static const int kUniformsPerBlock = 8;

    enum {
        kUniformBinding = 0,
        kLastUniformBinding = kUniformBinding,
    };

    // fUBOffset is only valid if the GrSLType of the fVariable is not a sampler
    struct UniformInfo {
        GrShaderVar fVariable;
        uint32_t    fVisibility;
        uint32_t    fUBOffset;
    };
    typedef GrTAllocator<UniformInfo> UniformInfoArray;

    const GrShaderVar& getUniformVariable(UniformHandle u) const override {
        return fUniforms[u.toIndex()].fVariable;
    }

    const char* getUniformCStr(UniformHandle u) const override {
        return this->getUniformVariable(u).c_str();
    }

private:
    explicit GrMtlUniformHandler(GrGLSLProgramBuilder* program)
        : INHERITED(program)
        , fUniforms(kUniformsPerBlock)
        , fSamplers(kUniformsPerBlock)
        , fCurrentUBOOffset(0)
        , fCurrentUBOMaxAlignment(0x0) {
    }

    UniformHandle internalAddUniformArray(uint32_t visibility,
                                          GrSLType type,
                                          const char* name,
                                          bool mangleName,
                                          int arrayCount,
                                          const char** outName) override;

    void updateUniformVisibility(UniformHandle u, uint32_t visibility) override {
        fUniforms[u.toIndex()].fVisibility |= visibility;
    }

    SamplerHandle addSampler(const GrTextureProxy*,
                             const GrSamplerState&,
                             const GrSwizzle&,
                             const char* name,
                             const GrShaderCaps*) override;

    int numSamplers() const { return fSamplers.count(); }
    const char* samplerVariable(SamplerHandle handle) const override {
        return fSamplers[handle.toIndex()].fVariable.c_str();
    }
    GrSwizzle samplerSwizzle(SamplerHandle handle) const override {
        return fSamplerSwizzles[handle.toIndex()];
    }
    uint32_t samplerVisibility(SamplerHandle handle) const {
        return fSamplers[handle.toIndex()].fVisibility;
    }

    void appendUniformDecls(GrShaderFlags, SkString*) const override;

    const UniformInfo& getUniformInfo(UniformHandle u) const {
        return fUniforms[u.toIndex()];
    }

    UniformInfoArray    fUniforms;
    UniformInfoArray    fSamplers;
    SkTArray<GrSwizzle> fSamplerSwizzles;

    uint32_t            fCurrentUBOOffset;
    uint32_t            fCurrentUBOMaxAlignment;

    friend class GrMtlPipelineStateBuilder;

    typedef GrGLSLUniformHandler INHERITED;
};

#endif
