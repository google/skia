/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSurface_DEFINED
#define SkSurface_DEFINED

#include "include/core/SkImage.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurfaceProps.h"

#include "include/gpu/GrTypes.h"

#if defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26
#include <android/hardware_buffer.h>
#endif

class SkCanvas;
class SkDeferredDisplayList;
class SkPaint;
class SkSurfaceCharacterization;
class GrBackendRenderTarget;
class GrBackendSemaphore;
class GrBackendTexture;
class GrContext;
class GrRecordingContext;
class GrRenderTarget;

/** \class SkSurface
    SkSurface is responsible for managing the pixels that a canvas draws into. The pixels can be
    allocated either in CPU memory (a raster surface) or on the GPU (a GrRenderTarget surface).
    SkSurface takes care of allocating a SkCanvas that will draw into the surface. Call
    surface->getCanvas() to use that canvas (but don't delete it, it is owned by the surface).
    SkSurface always has non-zero dimensions. If there is a request for a new surface, and either
    of the requested dimensions are zero, then nullptr will be returned.
*/
class SK_API SkSurface : public SkRefCnt {
public:

    /** Allocates raster SkSurface. SkCanvas returned by SkSurface draws directly into pixels.

        SkSurface is returned if all parameters are valid.
        Valid parameters include:
        info dimensions are greater than zero;
        info contains SkColorType and SkAlphaType supported by raster surface;
        pixels is not nullptr;
        rowBytes is large enough to contain info width pixels of SkColorType.

        Pixel buffer size should be info height times computed rowBytes.
        Pixels are not initialized.
        To access pixels after drawing, call flush() or peekPixels().

        @param imageInfo     width, height, SkColorType, SkAlphaType, SkColorSpace,
                             of raster surface; width and height must be greater than zero
        @param pixels        pointer to destination pixels buffer
        @param rowBytes      interval from one SkSurface row to the next
        @param surfaceProps  LCD striping orientation and setting for device independent fonts;
                             may be nullptr
        @return              SkSurface if all parameters are valid; otherwise, nullptr
    */
    static sk_sp<SkSurface> MakeRasterDirect(const SkImageInfo& imageInfo, void* pixels,
                                             size_t rowBytes,
                                             const SkSurfaceProps* surfaceProps = nullptr);

    /** Allocates raster SkSurface. SkCanvas returned by SkSurface draws directly into pixels.
        releaseProc is called with pixels and context when SkSurface is deleted.

        SkSurface is returned if all parameters are valid.
        Valid parameters include:
        info dimensions are greater than zero;
        info contains SkColorType and SkAlphaType supported by raster surface;
        pixels is not nullptr;
        rowBytes is large enough to contain info width pixels of SkColorType.

        Pixel buffer size should be info height times computed rowBytes.
        Pixels are not initialized.
        To access pixels after drawing, call flush() or peekPixels().

        @param imageInfo     width, height, SkColorType, SkAlphaType, SkColorSpace,
                             of raster surface; width and height must be greater than zero
        @param pixels        pointer to destination pixels buffer
        @param rowBytes      interval from one SkSurface row to the next
        @param releaseProc   called when SkSurface is deleted; may be nullptr
        @param context       passed to releaseProc; may be nullptr
        @param surfaceProps  LCD striping orientation and setting for device independent fonts;
                             may be nullptr
        @return              SkSurface if all parameters are valid; otherwise, nullptr
    */
    static sk_sp<SkSurface> MakeRasterDirectReleaseProc(const SkImageInfo& imageInfo, void* pixels,
                                    size_t rowBytes,
                                    void (*releaseProc)(void* pixels, void* context),
                                    void* context, const SkSurfaceProps* surfaceProps = nullptr);

    /** Allocates raster SkSurface. SkCanvas returned by SkSurface draws directly into pixels.
        Allocates and zeroes pixel memory. Pixel memory size is imageInfo.height() times
        rowBytes, or times imageInfo.minRowBytes() if rowBytes is zero.
        Pixel memory is deleted when SkSurface is deleted.

        SkSurface is returned if all parameters are valid.
        Valid parameters include:
        info dimensions are greater than zero;
        info contains SkColorType and SkAlphaType supported by raster surface;
        rowBytes is large enough to contain info width pixels of SkColorType, or is zero.

        If rowBytes is not zero, subsequent images returned by makeImageSnapshot()
        have the same rowBytes.

        @param imageInfo     width, height, SkColorType, SkAlphaType, SkColorSpace,
                             of raster surface; width and height must be greater than zero
        @param rowBytes      interval from one SkSurface row to the next; may be zero
        @param surfaceProps  LCD striping orientation and setting for device independent fonts;
                             may be nullptr
        @return              SkSurface if all parameters are valid; otherwise, nullptr
    */
    static sk_sp<SkSurface> MakeRaster(const SkImageInfo& imageInfo, size_t rowBytes,
                                       const SkSurfaceProps* surfaceProps);

