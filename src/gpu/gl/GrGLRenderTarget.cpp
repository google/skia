/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTraceMemoryDump.h"
#include "include/gpu/GrContext.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpuResourcePriv.h"
#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/gl/GrGLGpu.h"
#include "src/gpu/gl/GrGLRenderTarget.h"
#include "src/gpu/gl/GrGLUtil.h"

#define GPUGL static_cast<GrGLGpu*>(this->getGpu())
#define GL_CALL(X) GR_GL_CALL(GPUGL->glInterface(), X)

// Because this class is virtually derived from GrSurface we must explicitly call its constructor.
// Constructor for wrapped render targets.
GrGLRenderTarget::GrGLRenderTarget(GrGLGpu* gpu,
                                   const GrSurfaceDesc& desc,
                                   GrGLenum format,
                                   const IDDesc& idDesc,
                                   GrGLStencilAttachment* stencil)
    : GrSurface(gpu, desc, GrProtected::kNo)
    , INHERITED(gpu, desc, GrProtected::kNo, stencil) {
    this->setFlags(gpu->glCaps(), idDesc);
    this->init(desc, format, idDesc);
    this->registerWithCacheWrapped(GrWrapCacheable::kNo);
}

GrGLRenderTarget::GrGLRenderTarget(GrGLGpu* gpu, const GrSurfaceDesc& desc, GrGLenum format,
                                   const IDDesc& idDesc)
    : GrSurface(gpu, desc, GrProtected::kNo)
    , INHERITED(gpu, desc, GrProtected::kNo) {
    this->setFlags(gpu->glCaps(), idDesc);
    this->init(desc, format, idDesc);
}

inline void GrGLRenderTarget::setFlags(const GrGLCaps& glCaps, const IDDesc& idDesc) {
    if (!idDesc.fRTFBOID) {
        this->setGLRTFBOIDIs0();
    }
}

void GrGLRenderTarget::init(const GrSurfaceDesc& desc, GrGLenum format, const IDDesc& idDesc) {
    fRTFBOID                = idDesc.fRTFBOID;
    fTexFBOID               = idDesc.fTexFBOID;
    fMSColorRenderbufferID  = idDesc.fMSColorRenderbufferID;
    fRTFBOOwnership         = idDesc.fRTFBOOwnership;

    fRTFormat               = format;

    fNumSamplesOwnedPerPixel = this->totalSamples();
}

sk_sp<GrGLRenderTarget> GrGLRenderTarget::MakeWrapped(GrGLGpu* gpu,
                                                      const GrSurfaceDesc& desc,
                                                      GrGLenum format,
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
        // Ownership of sb is passed to the GrRenderTarget so doesn't need to be deleted
        sb = new GrGLStencilAttachment(gpu, sbDesc, desc.fWidth, desc.fHeight,
                                       desc.fSampleCnt, format);
    }
    return sk_sp<GrGLRenderTarget>(new GrGLRenderTarget(gpu, desc, format, idDesc, sb));
}

GrBackendRenderTarget GrGLRenderTarget::getBackendRenderTarget() const {
    GrGLFramebufferInfo fbi;
    fbi.fFBOID = fRTFBOID;
    fbi.fFormat = this->getGLGpu()->glCaps().configSizedInternalFormat(this->config());
    int numStencilBits = 0;
    if (GrStencilAttachment* stencil = this->renderTargetPriv().getStencilAttachment()) {
        numStencilBits = stencil->bits();
    }

    return GrBackendRenderTarget(
            this->width(), this->height(), this->numSamples(), numStencilBits, fbi);
}

GrBackendFormat GrGLRenderTarget::backendFormat() const {
    // We should never have a GrGLRenderTarget (even a textureable one with a target that is not
    // texture 2D.
    return GrBackendFormat::MakeGL(fRTFormat, GR_GL_TEXTURE_2D);
}

