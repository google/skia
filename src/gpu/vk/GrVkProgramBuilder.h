/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkProgramBuilder_DEFINED
#define GrVkProgramBuilder_DEFINED

#include "glsl/GrGLSLProgramBuilder.h"

#include "GrPipeline.h"
#include "vk/GrVkUniformHandler.h"
#include "vk/GrVkVaryingHandler.h"

#include "shaderc/shaderc.h"
#include "vulkan/vulkan.h"

class GrVkGpu;
class GrVkRenderPass;
class GrVkProgram;

class GrVkProgramBuilder : public GrGLSLProgramBuilder {
public:
    /** Generates a shader program.
    *
    * The program implements what is specified in the stages given as input.
    * After successful generation, the builder result objects are available
    * to be used.
    * @return true if generation was successful.
    */
    static GrVkProgram* CreateProgram(GrVkGpu*,
                                      const DrawArgs&,
                                      GrPrimitiveType,
                                      const GrVkRenderPass& renderPass);

    const GrCaps* caps() const override;
    const GrGLSLCaps* glslCaps() const override;

    GrVkGpu* gpu() const { return fGpu; }

    void finalizeFragmentOutputColor(GrGLSLShaderVar& outputColor) override;

private:
    GrVkProgramBuilder(GrVkGpu*, const DrawArgs&);

    void emitSamplers(const GrProcessor&,
                      GrGLSLTextureSampler::TextureSamplerArray* outSamplers) override;

    GrVkProgram* finalize(const DrawArgs& args,
                          GrPrimitiveType primitiveType,
                          const GrVkRenderPass& renderPass);

    static bool CreateVkShaderModule(const GrVkGpu* gpu,
                                     VkShaderStageFlagBits stage,
                                     const GrGLSLShaderBuilder& builder,
                                     VkShaderModule* shaderModule,
                                     VkPipelineShaderStageCreateInfo* stageInfo);

    GrGLSLUniformHandler* uniformHandler() override { return &fUniformHandler; }
    const GrGLSLUniformHandler* uniformHandler() const override { return &fUniformHandler; }
    GrGLSLVaryingHandler* varyingHandler() override { return &fVaryingHandler; }

    GrVkGpu* fGpu;
    GrVkVaryingHandler        fVaryingHandler;
    GrVkUniformHandler        fUniformHandler;

    SkTArray<UniformHandle>   fSamplerUniforms;

    typedef GrGLSLProgramBuilder INHERITED;
};

#endif