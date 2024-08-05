/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkUniformHandler_DEFINED
#define GrVkUniformHandler_DEFINED

#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTArray.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/base/SkTBlockList.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/GrShaderVar.h"
#include "src/gpu/ganesh/glsl/GrGLSLUniformHandler.h"

#include <cstdint>

class GrBackendFormat;
class GrGLSLProgramBuilder;
class GrProcessor;
class GrVkSampler;
class SkString;
enum class SkSLType : char;
struct GrShaderCaps;

class GrVkUniformHandler : public GrGLSLUniformHandler {
public:
    static const int kUniformsPerBlock = 8;

    /**
     * Binding a descriptor set invalidates all higher index descriptor sets. We must bind
     * in the order of this enumeration. Samplers are after Uniforms because GrOps can specify
     * GP textures as dynamic state, meaning they get rebound for each draw in a pipeline while
     * uniforms are bound once before all the draws. We bind input attachments after samplers
     * so those also need to be rebound if we bind new samplers.
     */
    static constexpr int kUniformBufferDescSet = 0;
    static constexpr int kSamplerDescSet = 1;
    static constexpr int kInputDescSet = 2;

    static constexpr int kDescSetCount = kInputDescSet + 1;

    // The bindings within their respective sets for various descriptor types.
    static constexpr int kUniformBinding = 0;
    static constexpr int kInputBinding = 0;

    // The two types of memory layout we're concerned with
    enum Layout {
        kStd140Layout = 0,
        kStd430Layout = 1,

        kLastLayout = kStd430Layout
    };
    static constexpr int kLayoutCount = kLastLayout + 1;

    struct VkUniformInfo : public UniformInfo {
        // offsets are only valid if the SkSLType of the fVariable is not a sampler.
        uint32_t                fOffsets[kLayoutCount];
        // fImmutableSampler is used for sampling an image with a ycbcr conversion.
        const GrVkSampler*      fImmutableSampler = nullptr;
    };
    typedef SkTBlockList<VkUniformInfo> UniformInfoArray;

    ~GrVkUniformHandler() override;

    const GrShaderVar& getUniformVariable(UniformHandle u) const override {
        return fUniforms.item(u.toIndex()).fVariable;
    }

    const char* getUniformCStr(UniformHandle u) const override {
        return this->getUniformVariable(u).c_str();
    }

    /**
     * Returns the offset that the RTFlip synthetic uniform should use if it needs to be created.
     */
    uint32_t getRTFlipOffset() const;

    int numUniforms() const override {
        return fUniforms.count();
    }

    UniformInfo& uniform(int idx) override {
        return fUniforms.item(idx);
    }
    const UniformInfo& uniform(int idx) const override {
        return fUniforms.item(idx);
    }

    bool usePushConstants() const { return fUsePushConstants; }
    uint32_t currentOffset() const {
        return fUsePushConstants ? fCurrentOffsets[kStd430Layout] : fCurrentOffsets[kStd140Layout];
    }

private:
    explicit GrVkUniformHandler(GrGLSLProgramBuilder* program)
        : INHERITED(program)
        , fUniforms(kUniformsPerBlock)
        , fSamplers(kUniformsPerBlock)
        , fUsePushConstants(false)
        , fCurrentOffsets{0, 0} {
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

    SamplerHandle addInputSampler(const skgpu::Swizzle& swizzle, const char* name) override;

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

    const GrVkSampler* immutableSampler(UniformHandle u) const {
        return fSamplers.item(u.toIndex()).fImmutableSampler;
    }

    const char* inputSamplerVariable(SamplerHandle handle) const override {
        // Currently we will only ever have one input sampler variable, though in the future we may
        // expand to allow more inputs. For now assert that any requested handle maps to index 0,
        // to make sure we didn't add multiple input samplers.
        SkASSERT(handle.toIndex() == 0);
        return fInputUniform.fVariable.c_str();
    }
    skgpu::Swizzle inputSamplerSwizzle(SamplerHandle handle) const override {
        SkASSERT(handle.toIndex() == 0);
        return fInputSwizzle;
    }

    void appendUniformDecls(GrShaderFlags, SkString*) const override;

    const VkUniformInfo& getUniformInfo(UniformHandle u) const {
        return fUniforms.item(u.toIndex());
    }

    void determineIfUsePushConstants() const;

    UniformInfoArray         fUniforms;
    UniformInfoArray         fSamplers;
    skia_private::TArray<skgpu::Swizzle> fSamplerSwizzles;
    UniformInfo              fInputUniform;
    skgpu::Swizzle           fInputSwizzle;
    mutable bool             fUsePushConstants;

    uint32_t            fCurrentOffsets[kLayoutCount];

    friend class GrVkPipelineStateBuilder;
    friend class GrVkDescriptorSetManager;

    using INHERITED = GrGLSLUniformHandler;
};

#endif
