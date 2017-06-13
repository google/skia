/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrGLRenderTarget_DEFINED
#define GrGLRenderTarget_DEFINED

#include "GrGLIRect.h"
#include "GrRenderTarget.h"
#include "SkScalar.h"

class GrGLCaps;
class GrGLGpu;
class GrGLStencilAttachment;

class GrGLRenderTarget : public GrRenderTarget {
public:
    // set fTexFBOID to this value to indicate that it is multisampled but
    // Gr doesn't know how to resolve it.
    enum { kUnresolvableFBOID = 0 };

    struct IDDesc {
        GrGLuint                   fRTFBOID;
        GrBackendObjectOwnership   fRTFBOOwnership;
        GrGLuint                   fTexFBOID;
        GrGLuint                   fMSColorRenderbufferID;
        bool                       fIsMixedSampled;
    };

    static sk_sp<GrGLRenderTarget> MakeWrapped(GrGLGpu*,
                                               const GrSurfaceDesc&,
                                               const IDDesc&,
                                               int stencilBits);

    void setViewport(const GrGLIRect& rect) { fViewport = rect; }
    const GrGLIRect& getViewport() const { return fViewport; }

    // The following two functions return the same ID when a texture/render target is not
    // multisampled, and different IDs when it is multisampled.
    // FBO ID used to render into
    GrGLuint renderFBOID() const { return fRTFBOID; }
    // FBO ID that has texture ID attached.
    GrGLuint textureFBOID() const { return fTexFBOID; }

    // override of GrRenderTarget
    ResolveType getResolveType() const override {
        if (GrFSAAType::kUnifiedMSAA != this->fsaaType() || fRTFBOID == fTexFBOID) {
            // catches FBO 0 and non unified-MSAA case
            return kAutoResolves_ResolveType;
        } else if (kUnresolvableFBOID == fTexFBOID) {
            return kCantResolve_ResolveType;
        } else {
            return kCanResolve_ResolveType;
        }
    }

    GrBackendObject getRenderTargetHandle() const override { return fRTFBOID; }

    bool canAttemptStencilAttachment() const override;

    // GrGLRenderTarget overrides dumpMemoryStatistics so it can log its texture and renderbuffer
    // components seperately.
    void dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const override;

protected:
    // Constructor for subclasses.
    GrGLRenderTarget(GrGLGpu*, const GrSurfaceDesc&, const IDDesc&);

    void init(const GrSurfaceDesc&, const IDDesc&);

    void onAbandon() override;
    void onRelease() override;

    int numSamplesOwnedPerPixel() const { return fNumSamplesOwnedPerPixel; }

private:
    // Constructor for instances wrapping backend objects.
    GrGLRenderTarget(GrGLGpu*, const GrSurfaceDesc&, const IDDesc&, GrGLStencilAttachment*);

    static GrRenderTargetFlags ComputeFlags(const GrGLCaps&, const IDDesc&);

    GrGLGpu* getGLGpu() const;
    bool completeStencilAttachment() override;

    size_t onGpuMemorySize() const override;

    int msaaSamples() const;
    // The number total number of samples, including both MSAA and resolve texture samples.
    int totalSamples() const;

    GrGLuint    fRTFBOID;
    GrGLuint    fTexFBOID;
    GrGLuint    fMSColorRenderbufferID;

    GrBackendObjectOwnership fRTFBOOwnership;

    // when we switch to this render target we want to set the viewport to
    // only render to content area (as opposed to the whole allocation) and
    // we want the rendering to be at top left (GL has origin in bottom left)
    GrGLIRect   fViewport;

    // The RenderTarget needs to be able to report its VRAM footprint even after abandon and
    // release have potentially zeroed out the IDs (e.g., so the cache can reset itself). Since
    // the IDs are just required for the computation in totalSamples we cache that result here.
    int         fNumSamplesOwnedPerPixel;

    typedef GrRenderTarget INHERITED;
};

#endif
