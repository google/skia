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
#include "src/gpu/mtl/GrMtlPipeline.h"
#include "src/gpu/mtl/GrMtlPipelineStateDataManager.h"

#import <Metal/Metal.h>

class GrMtlFramebuffer;
class GrMtlGpu;
class GrMtlPipelineStateDataManager;
class GrMtlRenderCommandEncoder;
class GrMtlRenderPipeline;
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

    GrMtlPipelineState(GrMtlGpu*,
                       sk_sp<GrMtlRenderPipeline> pipeline,
                       MTLPixelFormat,
                       const GrGLSLBuiltinUniformHandles& builtinUniformHandles,
                       const UniformInfoArray& uniforms,
                       uint32_t uniformBufferSize,
                       uint32_t numSamplers,
                       std::unique_ptr<GrGeometryProcessor::ProgramImpl>,
                       std::unique_ptr<GrXferProcessor::ProgramImpl>,
                       std::vector<std::unique_ptr<GrFragmentProcessor::ProgramImpl>> fpImpls);

    const sk_sp<GrMtlRenderPipeline>& pipeline() const { return fPipeline; }

    void setData(GrMtlFramebuffer*, const GrProgramInfo&);

    void setTextures(const GrGeometryProcessor&,
                     const GrPipeline&,
                     const GrSurfaceProxy* const geomProcTextures[]);
    void bindTextures(GrMtlRenderCommandEncoder* renderCmdEncoder);

    void setDrawState(GrMtlRenderCommandEncoder*,
                      const GrSwizzle& writeSwizzle,
                      const GrXferProcessor&);

    static void SetDynamicScissorRectState(GrMtlRenderCommandEncoder* renderCmdEncoder,
                                           SkISize colorAttachmentDimensions,
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

    void setRenderTargetState(SkISize colorAttachmentDimensions, GrSurfaceOrigin);

    void bindUniforms(GrMtlRenderCommandEncoder*);

    void setBlendConstants(GrMtlRenderCommandEncoder*, const GrSwizzle&, const GrXferProcessor&);

    void setDepthStencilState(GrMtlRenderCommandEncoder* renderCmdEncoder);

    struct SamplerBindings {
        GrMtlSampler*  fSampler;
        id<MTLTexture> fTexture;

        SamplerBindings(GrSamplerState state, GrTexture* texture, GrMtlGpu*);
    };

    GrMtlGpu* fGpu;
    sk_sp<GrMtlRenderPipeline> fPipeline;
    MTLPixelFormat             fPixelFormat;

    RenderTargetState fRenderTargetState;
    GrGLSLBuiltinUniformHandles fBuiltinUniformHandles;

    GrStencilSettings fStencil;

    int fNumSamplers;
    SkTArray<SamplerBindings> fSamplerBindings;

    std::unique_ptr<GrGeometryProcessor::ProgramImpl>              fGPImpl;
    std::unique_ptr<GrXferProcessor::ProgramImpl>                  fXPImpl;
    std::vector<std::unique_ptr<GrFragmentProcessor::ProgramImpl>> fFPImpls;

    GrMtlPipelineStateDataManager fDataManager;
};

#endif
