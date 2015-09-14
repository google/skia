/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLRenderTarget.h"

#include "GrRenderTargetPriv.h"
#include "GrGLGpu.h"
#include "GrGLUtil.h"

#define GPUGL static_cast<GrGLGpu*>(this->getGpu())
#define GL_CALL(X) GR_GL_CALL(GPUGL->glInterface(), X)

// Because this class is virtually derived from GrSurface we must explicitly call its constructor.
GrGLRenderTarget::GrGLRenderTarget(GrGLGpu* gpu,
                                   const GrSurfaceDesc& desc,
                                   const IDDesc& idDesc,
                                   GrGLStencilAttachment* stencil)
    : GrSurface(gpu, idDesc.fLifeCycle, desc)
    , INHERITED(gpu, idDesc.fLifeCycle, desc, idDesc.fSampleConfig, stencil) {
    this->init(desc, idDesc);
    this->registerWithCache();
}

GrGLRenderTarget::GrGLRenderTarget(GrGLGpu* gpu, const GrSurfaceDesc& desc, const IDDesc& idDesc,
                                   Derived)
    : GrSurface(gpu, idDesc.fLifeCycle, desc)
    , INHERITED(gpu, idDesc.fLifeCycle, desc, idDesc.fSampleConfig) {
    this->init(desc, idDesc);
}

void GrGLRenderTarget::init(const GrSurfaceDesc& desc, const IDDesc& idDesc) {
    fRTFBOID                = idDesc.fRTFBOID;
    fTexFBOID               = idDesc.fTexFBOID;
    fMSColorRenderbufferID  = idDesc.fMSColorRenderbufferID;
    fRTLifecycle            = idDesc.fLifeCycle;

    fViewport.fLeft   = 0;
    fViewport.fBottom = 0;
    fViewport.fWidth  = desc.fWidth;
    fViewport.fHeight = desc.fHeight;

    // We own one color value for each MSAA sample.
    int colorValuesPerPixel = SkTMax(1, fDesc.fSampleCnt);
    if (fTexFBOID != kUnresolvableFBOID && fTexFBOID != fRTFBOID) {
        // If we own the resolve buffer then that is one more sample per pixel.
        colorValuesPerPixel += 1;
    } else if (fTexFBOID != 0) {
        // For auto-resolving FBOs, the MSAA buffer is free.
        colorValuesPerPixel = 1;
    }
    SkASSERT(kUnknown_GrPixelConfig != fDesc.fConfig);
    SkASSERT(!GrPixelConfigIsCompressed(fDesc.fConfig));
    size_t colorBytes = GrBytesPerPixel(fDesc.fConfig);
    SkASSERT(colorBytes > 0);
    fGpuMemorySize = colorValuesPerPixel * fDesc.fWidth * fDesc.fHeight * colorBytes;

    SkASSERT(fGpuMemorySize <= WorseCaseSize(desc));
}

GrGLRenderTarget* GrGLRenderTarget::CreateWrapped(GrGLGpu* gpu,
                                                  const GrSurfaceDesc& desc,
                                                  const IDDesc& idDesc,
                                                  int stencilBits) {
    GrGLStencilAttachment* sb = nullptr;
    if (stencilBits) {
        GrGLStencilAttachment::IDDesc sbDesc;
        GrGLStencilAttachment::Format format;
        format.fInternalFormat = GrGLStencilAttachment::kUnknownInternalFormat;
        format.fPacked = false;
        format.fStencilBits = stencilBits;
        format.fTotalBits = stencilBits;
        // Owndership of sb is passed to the GrRenderTarget so doesn't need to be deleted
        sb = new GrGLStencilAttachment(gpu, sbDesc, desc.fWidth, desc.fHeight,
                                       desc.fSampleCnt, format);
    }
    return (new GrGLRenderTarget(gpu, desc, idDesc, sb));
}

size_t GrGLRenderTarget::onGpuMemorySize() const {
    return fGpuMemorySize;
}

bool GrGLRenderTarget::completeStencilAttachment() {
    GrGLGpu* gpu = this->getGLGpu();
    const GrGLInterface* interface = gpu->glInterface();
    GrStencilAttachment* stencil = this->renderTargetPriv().getStencilAttachment();
    if (nullptr == stencil) {
        if (this->renderTargetPriv().getStencilAttachment()) {
            GR_GL_CALL(interface, FramebufferRenderbuffer(GR_GL_FRAMEBUFFER,
                                                          GR_GL_STENCIL_ATTACHMENT,
                                                          GR_GL_RENDERBUFFER, 0));
            GR_GL_CALL(interface, FramebufferRenderbuffer(GR_GL_FRAMEBUFFER,
                                                          GR_GL_DEPTH_ATTACHMENT,
                                                          GR_GL_RENDERBUFFER, 0));
#ifdef SK_DEBUG
            GrGLenum status;
            GR_GL_CALL_RET(interface, status, CheckFramebufferStatus(GR_GL_FRAMEBUFFER));
            SkASSERT(GR_GL_FRAMEBUFFER_COMPLETE == status);
#endif
        }
        return true;
    } else {
        const GrGLStencilAttachment* glStencil = static_cast<const GrGLStencilAttachment*>(stencil);
        GrGLuint rb = glStencil->renderbufferID();

        gpu->invalidateBoundRenderTarget();
        gpu->stats()->incRenderTargetBinds();
        GR_GL_CALL(interface, BindFramebuffer(GR_GL_FRAMEBUFFER, this->renderFBOID()));
        GR_GL_CALL(interface, FramebufferRenderbuffer(GR_GL_FRAMEBUFFER,
                                                      GR_GL_STENCIL_ATTACHMENT,
                                                      GR_GL_RENDERBUFFER, rb));
        if (glStencil->format().fPacked) {
            GR_GL_CALL(interface, FramebufferRenderbuffer(GR_GL_FRAMEBUFFER,
                                                          GR_GL_DEPTH_ATTACHMENT,
                                                          GR_GL_RENDERBUFFER, rb));
        } else {
            GR_GL_CALL(interface, FramebufferRenderbuffer(GR_GL_FRAMEBUFFER,
                                                          GR_GL_DEPTH_ATTACHMENT,
                                                          GR_GL_RENDERBUFFER, 0));
        }

#ifdef SK_DEBUG
        GrGLenum status;
        GR_GL_CALL_RET(interface, status, CheckFramebufferStatus(GR_GL_FRAMEBUFFER));
        SkASSERT(GR_GL_FRAMEBUFFER_COMPLETE == status);
#endif
        return true;
    }
}

void GrGLRenderTarget::onRelease() {
    if (kBorrowed_LifeCycle != fRTLifecycle) {
        if (fTexFBOID) {
            GL_CALL(DeleteFramebuffers(1, &fTexFBOID));
        }
        if (fRTFBOID && fRTFBOID != fTexFBOID) {
            GL_CALL(DeleteFramebuffers(1, &fRTFBOID));
        }
        if (fMSColorRenderbufferID) {
            GL_CALL(DeleteRenderbuffers(1, &fMSColorRenderbufferID));
        }
    }
    fRTFBOID                = 0;
    fTexFBOID               = 0;
    fMSColorRenderbufferID  = 0;
    INHERITED::onRelease();
}

void GrGLRenderTarget::onAbandon() {
    fRTFBOID                = 0;
    fTexFBOID               = 0;
    fMSColorRenderbufferID  = 0;
    INHERITED::onAbandon();
}

GrGLGpu* GrGLRenderTarget::getGLGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrGLGpu*>(this->getGpu());
}