    /** Allocates raster SkSurface. SkCanvas returned by SkSurface draws directly into pixels.
        Allocates and zeroes pixel memory. Pixel memory size is imageInfo.height() times
        imageInfo.minRowBytes().
        Pixel memory is deleted when SkSurface is deleted.

        SkSurface is returned if all parameters are valid.
        Valid parameters include:
        info dimensions are greater than zero;
        info contains SkColorType and SkAlphaType supported by raster surface.

        @param imageInfo  width, height, SkColorType, SkAlphaType, SkColorSpace,
                          of raster surface; width and height must be greater than zero
        @param props      LCD striping orientation and setting for device independent fonts;
                          may be nullptr
        @return           SkSurface if all parameters are valid; otherwise, nullptr
    */
    static sk_sp<SkSurface> MakeRaster(const SkImageInfo& imageInfo,
                                       const SkSurfaceProps* props = nullptr) {
        return MakeRaster(imageInfo, 0, props);
    }

    /** Allocates raster SkSurface. SkCanvas returned by SkSurface draws directly into pixels.
        Allocates and zeroes pixel memory. Pixel memory size is height times width times
        four. Pixel memory is deleted when SkSurface is deleted.

        Internally, sets SkImageInfo to width, height, native color type, and
        kPremul_SkAlphaType.

        SkSurface is returned if width and height are greater than zero.

        Use to create SkSurface that matches SkPMColor, the native pixel arrangement on
        the platform. SkSurface drawn to output device skips converting its pixel format.

        @param width         pixel column count; must be greater than zero
        @param height        pixel row count; must be greater than zero
        @param surfaceProps  LCD striping orientation and setting for device independent
                             fonts; may be nullptr
        @return              SkSurface if all parameters are valid; otherwise, nullptr
    */
    static sk_sp<SkSurface> MakeRasterN32Premul(int width, int height,
                                                const SkSurfaceProps* surfaceProps = nullptr);

    /** Caller data passed to RenderTarget/TextureReleaseProc; may be nullptr. */
    typedef void* ReleaseContext;

    /** User function called when supplied render target may be deleted. */
    typedef void (*RenderTargetReleaseProc)(ReleaseContext releaseContext);

    /** User function called when supplied texture may be deleted. */
    typedef void (*TextureReleaseProc)(ReleaseContext releaseContext);

    /** Wraps a GPU-backed texture into SkSurface. Caller must ensure the texture is
        valid for the lifetime of returned SkSurface. If sampleCnt greater than zero,
        creates an intermediate MSAA SkSurface which is used for drawing backendTexture.

        SkSurface is returned if all parameters are valid. backendTexture is valid if
        its pixel configuration agrees with colorSpace and context; for instance, if
        backendTexture has an sRGB configuration, then context must support sRGB,
        and colorSpace must be present. Further, backendTexture width and height must
        not exceed context capabilities, and the context must be able to support
        back-end textures.

        If SK_SUPPORT_GPU is defined as zero, has no effect and returns nullptr.

        @param context             GPU context
        @param backendTexture      texture residing on GPU
        @param origin              one of: kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin
        @param sampleCnt           samples per pixel, or 0 to disable full scene anti-aliasing
        @param colorType           one of:
                                   kUnknown_SkColorType, kAlpha_8_SkColorType, kRGB_565_SkColorType,
                                   kARGB_4444_SkColorType, kRGBA_8888_SkColorType,
                                   kRGB_888x_SkColorType, kBGRA_8888_SkColorType,
                                   kRGBA_1010102_SkColorType, kRGB_101010x_SkColorType,
                                   kGray_8_SkColorType, kRGBA_F16_SkColorType
        @param colorSpace          range of colors; may be nullptr
        @param surfaceProps        LCD striping orientation and setting for device independent
                                   fonts; may be nullptr
        @param textureReleaseProc  function called when texture can be released
        @param releaseContext      state passed to textureReleaseProc
        @return                    SkSurface if all parameters are valid; otherwise, nullptr
    */
    static sk_sp<SkSurface> MakeFromBackendTexture(GrContext* context,
                                                   const GrBackendTexture& backendTexture,
                                                   GrSurfaceOrigin origin, int sampleCnt,
                                                   SkColorType colorType,
                                                   sk_sp<SkColorSpace> colorSpace,
                                                   const SkSurfaceProps* surfaceProps,
                                                   TextureReleaseProc textureReleaseProc = nullptr,
                                                   ReleaseContext releaseContext = nullptr);

