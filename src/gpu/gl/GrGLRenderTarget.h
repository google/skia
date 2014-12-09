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

class GrGpuGL;

class GrGLRenderTarget : public GrRenderTarget {
public:
    // set fTexFBOID to this value to indicate that it is multisampled but
    // Gr doesn't know how to resolve it.
    enum { kUnresolvableFBOID = 0 };

    struct IDDesc {
        GrGLuint         fRTFBOID;
        GrGLuint         fTexFBOID;
        GrGLuint         fMSColorRenderbufferID;
        bool             fIsWrapped;
    };

    GrGLRenderTarget(GrGpuGL*, const GrSurfaceDesc&, const IDDesc&);

    virtual ~GrGLRenderTarget() { this->release(); }

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
    virtual GrBackendObject getRenderTargetHandle() const { return this->renderFBOID(); }
    virtual GrBackendObject getRenderTargetResolvedHandle() const { return this->textureFBOID(); }
    virtual ResolveType getResolveType() const {
        if (!this->isMultisampled() ||
            fRTFBOID == fTexFBOID) {
            // catches FBO 0 and non MSAA case
            return kAutoResolves_ResolveType;
        } else if (kUnresolvableFBOID == fTexFBOID) {
            return kCantResolve_ResolveType;
        } else {
            return kCanResolve_ResolveType;
        }
    }

    virtual size_t gpuMemorySize() const SK_OVERRIDE;

protected:
    // The public constructor registers this object with the cache. However, only the most derived
    // class should register with the cache. This constructor does not do the registration and
    // rather moves that burden onto the derived class.
    enum Derived { kDerived };
    GrGLRenderTarget(GrGpuGL*, const GrSurfaceDesc&, const IDDesc&, Derived);

    void init(const GrSurfaceDesc&, const IDDesc&);

    virtual void onAbandon() SK_OVERRIDE;
    virtual void onRelease() SK_OVERRIDE;

private:
    GrGLuint      fRTFBOID;
    GrGLuint      fTexFBOID;
    GrGLuint      fMSColorRenderbufferID;

    // We track this separately from GrGpuResource because this may be both a texture and a render
    // target, and the texture may be wrapped while the render target is not.
    bool fIsWrapped;

    // when we switch to this render target we want to set the viewport to
    // only render to content area (as opposed to the whole allocation) and
    // we want the rendering to be at top left (GL has origin in bottom left)
    GrGLIRect fViewport;

    // gpuMemorySize() needs to know what how many color values are owned per pixel. However,
    // abandon and release zero out the IDs and the cache needs to know the size even after those
    // actions.
    uint8_t fColorValuesPerPixel;

    typedef GrRenderTarget INHERITED;
};

#endif
