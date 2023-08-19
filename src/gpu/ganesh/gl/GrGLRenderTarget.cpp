/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/gl/GrGLRenderTarget.h"

#include "include/core/SkTraceMemoryDump.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "src/gpu/ganesh/GrBackendUtils.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpuResourcePriv.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "src/gpu/ganesh/gl/GrGLGpu.h"
#include "src/gpu/ganesh/gl/GrGLUtil.h"

#define GPUGL static_cast<GrGLGpu*>(this->getGpu())
#define GL_CALL(X) GR_GL_CALL(GPUGL->glInterface(), X)
#define GL_CALL_RET(RET, X) GR_GL_CALL_RET(GPUGL->glInterface(), RET, X)

// Because this class is virtually derived from GrSurface we must explicitly call its constructor.
// Constructor for wrapped render targets.
GrGLRenderTarget::GrGLRenderTarget(GrGLGpu* gpu,
                                   const SkISize& dimensions,
                                   GrGLFormat format,
                                   int sampleCount,
                                   const IDs& ids,
                                   sk_sp<GrGLAttachment> stencil,
                                   skgpu::Protected isProtected,
                                   std::string_view label)
        : GrSurface(gpu, dimensions, isProtected, label)
        , GrRenderTarget(gpu, dimensions, sampleCount, isProtected, label, std::move(stencil)) {
    this->init(format, ids);
    this->setFlags(gpu->glCaps(), ids);
    this->registerWithCacheWrapped(GrWrapCacheable::kNo);
}

GrGLRenderTarget::GrGLRenderTarget(GrGLGpu* gpu,
                                   const SkISize& dimensions,
                                   GrGLFormat format,
                                   int sampleCount,
                                   const IDs& ids,
                                   skgpu::Protected isProtected,
                                   std::string_view label)
        : GrSurface(gpu, dimensions, isProtected, label)
        , GrRenderTarget(gpu, dimensions, sampleCount, isProtected, label) {
    this->init(format, ids);
    this->setFlags(gpu->glCaps(), ids);
}

inline void GrGLRenderTarget::setFlags(const GrGLCaps& glCaps, const IDs& idDesc) {
    if ((fMultisampleFBOID | fSingleSampleFBOID) == 0) {
        this->setGLRTFBOIDIs0();
    }
}

void GrGLRenderTarget::init(GrGLFormat format, const IDs& idDesc) {
    fMultisampleFBOID = idDesc.fMultisampleFBOID;
    fSingleSampleFBOID = idDesc.fSingleSampleFBOID;
    fMSColorRenderbufferID = idDesc.fMSColorRenderbufferID;
    fRTFBOOwnership = idDesc.fRTFBOOwnership;
    fRTFormat = format;
    fTotalMemorySamplesPerPixel = idDesc.fTotalMemorySamplesPerPixel;
}

GrGLFormat stencil_bits_to_format(int stencilBits) {
    SkASSERT(stencilBits);
    switch (stencilBits) {
        case 8:
            // We pick the packed format here so when we query total size we are at least not
            // underestimating the total size of the stencil buffer. However, in reality this
            // rarely matters since we usually don't care about the size of wrapped objects.
            return GrGLFormat::kDEPTH24_STENCIL8;
        case 16:
            return GrGLFormat::kSTENCIL_INDEX16;
        default:
            SkASSERT(false);
            return GrGLFormat::kUnknown;
    }
}

