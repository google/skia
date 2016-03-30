/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkPipelineStateBuilder_DEFINED
#define GrVkPipelineStateBuilder_DEFINED

#include "glsl/GrGLSLProgramBuilder.h"

#include "GrPipeline.h"
#include "GrVkPipelineState.h"
#include "GrVkUniformHandler.h"
#include "GrVkVaryingHandler.h"

#include "shaderc/shaderc.h"
#include "vk/GrVkDefines.h"

class GrVkGpu;
class GrVkRenderPass;
class GrVkProgramDesc;

class GrVkPipelineStateBuilder : public GrGLSLProgramBuilder {
public:
    /** Generates a pipeline state.
    *
    * The GrVkPipelineState implements what is specified in the GrPipeline and GrPrimitiveProcessor
    * as input. After successful generation, the builder result objects are available to be used.
    * @return true if generation was successful.
    */
    static GrVkPipelineState* CreatePipelineState(GrVkGpu*,
                                                  const GrPipeline&,
                                                  const GrPrimitiveProcessor&,
                                                  GrPrimitiveType,
                                                  const GrVkPipelineState::Desc&,
                                                  const GrVkRenderPass& renderPass);

    const GrCaps* caps() const override;
    const GrGLSLCaps* glslCaps() const override;

    GrVkGpu* gpu() const { return fGpu; }

    void finalizeFragmentOutputColor(GrGLSLShaderVar& outputColor) override;
    void finalizeFragmentSecondaryColor(GrGLSLShaderVar& outputColor) override;

private:
    GrVkPipelineStateBuilder(GrVkGpu*,
                             const GrPipeline&,
                             const GrPrimitiveProcessor&,
                             const GrVkProgramDesc&);

    GrVkPipelineState* finalize(GrPrimitiveType primitiveType,
                                const GrVkRenderPass& renderPass,
                                const GrVkPipelineState::Desc&);

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

    typedef GrGLSLProgramBuilder INHERITED;
};

#endif