size_t GrGLRenderTarget::onGpuMemorySize() const {
    return GrSurface::ComputeSize(this->config(), this->width(), this->height(),
                                  fNumSamplesOwnedPerPixel, GrMipMapped::kNo);
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
        if (kChromium_GrGLDriver != gpu->glContext().driver()) {
            // This check can cause problems in Chromium if the context has been asynchronously
            // abandoned (see skbug.com/5200)
            GrGLenum status;
            GR_GL_CALL_RET(interface, status, CheckFramebufferStatus(GR_GL_FRAMEBUFFER));
            SkASSERT(GR_GL_FRAMEBUFFER_COMPLETE == status);
        }
#endif
        return true;
    } else {
        const GrGLStencilAttachment* glStencil = static_cast<const GrGLStencilAttachment*>(stencil);
        GrGLuint rb = glStencil->renderbufferID();

        gpu->invalidateBoundRenderTarget();
        gpu->bindFramebuffer(GR_GL_FRAMEBUFFER, this->renderFBOID());
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
        if (kChromium_GrGLDriver != gpu->glContext().driver()) {
            // This check can cause problems in Chromium if the context has been asynchronously
            // abandoned (see skbug.com/5200)
            GrGLenum status;
            GR_GL_CALL_RET(interface, status, CheckFramebufferStatus(GR_GL_FRAMEBUFFER));
            SkASSERT(GR_GL_FRAMEBUFFER_COMPLETE == status);
        }
#endif
        return true;
    }
}

void GrGLRenderTarget::onRelease() {
    if (GrBackendObjectOwnership::kBorrowed != fRTFBOOwnership) {
        GrGLGpu* gpu = this->getGLGpu();
        if (fTexFBOID) {
            gpu->deleteFramebuffer(fTexFBOID);
        }
        if (fRTFBOID && fRTFBOID != fTexFBOID) {
            gpu->deleteFramebuffer(fRTFBOID);
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

bool GrGLRenderTarget::canAttemptStencilAttachment() const {
    if (this->getGpu()->getContext()->priv().caps()->avoidStencilBuffers()) {
        return false;
    }

    // Only modify the FBO's attachments if we have created the FBO. Public APIs do not currently
    // allow for borrowed FBO ownership, so we can safely assume that if an object is owned,
    // Skia created it.
    return this->fRTFBOOwnership == GrBackendObjectOwnership::kOwned;
}

void GrGLRenderTarget::dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const {
    // Don't check this->fRefsWrappedObjects, as we might be the base of a GrGLTextureRenderTarget
    // which is multiply inherited from both ourselves and a texture. In these cases, one part
    // (texture, rt) may be wrapped, while the other is owned by Skia.
    bool refsWrappedRenderTargetObjects =
            this->fRTFBOOwnership == GrBackendObjectOwnership::kBorrowed;
    if (refsWrappedRenderTargetObjects && !traceMemoryDump->shouldDumpWrappedObjects()) {
        return;
    }

    // Don't log the framebuffer, as the framebuffer itself doesn't contribute to meaningful
    // memory usage. It is always a wrapper around either:
    // - a texture, which is owned elsewhere, and will be dumped there
    // - a renderbuffer, which will be dumped below.

    // Log any renderbuffer's contribution to memory.
    if (fMSColorRenderbufferID) {
        size_t size = GrSurface::ComputeSize(this->config(), this->width(), this->height(),
                                             this->msaaSamples(), GrMipMapped::kNo);

        // Due to this resource having both a texture and a renderbuffer component, dump as
        // skia/gpu_resources/resource_#/renderbuffer
        SkString resourceName = this->getResourceName();
        resourceName.append("/renderbuffer");

        this->dumpMemoryStatisticsPriv(traceMemoryDump, resourceName, "RenderTarget", size);

        SkString renderbuffer_id;
        renderbuffer_id.appendU32(fMSColorRenderbufferID);
        traceMemoryDump->setMemoryBacking(resourceName.c_str(), "gl_renderbuffer",
                                          renderbuffer_id.c_str());
    }
}

int GrGLRenderTarget::msaaSamples() const {
    if (fTexFBOID == kUnresolvableFBOID || fTexFBOID != fRTFBOID) {
        // If the render target's FBO is external (fTexFBOID == kUnresolvableFBOID), or if we own
        // the render target's FBO (fTexFBOID == fRTFBOID) then we use the provided sample count.
        return this->numSamples();
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
