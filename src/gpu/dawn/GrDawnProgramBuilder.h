/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnProgramBuilder_DEFINED
#define GrDawnProgramBuilder_DEFINED

#include "src/gpu/dawn/GrDawnProgramDataManager.h"
#include "src/gpu/dawn/GrDawnUniformHandler.h"
#include "src/gpu/dawn/GrDawnVaryingHandler.h"
#include "src/sksl/SkSLCompiler.h"
#include "dawn/dawncpp.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"

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
            if (kBottomLeft_GrSurfaceOrigin == fRenderTargetOrigin) {
                destVec[2] = -2.f / fRenderTargetSize.fHeight;
                destVec[3] = 1.f;
            } else {
                destVec[2] = 2.f / fRenderTargetSize.fHeight;
                destVec[3] = -1.f;
            }
        }
    };
    typedef GrGLSLBuiltinUniformHandles BuiltinUniformHandles;
    GrDawnProgram(const GrDawnUniformHandler::UniformInfoArray& uniforms,
                  uint32_t geometryUniformSize,
                  uint32_t fragmentUniformSize)
      : fDataManager(uniforms, geometryUniformSize, fragmentUniformSize) {
    }
    std::unique_ptr<GrGLSLPrimitiveProcessor> fGeometryProcessor;
    std::unique_ptr<GrGLSLXferProcessor> fXferProcessor;
    std::unique_ptr<std::unique_ptr<GrGLSLFragmentProcessor>[]> fFragmentProcessors;
    int fFragmentProcessorCnt;
    dawn::BindGroupLayout fBindGroupLayout;
    dawn::RenderPipeline fRenderPipeline;
    GrDawnProgramDataManager fDataManager;
    RenderTargetState fRenderTargetState;
    BuiltinUniformHandles fBuiltinUniformHandles;

    void setRenderTargetState(const GrRenderTarget*, GrSurfaceOrigin);
    dawn::BindGroup setData(GrDawnGpu* gpu, const GrRenderTarget*, GrSurfaceOrigin origin,
                            const GrPrimitiveProcessor&, const GrPipeline&,
                            const GrTextureProxy* const primProcTextures[]);
};

class GrDawnProgramBuilder : public GrGLSLProgramBuilder {
public:
    static sk_sp<GrDawnProgram> Build(GrDawnGpu*,
                                      GrRenderTarget* renderTarget,
                                      GrSurfaceOrigin origin,
                                      const GrPipeline&,
                                      const GrPrimitiveProcessor&,
                                      const GrTextureProxy* const primProcProxies[],
                                      GrPrimitiveType primitiveType,
                                      dawn::TextureFormat colorFormat,
                                      bool hasDepthStencil,
                                      dawn::TextureFormat depthStencilFormat,
                                      GrProgramDesc* desc);
    const GrCaps* caps() const override;
    GrGLSLUniformHandler* uniformHandler() override { return &fUniformHandler; }
    const GrGLSLUniformHandler* uniformHandler() const override { return &fUniformHandler; }
    GrGLSLVaryingHandler* varyingHandler() override { return &fVaryingHandler; }

    GrDawnGpu* gpu() const { return fGpu; }

private:
    GrDawnProgramBuilder(GrDawnGpu*,
                         GrRenderTarget*,
                         GrSurfaceOrigin,
                         const GrPrimitiveProcessor&,
                         const GrTextureProxy* const primProcProxies[],
                         const GrPipeline&,
                         GrProgramDesc*);
    dawn::ShaderModule createShaderModule(const GrGLSLShaderBuilder&, SkSL::Program::Kind,
                                          SkSL::Program::Inputs* inputs);
    GrDawnGpu*             fGpu;
    GrDawnVaryingHandler   fVaryingHandler;
    GrDawnUniformHandler   fUniformHandler;

    typedef GrGLSLProgramBuilder INHERITED;
};
#endif
