/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrGLRenderTarget_DEFINED
#define GrGLRenderTarget_DEFINED

#include "include/core/SkScalar.h"
#include "include/gpu/GrBackendSurface.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/gl/GrGLDefines.h"

class GrGLCaps;
class GrGLGpu;
class GrGLAttachment;

class GrGLRenderTarget : public GrRenderTarget {
public:
    using GrSurface::glRTFBOIDis0;
    bool alwaysClearStencil() const override { return this->glRTFBOIDis0(); }

    // set fSingleSampleFBOID to this value to indicate that it is multisampled but
    // Gr doesn't know how to resolve it.
    enum { kUnresolvableFBOID = 0 };

    struct IDs {
        GrGLuint                   fMultisampleFBOID;
        GrBackendObjectOwnership   fRTFBOOwnership;
        GrGLuint                   fSingleSampleFBOID;
        GrGLuint                   fMSColorRenderbufferID;
        int                        fTotalMemorySamplesPerPixel;
    };

    static sk_sp<GrGLRenderTarget> MakeWrapped(GrGLGpu*,
                                               const SkISize&,
                                               GrGLFormat,
                                               int sampleCount,
                                               const IDs&,
                                               int stencilBits);

    bool isFBO0(bool multisample) const {
        return (multisample ? fMultisampleFBOID : fSingleSampleFBOID) == 0;
    }

    bool isMultisampledRenderToTexture() const {
        return fMultisampleFBOID != 0 && fMultisampleFBOID == fSingleSampleFBOID;
    }

    GrBackendRenderTarget getBackendRenderTarget() const override;

    GrBackendFormat backendFormat() const override;

    bool canAttemptStencilAttachment(bool useMultisampleFBO) const override;

    // GrGLRenderTarget overrides dumpMemoryStatistics so it can log its texture and renderbuffer
    // components separately.
    void dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const override;

    GrGLFormat format() const { return fRTFormat; }

    bool hasDynamicMSAAAttachment() const { return SkToBool(fDynamicMSAAAttachment); }
    bool ensureDynamicMSAAAttachment();

    // Binds the render target to GL_FRAMEBUFFER for rendering.
    void bind(bool useMultisampleFBO) {
        this->bindInternal(GR_GL_FRAMEBUFFER, useMultisampleFBO);
    }

    // Binds the render target for copying, reading, or clearing pixel values. If we are an MSAA
    // render target with a separate resolve texture, we bind the multisampled FBO. Otherwise we
    // bind the single sample FBO.
    void bindForPixelOps(GrGLenum fboTarget) {
        this->bindInternal(fboTarget,
                           this->numSamples() > 1 && !this->isMultisampledRenderToTexture());
    }

    enum class ResolveDirection : bool {
        kSingleToMSAA,  // glCaps.canResolveSingleToMSAA() must be true.
        kMSAAToSingle
    };

    // Binds the multisampled and single sample FBOs, one to GL_DRAW_FRAMEBUFFER and the other to
    // GL_READ_FRAMEBUFFER, depending on ResolveDirection.
    void bindForResolve(ResolveDirection);

protected:
    // Constructor for subclasses.
    GrGLRenderTarget(GrGLGpu*,
                     const SkISize&,
                     GrGLFormat,
                     int sampleCount,
                     const IDs&);

    void init(GrGLFormat, const IDs&);

    // Binds the render target to the given target and ensures its stencil attachment is valid.
    void bindInternal(GrGLenum fboTarget, bool useMultisampleFBO);

    void onAbandon() override;
    void onRelease() override;

    int totalMemorySamplesPerPixel() const { return fTotalMemorySamplesPerPixel; }

private:
    // Constructor for instances wrapping backend objects.
    GrGLRenderTarget(
            GrGLGpu*, const SkISize&, GrGLFormat, int sampleCount, const IDs&,
            sk_sp<GrGLAttachment> stencil);

    void setFlags(const GrGLCaps&, const IDs&);

    GrGLGpu* getGLGpu() const;
    bool completeStencilAttachment(GrAttachment* stencil, bool useMultisampleFBO) override;

    size_t onGpuMemorySize() const override;

    sk_sp<GrGLAttachment> fDynamicMSAAAttachment;

    GrGLuint    fMultisampleFBOID;
    GrGLuint    fSingleSampleFBOID;
    GrGLuint    fMSColorRenderbufferID;
    GrGLFormat  fRTFormat;
    bool        fNeedsStencilAttachmentBind[2] = {false, false};
    bool        fDMSAARenderToTextureFBOIsMultisample = false;

    GrBackendObjectOwnership fRTFBOOwnership;

    // The RenderTarget needs to be able to report its VRAM footprint even after abandon and
    // release have potentially zeroed out the IDs (e.g., so the cache can reset itself). Since
    // the IDs are just required for the computation in totalSamples we cache that result here.
    int fTotalMemorySamplesPerPixel;

    using INHERITED = GrRenderTarget;
};

#endif