    /** Wraps a GPU-backed buffer into SkSurface. Caller must ensure backendRenderTarget
        is valid for the lifetime of returned SkSurface.

        SkSurface is returned if all parameters are valid. backendRenderTarget is valid if
        its pixel configuration agrees with colorSpace and context; for instance, if
        backendRenderTarget has an sRGB configuration, then context must support sRGB,
        and colorSpace must be present. Further, backendRenderTarget width and height must
        not exceed context capabilities, and the context must be able to support
        back-end render targets.

        If SK_SUPPORT_GPU is defined as zero, has no effect and returns nullptr.

        @param context                  GPU context
        @param backendRenderTarget      GPU intermediate memory buffer
        @param origin                   one of:
                                        kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin
        @param colorType                one of:
                                        kUnknown_SkColorType, kAlpha_8_SkColorType,
                                        kRGB_565_SkColorType,
                                        kARGB_4444_SkColorType, kRGBA_8888_SkColorType,
                                        kRGB_888x_SkColorType, kBGRA_8888_SkColorType,
                                        kRGBA_1010102_SkColorType, kRGB_101010x_SkColorType,
                                        kGray_8_SkColorType, kRGBA_F16_SkColorType
        @param colorSpace               range of colors
        @param surfaceProps             LCD striping orientation and setting for device independent
                                        fonts; may be nullptr
        @param releaseProc              function called when texture can be released
        @param releaseContext           state passed to textureReleaseProc
        @return                         SkSurface if all parameters are valid; otherwise, nullptr
    */
    static sk_sp<SkSurface> MakeFromBackendRenderTarget(GrContext* context,
                                                const GrBackendRenderTarget& backendRenderTarget,
                                                GrSurfaceOrigin origin,
                                                SkColorType colorType,
                                                sk_sp<SkColorSpace> colorSpace,
                                                const SkSurfaceProps* surfaceProps,
                                                RenderTargetReleaseProc releaseProc = nullptr,
                                                ReleaseContext releaseContext = nullptr);

    /** Wraps a GPU-backed texture into SkSurface. Caller must ensure backendTexture is
        valid for the lifetime of returned SkSurface. If sampleCnt greater than zero,
        creates an intermediate MSAA SkSurface which is used for drawing backendTexture.

        SkSurface is returned if all parameters are valid. backendTexture is valid if
        its pixel configuration agrees with colorSpace and context; for instance, if
        backendTexture has an sRGB configuration, then context must support sRGB,
        and colorSpace must be present. Further, backendTexture width and height must
        not exceed context capabilities.

        Returned SkSurface is available only for drawing into, and cannot generate an
        SkImage.

        If SK_SUPPORT_GPU is defined as zero, has no effect and returns nullptr.

        @param context         GPU context
        @param backendTexture  texture residing on GPU
        @param origin          one of: kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin
        @param sampleCnt       samples per pixel, or 0 to disable full scene anti-aliasing
        @param colorType       one of:
                               kUnknown_SkColorType, kAlpha_8_SkColorType, kRGB_565_SkColorType,
                               kARGB_4444_SkColorType, kRGBA_8888_SkColorType,
                               kRGB_888x_SkColorType, kBGRA_8888_SkColorType,
                               kRGBA_1010102_SkColorType, kRGB_101010x_SkColorType,
                               kGray_8_SkColorType, kRGBA_F16_SkColorType
        @param colorSpace      range of colors; may be nullptr
        @param surfaceProps    LCD striping orientation and setting for device independent
                               fonts; may be nullptr
        @return                SkSurface if all parameters are valid; otherwise, nullptr
    */
    static sk_sp<SkSurface> MakeFromBackendTextureAsRenderTarget(GrContext* context,
                                                            const GrBackendTexture& backendTexture,
                                                            GrSurfaceOrigin origin,
                                                            int sampleCnt,
                                                            SkColorType colorType,
                                                            sk_sp<SkColorSpace> colorSpace,
                                                            const SkSurfaceProps* surfaceProps);

#if defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26
    /** Private.
        Creates SkSurface from Android hardware buffer.
        Returned SkSurface takes a reference on the buffer. The ref on the buffer will be released
        when the SkSurface is destroyed and there is no pending work on the GPU involving the
        buffer.

        Only available on Android, when __ANDROID_API__ is defined to be 26 or greater.

        Currently this is only supported for buffers that can be textured as well as rendered to.
        In other workds that must have both AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT and
        AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE usage bits.

        @param context         GPU context
        @param hardwareBuffer  AHardwareBuffer Android hardware buffer
        @param origin          one of: kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin
        @param colorSpace      range of colors; may be nullptr
        @param surfaceProps    LCD striping orientation and setting for device independent
                               fonts; may be nullptr
        @return                created SkSurface, or nullptr
    */
    static sk_sp<SkSurface> MakeFromAHardwareBuffer(GrContext* context,
                                                    AHardwareBuffer* hardwareBuffer,
                                                    GrSurfaceOrigin origin,
                                                    sk_sp<SkColorSpace> colorSpace,
                                                    const SkSurfaceProps* surfaceProps);
#endif

