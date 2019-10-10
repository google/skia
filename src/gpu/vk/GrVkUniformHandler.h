/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkUniformHandler_DEFINED
#define GrVkUniformHandler_DEFINED

#include "include/gpu/vk/GrVkTypes.h"
#include "src/gpu/GrAllocator.h"
#include "src/gpu/GrSamplerState.h"
#include "src/gpu/GrShaderVar.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"
#include "src/gpu/vk/GrVkSampler.h"

class GrVkUniformHandler : public GrGLSLUniformHandler {
public:
    static const int kUniformsPerBlock = 8;

    enum {
        /**
         * Binding a descriptor set invalidates all higher index descriptor sets. We must bind
         * in the order of this enumeration. Samplers are after Uniforms because GrOps can specify
         * GP textures as dynamic state, meaning they get rebound for each GrMesh in a draw while
         * uniforms are bound once before all the draws.
         */
        kUniformBufferDescSet = 0,
        kSamplerDescSet = 1,
    };
    enum {
        kUniformBinding = 0
    };

    struct UniformInfo {
        GrShaderVar             fVariable;
        uint32_t                fVisibility;
        // fUBOffset is only valid if the GrSLType of the fVariable is not a sampler
        uint32_t                fUBOffset;
        // fImmutableSampler is used for sampling an image with a ycbcr conversion.
        const GrVkSampler*      fImmutableSampler = nullptr;
    };
    typedef GrTAllocator<UniformInfo> UniformInfoArray;

    ~GrVkUniformHandler() override;

    const GrShaderVar& getUniformVariable(UniformHandle u) const override {
        return fUniforms[u.toIndex()].fVariable;
    }

    const char* getUniformCStr(UniformHandle u) const override {
        return this->getUniformVariable(u).c_str();
    }

    /**
     * Returns the offset that the RTHeight synthetic uniform should use if it needs to be created.
     */
    uint32_t getRTHeightOffset() const;

private:
    explicit GrVkUniformHandler(GrGLSLProgramBuilder* program)
        : INHERITED(program)
        , fUniforms(kUniformsPerBlock)
        , fSamplers(kUniformsPerBlock)
        , fCurrentUBOOffset(0) {
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

    const GrVkSampler* immutableSampler(UniformHandle u) const {
        return fSamplers[u.toIndex()].fImmutableSampler;
    }

    void appendUniformDecls(GrShaderFlags, SkString*) const override;

    const UniformInfo& getUniformInfo(UniformHandle u) const {
        return fUniforms[u.toIndex()];
    }


    UniformInfoArray    fUniforms;
    UniformInfoArray    fSamplers;
    SkTArray<GrSwizzle> fSamplerSwizzles;

    uint32_t            fCurrentUBOOffset;

    friend class GrVkPipelineStateBuilder;
    friend class GrVkDescriptorSetManager;

    typedef GrGLSLUniformHandler INHERITED;
};

#endif
