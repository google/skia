/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrContextPriv_DEFINED
#define GrContextPriv_DEFINED

#include "GrContext.h"
#include "GrSurfaceContext.h"

class GrSemaphore;
class GrSurfaceProxy;
class GrPreFlushCallbackObject;

/** Class that adds methods to GrContext that are only intended for use internal to Skia.
    This class is purely a privileged window into GrContext. It should never have additional
    data members or virtual methods. */
class GrContextPriv {
public:
    GrDrawingManager* drawingManager() { return fContext->fDrawingManager.get(); }

    // Create a renderTargetContext that wraps an existing renderTarget
    sk_sp<GrRenderTargetContext> makeWrappedRenderTargetContext(sk_sp<GrRenderTarget>,
                                                                sk_sp<SkColorSpace>,
                                                                const SkSurfaceProps* = nullptr);

    // Create a surfaceContext that wraps an existing texture or renderTarget
    sk_sp<GrSurfaceContext> makeWrappedSurfaceContext(sk_sp<GrSurface>);

    sk_sp<GrSurfaceContext> makeWrappedSurfaceContext(sk_sp<GrSurfaceProxy>, sk_sp<SkColorSpace>);

    sk_sp<GrSurfaceContext> makeDeferredSurfaceContext(const GrSurfaceDesc&,
                                                       SkBackingFit,
                                                       SkBudgeted);

    // TODO: Maybe add a 'surfaceProps' param (that is ignored for non-RTs) and remove
    // makeBackendTextureRenderTargetContext & makeBackendTextureAsRenderTargetRenderTargetContext
    sk_sp<GrSurfaceContext> makeBackendSurfaceContext(const GrBackendTextureDesc& desc,
                                                      sk_sp<SkColorSpace> colorSpace);

    sk_sp<GrRenderTargetContext> makeBackendTextureRenderTargetContext(
                                                         const GrBackendTextureDesc& desc,
                                                         sk_sp<SkColorSpace> colorSpace,
                                                         const SkSurfaceProps* = nullptr);

    sk_sp<GrRenderTargetContext> makeBackendRenderTargetRenderTargetContext(
                                                              const GrBackendRenderTargetDesc& desc,
                                                              sk_sp<SkColorSpace> colorSpace,
                                                              const SkSurfaceProps* = nullptr);

    sk_sp<GrRenderTargetContext> makeBackendTextureAsRenderTargetRenderTargetContext(
                                                                 const GrBackendTextureDesc& desc,
                                                                 sk_sp<SkColorSpace> colorSpace,
                                                                 const SkSurfaceProps* = nullptr);

    bool disableGpuYUVConversion() const { return fContext->fDisableGpuYUVConversion; }

    /**
     * Call to ensure all drawing to the context has been issued to the
     * underlying 3D API.
     * The 'proxy' parameter is a hint. If it is supplied the context will guarantee that
     * the draws required for that proxy are flushed but it could do more. If no 'proxy' is
     * provided then all current work will be flushed.
     */
    void flush(GrSurfaceProxy*);

    /*
     * A ref will be taken on the preFlushCallbackObject which will be removed when the
     * context is destroyed.
     */
    void addPreFlushCallbackObject(sk_sp<GrPreFlushCallbackObject>);

    /**
     * After this returns any pending writes to the surface will have been issued to the
     * backend 3D API.
     */
    void flushSurfaceWrites(GrSurfaceProxy*);

    /**
     * After this returns any pending reads or writes to the surface will have been issued to the
     * backend 3D API.
     */
    void flushSurfaceIO(GrSurfaceProxy*);

    /**
     * Finalizes all pending reads and writes to the surface and also performs an MSAA resolve
     * if necessary.
     *
     * It is not necessary to call this before reading the render target via Skia/GrContext.
     * GrContext will detect when it must perform a resolve before reading pixels back from the
     * surface or using it as a texture.
     */
    void prepareSurfaceForExternalIO(GrSurfaceProxy*);

   /**
    * These flags can be used with the read/write pixels functions below.
    */
    enum PixelOpsFlags {
        /** The GrContext will not be flushed before the surface read or write. This means that
            the read or write may occur before previous draws have executed. */
        kDontFlush_PixelOpsFlag = 0x1,
        /** Any surface writes should be flushed to the backend 3D API after the surface operation
            is complete */
        kFlushWrites_PixelOp = 0x2,
        /** The src for write or dst read is unpremultiplied. This is only respected if both the
            config src and dst configs are an RGBA/BGRA 8888 format. */
        kUnpremul_PixelOpsFlag  = 0x4,
    };

    /**
     * Reads a rectangle of pixels from a surface.
     * @param surface       the surface to read from.
     * @param srcColorSpace color space of the surface
     * @param left          left edge of the rectangle to read (inclusive)
     * @param top           top edge of the rectangle to read (inclusive)
     * @param width         width of rectangle to read in pixels.
     * @param height        height of rectangle to read in pixels.
     * @param config        the pixel config of the destination buffer
     * @param dstColorSpace color space of the destination buffer
     * @param buffer        memory to read the rectangle into.
     * @param rowBytes      number of bytes bewtween consecutive rows. Zero means rows are tightly
     *                      packed.
     * @param pixelOpsFlags see PixelOpsFlags enum above.
     *
     * @return true if the read succeeded, false if not. The read can fail because of an unsupported
     *         pixel configs
     */
    bool readSurfacePixels(GrSurfaceProxy* src, SkColorSpace* srcColorSpace,
                           int left, int top, int width, int height,
                           GrPixelConfig config, SkColorSpace* dstColorSpace, void* buffer,
                           size_t rowBytes = 0,
                           uint32_t pixelOpsFlags = 0);

    /**
     * Writes a rectangle of pixels to a surface.
     * @param dst           the surface to write to.
     * @param dstColorSpace color space of the surface
     * @param left          left edge of the rectangle to write (inclusive)
     * @param top           top edge of the rectangle to write (inclusive)
     * @param width         width of rectangle to write in pixels.
     * @param height        height of rectangle to write in pixels.
     * @param config        the pixel config of the source buffer
     * @param srcColorSpace color space of the source buffer
     * @param buffer        memory to read pixels from
     * @param rowBytes      number of bytes between consecutive rows. Zero
     *                      means rows are tightly packed.
     * @param pixelOpsFlags see PixelOpsFlags enum above.
     * @return true if the write succeeded, false if not. The write can fail because of an
     *         unsupported combination of surface and src configs.
     */
    bool writeSurfacePixels(GrSurfaceProxy* dst, SkColorSpace* dstColorSpace,
                            int left, int top, int width, int height,
                            GrPixelConfig config, SkColorSpace* srcColorSpace, const void* buffer,
                            size_t rowBytes,
                            uint32_t pixelOpsFlags = 0);

private:
    explicit GrContextPriv(GrContext* context) : fContext(context) {}
    GrContextPriv(const GrContextPriv&); // unimpl
    GrContextPriv& operator=(const GrContextPriv&); // unimpl

    // No taking addresses of this type.
    const GrContextPriv* operator&() const;
    GrContextPriv* operator&();

    GrContext* fContext;

    friend class GrContext; // to construct/copy this type.
};

inline GrContextPriv GrContext::contextPriv() { return GrContextPriv(this); }

inline const GrContextPriv GrContext::contextPriv () const {
    return GrContextPriv(const_cast<GrContext*>(this));
}

#endif
