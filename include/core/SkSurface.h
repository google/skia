/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSurface_DEFINED
#define SkSurface_DEFINED

#include "SkRefCnt.h"
#include "SkImage.h"
#include "SkSurfaceProps.h"

#include "GrTypes.h"

class SkCanvas;
class SkDeferredDisplayList;
class SkPaint;
class SkSurfaceCharacterization;
class GrBackendRenderTarget;
class GrBackendSemaphore;
class GrContext;
class GrRenderTarget;

/**
 *  SkSurface is responsible for managing the pixels that a canvas draws into. The pixels can be
 *  allocated either in CPU memory (a Raster surface) or on the GPU (a RenderTarget surface).
 *
 *  SkSurface takes care of allocating a SkCanvas that will draw into the surface. Call
 *  surface->getCanvas() to use that canvas (but don't delete it, it is owned by the surface).
 *
 *  SkSurface always has non-zero dimensions. If there is a request for a new surface, and either
 *  of the requested dimensions are zero, then NULL will be returned.
 */
class SK_API SkSurface : public SkRefCnt {
public:
    /**
     *  Create a new surface, using the specified pixels/rowbytes as its
     *  backend.
     *
     *  If the requested surface cannot be created, or the request is not a
     *  supported configuration, NULL will be returned.
     *
     *  Callers are responsible for initialiazing the surface pixels.
     */
    static sk_sp<SkSurface> MakeRasterDirect(const SkImageInfo& imageInfo, void* pixels,
                                             size_t rowBytes,
                                             const SkSurfaceProps* surfaceProps = nullptr);

    /**
     *  The same as NewRasterDirect, but also accepts a call-back routine, which is invoked
     *  when the surface is deleted, and is passed the pixel memory and the specified context.
     */
    static sk_sp<SkSurface> MakeRasterDirectReleaseProc(const SkImageInfo& imageInfo, void* pixels,
                                    size_t rowBytes,
                                    void (*releaseProc)(void* pixels, void* context),
                                    void* context, const SkSurfaceProps* surfaceProps = nullptr);

    /**
     *  Return a new surface, with the memory for the pixels automatically allocated and
     *  zero-initialized, but respecting the specified rowBytes. If rowBytes==0, then a default
     *  value will be chosen. If a non-zero rowBytes is specified, then any images snapped off of
     *  this surface (via makeImageSnapshot()) are guaranteed to have the same rowBytes.
     *
     *  If the requested surface cannot be created, or the request is not a
     *  supported configuration, NULL will be returned.
     */
    static sk_sp<SkSurface> MakeRaster(const SkImageInfo& imageInfo, size_t rowBytes,
                                       const SkSurfaceProps* surfaceProps);

    /**
     *  Allocate a new surface, automatically computing the rowBytes.
     */
    static sk_sp<SkSurface> MakeRaster(const SkImageInfo& imageInfo,
                                       const SkSurfaceProps* props = nullptr) {
        return MakeRaster(imageInfo, 0, props);
    }

    /**
     *  Helper version of NewRaster. It creates a SkImageInfo with the
     *  specified width and height, and populates the rest of info to match
     *  pixels in SkPMColor format.
     */
    static sk_sp<SkSurface> MakeRasterN32Premul(int width, int height,
                                                const SkSurfaceProps* surfaceProps = nullptr) {
        return MakeRaster(SkImageInfo::MakeN32Premul(width, height), surfaceProps);
    }

    /**
     *  Used to wrap a pre-existing backend 3D API texture as a SkSurface. Skia will not assume
     *  ownership of the texture and the client must ensure the texture is valid for the lifetime
     *  of the SkSurface. If sampleCnt > 0, then we will create an intermediate mssa surface which
     *  we will use for rendering. We then resolve into the passed in texture.
     */
    static sk_sp<SkSurface> MakeFromBackendTexture(GrContext* context,
                                                   const GrBackendTexture& backendTexture,
                                                   GrSurfaceOrigin origin, int sampleCnt,
                                                   sk_sp<SkColorSpace> colorSpace,
                                                   const SkSurfaceProps* surfaceProps);

    /**
     *  Used to wrap a pre-existing backend 3D API texture as a SkSurface. Skia will not assume
     *  ownership of the texture and the client must ensure the texture is valid for the lifetime
     *  of the SkSurface. If sampleCnt > 0, then we will create an intermediate mssa surface which
     *  we will use for rendering. We then resolve into the passed in texture.
     *
     *  The GrBackendTexture must have a valid backend format supplied (GrGLTextureInfo::fFormat,
     *  GrVkImageInfo::fFormat, etc.) in it. The passed in SkColorType informs skia how it should
     *  interpret the backend format supplied by the GrBackendTexture. If the format in the
     *  GrBackendTexture is not compitable with the sampleCnt, SkColorType, and SkColorSpace we
     *  will return nullptr.
     */
    static sk_sp<SkSurface> MakeFromBackendTexture(GrContext* context,
                                                   const GrBackendTexture& backendTexture,
                                                   GrSurfaceOrigin origin, int sampleCnt,
                                                   SkColorType colorType,
                                                   sk_sp<SkColorSpace> colorSpace,
                                                   const SkSurfaceProps* surfaceProps);

