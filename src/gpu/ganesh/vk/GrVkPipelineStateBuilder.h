/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkPipelineStateBuilder_DEFINED
#define GrVkPipelineStateBuilder_DEFINED

#include "include/gpu/vk/GrVkTypes.h"
#include "src/gpu/ganesh/GrPipeline.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramBuilder.h"
#include "src/gpu/ganesh/vk/GrVkPipelineState.h"
#include "src/gpu/ganesh/vk/GrVkUniformHandler.h"
#include "src/gpu/ganesh/vk/GrVkVaryingHandler.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/ir/SkSLProgram.h"

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

    void finalizeFragmentSecondaryColor(GrShaderVar& outputColor) override;

private:
    GrVkPipelineStateBuilder(GrVkGpu*, const GrProgramDesc&, const GrProgramInfo&);

    GrVkPipelineState* finalize(const GrProgramDesc&, VkRenderPass compatibleRenderPass,
                                bool overrideSupbassForResolveLoad);

    // returns number of shader stages
    int loadShadersFromCache(SkReadBuffer* cached, VkShaderModule outShaderModules[],
                             VkPipelineShaderStageCreateInfo* outStageInfo);

    void storeShadersInCache(const std::string shaders[],
                             const SkSL::Program::Interface[],
                             bool isSkSL);

    bool createVkShaderModule(VkShaderStageFlagBits stage,
                              const std::string& sksl,
                              VkShaderModule* shaderModule,
                              VkPipelineShaderStageCreateInfo* stageInfo,
                              const SkSL::ProgramSettings& settings,
                              std::string* outSPIRV,
                              SkSL::Program::Interface* outInterface);

    bool installVkShaderModule(VkShaderStageFlagBits stage,
                               const GrGLSLShaderBuilder& builder,
                               VkShaderModule* shaderModule,
                               VkPipelineShaderStageCreateInfo* stageInfo,
                               std::string spirv,
                               SkSL::Program::Interface);

    GrGLSLUniformHandler* uniformHandler() override { return &fUniformHandler; }
    const GrGLSLUniformHandler* uniformHandler() const override { return &fUniformHandler; }
    GrGLSLVaryingHandler* varyingHandler() override { return &fVaryingHandler; }

    GrVkGpu* fGpu;
    GrVkVaryingHandler fVaryingHandler;
    GrVkUniformHandler fUniformHandler;

    using INHERITED = GrGLSLProgramBuilder;
};

#endif
