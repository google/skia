/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSurfaceContext_DEFINED
#define GrSurfaceContext_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/gpu/GrColorSpaceInfo.h"
#include "src/gpu/GrSurfaceProxy.h"

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

   /**
    * These flags can be used with the read/write pixels functions below.
    */
    enum PixelOpsFlags {
        /** The src for write or dst read is unpremultiplied. This is only respected if both the
            config src and dst configs are an RGBA/BGRA 8888 format. */
        kUnpremul_PixelOpsFlag  = 0x4,
    };

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

#if GR_TEST_UTILS
    // Accessors for tests to directly call read/writePixelsImpl
    bool writePixels(GrContext* direct, int left, int top, int width, int height,
                     GrColorType srcColorType, SkColorSpace* srcColorSpace,
                     const void* srcBuffer, size_t srcRowBytes = 0, uint32_t pixelOpsFlags = 0) {
        return writePixelsImpl(direct, left, top, width, height, srcColorType, srcColorSpace,
                               srcBuffer, srcRowBytes, pixelOpsFlags);
    }

    bool readPixels(GrContext* direct, int left, int top, int width, int height,
                    GrColorType dstColorType, SkColorSpace* dstColorSpace, void* buffer,
                    size_t rowBytes = 0, uint32_t pixelOpsFlags = 0) {
        return readPixelsImpl(direct, left, top, width, height, dstColorType, dstColorSpace,
                              buffer, rowBytes, pixelOpsFlags);
    }
#endif

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

#if GR_TEST_UTILS
    bool testCopy(GrSurfaceProxy* src, const SkIRect& srcRect, const SkIPoint& dstPoint) {
        return this->copy(src, srcRect, dstPoint);
    }

    bool testCopy(GrSurfaceProxy* src) {
        return this->copy(src);
    }
#endif


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
    friend class GrSurfaceProxy; // for copy

    /**
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
    bool copy(GrSurfaceProxy* src, const SkIRect& srcRect, const SkIPoint& dstPoint);

    bool copy(GrSurfaceProxy* src) {
        return this->copy(src, SkIRect::MakeWH(src->width(), src->height()), SkIPoint::Make(0, 0));
    }

    bool writePixelsImpl(GrContext* direct, int left, int top, int width, int height,
                         GrColorType srcColorType, SkColorSpace* srcColorSpace,
                         const void* srcBuffer, size_t srcRowBytes, uint32_t pixelOpsFlags);

    bool readPixelsImpl(GrContext* direct, int left, int top, int width,
                        int height, GrColorType dstColorType,
                        SkColorSpace* dstColorSpace, void* buffer, size_t rowBytes,
                        uint32_t pixelOpsFlags);

    GrColorSpaceInfo    fColorSpaceInfo;

    typedef SkRefCnt INHERITED;
};

#endif
