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

class GrGLGpu;
class GrGLStencilAttachment;

class GrGLRenderTarget : public GrRenderTarget {
public:
    // set fTexFBOID to this value to indicate that it is multisampled but
    // Gr doesn't know how to resolve it.
    enum { kUnresolvableFBOID = 0 };

    struct IDDesc {
        GrGLuint                     fRTFBOID;
        GrBackendObjectOwnership     fRTFBOOwnership;
        GrGLuint                     fTexFBOID;
        GrGLuint                     fMSColorRenderbufferID;
        GrRenderTarget::SampleConfig fSampleConfig;
    };

    static GrGLRenderTarget* CreateWrapped(GrGLGpu*,
                                           const GrSurfaceDesc&,
                                           const IDDesc&,
                                           int stencilBits);

    void setViewport(const GrGLIRect& rect) { fViewport = rect; }
    const GrGLIRect& getViewport() const { return fViewport; }

    // The following two functions return the same ID when a
    // texture/render target is multisampled, and different IDs when
    // it is.
    // FBO ID used to render into
    GrGLuint renderFBOID() const { return fRTFBOID; }
    // FBO ID that has texture ID attached.
    GrGLuint textureFBOID() const { return fTexFBOID; }

    // override of GrRenderTarget
    ResolveType getResolveType() const override {
        if (!this->isUnifiedMultisampled() ||
            fRTFBOID == fTexFBOID) {
            // catches FBO 0 and non MSAA case
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

    // In protected because subclass GrGLTextureRenderTarget calls this version.
    size_t onGpuMemorySize() const override;

private:
    // Constructor for instances wrapping backend objects.
    GrGLRenderTarget(GrGLGpu*, const GrSurfaceDesc&, const IDDesc&, GrGLStencilAttachment*);

    GrGLGpu* getGLGpu() const;
    bool completeStencilAttachment() override;

    // The total size of the resource (including all pixels) for a single sample.
    size_t totalBytesPerSample() const;
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

    // onGpuMemorySize() needs to know the VRAM footprint of the FBO(s). However, abandon and
    // release zero out the IDs and the cache needs to know the size even after those actions.
    size_t      fGpuMemorySize;

    typedef GrRenderTarget INHERITED;
};

#endif
