/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkUniformHandler_DEFINED
#define GrVkUniformHandler_DEFINED

#include "GrAllocator.h"
#include "GrShaderVar.h"
#include "glsl/GrGLSLUniformHandler.h"

class GrVkUniformHandler : public GrGLSLUniformHandler {
public:
    static const int kUniformsPerBlock = 8;

    enum {
        kUniformBufferDescSet = 0,
        kSamplerDescSet = 1,
    };
    enum {
        kVertexBinding = 0,
        kFragBinding = 1,
    };

    // fUBOffset is only valid if the GrSLType of the fVariable is not a sampler
    struct UniformInfo {
        GrShaderVar fVariable;
        uint32_t        fVisibility;
        uint32_t        fUBOffset;
    };
    typedef GrTAllocator<UniformInfo> UniformInfoArray;

    const GrShaderVar& getUniformVariable(UniformHandle u) const override {
        return fUniforms[u.toIndex()].fVariable;
    }

    const char* getUniformCStr(UniformHandle u) const override {
        return this->getUniformVariable(u).c_str();
    }

private:
    explicit GrVkUniformHandler(GrGLSLProgramBuilder* program)
        : INHERITED(program)
        , fUniforms(kUniformsPerBlock)
        , fSamplers(kUniformsPerBlock)
        , fCurrentVertexUBOOffset(0)
        , fCurrentFragmentUBOOffset(0)
        , fCurrentSamplerBinding(0) {
    }

    UniformHandle internalAddUniformArray(uint32_t visibility,
                                          GrSLType type,
                                          GrSLPrecision precision,
                                          const char* name,
                                          bool mangleName,
                                          int arrayCount,
                                          const char** outName) override;

    SamplerHandle addSampler(uint32_t visibility,
                             GrSwizzle swizzle,
                             GrSLType type,
                             GrSLPrecision precision,
                             const char* name) override;

    int numSamplers() const { return fSamplers.count(); }
    const GrShaderVar& samplerVariable(SamplerHandle handle) const override {
        return fSamplers[handle.toIndex()].fVariable;
    }
    GrSwizzle samplerSwizzle(SamplerHandle handle) const override {
        return fSamplerSwizzles[handle.toIndex()];
    }
    uint32_t samplerVisibility(SamplerHandle handle) const {
        return fSamplers[handle.toIndex()].fVisibility;
    }

    ImageStorageHandle addImageStorage(uint32_t visibility, GrSLType,  GrImageStorageFormat,
                                       GrSLMemoryModel, GrSLRestrict, GrIOType,
                                       const char* name) override {
        SkFAIL("Image storages not implemented for Vulkan.");
        return 0;
    }

    const GrShaderVar& imageStorageVariable(ImageStorageHandle handle) const override {
        SkFAIL("Image storages not implemented for Vulkan.");
        static const GrShaderVar* gVar = nullptr;
        return *gVar;
    }

    void appendUniformDecls(GrShaderFlags, SkString*) const override;

    bool hasVertexUniforms() const { return fCurrentVertexUBOOffset > 0; }
    bool hasFragmentUniforms() const { return fCurrentFragmentUBOOffset > 0; }


    const UniformInfo& getUniformInfo(UniformHandle u) const {
        return fUniforms[u.toIndex()];
    }


    UniformInfoArray    fUniforms;
    UniformInfoArray    fSamplers;
    SkTArray<GrSwizzle> fSamplerSwizzles;

    uint32_t            fCurrentVertexUBOOffset;
    uint32_t            fCurrentFragmentUBOOffset;
    uint32_t            fCurrentSamplerBinding;

    friend class GrVkPipelineStateBuilder;
    friend class GrVkDescriptorSetManager;

    typedef GrGLSLUniformHandler INHERITED;
};

#endif
