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

    GrGLuint singleSampleFBOID() const { return fSingleSampleFBOID; }
    GrGLuint multisampleFBOID() const { return fMultisampleFBOID; }

    GrBackendRenderTarget getBackendRenderTarget() const override;

    GrBackendFormat backendFormat() const override;

    bool canAttemptStencilAttachment(bool useMultisampleFBO) const override;

    // GrGLRenderTarget overrides dumpMemoryStatistics so it can log its texture and renderbuffer
    // components separately.
    void dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const override;

    GrGLFormat format() const { return fRTFormat; }

    bool hasDynamicMSAAAttachment() const { return SkToBool(fDynamicMSAAAttachment); }
    bool ensureDynamicMSAAAttachment();

protected:
    // Constructor for subclasses.
    GrGLRenderTarget(GrGLGpu*,
                     const SkISize&,
                     GrGLFormat,
                     int sampleCount,
                     const IDs&);

    void init(GrGLFormat, const IDs&);

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

    GrBackendObjectOwnership fRTFBOOwnership;

    // The RenderTarget needs to be able to report its VRAM footprint even after abandon and
    // release have potentially zeroed out the IDs (e.g., so the cache can reset itself). Since
    // the IDs are just required for the computation in totalSamples we cache that result here.
    int fTotalMemorySamplesPerPixel;

    using INHERITED = GrRenderTarget;
};

#endif