    /** Returns SkSurface on GPU indicated by context. Allocates memory for
        pixels, based on the width, height, and SkColorType in SkImageInfo.  budgeted
        selects whether allocation for pixels is tracked by context. imageInfo
        describes the pixel format in SkColorType, and transparency in
        SkAlphaType, and color matching in SkColorSpace.

        sampleCount requests the number of samples per pixel.
        Pass zero to disable multi-sample anti-aliasing.  The request is rounded
        up to the next supported count, or rounded down if it is larger than the
        maximum supported count.

        surfaceOrigin pins either the top-left or the bottom-left corner to the origin.

        shouldCreateWithMips hints that SkImage returned by makeImageSnapshot() is mip map.

        If SK_SUPPORT_GPU is defined as zero, has no effect and returns nullptr.

        @param context               GPU context
        @param budgeted              one of: SkBudgeted::kNo, SkBudgeted::kYes
        @param imageInfo             width, height, SkColorType, SkAlphaType, SkColorSpace;
                                     width, or height, or both, may be zero
        @param sampleCount           samples per pixel, or 0 to disable full scene anti-aliasing
        @param surfaceOrigin         one of: kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin
        @param surfaceProps          LCD striping orientation and setting for device independent
                                     fonts; may be nullptr
        @param shouldCreateWithMips  hint that SkSurface will host mip map images
        @return                      SkSurface if all parameters are valid; otherwise, nullptr
    */
    static sk_sp<SkSurface> MakeRenderTarget(GrContext* context, SkBudgeted budgeted,
                                             const SkImageInfo& imageInfo,
                                             int sampleCount, GrSurfaceOrigin surfaceOrigin,
                                             const SkSurfaceProps* surfaceProps,
                                             bool shouldCreateWithMips = false);

    /** Returns SkSurface on GPU indicated by context. Allocates memory for
        pixels, based on the width, height, and SkColorType in SkImageInfo.  budgeted
        selects whether allocation for pixels is tracked by context. imageInfo
        describes the pixel format in SkColorType, and transparency in
        SkAlphaType, and color matching in SkColorSpace.

        sampleCount requests the number of samples per pixel.
        Pass zero to disable multi-sample anti-aliasing.  The request is rounded
        up to the next supported count, or rounded down if it is larger than the
        maximum supported count.

        SkSurface bottom-left corner is pinned to the origin.

        @param context      GPU context
        @param budgeted     one of: SkBudgeted::kNo, SkBudgeted::kYes
        @param imageInfo    width, height, SkColorType, SkAlphaType, SkColorSpace,
                            of raster surface; width, or height, or both, may be zero
        @param sampleCount  samples per pixel, or 0 to disable multi-sample anti-aliasing
        @param props        LCD striping orientation and setting for device independent
                            fonts; may be nullptr
        @return             SkSurface if all parameters are valid; otherwise, nullptr
    */
    static sk_sp<SkSurface> MakeRenderTarget(GrContext* context, SkBudgeted budgeted,
                                             const SkImageInfo& imageInfo, int sampleCount,
                                             const SkSurfaceProps* props) {
        return MakeRenderTarget(context, budgeted, imageInfo, sampleCount,
                                kBottomLeft_GrSurfaceOrigin, props);
    }

