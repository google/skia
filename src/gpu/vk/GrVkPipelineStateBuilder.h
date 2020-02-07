/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkPipelineStateBuilder_DEFINED
#define GrVkPipelineStateBuilder_DEFINED

#include "include/gpu/vk/GrVkTypes.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/gpu/vk/GrVkPipelineState.h"
#include "src/gpu/vk/GrVkUniformHandler.h"
#include "src/gpu/vk/GrVkVaryingHandler.h"
#include "src/sksl/SkSLCompiler.h"

class GrProgramDesc;
class GrVkGpu;
class GrVkRenderPass;
class SkReader32;

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
                                                  GrRenderTarget*,
                                                  const GrProgramInfo&,
                                                  GrProgramDesc*,
                                                  VkRenderPass compatibleRenderPass);

    const GrCaps* caps() const override;

    GrVkGpu* gpu() const { return fGpu; }

    void finalizeFragmentOutputColor(GrShaderVar& outputColor) override;
    void finalizeFragmentSecondaryColor(GrShaderVar& outputColor) override;

private:
    GrVkPipelineStateBuilder(GrVkGpu*, GrRenderTarget*, const GrProgramInfo&, GrProgramDesc*);

    GrVkPipelineState* finalize(VkRenderPass compatibleRenderPass, GrProgramDesc*);

    // returns number of shader stages
    int loadShadersFromCache(SkReader32* cached, VkShaderModule outShaderModules[],
                             VkPipelineShaderStageCreateInfo* outStageInfo);

    void storeShadersInCache(const SkSL::String shaders[], const SkSL::Program::Inputs inputs[],
                             bool isSkSL);

    bool createVkShaderModule(VkShaderStageFlagBits stage,
                              const SkSL::String& sksl,
                              VkShaderModule* shaderModule,
                              VkPipelineShaderStageCreateInfo* stageInfo,
                              const SkSL::Program::Settings& settings,
                              GrProgramDesc* desc,
                              SkSL::String* outSPIRV,
                              SkSL::Program::Inputs* outInputs);

    bool installVkShaderModule(VkShaderStageFlagBits stage,
                               const GrGLSLShaderBuilder& builder,
                               VkShaderModule* shaderModule,
                               VkPipelineShaderStageCreateInfo* stageInfo,
                               SkSL::String spirv,
                               SkSL::Program::Inputs inputs);

    GrGLSLUniformHandler* uniformHandler() override { return &fUniformHandler; }
    const GrGLSLUniformHandler* uniformHandler() const override { return &fUniformHandler; }
    GrGLSLVaryingHandler* varyingHandler() override { return &fVaryingHandler; }

    GrVkGpu* fGpu;
    GrVkVaryingHandler fVaryingHandler;
    GrVkUniformHandler fUniformHandler;

    typedef GrGLSLProgramBuilder INHERITED;
};

#endif