    static sk_sp<SkSurface> MakeFromBackendRenderTarget(GrContext* context,
                                                const GrBackendRenderTarget& backendRenderTarget,
                                                GrSurfaceOrigin origin,
                                                sk_sp<SkColorSpace> colorSpace,
                                                const SkSurfaceProps* surfaceProps);

    /**
     *  The GrBackendRenderTarget must have a valid backend format set (GrGLTextureInfo::fFormat,
     *  GrVkImageInfo::fFormat, etc.) in it. The passed in SkColorType informs skia how it should
     *  interpret the backend format supplied by the GrBackendRenderTarget. If the format in the
     *  GrBackendRenderTarget is not compitable with the sampleCnt, SkColorType, and SkColorSpace
     *  we will return nullptr.
     */
    static sk_sp<SkSurface> MakeFromBackendRenderTarget(GrContext* context,
                                                const GrBackendRenderTarget& backendRenderTarget,
                                                GrSurfaceOrigin origin,
                                                SkColorType colorType,
                                                sk_sp<SkColorSpace> colorSpace,
                                                const SkSurfaceProps* surfaceProps);

    /**
     *  Used to wrap a pre-existing 3D API texture as a SkSurface. Skia will treat the texture as
     *  a rendering target only, but unlike NewFromBackendRenderTarget, Skia will manage and own
     *  the associated render target objects (but not the provided texture). Skia will not assume
     *  ownership of the texture and the client must ensure the texture is valid for the lifetime
     *  of the SkSurface.
     */
    static sk_sp<SkSurface> MakeFromBackendTextureAsRenderTarget(GrContext* context,
                                                            const GrBackendTexture& backendTexture,
                                                            GrSurfaceOrigin origin,
                                                            int sampleCnt,
                                                            sk_sp<SkColorSpace> colorSpace,
                                                            const SkSurfaceProps* surfaceProps);

    /**
     *  Used to wrap a pre-existing 3D API texture as a SkSurface. Skia will treat the texture as
     *  a rendering target only, but unlike NewFromBackendRenderTarget, Skia will manage and own
     *  the associated render target objects (but not the provided texture). Skia will not assume
     *  ownership of the texture and the client must ensure the texture is valid for the lifetime
     *  of the SkSurface.
     *
     *  The GrBackendTexture must have a valid backend format supplied (GrGLTextureInfo::fFormat,
     *  GrVkImageInfo::fFormat, etc.) in it. The passed in SkColorType informs skia how it should
     *  interpret the backend format supplied by the GrBackendTexture. If the format in the
     *  GrBackendTexture is not compitable with the sampleCnt, SkColorType, and SkColorSpace we
     *  will return nullptr.
     */
    static sk_sp<SkSurface> MakeFromBackendTextureAsRenderTarget(GrContext* context,
                                                            const GrBackendTexture& backendTexture,
                                                            GrSurfaceOrigin origin,
                                                            int sampleCnt,
                                                            SkColorType colorType,
                                                            sk_sp<SkColorSpace> colorSpace,
                                                            const SkSurfaceProps* surfaceProps);

    /**
     *  Return a new surface whose contents will be drawn to an offscreen
     *  render target, allocated by the surface. The optional shouldCreateWithMips flag is a hint
     *  that this surface may be snapped to an SkImage which will be used with mip maps so we should
     *  create the backend gpu RenderTarget with mips to avoid a copy later on.
     */
    static sk_sp<SkSurface> MakeRenderTarget(GrContext* context, SkBudgeted budgeted,
                                             const SkImageInfo& imageInfo,
                                             int sampleCount, GrSurfaceOrigin surfaceOrigin,
                                             const SkSurfaceProps* surfaceProps,
                                             bool shouldCreateWithMips = false);

    static sk_sp<SkSurface> MakeRenderTarget(GrContext* context, SkBudgeted budgeted,
                                             const SkImageInfo& imageInfo, int sampleCount,
                                             const SkSurfaceProps* props) {
        return MakeRenderTarget(context, budgeted, imageInfo, sampleCount,
                                kBottomLeft_GrSurfaceOrigin, props);
    }

