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
class GrGLTexture;
class GrGLTexID;

class GrGLRenderTarget : public GrRenderTarget {

public:
    // set fTexFBOID to this value to indicate that it is multisampled but
    // Gr doesn't know how to resolve it.
    enum { kUnresolvableFBOID = 0 };

    struct Desc {
        GrGLuint         fRTFBOID;
        GrGLuint         fTexFBOID;
        GrGLuint         fMSColorRenderbufferID;
        bool             fIsWrapped;
        GrPixelConfig    fConfig;
        int              fSampleCnt;
        GrSurfaceOrigin  fOrigin;
    };

    // creates a GrGLRenderTarget associated with a texture
    GrGLRenderTarget(GrGpuGL*          gpu,
                     const Desc&       desc,
                     const GrGLIRect&  viewport,
                     GrGLTexID*        texID,
                     GrGLTexture*      texture);

    // creates an independent GrGLRenderTarget
    GrGLRenderTarget(GrGpuGL*          gpu,
                     const Desc&       desc,
                     const GrGLIRect&  viewport);

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
    virtual GrBackendObject getRenderTargetHandle() const {
        return this->renderFBOID();
    }
    virtual GrBackendObject getRenderTargetResolvedHandle() const {
        return this->textureFBOID();
    }
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

protected:
    // override of GrResource
    virtual void onAbandon() SK_OVERRIDE;
    virtual void onRelease() SK_OVERRIDE;

private:
    GrGLuint      fRTFBOID;
    GrGLuint      fTexFBOID;

    GrGLuint      fMSColorRenderbufferID;

    // when we switch to this render target we want to set the viewport to
    // only render to to content area (as opposed to the whole allocation) and
    // we want the rendering to be at top left (GL has origin in bottom left)
    GrGLIRect fViewport;

    // non-NULL if this RT was created by Gr with an associated GrGLTexture.
    GrGLTexID* fTexIDObj;

    void init(const Desc& desc, const GrGLIRect& viewport, GrGLTexID* texID);

    typedef GrRenderTarget INHERITED;
};

#endif