sk_sp<GrGLRenderTarget> GrGLRenderTarget::MakeWrapped(GrGLGpu* gpu,
                                                      const SkISize& dimensions,
                                                      GrGLFormat format,
                                                      int sampleCount,
                                                      const IDs& idDesc,
                                                      int stencilBits,
                                                      skgpu::Protected isProtected,
                                                      std::string_view label) {
    sk_sp<GrGLAttachment> sb;
    if (stencilBits) {
        // We pick a "fake" actual format that matches the number of stencil bits. When wrapping
        // an FBO with some number of stencil bits all we care about in the future is that we have
        // a format with the same number of stencil bits. We don't even directly use the format or
        // any other properties. Thus it is fine for us to just assign an arbitrary format that
        // matches the stencil bit count.
        GrGLFormat sFmt = stencil_bits_to_format(stencilBits);

        // We don't have the actual renderbufferID but we need to make an attachment for the stencil
        // so we just set it to an invalid value of 0 to make sure we don't explicitly use it or try
        // and delete it.
        sb = GrGLAttachment::MakeWrappedRenderBuffer(gpu,
                                                     /*renderbufferID=*/0,
                                                     dimensions,
                                                     GrAttachment::UsageFlags::kStencilAttachment,
                                                     sampleCount,
                                                     sFmt);
    }
    return sk_sp<GrGLRenderTarget>(new GrGLRenderTarget(
            gpu, dimensions, format, sampleCount, idDesc, std::move(sb), isProtected, label));
}

GrBackendRenderTarget GrGLRenderTarget::getBackendRenderTarget() const {
    bool useMultisampleFBO = (this->numSamples() > 1);
    GrGLFramebufferInfo fbi;
    fbi.fFBOID = (useMultisampleFBO) ? fMultisampleFBOID : fSingleSampleFBOID;
    fbi.fFormat = GrGLFormatToEnum(this->format());
    fbi.fProtected = skgpu::Protected(this->isProtected());
    int numStencilBits = 0;
    if (GrAttachment* stencil = this->getStencilAttachment(useMultisampleFBO)) {
        numStencilBits = GrBackendFormatStencilBits(stencil->backendFormat());
    }

    return GrBackendRenderTargets::MakeGL(
            this->width(), this->height(), this->numSamples(), numStencilBits, fbi);
}

GrBackendFormat GrGLRenderTarget::backendFormat() const {
    // We should never have a GrGLRenderTarget (even a textureable one with a target that is not
    // texture 2D.
    return GrBackendFormats::MakeGL(GrGLFormatToEnum(fRTFormat), GR_GL_TEXTURE_2D);
}

size_t GrGLRenderTarget::onGpuMemorySize() const {
    return GrSurface::ComputeSize(this->backendFormat(), this->dimensions(),
                                  fTotalMemorySamplesPerPixel, GrMipmapped::kNo);
}

void GrGLRenderTarget::onSetLabel() {
    SkASSERT(fMSColorRenderbufferID);
    SkASSERT(fRTFBOOwnership == GrBackendObjectOwnership::kOwned);
}

bool GrGLRenderTarget::completeStencilAttachment(GrAttachment* stencil, bool useMultisampleFBO) {
    // We defer attaching the new stencil buffer until the next time our framebuffer is bound.
    if (this->getStencilAttachment(useMultisampleFBO) != stencil) {
        fNeedsStencilAttachmentBind[useMultisampleFBO] = true;
    }
    return true;
}

bool GrGLRenderTarget::ensureDynamicMSAAAttachment() {
    SkASSERT(this->numSamples() == 1);
    if (fMultisampleFBOID) {
        return true;
    }
    SkASSERT(!fDynamicMSAAAttachment);

    GrResourceProvider* resourceProvider = this->getContext()->priv().resourceProvider();
    const GrCaps& caps = *this->getGpu()->caps();

    int internalSampleCount = caps.internalMultisampleCount(this->backendFormat());
    if (internalSampleCount <= 1) {
        return false;
    }

    if (resourceProvider->caps()->msaaResolvesAutomatically() && this->asTexture()) {
        // We can use EXT_multisampled_render_to_texture for MSAA. We will configure the FBO as MSAA
        // or not during bindFBO().
        fMultisampleFBOID = fSingleSampleFBOID;
        return true;
    }

    GL_CALL(GenFramebuffers(1, &fMultisampleFBOID));
    if (!fMultisampleFBOID) {
        return false;
    }

    this->getGLGpu()->bindFramebuffer(GR_GL_FRAMEBUFFER, fMultisampleFBOID);

    fDynamicMSAAAttachment.reset(
            static_cast<GrGLAttachment*>(resourceProvider->getDiscardableMSAAAttachment(
                    this->dimensions(), this->backendFormat(), internalSampleCount,
                    GrProtected(this->isProtected()), GrMemoryless::kNo).release()));
    if (!fDynamicMSAAAttachment) {
        return false;
    }

    GL_CALL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER, GR_GL_COLOR_ATTACHMENT0, GR_GL_RENDERBUFFER,
                                    fDynamicMSAAAttachment->renderbufferID()));
    return true;
}

