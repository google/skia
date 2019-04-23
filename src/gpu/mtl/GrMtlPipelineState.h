/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlPipelineState_DEFINED
#define GrMtlPipelineState_DEFINED

#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrStencilSettings.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/gpu/mtl/GrMtlBuffer.h"
#include "src/gpu/mtl/GrMtlPipelineStateDataManager.h"

#import <metal/metal.h>

class GrMtlGpu;
class GrMtlPipelineStateDataManager;
class GrMtlSampler;
class GrMtlTexture;
class GrPipeline;

/**
 * Wraps a MTLRenderPipelineState object and also contains more info about the pipeline as needed
 * by Ganesh
 */
class GrMtlPipelineState {
public:
    using UniformInfoArray = GrMtlPipelineStateDataManager::UniformInfoArray;
    using UniformHandle = GrGLSLProgramDataManager::UniformHandle;

    GrMtlPipelineState(
            GrMtlGpu* gpu,
            id<MTLRenderPipelineState> pipelineState,
            MTLPixelFormat pixelFormat,
            const GrGLSLBuiltinUniformHandles& builtinUniformHandles,
            const UniformInfoArray& uniforms,
            uint32_t geometryUniformBufferSize,
            uint32_t fragmentUniformBufferSize,
            uint32_t numSamplers,
            std::unique_ptr<GrGLSLPrimitiveProcessor> geometryProcessor,
            std::unique_ptr<GrGLSLXferProcessor> xferPRocessor,
            std::unique_ptr<std::unique_ptr<GrGLSLFragmentProcessor>[]> fragmentProcessors,
            int fFragmentProcessorCnt);

    id<MTLRenderPipelineState> mtlPipelineState() { return fPipelineState; }

    void setData(const GrRenderTarget*, GrSurfaceOrigin,
                 const GrPrimitiveProcessor& primPRoc, const GrPipeline& pipeline,
                 const GrTextureProxy* const primProcTextures[]);

    void setDrawState(id<MTLRenderCommandEncoder>, GrPixelConfig, const GrXferProcessor&);

    static void SetDynamicScissorRectState(id<MTLRenderCommandEncoder> renderCmdEncoder,
                                           const GrRenderTarget* renderTarget,
                                           GrSurfaceOrigin rtOrigin,
                                           SkIRect scissorRect);

private:
    /**
    * We use the RT's size and origin to adjust from Skia device space to Metal normalized device
    * space and to make device space positions have the correct origin for processors that require
    * them.
    */
    struct RenderTargetState {
        SkISize         fRenderTargetSize;
        GrSurfaceOrigin fRenderTargetOrigin;

        RenderTargetState() { this->invalidate(); }
        void invalidate() {
            fRenderTargetSize.fWidth = -1;
            fRenderTargetSize.fHeight = -1;
            fRenderTargetOrigin = (GrSurfaceOrigin)-1;
        }

        /**
        * Gets a float4 that adjusts the position from Skia device coords to Metals normalized
        * device coords. Assuming the transformed position, pos, is a homogeneous float3, the vec,
        * v, is applied as such:
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

    void setRenderTargetState(const GrRenderTarget*, GrSurfaceOrigin);

    void bind(id<MTLRenderCommandEncoder>);

    void setBlendConstants(id<MTLRenderCommandEncoder>, GrPixelConfig, const GrXferProcessor&);

    void setDepthStencilState(id<MTLRenderCommandEncoder> renderCmdEncoder);

    struct SamplerBindings {
        id<MTLSamplerState> fSampler;
        id<MTLTexture> fTexture;

        SamplerBindings(const GrSamplerState& state, GrTexture* texture, GrMtlGpu*);
    };

    GrMtlGpu* fGpu;
    id<MTLRenderPipelineState> fPipelineState;
    MTLPixelFormat             fPixelFormat;

    RenderTargetState fRenderTargetState;
    GrGLSLBuiltinUniformHandles fBuiltinUniformHandles;

    GrStencilSettings fStencil;

    int fNumSamplers;
    SkTArray<SamplerBindings> fSamplerBindings;

    std::unique_ptr<GrGLSLPrimitiveProcessor> fGeometryProcessor;
    std::unique_ptr<GrGLSLXferProcessor> fXferProcessor;
    std::unique_ptr<std::unique_ptr<GrGLSLFragmentProcessor>[]> fFragmentProcessors;
    int fFragmentProcessorCnt;

    GrMtlPipelineStateDataManager fDataManager;
};

#endif
