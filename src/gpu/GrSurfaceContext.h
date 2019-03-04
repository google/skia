/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSurfaceContext_DEFINED
#define GrSurfaceContext_DEFINED

#include "../private/GrSurfaceProxy.h"
#include "GrColorSpaceInfo.h"
#include "SkRefCnt.h"

class GrAuditTrail;
class GrDrawingManager;
class GrOpList;
class GrRecordingContext;
class GrRenderTargetContext;
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

    const GrColorSpaceInfo& colorSpaceInfo() const { return fColorSpaceInfo; }

    // TODO: these two calls would be way cooler if this object had a GrSurfaceProxy pointer
    int width() const { return this->asSurfaceProxy()->width(); }
    int height() const { return this->asSurfaceProxy()->height(); }

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
    bool copy(GrSurfaceProxy* src, const SkIRect& srcRect, const SkIPoint& dstPoint);

    bool copy(GrSurfaceProxy* src) {
        return this->copy(src,
                          SkIRect::MakeWH(src->width(), src->height()),
                          SkIPoint::Make(0, 0));
    }

    /**
     * Reads a rectangle of pixels from the render target context.
     * @param dstInfo       image info for the destination
     * @param dstBuffer     destination pixels for the read
     * @param dstRowBytes   bytes in a row of 'dstBuffer'
     * @param x             x offset w/in the render target context from which to read
     * @param y             y offset w/in the render target context from which to read
     *
     * @return true if the read succeeded, false if not. The read can fail because of an
     *              unsupported pixel config.
     */
    bool readPixels(const SkImageInfo& dstInfo, void* dstBuffer, size_t dstRowBytes,
                    int x, int y, uint32_t flags = 0);

    /**
     * Writes a rectangle of pixels [srcInfo, srcBuffer, srcRowbytes] into the
     * renderTargetContext at the specified position.
     * @param srcInfo       image info for the source pixels
     * @param srcBuffer     source for the write
     * @param srcRowBytes   bytes in a row of 'srcBuffer'
     * @param x             x offset w/in the render target context at which to write
     * @param y             y offset w/in the render target context at which to write
     *
     * @return true if the write succeeded, false if not. The write can fail because of an
     *              unsupported pixel config.
     */
    bool writePixels(const SkImageInfo& srcInfo, const void* srcBuffer, size_t srcRowBytes,
                     int x, int y, uint32_t flags = 0);

    // TODO: this is virtual b.c. this object doesn't have a pointer to the wrapped GrSurfaceProxy?
    virtual GrSurfaceProxy* asSurfaceProxy() = 0;
    virtual const GrSurfaceProxy* asSurfaceProxy() const = 0;
    virtual sk_sp<GrSurfaceProxy> asSurfaceProxyRef() = 0;

    virtual GrTextureProxy* asTextureProxy() = 0;
    virtual const GrTextureProxy* asTextureProxy() const = 0;
    virtual sk_sp<GrTextureProxy> asTextureProxyRef() = 0;

    virtual GrRenderTargetProxy* asRenderTargetProxy() = 0;
    virtual sk_sp<GrRenderTargetProxy> asRenderTargetProxyRef() = 0;

    virtual GrRenderTargetContext* asRenderTargetContext() { return nullptr; }

    GrAuditTrail* auditTrail();

    // Provides access to functions that aren't part of the public API.
    GrSurfaceContextPriv surfPriv();
    const GrSurfaceContextPriv surfPriv() const;

protected:
    friend class GrSurfaceContextPriv;

    GrSurfaceContext(GrRecordingContext*, GrPixelConfig, sk_sp<SkColorSpace>);

    GrDrawingManager* drawingManager();
    const GrDrawingManager* drawingManager() const;

    virtual GrOpList* getOpList() = 0;
    SkDEBUGCODE(virtual void validate() const = 0;)

    SkDEBUGCODE(GrSingleOwner* singleOwner();)

    GrRecordingContext* fContext;

private:
    GrColorSpaceInfo    fColorSpaceInfo;

    typedef SkRefCnt INHERITED;
};

#endif
