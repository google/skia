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
#include "SkTraceMemoryDump.h"

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

    fGpuMemorySize = this->totalSamples() * this->totalBytesPerSample();

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

void GrGLRenderTarget::dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const {
    // Don't log the backing texture's contribution to the memory size. This will be handled by the
    // texture object.

    // Log any renderbuffer's contribution to memory. We only do this if we own the renderbuffer
    // (have a fMSColorRenderbufferID).
    if (fMSColorRenderbufferID) {
        size_t size = this->msaaSamples() * this->totalBytesPerSample();

        // Due to this resource having both a texture and a renderbuffer component, dump as
        // skia/gpu_resources/resource_#/renderbuffer
        SkString dumpName("skia/gpu_resources/resource_");
        dumpName.appendS32(this->getUniqueID());
        dumpName.append("/renderbuffer");

        traceMemoryDump->dumpNumericValue(dumpName.c_str(), "size", "bytes", size);

        if (this->isPurgeable()) {
            traceMemoryDump->dumpNumericValue(dumpName.c_str(), "purgeable_size", "bytes", size);
        }

        SkString renderbuffer_id;
        renderbuffer_id.appendU32(fMSColorRenderbufferID);
        traceMemoryDump->setMemoryBacking(dumpName.c_str(), "gl_renderbuffer",
                                          renderbuffer_id.c_str());
    }
}

size_t GrGLRenderTarget::totalBytesPerSample() const {
    SkASSERT(kUnknown_GrPixelConfig != fDesc.fConfig);
    SkASSERT(!GrPixelConfigIsCompressed(fDesc.fConfig));
    size_t colorBytes = GrBytesPerPixel(fDesc.fConfig);
    SkASSERT(colorBytes > 0);

    return fDesc.fWidth * fDesc.fHeight * colorBytes;
}

int GrGLRenderTarget::msaaSamples() const {
    if (fTexFBOID == kUnresolvableFBOID || fTexFBOID != fRTFBOID) {
        // If the render target's FBO is external (fTexFBOID == kUnresolvableFBOID), or if we own
        // the render target's FBO (fTexFBOID == fRTFBOID) then we use the provided sample count.
        return SkTMax(1, fDesc.fSampleCnt);
    }

    // When fTexFBOID == fRTFBOID, we either are not using MSAA, or MSAA is auto resolving, so use
    // 0 for the sample count.
    return 0;
}

int GrGLRenderTarget::totalSamples() const {
  int total_samples = this->msaaSamples();

  if (fTexFBOID != kUnresolvableFBOID) {
      // If we own the resolve buffer then that is one more sample per pixel.
      total_samples += 1;
  }

  return total_samples;
}
