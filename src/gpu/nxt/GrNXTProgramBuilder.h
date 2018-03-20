/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrNXTProgramBuilder_DEFINED
#define GrNXTProgramBuilder_DEFINED

#include "SkSLCompiler.h"
#include "nxt/GrNXTProgramDataManager.h"
#include "nxt/GrNXTUniformHandler.h"
#include "nxt/GrNXTVaryingHandler.h"
#include "nxt/nxtcpp.h"
#include "glsl/GrGLSLProgramBuilder.h"

class GrPipeline;

struct GrRenderTargetState {
    SkISize         fRenderTargetSize;
    GrSurfaceOrigin fRenderTargetOrigin;

    GrRenderTargetState() { this->invalidate(); }
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

struct GrNXTProgram : public SkRefCnt {
    typedef GrGLSLProgramBuilder::BuiltinUniformHandles BuiltinUniformHandles;
    GrNXTProgram(const GrNXTUniformHandler::UniformInfoArray& uniforms,
                 uint32_t geometryUniformSize,
                 uint32_t fragmentUniformSize)
      : fDataManager(uniforms, geometryUniformSize, fragmentUniformSize) {
    }
    std::unique_ptr<GrGLSLPrimitiveProcessor> fGeometryProcessor;
    std::unique_ptr<GrGLSLXferProcessor> fXferProcessor;
    GrGLSLFragProcs fFragmentProcessors;
    nxt::BindGroupLayout fBindGroupLayout;
    nxt::RenderPipeline fRenderPipeline;
    GrNXTProgramDataManager fDataManager;
    GrRenderTargetState fRenderTargetState;
    BuiltinUniformHandles fBuiltinUniformHandles;

    void setRenderTargetState(const GrRenderTargetProxy*);
    nxt::BindGroup setData(GrNXTGpu* gpu, const GrPrimitiveProcessor&, const GrPipeline&, nxt::CommandBufferBuilder builder);
};

class GrNXTProgramBuilder : public GrGLSLProgramBuilder {
public:
    static sk_sp<GrNXTProgram> Build(GrNXTGpu*,
                                     const GrPipeline&,
                                     const GrPrimitiveProcessor&,
                                     GrPrimitiveType primitiveType,
                                     nxt::RenderPass renderPass,
                                     GrProgramDesc* desc);
    const GrCaps* caps() const override;
    GrGLSLUniformHandler* uniformHandler() override { return &fUniformHandler; }
    const GrGLSLUniformHandler* uniformHandler() const override { return &fUniformHandler; }
    GrGLSLVaryingHandler* varyingHandler() override { return &fVaryingHandler; }

    GrNXTGpu* gpu() const { return fGpu; }

private:
    GrNXTProgramBuilder(GrNXTGpu*, const GrPipeline&, const GrPrimitiveProcessor&,
                        GrProgramDesc*);
    nxt::ShaderModule CreateShaderModule(nxt::Device, const GrGLSLShaderBuilder&, SkSL::Program::Kind, SkSL::Program::Inputs* inputs);
    GrNXTGpu*             fGpu;
    GrNXTVaryingHandler   fVaryingHandler;
    GrNXTUniformHandler   fUniformHandler;

    typedef GrGLSLProgramBuilder INHERITED;
};
#endif
