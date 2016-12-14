/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSurfaceContext_DEFINED
#define GrSurfaceContext_DEFINED

#include "GrTypes.h"
#include "SkRefCnt.h"

class GrAuditTrail;
class GrContext;
class GrRenderTargetProxy;
class GrSingleOwner;
class GrSurface;
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

    virtual bool copySurface(GrSurface* src, const SkIRect& srcRect, const SkIPoint& dstPoint) = 0;

    // TODO: this is virtual b.c. this object doesn't have a pointer to the wrapped GrSurfaceProxy?
    virtual GrSurfaceProxy* asDeferredSurface() = 0;
    virtual GrTextureProxy* asDeferredTexture() = 0;
    virtual GrRenderTargetProxy* asDeferredRenderTarget() = 0;

    GrAuditTrail* auditTrail() { return fAuditTrail; }

    /**
     * Reads a rectangle of pixels from the surface.
     * @param left          left edge of the rectangle to read (inclusive)
     * @param top           top edge of the rectangle to read (inclusive)
     * @param width         width of rectangle to read in pixels.
     * @param height        height of rectangle to read in pixels.
     * @param config        the pixel config of the destination buffer
     * @param buffer        memory to read the rectangle into.
     * @param rowBytes      number of bytes between consecutive rows. Zero means rows are tightly
     *                      packed.
     * @param pixelOpsFlags See the GrContext::PixelOpsFlags enum.
     *
     * @return true if the read succeeded, false if not. The read can fail because of an unsupported
     *              pixel config.
     */
    bool readPixels(int left, int top, int width, int height,
                    GrPixelConfig config,
                    void* buffer,
                    size_t rowBytes = 0,
                    uint32_t pixelOpsFlags = 0);

    /**
     * Copy the src pixels [buffer, rowbytes, pixelconfig] into the surface at the specified
     * rectangle.
     * @param left          left edge of the rectangle to write (inclusive)
     * @param top           top edge of the rectangle to write (inclusive)
     * @param width         width of rectangle to write in pixels.
     * @param height        height of rectangle to write in pixels.
     * @param config        the pixel config of the source buffer
     * @param buffer        memory to read the rectangle from.
     * @param rowBytes      number of bytes between consecutive rows. Zero means rows are tightly
     *                      packed.
     * @param pixelOpsFlags See the GrContext::PixelOpsFlags enum.
     *
     * @return true if the write succeeded, false if not. The write can fail because of an
     *              unsupported pixel config.
     */
    bool writePixels(int left, int top, int width, int height,
                     GrPixelConfig config,
                     const void* buffer,
                     size_t rowBytes = 0,
                     uint32_t pixelOpsFlags = 0);

protected:
    GrSurfaceContext(GrContext*, GrAuditTrail*, GrSingleOwner*);

    SkDEBUGCODE(GrSingleOwner* singleOwner() { return fSingleOwner; })

    GrContext*            fContext;
    GrAuditTrail*         fAuditTrail;

    // In debug builds we guard against improper thread handling
    SkDEBUGCODE(mutable GrSingleOwner* fSingleOwner;)
};

#endif
