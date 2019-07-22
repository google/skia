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
#include "include/gpu/GrRenderTarget.h"
#include "src/gpu/gl/GrGLIRect.h"

class GrGLCaps;
class GrGLGpu;
class GrGLStencilAttachment;

class GrGLRenderTarget : public GrRenderTarget {
public:
    bool alwaysClearStencil() const override { return 0 == fRTFBOID; }

    // set fTexFBOID to this value to indicate that it is multisampled but
    // Gr doesn't know how to resolve it.
    enum { kUnresolvableFBOID = 0 };

    struct IDDesc {
        GrGLuint                   fRTFBOID;
        GrBackendObjectOwnership   fRTFBOOwnership;
        GrGLuint                   fTexFBOID;
        GrGLuint                   fMSColorRenderbufferID;
    };

    static sk_sp<GrGLRenderTarget> MakeWrapped(GrGLGpu*,
                                               const GrSurfaceDesc&,
                                               int sampleCount,
                                               GrGLenum format,
                                               const IDDesc&,
                                               int stencilBits);

    // The following two functions return the same ID when a texture/render target is not
    // multisampled, and different IDs when it is multisampled.
    // FBO ID used to render into
    GrGLuint renderFBOID() const { return fRTFBOID; }
    // FBO ID that has texture ID attached.
    GrGLuint textureFBOID() const { return fTexFBOID; }

    // override of GrRenderTarget
    ResolveType getResolveType() const override {
        if (this->numSamples() <= 1 || fRTFBOID == fTexFBOID) {  // Also catches FBO 0.
            return kAutoResolves_ResolveType;
        } else if (kUnresolvableFBOID == fTexFBOID) {
            return kCantResolve_ResolveType;
        } else {
            return kCanResolve_ResolveType;
        }
    }

    GrBackendRenderTarget getBackendRenderTarget() const override;

    GrBackendFormat backendFormat() const override;

    bool canAttemptStencilAttachment() const override;

    // GrGLRenderTarget overrides dumpMemoryStatistics so it can log its texture and renderbuffer
    // components seperately.
    void dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const override;

protected:
    // Constructor for subclasses.
    GrGLRenderTarget(GrGLGpu*, const GrSurfaceDesc&, int sampleCount, GrGLenum format,
                     const IDDesc&);

    void init(const GrSurfaceDesc&, GrGLenum format, const IDDesc&);

    void onAbandon() override;
    void onRelease() override;

    int numSamplesOwnedPerPixel() const { return fNumSamplesOwnedPerPixel; }

private:
    // Constructor for instances wrapping backend objects.
    GrGLRenderTarget(GrGLGpu*, const GrSurfaceDesc&, int sampleCount, GrGLenum format,
                     const IDDesc&, GrGLStencilAttachment*);

    void setFlags(const GrGLCaps&, const IDDesc&);

    GrGLGpu* getGLGpu() const;
    bool completeStencilAttachment() override;

    size_t onGpuMemorySize() const override;

    int msaaSamples() const;
    // The number total number of samples, including both MSAA and resolve texture samples.
    int totalSamples() const;

    GrGLuint    fRTFBOID;
    GrGLuint    fTexFBOID;
    GrGLuint    fMSColorRenderbufferID;
    GrGLenum    fRTFormat;

    GrBackendObjectOwnership fRTFBOOwnership;

    // The RenderTarget needs to be able to report its VRAM footprint even after abandon and
    // release have potentially zeroed out the IDs (e.g., so the cache can reset itself). Since
    // the IDs are just required for the computation in totalSamples we cache that result here.
    int         fNumSamplesOwnedPerPixel;

    typedef GrRenderTarget INHERITED;
};

#endif
