/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkPipelineStateBuilder_DEFINED
#define GrVkPipelineStateBuilder_DEFINED

#include "GrPipeline.h"
#include "GrProgramDesc.h"
#include "GrVkPipelineState.h"
#include "GrVkUniformHandler.h"
#include "GrVkVaryingHandler.h"
#include "SkSLCompiler.h"
#include "glsl/GrGLSLProgramBuilder.h"
#include "vk/GrVkTypes.h"

class GrVkGpu;
class GrVkRenderPass;

class GrVkPipelineStateBuilder : public GrGLSLProgramBuilder {
public:
    /**
     * For Vulkan we want to cache the entire VkPipeline for reuse of draws. The Desc here holds all
     * the information needed to differentiate one pipeline from another.
     *
     * The GrProgramDesc contains all the information need to create the actual shaders for the
     * pipeline.
     *
     * For Vulkan we need to add to the GrProgramDesc to include the rest of the state on the
     * pipline. This includes stencil settings, blending information, render pass format, draw face
     * information, and primitive type. Note that some state is set dynamically on the pipeline for
     * each draw  and thus is not included in this descriptor. This includes the viewport, scissor,
     * and blend constant.
     */
    class Desc : public GrProgramDesc {
    public:
        static bool Build(Desc*,
                          GrRenderTarget*,
                          const GrPrimitiveProcessor&,
                          const GrPipeline&,
                          const GrStencilSettings&,
                          GrPrimitiveType primitiveType,
                          GrVkGpu* gpu);

        size_t shaderKeyLength() const { return fShaderKeyLength; }

    private:
        size_t fShaderKeyLength;

        typedef GrProgramDesc INHERITED;
    };

    /** Generates a pipeline state.
    *
    * The GrVkPipelineState implements what is specified in the GrPipeline and GrPrimitiveProcessor
    * as input. After successful generation, the builder result objects are available to be used.
    * This function may modify the program key by setting the surface origin key to 0 (unspecified)
    * if it turns out the program does not care about the surface origin.
    * @return true if generation was successful.
    */
    static GrVkPipelineState* CreatePipelineState(GrVkGpu*,
                                                  GrRenderTarget*, GrSurfaceOrigin,
                                                  const GrPrimitiveProcessor&,
                                                  const GrTextureProxy* const primProcProxies[],
                                                  const GrPipeline&,
                                                  const GrStencilSettings&,
                                                  GrPrimitiveType,
                                                  Desc*,
                                                  VkRenderPass compatibleRenderPass);

    const GrCaps* caps() const override;

    GrVkGpu* gpu() const { return fGpu; }

    void finalizeFragmentOutputColor(GrShaderVar& outputColor) override;
    void finalizeFragmentSecondaryColor(GrShaderVar& outputColor) override;

private:
    GrVkPipelineStateBuilder(GrVkGpu*, GrRenderTarget*, GrSurfaceOrigin,
                             const GrPipeline&,
                             const GrPrimitiveProcessor&,
                             const GrTextureProxy* const primProcProxies[],
                             GrProgramDesc*);

    GrVkPipelineState* finalize(const GrStencilSettings&,
                                GrPrimitiveType primitiveType,
                                VkRenderPass compatibleRenderPass,
                                Desc*);

    // returns number of shader stages
    int loadShadersFromCache(const SkData& cached,
                             VkShaderModule* outVertShaderModule,
                             VkShaderModule* outFragShaderModule,
                             VkShaderModule* outGeomShaderModule,
                             VkPipelineShaderStageCreateInfo* outStageInfo);

    void storeShadersInCache(const SkSL::String& vert,
                             const SkSL::Program::Inputs& vertInputs,
                             const SkSL::String& frag,
                             const SkSL::Program::Inputs& fragInputs,
                             const SkSL::String& geom,
                             const SkSL::Program::Inputs& geomInputs);

    bool createVkShaderModule(VkShaderStageFlagBits stage,
                              const GrGLSLShaderBuilder& builder,
                              VkShaderModule* shaderModule,
                              VkPipelineShaderStageCreateInfo* stageInfo,
                              const SkSL::Program::Settings& settings,
                              Desc* desc,
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
