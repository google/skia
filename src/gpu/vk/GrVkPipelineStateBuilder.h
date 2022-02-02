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
class SkReadBuffer;

class GrVkPipelineStateBuilder : public GrGLSLProgramBuilder {
public:
    /** Generates a pipeline state.
     *
     * The return GrVkPipelineState implements the supplied GrProgramInfo.
     *
     * @return the created pipeline if generation was successful; nullptr otherwise
     */
    static GrVkPipelineState* CreatePipelineState(GrVkGpu*,
                                                  const GrProgramDesc&,
                                                  const GrProgramInfo&,
                                                  VkRenderPass compatibleRenderPass,
                                                  bool overrideSubpassForResolveLoad);

    const GrCaps* caps() const override;

    GrVkGpu* gpu() const { return fGpu; }

    SkSL::Compiler* shaderCompiler() const override;

    void finalizeFragmentOutputColor(GrShaderVar& outputColor) override;
    void finalizeFragmentSecondaryColor(GrShaderVar& outputColor) override;

private:
    GrVkPipelineStateBuilder(GrVkGpu*, const GrProgramDesc&, const GrProgramInfo&);

    GrVkPipelineState* finalize(const GrProgramDesc&, VkRenderPass compatibleRenderPass,
                                bool overrideSupbassForResolveLoad);

    // returns number of shader stages
    int loadShadersFromCache(SkReadBuffer* cached, VkShaderModule outShaderModules[],
                             VkPipelineShaderStageCreateInfo* outStageInfo);

    void storeShadersInCache(const std::string shaders[], const SkSL::Program::Inputs inputs[],
                             bool isSkSL);

    bool createVkShaderModule(VkShaderStageFlagBits stage,
                              const std::string& sksl,
                              VkShaderModule* shaderModule,
                              VkPipelineShaderStageCreateInfo* stageInfo,
                              const SkSL::Program::Settings& settings,
                              std::string* outSPIRV,
                              SkSL::Program::Inputs* outInputs);

    bool installVkShaderModule(VkShaderStageFlagBits stage,
                               const GrGLSLShaderBuilder& builder,
                               VkShaderModule* shaderModule,
                               VkPipelineShaderStageCreateInfo* stageInfo,
                               std::string spirv,
                               SkSL::Program::Inputs inputs);

    GrGLSLUniformHandler* uniformHandler() override { return &fUniformHandler; }
    const GrGLSLUniformHandler* uniformHandler() const override { return &fUniformHandler; }
    GrGLSLVaryingHandler* varyingHandler() override { return &fVaryingHandler; }

    GrVkGpu* fGpu;
    GrVkVaryingHandler fVaryingHandler;
    GrVkUniformHandler fUniformHandler;

    using INHERITED = GrGLSLProgramBuilder;
};

#endif
