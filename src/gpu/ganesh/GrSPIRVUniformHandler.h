/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSPIRVUniformHandler_DEFINED
#define GrSPIRVUniformHandler_DEFINED

#include "include/private/base/SkTArray.h"
#include "src/base/SkTBlockList.h"
#include "src/gpu/ganesh/glsl/GrGLSLUniformHandler.h"

/*
 * This class can be used for basic SPIR-V uniform handling. It will make a single uniform buffer
 * for all the uniforms and will be placed in the first set and binding. Textures and samplers are
 * placed in the second set and kept as separate objects. They are interleaved as sampler texture
 * pairs with each object in the next binding slot.
 */
class GrSPIRVUniformHandler : public GrGLSLUniformHandler {
public:
    static const int kUniformsPerBlock = 8;

    const GrShaderVar& getUniformVariable(UniformHandle u) const override;
    const char* getUniformCStr(UniformHandle u) const override;

    struct SPIRVUniformInfo : public UniformInfo {
        int fUBOOffset;
    };
    typedef SkTBlockList<SPIRVUniformInfo> UniformInfoArray;
    enum {
        kUniformBinding = 0,
        kUniformDescriptorSet = 0,
        kSamplerTextureDescriptorSet = 1,
    };
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

private:
    explicit GrSPIRVUniformHandler(GrGLSLProgramBuilder* program);

    SamplerHandle addSampler(const GrBackendFormat&, GrSamplerState, const skgpu::Swizzle&,
                             const char* name, const GrShaderCaps*) override;
    const char* samplerVariable(SamplerHandle handle) const override;
    skgpu::Swizzle samplerSwizzle(SamplerHandle handle) const override;
    void appendUniformDecls(GrShaderFlags visibility, SkString*) const override;
    UniformHandle internalAddUniformArray(const GrProcessor* owner,
                                          uint32_t visibility,
                                          SkSLType type,
                                          const char* name,
                                          bool mangleName,
                                          int arrayCount,
                                          const char** outName) override;

    UniformInfoArray fUniforms;
    UniformInfoArray fSamplers;
    skia_private::TArray<skgpu::Swizzle> fSamplerSwizzles;

    uint32_t fCurrentUBOOffset = 0;
    uint32_t fRTFlipOffset = 0;

    friend class GrD3DPipelineStateBuilder;
    friend class GrDawnProgramBuilder;

    using INHERITED = GrGLSLUniformHandler;
};

#endif