    static sk_sp<SkSurface> MakeRenderTarget(GrContext* context, SkBudgeted budgeted,
                                             const SkImageInfo& imageInfo) {
        if (!imageInfo.width() || !imageInfo.height()) {
            return nullptr;
        }
        return MakeRenderTarget(context, budgeted, imageInfo, 0, kBottomLeft_GrSurfaceOrigin,
                                nullptr);
    }

    /**
     *  Returns a surface that stores no pixels. It can be drawn to via its canvas, but that
     *  canvas does not draw anything. Calling makeImageSnapshot() will return nullptr.
     */
    static sk_sp<SkSurface> MakeNull(int width, int height);

    int width() const { return fWidth; }
    int height() const { return fHeight; }

    /**
     *  Returns a unique non-zero, unique value identifying the content of this
     *  surface. Each time the content is changed changed, either by drawing
     *  into this surface, or explicitly calling notifyContentChanged()) this
     *  method will return a new value.
     *
     *  If this surface is empty (i.e. has a zero-dimention), this will return
     *  0.
     */
    uint32_t generationID();

    /**
     *  Modes that can be passed to notifyContentWillChange
     */
    enum ContentChangeMode {
        /**
         *  Use this mode if it is known that the upcoming content changes will
         *  clear or overwrite prior contents, thus making them discardable.
         */
        kDiscard_ContentChangeMode,
        /**
         *  Use this mode if prior surface contents need to be preserved or
         *  if in doubt.
         */
        kRetain_ContentChangeMode,
    };

    /**
     *  Call this if the contents are about to change. This will (lazily) force a new
     *  value to be returned from generationID() when it is called next.
     *
     *  CAN WE DEPRECATE THIS?
     */
    void notifyContentWillChange(ContentChangeMode mode);

    enum BackendHandleAccess {
        kFlushRead_BackendHandleAccess,     //!< caller may read from the backend object
        kFlushWrite_BackendHandleAccess,    //!< caller may write to the backend object
        kDiscardWrite_BackendHandleAccess,  //!< caller must over-write the entire backend object
    };

    /*
     * These are legacy aliases which will be removed soon
     */
    static const BackendHandleAccess kFlushRead_TextureHandleAccess =
            kFlushRead_BackendHandleAccess;
    static const BackendHandleAccess kFlushWrite_TextureHandleAccess =
            kFlushWrite_BackendHandleAccess;
    static const BackendHandleAccess kDiscardWrite_TextureHandleAccess =
            kDiscardWrite_BackendHandleAccess;


    /**
     *  Retrieves the backend API handle of the texture used by this surface, or 0 if the surface
     *  is not backed by a GPU texture.
     *
     *  The returned texture-handle is only valid until the next draw-call into the surface,
     *  or the surface is deleted.
     */
    GrBackendObject getTextureHandle(BackendHandleAccess backendHandleAccess);

    /**
     *  Retrieves the backend API handle of the RenderTarget backing this surface.  Callers must
     *  ensure this function returns 'true' or else the GrBackendObject will be invalid
     *
     *  In OpenGL this will return the FramebufferObject ID.
     */
    bool getRenderTargetHandle(GrBackendObject* backendObject,
                               BackendHandleAccess backendHandleAccess);

    /**
     *  Return a canvas that will draw into this surface. This will always
     *  return the same canvas for a given surface, and is manged/owned by the
     *  surface. It should not be used when its parent surface has gone out of
     *  scope.
     */
    SkCanvas* getCanvas();

    /**
     *  Return a new surface that is "compatible" with this one, in that it will
     *  efficiently be able to be drawn into this surface. Typical calling
     *  pattern:
     *
     *  SkSurface* A = SkSurface::New...();
     *  SkCanvas* canvasA = surfaceA->newCanvas();
     *  ...
     *  SkSurface* surfaceB = surfaceA->newSurface(...);
     *  SkCanvas* canvasB = surfaceB->newCanvas();
     *  ... // draw using canvasB
     *  canvasA->drawSurface(surfaceB); // <--- this will always be optimal!
     */
    sk_sp<SkSurface> makeSurface(const SkImageInfo& imageInfo);

    /**
     *  Returns an image of the current state of the surface pixels up to this
     *  point. Subsequent changes to the surface (by drawing into its canvas)
     *  will not be reflected in this image. For the GPU-backend, the budgeting
     *  decision for the snapped image will match that of the surface.
     */
    sk_sp<SkImage> makeImageSnapshot();

    /**
     *  Though the caller could get a snapshot image explicitly, and draw that,
     *  it seems that directly drawing a surface into another canvas might be
     *  a common pattern, and that we could possibly be more efficient, since
     *  we'd know that the "snapshot" need only live until we've handed it off
     *  to the canvas.
     */
    void draw(SkCanvas* canvas, SkScalar x, SkScalar y, const SkPaint* paint);

