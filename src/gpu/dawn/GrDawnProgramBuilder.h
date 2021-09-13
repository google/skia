/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnProgramBuilder_DEFINED
#define GrDawnProgramBuilder_DEFINED

#include "src/gpu/GrSPIRVUniformHandler.h"
#include "src/gpu/GrSPIRVVaryingHandler.h"
#include "src/gpu/dawn/GrDawnProgramDataManager.h"
#include "src/sksl/SkSLCompiler.h"
#include "dawn/webgpu_cpp.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"

#include <vector>

class GrPipeline;

struct GrDawnProgram : public SkRefCnt {
    struct RenderTargetState {
        SkISize         fRenderTargetSize;
        GrSurfaceOrigin fRenderTargetOrigin;

        RenderTargetState() { this->invalidate(); }
        void invalidate() {
            fRenderTargetSize.fWidth = -1;
            fRenderTargetSize.fHeight = -1;
            fRenderTargetOrigin = (GrSurfaceOrigin) -1;
        }
    };
    typedef GrGLSLBuiltinUniformHandles BuiltinUniformHandles;
    GrDawnProgram(const GrSPIRVUniformHandler::UniformInfoArray& uniforms,
                  uint32_t uniformBufferSize)
      : fDataManager(uniforms, uniformBufferSize) {
    }
    std::unique_ptr<GrGeometryProcessor::ProgramImpl> fGPImpl;
    std::unique_ptr<GrXferProcessor::ProgramImpl> fXPImpl;
    std::vector<std::unique_ptr<GrFragmentProcessor::ProgramImpl>> fFPImpls;
    std::vector<wgpu::BindGroupLayout> fBindGroupLayouts;
    wgpu::RenderPipeline fRenderPipeline;
    GrDawnProgramDataManager fDataManager;
    RenderTargetState fRenderTargetState;
    BuiltinUniformHandles fBuiltinUniformHandles;

    void setRenderTargetState(const GrRenderTarget*, GrSurfaceOrigin);
    wgpu::BindGroup setUniformData(GrDawnGpu*, const GrRenderTarget*, const GrProgramInfo&);
    wgpu::BindGroup setTextures(GrDawnGpu*,
                                const GrGeometryProcessor&,
                                const GrPipeline&,
                                const GrSurfaceProxy* const geomProcTextures[]);
};

class GrDawnProgramBuilder : public GrGLSLProgramBuilder {
public:
    static sk_sp<GrDawnProgram> Build(GrDawnGpu*,
                                      GrRenderTarget*,
                                      const GrProgramInfo&,
                                      wgpu::TextureFormat colorFormat,
                                      bool hasDepthStencil,
                                      wgpu::TextureFormat depthStencilFormat,
                                      GrProgramDesc*);
    const GrCaps* caps() const override;
    GrGLSLUniformHandler* uniformHandler() override { return &fUniformHandler; }
    const GrGLSLUniformHandler* uniformHandler() const override { return &fUniformHandler; }
    GrGLSLVaryingHandler* varyingHandler() override { return &fVaryingHandler; }

    GrDawnGpu* gpu() const { return fGpu; }

    SkSL::Compiler* shaderCompiler() const override;

private:
    GrDawnProgramBuilder(GrDawnGpu*,
                         const GrProgramInfo&,
                         GrProgramDesc*);
    wgpu::ShaderModule createShaderModule(const GrGLSLShaderBuilder&, SkSL::ProgramKind,
                                          bool flipY, SkSL::Program::Inputs* inputs);
    GrDawnGpu*             fGpu;
    GrSPIRVVaryingHandler   fVaryingHandler;
    GrSPIRVUniformHandler   fUniformHandler;

    using INHERITED = GrGLSLProgramBuilder;
};
#endif
