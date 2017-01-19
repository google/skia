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
#include "SkSLCompiler.h"

#include "vk/GrVkDefines.h"

class GrProgramDesc;
class GrVkGpu;
class GrVkRenderPass;

class GrVkPipelineStateBuilder : public GrGLSLProgramBuilder {
public:
    /** Generates a pipeline state.
    *
    * The GrVkPipelineState implements what is specified in the GrPipeline and GrPrimitiveProcessor
    * as input. After successful generation, the builder result objects are available to be used.
    * This function may modify the program key by setting the surface origin key to 0 (unspecified)
    * if it turns out the program does not care about the surface origin.
    * @return true if generation was successful.
    */
    static GrVkPipelineState* CreatePipelineState(GrVkGpu*,
                                                  const GrPipeline&,
                                                  const GrStencilSettings&,
                                                  const GrPrimitiveProcessor&,
                                                  GrPrimitiveType,
                                                  GrVkPipelineState::Desc*,
                                                  const GrVkRenderPass& renderPass);

    const GrCaps* caps() const override;

    GrVkGpu* gpu() const { return fGpu; }

    void finalizeFragmentOutputColor(GrShaderVar& outputColor) override;
    void finalizeFragmentSecondaryColor(GrShaderVar& outputColor) override;

private:
    GrVkPipelineStateBuilder(GrVkGpu*,
                             const GrPipeline&,
                             const GrPrimitiveProcessor&,
                             GrProgramDesc*);

    GrVkPipelineState* finalize(const GrStencilSettings&,
                                GrPrimitiveType primitiveType,
                                const GrVkRenderPass& renderPass,
                                GrVkPipelineState::Desc*);

    bool createVkShaderModule(VkShaderStageFlagBits stage,
                              const GrGLSLShaderBuilder& builder,
                              VkShaderModule* shaderModule,
                              VkPipelineShaderStageCreateInfo* stageInfo,
                              const SkSL::Program::Settings& settings,
                              GrVkPipelineState::Desc* desc);

    GrGLSLUniformHandler* uniformHandler() override { return &fUniformHandler; }
    const GrGLSLUniformHandler* uniformHandler() const override { return &fUniformHandler; }
    GrGLSLVaryingHandler* varyingHandler() override { return &fVaryingHandler; }

    GrVkGpu* fGpu;
    GrVkVaryingHandler        fVaryingHandler;
    GrVkUniformHandler        fUniformHandler;

    typedef GrGLSLProgramBuilder INHERITED;
};

#endif
