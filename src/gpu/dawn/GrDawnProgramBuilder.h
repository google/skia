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

        /**
         * Gets a float4 that adjusts the position from Skia device coords to GL's normalized device
         * coords. Assuming the transformed position, pos, is a homogeneous float3, the vec, v, is
         * applied as such:
         * pos.x = dot(v.xy, pos.xz)
         * pos.y = dot(v.zw, pos.yz)
         */
        void getRTAdjustmentVec(float* destVec) {
            destVec[0] = 2.f / fRenderTargetSize.fWidth;
            destVec[1] = -1.f;
            if (kTopLeft_GrSurfaceOrigin == fRenderTargetOrigin) {
                destVec[2] = -2.f / fRenderTargetSize.fHeight;
                destVec[3] = 1.f;
            } else {
                destVec[2] = 2.f / fRenderTargetSize.fHeight;
                destVec[3] = -1.f;
            }
        }
    };
    typedef GrGLSLBuiltinUniformHandles BuiltinUniformHandles;
    GrDawnProgram(const GrSPIRVUniformHandler::UniformInfoArray& uniforms,
                  uint32_t uniformBufferSize)
      : fDataManager(uniforms, uniformBufferSize) {
    }
    std::unique_ptr<GrGLSLPrimitiveProcessor> fGeometryProcessor;
    std::unique_ptr<GrGLSLXferProcessor> fXferProcessor;
    std::vector<std::unique_ptr<GrGLSLFragmentProcessor>> fFPImpls;
    std::vector<wgpu::BindGroupLayout> fBindGroupLayouts;
    wgpu::RenderPipeline fRenderPipeline;
    GrDawnProgramDataManager fDataManager;
    RenderTargetState fRenderTargetState;
    BuiltinUniformHandles fBuiltinUniformHandles;

    void setRenderTargetState(const GrRenderTarget*, GrSurfaceOrigin);
    wgpu::BindGroup setUniformData(GrDawnGpu*, const GrRenderTarget*, const GrProgramInfo&);
    wgpu::BindGroup setTextures(GrDawnGpu* gpu,
                                const GrPrimitiveProcessor& primProc,
                                const GrPipeline& pipeline,
                                const GrSurfaceProxy* const primProcTextures[]);
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
                         GrRenderTarget*,
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