void GrGLRenderTarget::bindInternal(GrGLenum fboTarget, bool useMultisampleFBO) {
    GrGLuint fboId = useMultisampleFBO ? fMultisampleFBOID : fSingleSampleFBOID;
    this->getGLGpu()->bindFramebuffer(fboTarget, fboId);

    if (fSingleSampleFBOID != 0 &&
        fSingleSampleFBOID == fMultisampleFBOID &&
        useMultisampleFBO != fDMSAARenderToTextureFBOIsMultisample) {
        auto* glTex = static_cast<GrGLTexture*>(this->asTexture());
        if (this->getGLGpu()->glCaps().bindTexture0WhenChangingTextureFBOMultisampleCount()) {
            GL_CALL(FramebufferTexture2D(fboTarget,
                                         GR_GL_COLOR_ATTACHMENT0,
                                         GR_GL_TEXTURE_2D,
                                         0 /*texture*/,
                                         0 /*mipMapLevel*/));
        }
        if (useMultisampleFBO) {
            int sampleCount = this->numSamples() > 1 ?
                    this->numSamples() :
                    this->getGpu()->caps()->internalMultisampleCount(this->backendFormat());
            GL_CALL(FramebufferTexture2DMultisample(fboTarget,
                                                    GR_GL_COLOR_ATTACHMENT0,
                                                    glTex->target(),
                                                    glTex->textureID(),
                                                    0 /*mipMapLevel*/,
                                                    sampleCount));
        } else {
            GL_CALL(FramebufferTexture2D(fboTarget,
                                         GR_GL_COLOR_ATTACHMENT0,
                                         glTex->target(),
                                         glTex->textureID(),
                                         0 /*mipMapLevel*/));
        }
        fDMSAARenderToTextureFBOIsMultisample = useMultisampleFBO;
        fNeedsStencilAttachmentBind[useMultisampleFBO] = true;
    }

    // Make sure the stencil attachment is valid. Even though a color buffer op doesn't use stencil,
    // our FBO still needs to be "framebuffer complete".
    if (fNeedsStencilAttachmentBind[useMultisampleFBO]) {
        if (auto stencil = this->getStencilAttachment(useMultisampleFBO)) {
            const GrGLAttachment* glStencil = static_cast<const GrGLAttachment*>(stencil);
            GL_CALL(FramebufferRenderbuffer(fboTarget,
                                            GR_GL_STENCIL_ATTACHMENT,
                                            GR_GL_RENDERBUFFER,
                                            glStencil->renderbufferID()));
            if (GrGLFormatIsPackedDepthStencil(glStencil->format())) {
                GL_CALL(FramebufferRenderbuffer(fboTarget,
                                                GR_GL_DEPTH_ATTACHMENT,
                                                GR_GL_RENDERBUFFER,
                                                glStencil->renderbufferID()));
            } else {
                GL_CALL(FramebufferRenderbuffer(fboTarget,
                                                GR_GL_DEPTH_ATTACHMENT,
                                                GR_GL_RENDERBUFFER,
                                                0));
            }
        } else {
            GL_CALL(FramebufferRenderbuffer(fboTarget,
                                            GR_GL_STENCIL_ATTACHMENT,
                                            GR_GL_RENDERBUFFER,
                                            0));
            GL_CALL(FramebufferRenderbuffer(fboTarget,
                                            GR_GL_DEPTH_ATTACHMENT,
                                            GR_GL_RENDERBUFFER,
                                            0));
        }
#ifdef SK_DEBUG
        if (!this->getGLGpu()->glCaps().skipErrorChecks() &&
            !this->getGLGpu()->glCaps().rebindColorAttachmentAfterCheckFramebufferStatus()) {
            GrGLenum status;
            GL_CALL_RET(status, CheckFramebufferStatus(fboTarget));
            if (status != GR_GL_FRAMEBUFFER_COMPLETE) {
                // This can fail if the context has been asynchronously abandoned (see
                // skbug.com/5200).
                SkDebugf("WARNING: failed to attach stencil.\n");
            }
        }
#endif
        fNeedsStencilAttachmentBind[useMultisampleFBO] = false;
    }
}