    /** Returns SkSurface on GPU indicated by context. Allocates memory for
        pixels, based on the width, height, and SkColorType in SkImageInfo.  budgeted
        selects whether allocation for pixels is tracked by context. imageInfo
        describes the pixel format in SkColorType, and transparency in
        SkAlphaType, and color matching in SkColorSpace.

        SkSurface bottom-left corner is pinned to the origin.

        @param context    GPU context
        @param budgeted   one of: SkBudgeted::kNo, SkBudgeted::kYes
        @param imageInfo  width, height, SkColorType, SkAlphaType, SkColorSpace,
                          of raster surface; width, or height, or both, may be zero
        @return           SkSurface if all parameters are valid; otherwise, nullptr
    */
    static sk_sp<SkSurface> MakeRenderTarget(GrContext* context, SkBudgeted budgeted,
                                             const SkImageInfo& imageInfo) {
        if (!imageInfo.width() || !imageInfo.height()) {
            return nullptr;
        }
        return MakeRenderTarget(context, budgeted, imageInfo, 0, kBottomLeft_GrSurfaceOrigin,
                                nullptr);
    }

    /** Returns SkSurface on GPU indicated by context that is compatible with the provided
        characterization. budgeted selects whether allocation for pixels is tracked by context.

        @param context           GPU context
        @param characterization  description of the desired SkSurface
        @param budgeted          one of: SkBudgeted::kNo, SkBudgeted::kYes
        @return                  SkSurface if all parameters are valid; otherwise, nullptr
    */
    static sk_sp<SkSurface> MakeRenderTarget(GrRecordingContext* context,
                                             const SkSurfaceCharacterization& characterization,
                                             SkBudgeted budgeted);

    /** Returns SkSurface without backing pixels. Drawing to SkCanvas returned from SkSurface
        has no effect. Calling makeImageSnapshot() on returned SkSurface returns nullptr.

        @param width   one or greater
        @param height  one or greater
        @return        SkSurface if width and height are positive; otherwise, nullptr
    */
    static sk_sp<SkSurface> MakeNull(int width, int height);

    /** Returns pixel count in each row; may be zero or greater.

        @return  number of pixel columns
    */
    int width() const { return fWidth; }

    /** Returns pixel row count; may be zero or greater.

        @return  number of pixel rows
    */
    int height() const { return fHeight; }

    /** Returns unique value identifying the content of SkSurface. Returned value changes
        each time the content changes. Content is changed by drawing, or by calling
        notifyContentWillChange().

        @return  unique content identifier
    */
    uint32_t generationID();

    /** \enum SkSurface::ContentChangeMode
        ContentChangeMode members are parameters to notifyContentWillChange().
    */
    enum ContentChangeMode {
        kDiscard_ContentChangeMode, //!< discards surface on change
        kRetain_ContentChangeMode,  //!< preserves surface on change
    };

    /** Notifies that SkSurface contents will be changed by code outside of Skia.
        Subsequent calls to generationID() return a different value.

        TODO: Can kRetain_ContentChangeMode be deprecated?

        @param mode  one of: kDiscard_ContentChangeMode, kRetain_ContentChangeMode
    */
    void notifyContentWillChange(ContentChangeMode mode);

    enum BackendHandleAccess {
        kFlushRead_BackendHandleAccess,    //!< back-end object is readable
        kFlushWrite_BackendHandleAccess,   //!< back-end object is writable
        kDiscardWrite_BackendHandleAccess, //!< back-end object must be overwritten
    };

    /** Deprecated.
    */
    static const BackendHandleAccess kFlushRead_TextureHandleAccess =
            kFlushRead_BackendHandleAccess;

    /** Deprecated.
    */
    static const BackendHandleAccess kFlushWrite_TextureHandleAccess =
            kFlushWrite_BackendHandleAccess;

    /** Deprecated.
    */
    static const BackendHandleAccess kDiscardWrite_TextureHandleAccess =
            kDiscardWrite_BackendHandleAccess;

    /** Retrieves the back-end texture. If SkSurface has no back-end texture, an invalid
        object is returned. Call GrBackendTexture::isValid to determine if the result
        is valid.

        The returned GrBackendTexture should be discarded if the SkSurface is drawn to or deleted.

        @param backendHandleAccess  one of:  kFlushRead_BackendHandleAccess,
                                    kFlushWrite_BackendHandleAccess,
                                    kDiscardWrite_BackendHandleAccess
        @return                     GPU texture reference; invalid on failure
    */
    GrBackendTexture getBackendTexture(BackendHandleAccess backendHandleAccess);

