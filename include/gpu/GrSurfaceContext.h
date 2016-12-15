/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSurfaceContext_DEFINED
#define GrSurfaceContext_DEFINED

#include "../private/GrSurfaceProxy.h"

#include "SkRefCnt.h"

class GrAuditTrail;
class GrContext;
class GrRenderTargetProxy;
class GrSingleOwner;
class GrSurface;
class GrSurfaceContextPriv;
class GrSurfaceProxy;
class GrTextureProxy;
struct SkIPoint;
struct SkIRect;

/**
 * A helper object to orchestrate commands for a particular surface
 */
class SK_API GrSurfaceContext : public SkRefCnt {
public:
    ~GrSurfaceContext() override {}

    /*
     * Copy 'src' into the proxy backing this context
     * @param src       src of pixels
     * @param srcRect   the subset of 'src' to copy
     * @param dstPoint  the origin of the 'srcRect' in the destination coordinate space
     * @return          true if the copy succeeded; false otherwise
     *
     * Note: Notionally, 'srcRect' is clipped to 'src's extent with 'dstPoint' being adjusted.
     *       Then the 'srcRect' offset by 'dstPoint' is clipped against the dst's extent.
     *       The end result is only valid src pixels and dst pixels will be touched but the copied
     *       regions will not be shifted.
     */
    bool copy(GrSurfaceProxy* src, const SkIRect& srcRect, const SkIPoint& dstPoint) {
        return this->onCopy(src, srcRect, dstPoint);
    }

    bool copy(GrSurfaceProxy* src) {
        return this->onCopy(src,
                            SkIRect::MakeWH(src->width(), src->height()),
                            SkIPoint::Make(0, 0));
    }

    // TODO: this is virtual b.c. this object doesn't have a pointer to the wrapped GrSurfaceProxy?
    virtual GrSurfaceProxy* asDeferredSurface() = 0;
    virtual GrTextureProxy* asDeferredTexture() = 0;
    virtual GrRenderTargetProxy* asDeferredRenderTarget() = 0;

    GrAuditTrail* auditTrail() { return fAuditTrail; }

    // Provides access to functions that aren't part of the public API.
    GrSurfaceContextPriv surfPriv();
    const GrSurfaceContextPriv surfPriv() const;

protected:
    friend class GrSurfaceContextPriv;

    GrSurfaceContext(GrContext*, GrAuditTrail*, GrSingleOwner*);

    SkDEBUGCODE(GrSingleOwner* singleOwner() { return fSingleOwner; })

    GrContext*            fContext;
    GrAuditTrail*         fAuditTrail;

    // In debug builds we guard against improper thread handling
    SkDEBUGCODE(mutable GrSingleOwner* fSingleOwner;)

private:
    virtual bool onCopy(GrSurfaceProxy* src,
                        const SkIRect& srcRect,
                        const SkIPoint& dstPoint) = 0;

    typedef SkRefCnt INHERITED;
};

#endif