    /**
     *  If the surface has direct access to its pixels (i.e. they are in local
     *  RAM) return true, and if not null, set the pixmap parameter to point to the information
     *  about the surface's pixels. The pixel address in the pixmap is only valid while
     *  the surface object is in scope, and no API call is made on the surface
     *  or its canvas.
     *
     *  On failure, returns false and the pixmap parameter is ignored.
     */
    bool peekPixels(SkPixmap* pixmap);

    /**
     *  Copy the pixels from the surface into the specified pixmap,
     *  converting them into the pixmap's format. The surface pixels are read
     *  starting at the specified (srcX,srcY) location.
     *
     *  The pixmap and (srcX,srcY) offset specifies a source rectangle
     *
     *      srcR.setXYWH(srcX, srcY, pixmap.width(), pixmap.height());
     *
     *  srcR is intersected with the bounds of the base-layer. If this intersection is not empty,
     *  then we have two sets of pixels (of equal size). Replace the dst pixels with the
     *  corresponding src pixels, performing any colortype/alphatype transformations needed
     *  (in the case where the src and dst have different colortypes or alphatypes).
     *
     *  This call can fail, returning false, for several reasons:
     *  - If srcR does not intersect the surface bounds.
     *  - If the requested colortype/alphatype cannot be converted from the surface's types.
     */
    bool readPixels(const SkPixmap& dst, int srcX, int srcY);
    bool readPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes,
                    int srcX, int srcY);
    bool readPixels(const SkBitmap& dst, int srcX, int srcY);

    const SkSurfaceProps& props() const { return fProps; }

    /**
     * Issue any pending surface IO to the current backend 3D API and resolve any surface MSAA.
     *
     * The flush calls below are the new preferred way to flush calls to a surface, and this call
     * will eventually be removed.
     */
    void prepareForExternalIO();

    /**
     * Issue any pending surface IO to the current backend 3D API
     */
    void flush();

    /**
     * Issue any pending surface IO to the current backend 3D API. After issuing all commands,
     * numSemaphore semaphores will be signaled by the gpu. The client passes in an array of
     * numSemaphores GrBackendSemaphores. In general these GrBackendSemaphore's can be either
     * initialized or not. If they are initialized, the backend uses the passed in semaphore.
     * If it is not initialized, a new semaphore is created and the GrBackendSemaphore object
     * is initialized with that semaphore.
     *
     * The client will own and be responsible for deleting the underlying semaphores that are stored
     * and returned in initialized GrBackendSemaphore objects. The GrBackendSemaphore objects
     * themselves can be deleted as soon as this function returns.
     *
     * If the backend API is OpenGL only uninitialized GrBackendSemaphores are supported.
     * If the backend API is Vulkan either initialized or unitialized semaphores are supported.
     * If unitialized, the semaphores which are created will be valid for use only with the VkDevice
     * with which they were created.
     *
     * If this call returns GrSemaphoresSubmited::kNo, the GPU backend will not have created or
     * added any semaphores to signal on the GPU. Thus the client should not have the GPU wait on
     * any of the semaphores. However, any pending surface IO will still be flushed.
     */
    GrSemaphoresSubmitted flushAndSignalSemaphores(int numSemaphores,
                                                   GrBackendSemaphore signalSemaphores[]);

    /**
     * Inserts a list of GPU semaphores that the current backend 3D API must wait on before
     * executing any more commands on the GPU for this surface. Skia will take ownership of the
     * underlying semaphores and delete them once they have been signaled and waited on.
     *
     * If this call returns false, then the GPU backend will not wait on any passed in semaphores,
     * and the client will still own the semaphores.
     */
    bool wait(int numSemaphores, const GrBackendSemaphore* waitSemaphores);

    /**
     * This creates a characterization of this SkSurface's properties that can
     * be used to perform gpu-backend preprocessing in a separate thread (via
     * the SkDeferredDisplayListRecorder).
     * It will return false on failure (e.g., if the SkSurface is cpu-backed).
     */
    bool characterize(SkSurfaceCharacterization* characterization) const;

    /**
     * Draw a deferred display list (created via SkDeferredDisplayListRecorder).
     * The draw will be skipped if the characterization stored in the display list
     * isn't compatible with this surface.
     */
    bool draw(SkDeferredDisplayList* deferredDisplayList);

protected:
    SkSurface(int width, int height, const SkSurfaceProps* surfaceProps);
    SkSurface(const SkImageInfo& imageInfo, const SkSurfaceProps* surfaceProps);

    // called by subclass if their contents have changed
    void dirtyGenerationID() {
        fGenerationID = 0;
    }

private:
    const SkSurfaceProps fProps;
    const int            fWidth;
    const int            fHeight;
    uint32_t             fGenerationID;

    typedef SkRefCnt INHERITED;
};

#endif
