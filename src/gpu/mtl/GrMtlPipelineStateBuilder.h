/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlPipelineStateBuilder_DEFINED
#define GrMtlPipelineStateBuilder_DEFINED

#include "src/gpu/GrPipeline.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/gpu/mtl/GrMtlUniformHandler.h"
#include "src/gpu/mtl/GrMtlVaryingHandler.h"
#include "src/sksl/SkSLCompiler.h"

#import <Metal/Metal.h>

class GrProgramDesc;
class GrProgramInfo;
class GrMtlCaps;
class GrMtlGpu;
class GrMtlPipelineState;
class SkReader32;

class GrMtlPipelineStateBuilder : public GrGLSLProgramBuilder {
public:
    /** Generates a pipeline state.
     *
     * The GrMtlPipelineState implements what is specified in the GrPipeline and
     * GrPrimitiveProcessor as input. After successful generation, the builder result objects are
     * available to be used.
     * @return the created pipeline if generation was successful; nullptr otherwise
     */
    static GrMtlPipelineState* CreatePipelineState(GrMtlGpu*,
                                                   GrRenderTarget*,
                                                   const GrProgramDesc&,
                                                   const GrProgramInfo&);

private:
    GrMtlPipelineStateBuilder(GrMtlGpu*, GrRenderTarget*,
                              const GrProgramDesc&, const GrProgramInfo&);

    GrMtlPipelineState* finalize(GrRenderTarget*, const GrProgramDesc&, const GrProgramInfo&);

    const GrCaps* caps() const override;

    void finalizeFragmentOutputColor(GrShaderVar& outputColor) override;

    void finalizeFragmentSecondaryColor(GrShaderVar& outputColor) override;

    id<MTLLibrary> generateMtlShaderLibrary(const SkSL::String& sksl,
                                            SkSL::Program::Kind kind,
                                            const SkSL::Program::Settings& settings,
                                            SkSL::String* msl,
                                            SkSL::Program::Inputs* inputs);
    id<MTLLibrary> compileMtlShaderLibrary(const SkSL::String& shader,
                                           SkSL::Program::Inputs inputs);
    void storeShadersInCache(const SkSL::String shaders[], const SkSL::Program::Inputs inputs[],
                             bool isSkSL);
    void loadShadersFromCache(SkReader32* cached, __strong id<MTLLibrary> outLibraries[]);

    GrGLSLUniformHandler* uniformHandler() override { return &fUniformHandler; }
    const GrGLSLUniformHandler* uniformHandler() const override { return &fUniformHandler; }
    GrGLSLVaryingHandler* varyingHandler() override { return &fVaryingHandler; }

    GrMtlGpu* fGpu;
    GrMtlUniformHandler fUniformHandler;
    GrMtlVaryingHandler fVaryingHandler;

    typedef GrGLSLProgramBuilder INHERITED;
};
#endif
