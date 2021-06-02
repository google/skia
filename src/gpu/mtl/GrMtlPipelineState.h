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

#import <Metal/Metal.h>

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
            GrMtlGpu*,
            id<MTLRenderPipelineState>,
            MTLPixelFormat,
            const GrGLSLBuiltinUniformHandles& builtinUniformHandles,
            const UniformInfoArray& uniforms,
            uint32_t uniformBufferSize,
            uint32_t numSamplers,
            std::unique_ptr<GrGLSLGeometryProcessor>,
            std::unique_ptr<GrGLSLXferProcessor>,
            std::vector<std::unique_ptr<GrGLSLFragmentProcessor>> fpImpls);

    id<MTLRenderPipelineState> mtlPipelineState() { return fPipelineState; }

    void setData(const GrRenderTarget*, const GrProgramInfo&);

    void setTextures(const GrGeometryProcessor&,
                     const GrPipeline&,
                     const GrSurfaceProxy* const geomProcTextures[]);
    void bindTextures(id<MTLRenderCommandEncoder> renderCmdEncoder);

    void setDrawState(id<MTLRenderCommandEncoder>,
                      const GrSwizzle& writeSwizzle,
                      const GrXferProcessor&);

    static void SetDynamicScissorRectState(id<MTLRenderCommandEncoder> renderCmdEncoder,
                                           const GrRenderTarget* renderTarget,
                                           GrSurfaceOrigin rtOrigin,
                                           SkIRect scissorRect);

    bool doesntSampleAttachment(const MTLRenderPassAttachmentDescriptor*) const;

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
    };

    void setRenderTargetState(const GrRenderTarget*, GrSurfaceOrigin);

    void bindUniforms(id<MTLRenderCommandEncoder>);

    void setBlendConstants(id<MTLRenderCommandEncoder>, const GrSwizzle&, const GrXferProcessor&);

    void setDepthStencilState(id<MTLRenderCommandEncoder> renderCmdEncoder);

    struct SamplerBindings {
        GrMtlSampler*  fSampler;
        id<MTLTexture> fTexture;

        SamplerBindings(GrSamplerState state, GrTexture* texture, GrMtlGpu*);
    };

    GrMtlGpu* fGpu;
    id<MTLRenderPipelineState> fPipelineState;
    MTLPixelFormat             fPixelFormat;

    RenderTargetState fRenderTargetState;
    GrGLSLBuiltinUniformHandles fBuiltinUniformHandles;

    GrStencilSettings fStencil;

    int fNumSamplers;
    SkTArray<SamplerBindings> fSamplerBindings;

    std::unique_ptr<GrGLSLGeometryProcessor> fGeometryProcessor;
    std::unique_ptr<GrGLSLXferProcessor> fXferProcessor;
    std::vector<std::unique_ptr<GrGLSLFragmentProcessor>> fFPImpls;

    GrMtlPipelineStateDataManager fDataManager;
};

#endif
