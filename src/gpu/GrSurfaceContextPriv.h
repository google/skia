/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSurfaceContextPriv_DEFINED
#define GrSurfaceContextPriv_DEFINED

#include "src/gpu/GrSurfaceContext.h"

/** Class that adds methods to GrSurfaceContext that are only intended for use internal to
    Skia. This class is purely a privileged window into GrSurfaceContext. It should never have
    additional data members or virtual methods. */
class GrSurfaceContextPriv {
public:
    GrRecordingContext* getContext() { return fSurfaceContext->fContext; }

    /*
     * Copy 'src' into the proxy backing this context. This call will not do any draw fallback.
     * Currently only writePixels and replaceRenderTarget call this directly. All other copies
     * should go through GrSurfaceProxy::Copy.
     * @param src       src of pixels
     * @param srcRect   the subset of 'src' to copy
     * @param dstPoint  the origin of the 'srcRect' in the destination coordinate space
     * @return          true if the copy succeeded; false otherwise
     *
     * Note: Notionally, 'srcRect' is clipped to 'src's extent with 'dstPoint' being adjusted.
     *       Then the 'srcRect' offset by 'dstPoint' is clipped against the dst's extent.
     *       The end result is only valid src pixels and dst pixels will be touched but the copied
     *       regions will not be shifted. The 'src' must have the same origin as the backing proxy
     *       of fSurfaceContext.
     */
    bool copyNoDraw(GrSurfaceProxy* src, const SkIRect& srcRect, const SkIPoint& dstPoint);

    bool copyNoDraw(GrSurfaceProxy* src) {
        return this->copyNoDraw(src, SkIRect::MakeWH(src->width(), src->height()),
                                SkIPoint::Make(0, 0));
    }

private:
    explicit GrSurfaceContextPriv(GrSurfaceContext* surfaceContext)
        : fSurfaceContext(surfaceContext) {
    }

    GrSurfaceContextPriv(const GrSurfaceContextPriv&) {} // unimpl
    GrSurfaceContextPriv& operator=(const GrSurfaceContextPriv&); // unimpl

    // No taking addresses of this type.
    const GrSurfaceContextPriv* operator&() const;
    GrSurfaceContextPriv* operator&();

    GrSurfaceContext* fSurfaceContext;

    friend class GrSurfaceContext; // to construct/copy this type.
};

inline GrSurfaceContextPriv GrSurfaceContext::surfPriv() {
    return GrSurfaceContextPriv(this);
}

inline const GrSurfaceContextPriv GrSurfaceContext::surfPriv() const {
    return GrSurfaceContextPriv(const_cast<GrSurfaceContext*>(this));
}

#endif