    /** Retrieves the back-end render target. If SkSurface has no back-end render target, an invalid
        object is returned. Call GrBackendRenderTarget::isValid to determine if the result
        is valid.

        The returned GrBackendRenderTarget should be discarded if the SkSurface is drawn to
        or deleted.

        @param backendHandleAccess  one of:  kFlushRead_BackendHandleAccess,
                                    kFlushWrite_BackendHandleAccess,
                                    kDiscardWrite_BackendHandleAccess
        @return                     GPU render target reference; invalid on failure
    */
    GrBackendRenderTarget getBackendRenderTarget(BackendHandleAccess backendHandleAccess);

    /** Returns SkCanvas that draws into SkSurface. Subsequent calls return the same SkCanvas.
        SkCanvas returned is managed and owned by SkSurface, and is deleted when SkSurface
        is deleted.

        @return  drawing SkCanvas for SkSurface
    */
    SkCanvas* getCanvas();

    /** Returns a compatible SkSurface, or nullptr. Returned SkSurface contains
        the same raster, GPU, or null properties as the original. Returned SkSurface
        does not share the same pixels.

        Returns nullptr if imageInfo width or height are zero, or if imageInfo
        is incompatible with SkSurface.

        @param imageInfo  width, height, SkColorType, SkAlphaType, SkColorSpace,
                          of SkSurface; width and height must be greater than zero
        @return           compatible SkSurface or nullptr
    */
    sk_sp<SkSurface> makeSurface(const SkImageInfo& imageInfo);

    /** Returns SkImage capturing SkSurface contents. Subsequent drawing to SkSurface contents
        are not captured. SkImage allocation is accounted for if SkSurface was created with
        SkBudgeted::kYes.

        @return  SkImage initialized with SkSurface contents
    */
    sk_sp<SkImage> makeImageSnapshot();

    /**
     *  Like the no-parameter version, this returns an image of the current surface contents.
     *  This variant takes a rectangle specifying the subset of the surface that is of interest.
     *  These bounds will be sanitized before being used.
     *  - If bounds extends beyond the surface, it will be trimmed to just the intersection of
     *    it and the surface.
     *  - If bounds does not intersect the surface, then this returns nullptr.
     *  - If bounds == the surface, then this is the same as calling the no-parameter variant.
     */
    sk_sp<SkImage> makeImageSnapshot(const SkIRect& bounds);

    /** Draws SkSurface contents to canvas, with its top-left corner at (x, y).

        If SkPaint paint is not nullptr, apply SkColorFilter, alpha, SkImageFilter,
        SkBlendMode, and SkDrawLooper.

        @param canvas  SkCanvas drawn into
        @param x       horizontal offset in SkCanvas
        @param y       vertical offset in SkCanvas
        @param paint   SkPaint containing SkBlendMode, SkColorFilter, SkImageFilter,
                       and so on; or nullptr
    */
    void draw(SkCanvas* canvas, SkScalar x, SkScalar y, const SkPaint* paint);

    /** Copies SkSurface pixel address, row bytes, and SkImageInfo to SkPixmap, if address
        is available, and returns true. If pixel address is not available, return
        false and leave SkPixmap unchanged.

        pixmap contents become invalid on any future change to SkSurface.

        @param pixmap  storage for pixel state if pixels are readable; otherwise, ignored
        @return        true if SkSurface has direct access to pixels
    */
    bool peekPixels(SkPixmap* pixmap);

    /** Copies SkRect of pixels to dst.

        Source SkRect corners are (srcX, srcY) and SkSurface (width(), height()).
        Destination SkRect corners are (0, 0) and (dst.width(), dst.height()).
        Copies each readable pixel intersecting both rectangles, without scaling,
        converting to dst.colorType() and dst.alphaType() if required.

        Pixels are readable when SkSurface is raster, or backed by a GPU.

        The destination pixel storage must be allocated by the caller.

        Pixel values are converted only if SkColorType and SkAlphaType
        do not match. Only pixels within both source and destination rectangles
        are copied. dst contents outside SkRect intersection are unchanged.

        Pass negative values for srcX or srcY to offset pixels across or down destination.

        Does not copy, and returns false if:
        - Source and destination rectangles do not intersect.
        - SkPixmap pixels could not be allocated.
        - dst.rowBytes() is too small to contain one row of pixels.

        @param dst   storage for pixels copied from SkSurface
        @param srcX  offset into readable pixels on x-axis; may be negative
        @param srcY  offset into readable pixels on y-axis; may be negative
        @return      true if pixels were copied
    */
    bool readPixels(const SkPixmap& dst, int srcX, int srcY);

