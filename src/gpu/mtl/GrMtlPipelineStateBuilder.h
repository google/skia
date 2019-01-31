/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlPipelineStateBuilder_DEFINED
#define GrMtlPipelineStateBuilder_DEFINED

#include "GrPipeline.h"
#include "GrProgramDesc.h"
#include "GrMtlUniformHandler.h"
#include "GrMtlVaryingHandler.h"
#include "SkSLCompiler.h"
#include "glsl/GrGLSLProgramBuilder.h"

#import <metal/metal.h>

class GrMtlGpu;
class GrMtlPipelineState;

class GrMtlPipelineStateBuilder : public GrGLSLProgramBuilder {
public:
    static GrMtlPipelineState* CreatePipelineState(GrRenderTarget*, GrSurfaceOrigin,
                                                   const GrPrimitiveProcessor&,
                                                   const GrTextureProxy* const primProcProxies[],
                                                   const GrPipeline&,
                                                   GrProgramDesc*,
                                                   GrMtlGpu*);

private:
    GrMtlPipelineStateBuilder(GrRenderTarget*, GrSurfaceOrigin,
                              const GrPrimitiveProcessor&,
                              const GrTextureProxy* const primProcProxies[],
                              const GrPipeline&,
                              GrProgramDesc*, GrMtlGpu*);

    const GrCaps* caps() const override;

    GrGLSLUniformHandler* uniformHandler() override { return &fUniformHandler; }

    const GrGLSLUniformHandler* uniformHandler() const override { return &fUniformHandler; }

    GrGLSLVaryingHandler* varyingHandler() override { return &fVaryingHandler; }

    void finalizeFragmentOutputColor(GrShaderVar& outputColor) override;

    void finalizeFragmentSecondaryColor(GrShaderVar& outputColor) override;

    id<MTLLibrary> createMtlShaderLibrary(const GrGLSLShaderBuilder& builder,
                                          SkSL::Program::Kind kind,
                                          const SkSL::Program::Settings& settings,
                                          GrProgramDesc* desc);

    GrMtlPipelineState* finalize(const GrPrimitiveProcessor&, const GrPipeline&, GrProgramDesc*);

    GrMtlGpu* fGpu;
    GrMtlUniformHandler fUniformHandler;
    GrMtlVaryingHandler fVaryingHandler;

    typedef GrGLSLProgramBuilder INHERITED;
};
#endif
