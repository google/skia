/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkPipelineStateBuilder_DEFINED
#define GrVkPipelineStateBuilder_DEFINED

#include "include/core/SkString.h"
#include "include/private/gpu/vk/SkiaVulkan.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLShaderBuilder.h"
#include "src/gpu/ganesh/vk/GrVkUniformHandler.h"
#include "src/gpu/ganesh/vk/GrVkVaryingHandler.h"
#include "src/sksl/ir/SkSLProgram.h"

#include <string>

class GrCaps;
class GrGLSLUniformHandler;
class GrGLSLVaryingHandler;
class GrProgramDesc;
class GrProgramInfo;
class GrShaderVar;
class GrVkGpu;
class GrVkPipelineState;
class SkReadBuffer;
namespace SkSL {
struct NativeShader;
struct ProgramSettings;
}  // namespace SkSL

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

    void storeShadersInCache(const SkSL::NativeShader binaryShaders[],
                             const SkSL::Program::Interface[]);

    bool createVkShaderModule(VkShaderStageFlagBits stage,
                              const std::string& sksl,
                              VkShaderModule* shaderModule,
                              VkPipelineShaderStageCreateInfo* stageInfo,
                              const SkSL::ProgramSettings& settings,
                              SkSL::NativeShader* outSPIRV,
                              SkSL::Program::Interface* outInterface);

    bool installVkShaderModule(VkShaderStageFlagBits stage,
                               const GrGLSLShaderBuilder& builder,
                               VkShaderModule* shaderModule,
                               VkPipelineShaderStageCreateInfo* stageInfo,
                               const SkSL::NativeShader& spirv,
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