    /** Copies SkRect of pixels from SkCanvas into dstPixels.

        Source SkRect corners are (srcX, srcY) and SkSurface (width(), height()).
        Destination SkRect corners are (0, 0) and (dstInfo.width(), dstInfo.height()).
        Copies each readable pixel intersecting both rectangles, without scaling,
        converting to dstInfo.colorType() and dstInfo.alphaType() if required.

        Pixels are readable when SkSurface is raster, or backed by a GPU.

        The destination pixel storage must be allocated by the caller.

        Pixel values are converted only if SkColorType and SkAlphaType
        do not match. Only pixels within both source and destination rectangles
        are copied. dstPixels contents outside SkRect intersection are unchanged.

        Pass negative values for srcX or srcY to offset pixels across or down destination.

        Does not copy, and returns false if:
        - Source and destination rectangles do not intersect.
        - SkSurface pixels could not be converted to dstInfo.colorType() or dstInfo.alphaType().
        - dstRowBytes is too small to contain one row of pixels.

        @param dstInfo      width, height, SkColorType, and SkAlphaType of dstPixels
        @param dstPixels    storage for pixels; dstInfo.height() times dstRowBytes, or larger
        @param dstRowBytes  size of one destination row; dstInfo.width() times pixel size, or larger
        @param srcX         offset into readable pixels on x-axis; may be negative
        @param srcY         offset into readable pixels on y-axis; may be negative
        @return             true if pixels were copied
    */
    bool readPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes,
                    int srcX, int srcY);

    /** Copies SkRect of pixels from SkSurface into bitmap.

        Source SkRect corners are (srcX, srcY) and SkSurface (width(), height()).
        Destination SkRect corners are (0, 0) and (bitmap.width(), bitmap.height()).
        Copies each readable pixel intersecting both rectangles, without scaling,
        converting to bitmap.colorType() and bitmap.alphaType() if required.

        Pixels are readable when SkSurface is raster, or backed by a GPU.

        The destination pixel storage must be allocated by the caller.

        Pixel values are converted only if SkColorType and SkAlphaType
        do not match. Only pixels within both source and destination rectangles
        are copied. dst contents outside SkRect intersection are unchanged.

        Pass negative values for srcX or srcY to offset pixels across or down destination.

        Does not copy, and returns false if:
        - Source and destination rectangles do not intersect.
        - SkSurface pixels could not be converted to dst.colorType() or dst.alphaType().
        - dst pixels could not be allocated.
        - dst.rowBytes() is too small to contain one row of pixels.

        @param dst   storage for pixels copied from SkSurface
        @param srcX  offset into readable pixels on x-axis; may be negative
        @param srcY  offset into readable pixels on y-axis; may be negative
        @return      true if pixels were copied
    */
    bool readPixels(const SkBitmap& dst, int srcX, int srcY);

    /** Copies SkRect of pixels from the src SkPixmap to the SkSurface.

        Source SkRect corners are (0, 0) and (src.width(), src.height()).
        Destination SkRect corners are (dstX, dstY) and
        (dstX + Surface width(), dstY + Surface height()).

        Copies each readable pixel intersecting both rectangles, without scaling,
        converting to SkSurface colorType() and SkSurface alphaType() if required.

        @param src   storage for pixels to copy to SkSurface
        @param dstX  x-axis position relative to SkSurface to begin copy; may be negative
        @param dstY  y-axis position relative to SkSurface to begin copy; may be negative
    */
    void writePixels(const SkPixmap& src, int dstX, int dstY);

    /** Copies SkRect of pixels from the src SkBitmap to the SkSurface.

        Source SkRect corners are (0, 0) and (src.width(), src.height()).
        Destination SkRect corners are (dstX, dstY) and
        (dstX + Surface width(), dstY + Surface height()).

        Copies each readable pixel intersecting both rectangles, without scaling,
        converting to SkSurface colorType() and SkSurface alphaType() if required.

        @param src   storage for pixels to copy to SkSurface
        @param dstX  x-axis position relative to SkSurface to begin copy; may be negative
        @param dstY  y-axis position relative to SkSurface to begin copy; may be negative
    */
    void writePixels(const SkBitmap& src, int dstX, int dstY);

    /** Returns SkSurfaceProps for surface.

        @return  LCD striping orientation and setting for device independent fonts
    */
    const SkSurfaceProps& props() const { return fProps; }

    /** Issues pending SkSurface commands to the GPU-backed API and resolves any SkSurface MSAA.

        Skia flushes as needed, so it is not necessary to call this if Skia manages
        drawing and object lifetime. Call when interleaving Skia calls with native
        GPU calls.
    */
    void flush();

    enum class BackendSurfaceAccess {
        kNoAccess,  //!< back-end object will not be used by client
        kPresent,   //!< back-end surface will be used for presenting to screen
    };

    /** Issues pending SkSurface commands to the GPU-backed API and resolves any SkSurface MSAA.
        The work that is submitted to the GPU will be dependent on the BackendSurfaceAccess that is
        passed in.

        If BackendSurfaceAccess::kNoAccess is passed in all commands will be issued to the GPU.

        If BackendSurfaceAccess::kPresent is passed in and the backend API is not Vulkan, it is
        treated the same as kNoAccess. If the backend API is Vulkan, the VkImage that backs the
        SkSurface will be transferred back to its original queue. If the SkSurface was created by
        wrapping a VkImage, the queue will be set to the queue which was originally passed in on
        the GrVkImageInfo. Additionally, if the original queue was not external or foreign the
        layout of the VkImage will be set to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR.

        The GrFlushInfo describes additional options to flush. Please see documentation at
        GrFlushInfo for more info.

        If GrSemaphoresSubmitted::kNo is returned, the GPU back-end did not create or
        add any semaphores to signal on the GPU; the caller should not instruct the GPU
        to wait on any of the semaphores passed in the GrFlushInfo.

        Pending surface commands are flushed regardless of the return result.

        @param access  type of access the call will do on the backend object after flush
        @param info    flush options
        @return        one of: GrSemaphoresSubmitted::kYes, GrSemaphoresSubmitted::kNo
    */
    GrSemaphoresSubmitted flush(BackendSurfaceAccess access, const GrFlushInfo& info);

    /** Deprecated
     */
    GrSemaphoresSubmitted flush(BackendSurfaceAccess access, GrFlushFlags flags,
                                int numSemaphores, GrBackendSemaphore signalSemaphores[],
                                GrGpuFinishedProc finishedProc = nullptr,
                                GrGpuFinishedContext finishedContext = nullptr);

    /** The below enum and flush call are deprecated
     */
    enum FlushFlags {
        kNone_FlushFlags = 0,
        // flush will wait till all submitted GPU work is finished before returning.
        kSyncCpu_FlushFlag = 0x1,
    };
    GrSemaphoresSubmitted flush(BackendSurfaceAccess access, FlushFlags flags,
                                int numSemaphores, GrBackendSemaphore signalSemaphores[]);

    /** Deprecated.
    */
    GrSemaphoresSubmitted flushAndSignalSemaphores(int numSemaphores,
                                                   GrBackendSemaphore signalSemaphores[]);

    /** Inserts a list of GPU semaphores that the current GPU-backed API must wait on before
        executing any more commands on the GPU for this surface. Skia will take ownership of the
        underlying semaphores and delete them once they have been signaled and waited on.
        If this call returns false, then the GPU back-end will not wait on any passed in semaphores,
        and the client will still own the semaphores.

        @param numSemaphores   size of waitSemaphores array
        @param waitSemaphores  array of semaphore containers
        @return                true if GPU is waiting on semaphores
    */
    bool wait(int numSemaphores, const GrBackendSemaphore* waitSemaphores);

    /** Initializes SkSurfaceCharacterization that can be used to perform GPU back-end
        processing in a separate thread. Typically this is used to divide drawing
        into multiple tiles. SkDeferredDisplayListRecorder records the drawing commands
        for each tile.

        Return true if SkSurface supports characterization. raster surface returns false.

        @param characterization  properties for parallel drawing
        @return                  true if supported
    */
    bool characterize(SkSurfaceCharacterization* characterization) const;

    /** Draws deferred display list created using SkDeferredDisplayListRecorder.
        Has no effect and returns false if SkSurfaceCharacterization stored in
        deferredDisplayList is not compatible with SkSurface.

        raster surface returns false.

        @param deferredDisplayList  drawing commands
        @return                     false if deferredDisplayList is not compatible
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