void GrGLRenderTarget::bindForResolve(GrGLGpu::ResolveDirection resolveDirection) {
    // If the multisample FBO is nonzero, it means we always have something to resolve (even if the
    // single sample buffer is FBO 0). If it's zero, then there's nothing to resolve.
    SkASSERT(fMultisampleFBOID != 0);

    // In the EXT_multisampled_render_to_texture case, we shouldn't be resolving anything.
    SkASSERT(!this->isMultisampledRenderToTexture());

    if (resolveDirection == GrGLGpu::ResolveDirection::kMSAAToSingle) {
        this->bindInternal(GR_GL_READ_FRAMEBUFFER, true);
        this->bindInternal(GR_GL_DRAW_FRAMEBUFFER, false);
    } else {
        SkASSERT(resolveDirection == GrGLGpu::ResolveDirection::kSingleToMSAA);
        SkASSERT(this->getGLGpu()->glCaps().canResolveSingleToMSAA());
        this->bindInternal(GR_GL_READ_FRAMEBUFFER, false);
        this->bindInternal(GR_GL_DRAW_FRAMEBUFFER, true);
    }
}

void GrGLRenderTarget::onRelease() {
    if (GrBackendObjectOwnership::kBorrowed != fRTFBOOwnership) {
        GrGLGpu* gpu = this->getGLGpu();
        if (fSingleSampleFBOID) {
            gpu->deleteFramebuffer(fSingleSampleFBOID);
        }
        if (fMultisampleFBOID && fMultisampleFBOID != fSingleSampleFBOID) {
            gpu->deleteFramebuffer(fMultisampleFBOID);
        }
        if (fMSColorRenderbufferID) {
            GL_CALL(DeleteRenderbuffers(1, &fMSColorRenderbufferID));
        }
    }
    fMultisampleFBOID       = 0;
    fSingleSampleFBOID      = 0;
    fMSColorRenderbufferID  = 0;
    INHERITED::onRelease();
}

void GrGLRenderTarget::onAbandon() {
    fMultisampleFBOID       = 0;
    fSingleSampleFBOID      = 0;
    fMSColorRenderbufferID  = 0;
    INHERITED::onAbandon();
}

GrGLGpu* GrGLRenderTarget::getGLGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrGLGpu*>(this->getGpu());
}

bool GrGLRenderTarget::canAttemptStencilAttachment(bool useMultisampleFBO) const {
    // This cap should have been handled at a higher level.
    SkASSERT(!this->getGpu()->getContext()->priv().caps()->avoidStencilBuffers());
    // Only modify the FBO's attachments if we have created the FBO. Public APIs do not currently
    // allow for borrowed FBO ownership, so we can safely assume that if an object is owned,
    // Skia created it.
    return this->fRTFBOOwnership == GrBackendObjectOwnership::kOwned ||
           // The dmsaa attachment is always owned and always supports adding stencil.
           (this->numSamples() == 1 && useMultisampleFBO);
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

    int numSamplesNotInTexture = fTotalMemorySamplesPerPixel;
    if (this->asTexture()) {
        --numSamplesNotInTexture;  // GrGLTexture::dumpMemoryStatistics accounts for 1 sample.
    }
    if (numSamplesNotInTexture >= 1) {
        size_t size = GrSurface::ComputeSize(this->backendFormat(), this->dimensions(),
                                             numSamplesNotInTexture, GrMipmapped::kNo);

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
