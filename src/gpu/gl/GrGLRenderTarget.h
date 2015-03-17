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

/** Represents a GL FBO object. It has a gen ID which is valid whenever the FBO ID owned by the
    object is valid. The gen IDs are not recycled after FBOs are freed, unlike FBO IDs, and so
    can be used to uniquely identity FBO ID instantiations. If this object owns an FBO ID, the ID
    must be deleted or abandoned before this object is freed. FBO IDs should never be owned by
    more than one instance. */
class GrGLFBO : public SkNVRefCnt<GrGLFBO> {
public:
    SK_DECLARE_INST_COUNT(GrGLFBO);

    /** Initializes to an FBO. The FBO should already be valid in the relevant GL context. */
    GrGLFBO(GrGLint id) : fID(id), fIsValid(true) {}

    /** Initializes to an FBO ID generated using the interface. */
    GrGLFBO(const GrGLInterface* gl) {
        GR_GL_CALL(gl, GenFramebuffers(1, &fID));
        fIsValid = SkToBool(fID);
    }

    ~GrGLFBO() { SkASSERT(!this->isValid()); }

    /** Has this object been released or abandoned? */
    bool isValid() const { return fIsValid; }
    
    GrGLint fboID() const { SkASSERT(this->isValid()); return fID; }

    bool isDefaultFramebuffer() const { return fIsValid && 0 == fID; }

    /** Give up ownership of the FBO ID owned by this object without deleting it. */
    void abandon();

    /** Delete and give up ownership of the the FBO ID if it is valid. */
    void release(const GrGLInterface*);

private:
    static uint32_t NextGenID() {
        static int32_t gGenID = SK_InvalidGenID + 1;
        return static_cast<uint32_t>(sk_atomic_inc(&gGenID));
    }

    GrGLuint    fID;
    bool        fIsValid;

    typedef SkRefCnt INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

/** GL-specific subclass of GrRenderTarget. */
class GrGLRenderTarget : public GrRenderTarget {
public:
    struct IDDesc {
        SkAutoTUnref<GrGLFBO>       fRenderFBO;
        SkAutoTUnref<GrGLFBO>       fTextureFBO;
        GrGLuint                    fMSColorRenderbufferID;
        GrGpuResource::LifeCycle    fLifeCycle;
    };

    GrGLRenderTarget(GrGLGpu*, const GrSurfaceDesc&, const IDDesc&);

    void setViewport(const GrGLIRect& rect) { fViewport = rect; }
    const GrGLIRect& getViewport() const { return fViewport; }

    // For multisampled renderbuffer render targets, these will return different GrGLFBO objects. If
    // the render target is not texturable, textureFBO() returns NULL. If the render target auto
    // resolves to a texture, the same object is returned.

    // FBO that should be rendered into. Always non-NULL unless this resource is destroyed
    // (this->wasDestroyed()).
    const GrGLFBO* renderFBO() const {
        SkASSERT(fRenderFBO && fRenderFBO->isValid());
        return fRenderFBO;
    }

    // FBO that has the target's texture ID attached. The return value may be:
    //      * NULL when this render target is not a texture,
    //      * the same as renderFBO() when this surface is not multisampled or auto-resolves,
    //      * or different than renderFBO() when it requires explicit resolving via
    //        glBlitFramebuffer.
    const GrGLFBO* textureFBO() const {
        SkASSERT(!fTextureFBO || fTextureFBO->isValid());
        return fTextureFBO;
    }

    // override of GrRenderTarget
    ResolveType getResolveType() const SK_OVERRIDE {
        if (!this->isMultisampled() || this->renderFBO() == this->textureFBO()) {
            // catches FBO 0 and non MSAA case
            return kAutoResolves_ResolveType;
        } else if (!this->textureFBO()) {
            return kCantResolve_ResolveType;
        } else {
            return kCanResolve_ResolveType;
        }
    }

    /** When we don't own the FBO ID we don't attempt to modify its attachments. */
    bool canAttemptStencilAttachment() const SK_OVERRIDE { return !fIsWrapped; }

protected:
    // The public constructor registers this object with the cache. However, only the most derived
    // class should register with the cache. This constructor does not do the registration and
    // rather moves that burden onto the derived class.
    enum Derived { kDerived };
    GrGLRenderTarget(GrGLGpu*, const GrSurfaceDesc&, const IDDesc&, Derived);

    void init(const GrSurfaceDesc&, const IDDesc&);

    void onAbandon() SK_OVERRIDE;
    void onRelease() SK_OVERRIDE;

    // In protected because subclass GrGLTextureRenderTarget calls this version.
    size_t onGpuMemorySize() const SK_OVERRIDE;

private:
    SkAutoTUnref<GrGLFBO>   fRenderFBO;
    SkAutoTUnref<GrGLFBO>   fTextureFBO;
    GrGLuint                fMSColorRenderbufferID;

    // We track this separately from GrGpuResource because this may be both a texture and a render
    // target, and the texture may be wrapped while the render target is not.
    bool                    fIsWrapped;

    // when we switch to this render target we want to set the viewport to
    // only render to content area (as opposed to the whole allocation) and
    // we want the rendering to be at top left (GL has origin in bottom left)
    GrGLIRect               fViewport;

    // onGpuMemorySize() needs to know what how many color values are owned per pixel. However,
    // abandon and release zero out the IDs and the cache needs to know the size even after those
    // actions.
    uint8_t                 fColorValuesPerPixel;

    typedef GrRenderTarget INHERITED;
};

#endif
