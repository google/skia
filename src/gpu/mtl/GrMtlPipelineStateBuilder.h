/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlPipelineStateBuilder_DEFINED
#define GrMtlPipelineStateBuilder_DEFINED

#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrProgramDesc.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/gpu/mtl/GrMtlUniformHandler.h"
#include "src/gpu/mtl/GrMtlVaryingHandler.h"
#include "src/sksl/SkSLCompiler.h"

#import <Metal/Metal.h>

class GrProgramInfo;
class GrMtlGpu;
class GrMtlPipelineState;

class GrMtlPipelineStateBuilder : public GrGLSLProgramBuilder {
public:
    /**
     * For Metal we want to cache the entire pipeline for reuse of draws. The Desc here holds all
     * the information needed to differentiate one pipeline from another.
     *
     * The GrProgramDesc contains all the information need to create the actual shaders for the
     * pipeline.
     *
     * For Metal we need to add to the GrProgramDesc to include the rest of the state on the
     * pipeline. This includes blending information and primitive type. The pipeline is immutable
     * so any remaining dynamic state is set via the MtlRenderCmdEncoder.
     */
    class Desc : public GrProgramDesc {
    public:
        static bool Build(Desc*, GrRenderTarget*,
                          const GrProgramInfo&, GrPrimitiveType, GrMtlGpu* gpu);

        size_t shaderKeyLength() const { return fShaderKeyLength; }

    private:
        size_t fShaderKeyLength;

        typedef GrProgramDesc INHERITED;
    };

    /** Generates a pipeline state.
     *
     * The GrMtlPipelineState implements what is specified in the GrPipeline and
     * GrPrimitiveProcessor as input. After successful generation, the builder result objects are
     * available to be used. This function may modify the program key by setting the surface origin
     * key to 0 (unspecified) if it turns out the program does not care about the surface origin.
     * @return true if generation was successful.
     */
    static GrMtlPipelineState* CreatePipelineState(GrMtlGpu*,
                                                   GrRenderTarget*,
                                                   const GrProgramInfo&,
                                                   Desc*);

private:
    GrMtlPipelineStateBuilder(GrMtlGpu*, GrRenderTarget*, const GrProgramInfo&, GrProgramDesc*);

    GrMtlPipelineState* finalize(GrRenderTarget*, const GrProgramInfo&, Desc*);

    const GrCaps* caps() const override;

    void finalizeFragmentOutputColor(GrShaderVar& outputColor) override;

    void finalizeFragmentSecondaryColor(GrShaderVar& outputColor) override;

    id<MTLLibrary> createMtlShaderLibrary(const GrGLSLShaderBuilder& builder,
                                          SkSL::Program::Kind kind,
                                          const SkSL::Program::Settings& settings,
                                          GrProgramDesc* desc);

    GrGLSLUniformHandler* uniformHandler() override { return &fUniformHandler; }
    const GrGLSLUniformHandler* uniformHandler() const override { return &fUniformHandler; }
    GrGLSLVaryingHandler* varyingHandler() override { return &fVaryingHandler; }

    GrMtlGpu* fGpu;
    GrMtlUniformHandler fUniformHandler;
    GrMtlVaryingHandler fVaryingHandler;

    typedef GrGLSLProgramBuilder INHERITED;
};
#endif
