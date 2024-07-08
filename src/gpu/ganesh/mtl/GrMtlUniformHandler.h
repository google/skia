/*
* Copyright 2018 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrMtlUniformHandler_DEFINED
#define GrMtlUniformHandler_DEFINED

#include "include/private/base/SkTArray.h"
#include "src/base/SkTBlockList.h"
#include "src/gpu/ganesh/GrShaderVar.h"
#include "src/gpu/ganesh/glsl/GrGLSLUniformHandler.h"

#include <vector>

// TODO: this class is basically copy and pasted from GrVkUniformHandler so that we can have
// some shaders working. The SkSL Metal code generator was written to work with GLSL generated for
// the Ganesh Vulkan backend, so it should all work. There might be better ways to do things in
// Metal and/or some Vulkan GLSLisms left in.
class GrMtlUniformHandler : public GrGLSLUniformHandler {
public:
    static constexpr size_t kUniformsPerBlock = 8;
    static constexpr size_t kUniformBinding = 0;
    static constexpr size_t kLastUniformBinding = kUniformBinding;
    static constexpr size_t kUniformBindingCount = 1;

    // fUBOffset is only valid if the SkSLType of the fVariable is not a sampler
    struct MtlUniformInfo : public UniformInfo {
        uint32_t fUBOffset;
    };
    typedef SkTBlockList<MtlUniformInfo> UniformInfoArray;

    const GrShaderVar& getUniformVariable(UniformHandle u) const override {
        return fUniforms.item(u.toIndex()).fVariable;
    }

    const char* getUniformCStr(UniformHandle u) const override {
        return this->getUniformVariable(u).c_str();
    }

    int numUniforms() const override {
        return fUniforms.count();
    }

    UniformInfo& uniform(int idx) override {
        return fUniforms.item(idx);
    }
    const UniformInfo& uniform(int idx) const override {
        return fUniforms.item(idx);
    }

private:
    explicit GrMtlUniformHandler(GrGLSLProgramBuilder* program)
        : INHERITED(program)
        , fUniforms(kUniformsPerBlock)
        , fSamplers(kUniformsPerBlock)
        , fCurrentUBOOffset(0)
        , fCurrentUBOMaxAlignment(0x0) {
    }

    UniformHandle internalAddUniformArray(const GrProcessor* owner,
                                          uint32_t visibility,
                                          SkSLType type,
                                          const char* name,
                                          bool mangleName,
                                          int arrayCount,
                                          const char** outName) override;

    SamplerHandle addSampler(const GrBackendFormat&,
                             GrSamplerState,
                             const skgpu::Swizzle&,
                             const char* name,
                             const GrShaderCaps*) override;

    int numSamplers() const { return fSamplers.count(); }
    const char* samplerVariable(SamplerHandle handle) const override {
        return fSamplers.item(handle.toIndex()).fVariable.c_str();
    }
    skgpu::Swizzle samplerSwizzle(SamplerHandle handle) const override {
        return fSamplerSwizzles[handle.toIndex()];
    }
    uint32_t samplerVisibility(SamplerHandle handle) const {
        return fSamplers.item(handle.toIndex()).fVisibility;
    }

    void appendUniformDecls(GrShaderFlags, SkString*) const override;

    const UniformInfo& getUniformInfo(UniformHandle u) const {
        return fUniforms.item(u.toIndex());
    }

    UniformInfoArray    fUniforms;
    UniformInfoArray    fSamplers;
    skia_private::TArray<skgpu::Swizzle> fSamplerSwizzles;

    uint32_t            fCurrentUBOOffset;
    uint32_t            fCurrentUBOMaxAlignment;

    friend class GrMtlPipelineStateBuilder;

    using INHERITED = GrGLSLUniformHandler;
};

#endif
