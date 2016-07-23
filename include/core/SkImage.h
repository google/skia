/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_DEFINED
#define SkImage_DEFINED

#include "SkFilterQuality.h"
#include "SkImageInfo.h"
#include "SkImageEncoder.h"
#include "SkRefCnt.h"
#include "SkScalar.h"
#include "SkShader.h"

class SkData;
class SkCanvas;
class SkColorTable;
class SkImageGenerator;
class SkPaint;
class SkPicture;
class SkPixelSerializer;
class SkString;
class SkSurface;
class GrContext;
class GrContextThreadSafeProxy;
class GrTexture;

/**
 *  SkImage is an abstraction for drawing a rectagle of pixels, though the
 *  particular type of image could be actually storing its data on the GPU, or
 *  as drawing commands (picture or PDF or otherwise), ready to be played back
 *  into another canvas.
 *
 *  The content of SkImage is always immutable, though the actual storage may
 *  change, if for example that image can be re-created via encoded data or
 *  other means.
 *
 *  SkImage always has a non-zero dimensions. If there is a request to create a new image, either
 *  directly or via SkSurface, and either of the requested dimensions are zero, then NULL will be
 *  returned.
 */
class SK_API SkImage : public SkRefCnt {
public:
    typedef SkImageInfo Info;
    typedef void* ReleaseContext;

    static sk_sp<SkImage> MakeRasterCopy(const SkPixmap&);
    static sk_sp<SkImage> MakeRasterData(const Info&, sk_sp<SkData> pixels, size_t rowBytes);

    typedef void (*RasterReleaseProc)(const void* pixels, ReleaseContext);

    /**
     *  Return a new Image referencing the specified pixels. These must remain valid and unchanged
     *  until the specified release-proc is called, indicating that Skia no longer has a reference
     *  to the pixels.
     *
     *  Returns NULL if the requested pixmap info is unsupported.
     */
    static sk_sp<SkImage> MakeFromRaster(const SkPixmap&, RasterReleaseProc, ReleaseContext);

    /**
     *  Construct a new image from the specified bitmap. If the bitmap is marked immutable, and
     *  its pixel memory is shareable, it may be shared instead of copied.
     */
    static sk_sp<SkImage> MakeFromBitmap(const SkBitmap&);
    
    /**
     *  Construct a new SkImage based on the given ImageGenerator. Returns NULL on error.
     *  This function will always take ownership of the passed generator.
     *
     *  If a subset is specified, it must be contained within the generator's bounds.
     */
    static sk_sp<SkImage> MakeFromGenerator(SkImageGenerator*, const SkIRect* subset = NULL);

    /**
     *  Construct a new SkImage based on the specified encoded data. Returns NULL on failure,
     *  which can mean that the format of the encoded data was not recognized/supported.
     *
     *  If a subset is specified, it must be contained within the encoded data's bounds.
     */
    static sk_sp<SkImage> MakeFromEncoded(sk_sp<SkData> encoded, const SkIRect* subset = NULL);

    /**
     *  Create a new image from the specified descriptor. Note - the caller is responsible for
     *  managing the lifetime of the underlying platform texture.
     *
     *  Will return NULL if the specified descriptor is unsupported.
     */
    static sk_sp<SkImage> MakeFromTexture(GrContext* ctx, const GrBackendTextureDesc& desc) {
        return MakeFromTexture(ctx, desc, kPremul_SkAlphaType, NULL, NULL);
    }

    static sk_sp<SkImage> MakeFromTexture(GrContext* ctx, const GrBackendTextureDesc& de,
                                          SkAlphaType at) {
        return MakeFromTexture(ctx, de, at, NULL, NULL);
    }

    typedef void (*TextureReleaseProc)(ReleaseContext);

    /**
     *  Create a new image from the specified descriptor. The underlying platform texture must stay
     *  valid and unaltered until the specified release-proc is invoked, indicating that Skia
     *  no longer is holding a reference to it.
     *
     *  Will return NULL if the specified descriptor is unsupported.
     */
    static sk_sp<SkImage> MakeFromTexture(GrContext*, const GrBackendTextureDesc&, SkAlphaType,
                                          TextureReleaseProc, ReleaseContext);

    /**
     *  Create a new image from the specified descriptor. Note - Skia will delete or recycle the
     *  texture when the image is released.
     *
     *  Will return NULL if the specified descriptor is unsupported.
     */
    static sk_sp<SkImage> MakeFromAdoptedTexture(GrContext*, const GrBackendTextureDesc&,
                                                 SkAlphaType = kPremul_SkAlphaType);

    /**
     *  Create a new image by copying the pixels from the specified descriptor. No reference is
     *  kept to the original platform texture.
     *
     *  Will return NULL if the specified descriptor is unsupported.
     */
    static sk_sp<SkImage> MakeFromTextureCopy(GrContext*, const GrBackendTextureDesc&,
                                              SkAlphaType = kPremul_SkAlphaType);

    /**
     *  Create a new image by copying the pixels from the specified y, u, v textures. The data
     *  from the textures is immediately ingested into the image and the textures can be modified or
     *  deleted after the function returns. The image will have the dimensions of the y texture.
     */
    static sk_sp<SkImage> MakeFromYUVTexturesCopy(GrContext*, SkYUVColorSpace,
                                                  const GrBackendObject yuvTextureHandles[3],
                                                  const SkISize yuvSizes[3],
                                                  GrSurfaceOrigin);

    /**
     *  Create a new image by copying the pixels from the specified y and uv textures. The data
     *  from the textures is immediately ingested into the image and the textures can be modified or
     *  deleted after the function returns. The image will have the dimensions of the y texture.
     */
    static sk_sp<SkImage> MakeFromNV12TexturesCopy(GrContext*, SkYUVColorSpace,
                                                   const GrBackendObject nv12TextureHandles[2],
                                                   const SkISize nv12Sizes[2], GrSurfaceOrigin);

    static sk_sp<SkImage> MakeFromPicture(sk_sp<SkPicture>, const SkISize& dimensions,
                                          const SkMatrix*, const SkPaint*);

    static sk_sp<SkImage> MakeTextureFromPixmap(GrContext*, const SkPixmap&, SkBudgeted budgeted);

    ///////////////////////////////////////////////////////////////////////////////////////////////

    int width() const { return fWidth; }
    int height() const { return fHeight; }
    SkISize dimensions() const { return SkISize::Make(fWidth, fHeight); }
    SkIRect bounds() const { return SkIRect::MakeWH(fWidth, fHeight); }
    uint32_t uniqueID() const { return fUniqueID; }
    virtual bool isOpaque() const { return false; }

    /**
     * Extracts YUV planes from the SkImage and stores them in client-provided memory. The sizes
     * planes and rowBytes arrays are ordered [y, u, v].
     */
    bool readYUV8Planes(const SkISize[3], void* const planes[3], const size_t rowBytes[3],
                        SkYUVColorSpace) const;

#ifdef SK_SUPPORT_LEGACY_CREATESHADER_PTR
    SkShader* newShader(SkShader::TileMode, SkShader::TileMode,
                        const SkMatrix* localMatrix = nullptr) const;
#endif

    sk_sp<SkShader> makeShader(SkShader::TileMode, SkShader::TileMode,
                               const SkMatrix* localMatrix = nullptr) const;

    /**
     *  If the image has direct access to its pixels (i.e. they are in local RAM)
     *  return true, and if not null, return in the pixmap parameter the info about the
     *  images pixels.
     *
     *  On failure, return false and ignore the pixmap parameter.
     */
    bool peekPixels(SkPixmap* pixmap) const;

#ifdef SK_SUPPORT_LEGACY_PEEKPIXELS_PARMS
    /**
     *  If the image has direct access to its pixels (i.e. they are in local
     *  RAM) return the (const) address of those pixels, and if not null, return
     *  the ImageInfo and rowBytes. The returned address is only valid while
     *  the image object is in scope.
     *
     *  On failure, returns NULL and the info and rowBytes parameters are
     *  ignored.
     *
     *  DEPRECATED -- use the SkPixmap variant instead
     */
    const void* peekPixels(SkImageInfo* info, size_t* rowBytes) const;
#endif

    /**
     *  Some images have to perform preliminary work in preparation for drawing. This can be
     *  decoding, uploading to a GPU, or other tasks. These happen automatically when an image
     *  is drawn, and often they are cached so that the cost is only paid the first time.
     *
     *  Preroll() can be called before drawing to try to perform this prepatory work ahead of time.
     *  For images that have no such work, this returns instantly. Others may do some thing to
     *  prepare their cache and then return.
     *
     *  If the image will drawn to a GPU-backed canvas or surface, pass the associated GrContext.
     *  If the image will be drawn to any other type of canvas or surface, pass null.
     */
    void preroll(GrContext* = nullptr) const;

    // DEPRECATED - currently used by Canvas2DLayerBridge in Chromium.
    GrTexture* getTexture() const;

    /**
     *  Returns true if the image is texture backed.
     */
    bool isTextureBacked() const;

    /**
     *  Retrieves the backend API handle of the texture. If flushPendingGrContextIO then the
     *  GrContext will issue to the backend API any deferred IO operations on the texture before
     *  returning.
     */
    GrBackendObject getTextureHandle(bool flushPendingGrContextIO) const;

    /**
     *  Hints to image calls where the system might cache computed intermediates (e.g. the results
     *  of decoding or a read-back from the GPU. Passing kAllow signals that the system's default
     *  behavior is fine. Passing kDisallow signals that caching should be avoided.
     */
     enum CachingHint {
        kAllow_CachingHint,
        kDisallow_CachingHint,
    };

    /**
     *  Copy the pixels from the image into the specified buffer (pixels + rowBytes),
     *  converting them into the requested format (dstInfo). The image pixels are read
     *  starting at the specified (srcX,srcY) location.
     *
     *  The specified ImageInfo and (srcX,srcY) offset specifies a source rectangle
     *
     *      srcR.setXYWH(srcX, srcY, dstInfo.width(), dstInfo.height());
     *
     *  srcR is intersected with the bounds of the image. If this intersection is not empty,
     *  then we have two sets of pixels (of equal size). Replace the dst pixels with the
     *  corresponding src pixels, performing any colortype/alphatype transformations needed
     *  (in the case where the src and dst have different colortypes or alphatypes).
     *
     *  This call can fail, returning false, for several reasons:
     *  - If srcR does not intersect the image bounds.
     *  - If the requested colortype/alphatype cannot be converted from the image's types.
     */
    bool readPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes,
                    int srcX, int srcY, CachingHint = kAllow_CachingHint) const;

    bool readPixels(const SkPixmap& dst, int srcX, int srcY,
                    CachingHint = kAllow_CachingHint) const;

    /**
     *  Copy the pixels from this image into the dst pixmap, converting as needed into dst's
     *  colortype/alphatype. If the conversion cannot be performed, false is returned.
     *
     *  If dst's dimensions differ from the src dimension, the image will be scaled, applying the
     *  specified filter-quality.
     */
    bool scalePixels(const SkPixmap& dst, SkFilterQuality, CachingHint = kAllow_CachingHint) const;

    /**
     *  Encode the image's pixels and return the result as a new SkData, which
     *  the caller must manage (i.e. call unref() when they are done).
     *
     *  If the image type cannot be encoded, or the requested encoder type is
     *  not supported, this will return NULL.
     *
     *  Note: this will attempt to encode the image's pixels in the specified format,
     *  even if the image returns a data from refEncoded(). That data will be ignored.
     */
    SkData* encode(SkImageEncoder::Type, int quality) const;

    /**
     *  Encode the image and return the result as a caller-managed SkData.  This will
     *  attempt to reuse existing encoded data (as returned by refEncoded).
     *
     *  We defer to the SkPixelSerializer both for vetting existing encoded data
     *  (useEncodedData) and for encoding the image (encode) when no such data is
     *  present or is rejected by the serializer.
     *
     *  If not specified, we use a default serializer which 1) always accepts existing data
     *  (in any format) and 2) encodes to PNG.
     *
     *  If no compatible encoded data exists and encoding fails, this method will also
     *  fail (return NULL).
     */
    SkData* encode(SkPixelSerializer* = nullptr) const;

    /**
     *  If the image already has its contents in encoded form (e.g. PNG or JPEG), return a ref
     *  to that data (which the caller must call unref() on). The caller is responsible for calling
     *  unref on the data when they are done.
     *
     *  If the image does not already has its contents in encoded form, return NULL.
     *
     *  Note: to force the image to return its contents as encoded data, try calling encode(...).
     */
    SkData* refEncoded() const;

    const char* toString(SkString*) const;

    /**
     *  Return a new image that is a subset of this image. The underlying implementation may
     *  share the pixels, or it may make a copy.
     *
     *  If subset does not intersect the bounds of this image, or the copy/share cannot be made,
     *  NULL will be returned.
     */
    sk_sp<SkImage> makeSubset(const SkIRect& subset) const;

    /**
     *  Ensures that an image is backed by a texture (when GrContext is non-null). If no
     *  transformation is required, the returned image may be the same as this image. If the this
     *  image is from a different GrContext, this will fail.
     */
    sk_sp<SkImage> makeTextureImage(GrContext*) const;

    /**
     *  Apply a given image filter to this image, and return the filtered result.
     *
     *  The subset represents the active portion of this image. The return value is similarly an
     *  SkImage, with an active subset (outSubset). This is usually used with texture-backed
     *  images, where the texture may be approx-match and thus larger than the required size.
     *
     *  clipBounds constrains the device-space extent of the image which may be produced to the
     *  given rect.
     *
     *  offset is the amount to translate the resulting image relative to the src when it is drawn.
     *  This is an out-param.
     *
     *  If the result image cannot be created, or the result would be transparent black, null
     *  is returned, in which case the offset and outSubset parameters should be ignored by the
     *  caller.
     */
    sk_sp<SkImage> makeWithFilter(const SkImageFilter* filter, const SkIRect& subset,
                                  const SkIRect& clipBounds, SkIRect* outSubset,
                                  SkIPoint* offset) const;

    /** Drawing params for which a deferred texture image data should be optimized. */
    struct DeferredTextureImageUsageParams {
        DeferredTextureImageUsageParams() : fPreScaleMipLevel(0) {}
        DeferredTextureImageUsageParams(const SkMatrix matrix, const SkFilterQuality quality, 
                                        int preScaleMipLevel)
            : fMatrix(matrix), fQuality(quality), fPreScaleMipLevel(preScaleMipLevel) {}
        SkMatrix        fMatrix;
        SkFilterQuality fQuality;
        int             fPreScaleMipLevel;
    };

    /**
     * This method allows clients to capture the data necessary to turn a SkImage into a texture-
     * backed image. If the original image is codec-backed this will decode into a format optimized
     * for the context represented by the proxy. This method is thread safe with respect to the
     * GrContext whence the proxy came. Clients allocate and manage the storage of the deferred
     * texture data and control its lifetime. No cleanup is required, thus it is safe to simply free
     * the memory out from under the data.
     *
     * The same method is used both for getting the size necessary for pre-uploaded texture data
     * and for retrieving the data. The params array represents the set of draws over which to
     * optimize the pre-upload data.
     *
     * When called with a null buffer this returns the size that the client must allocate in order
     * to create deferred texture data for this image (or zero if this is an inappropriate
     * candidate). The buffer allocated by the client should be 8 byte aligned.
     *
     * When buffer is not null this fills in the deferred texture data for this image in the
     * provided buffer (assuming this is an appropriate candidate image and the buffer is
     * appropriately aligned). Upon success the size written is returned, otherwise 0.
     */
    size_t getDeferredTextureImageData(const GrContextThreadSafeProxy&,
                                       const DeferredTextureImageUsageParams[],
                                       int paramCnt,
                                       void* buffer) const;

    /**
     * Returns a texture-backed image from data produced in SkImage::getDeferredTextureImageData.
     * The context must be the context that provided the proxy passed to
     * getDeferredTextureImageData.
     */
    static sk_sp<SkImage> MakeFromDeferredTextureImageData(GrContext*, const void*, SkBudgeted);

    // Helper functions to convert to SkBitmap

    enum LegacyBitmapMode {
        kRO_LegacyBitmapMode,
        kRW_LegacyBitmapMode,
    };

    /**
     *  Attempt to create a bitmap with the same pixels as the image. The result will always be
     *  a raster-backed bitmap (texture-backed bitmaps are DEPRECATED, and not supported here).
     *
     *  If the mode is kRO (read-only), the resulting bitmap will be marked as immutable.
     *
     *  On succcess, returns true. On failure, returns false and the bitmap parameter will be reset
     *  to empty.
     */
    bool asLegacyBitmap(SkBitmap*, LegacyBitmapMode) const;

    /**
     *  Returns true if the image is backed by an image-generator or other src that creates
     *  (and caches) its pixels / texture on-demand.
     */
    bool isLazyGenerated() const;


#ifdef SK_SUPPORT_LEGACY_IMAGEFACTORY
    static SkImage* NewRasterCopy(const Info&, const void* pixels, size_t rowBytes,
                                  SkColorTable* ctable = nullptr);
    static SkImage* NewRasterData(const Info&, SkData* pixels, size_t rowBytes);
    static SkImage* NewFromRaster(const Info&, const void* pixels, size_t rowBytes,
                                  RasterReleaseProc, ReleaseContext);
    static SkImage* NewFromBitmap(const SkBitmap&);
    static SkImage* NewFromGenerator(SkImageGenerator*, const SkIRect* subset = NULL);
    static SkImage* NewFromEncoded(SkData* encoded, const SkIRect* subset = NULL);
    static SkImage* NewFromTexture(GrContext* ctx, const GrBackendTextureDesc& desc) {
        return NewFromTexture(ctx, desc, kPremul_SkAlphaType, NULL, NULL);
    }

    static SkImage* NewFromTexture(GrContext* ctx, const GrBackendTextureDesc& de, SkAlphaType at) {
        return NewFromTexture(ctx, de, at, NULL, NULL);
    }
    static SkImage* NewFromTexture(GrContext*, const GrBackendTextureDesc&, SkAlphaType,
                                   TextureReleaseProc, ReleaseContext);
    static SkImage* NewFromAdoptedTexture(GrContext*, const GrBackendTextureDesc&,
                                          SkAlphaType = kPremul_SkAlphaType);
    static SkImage* NewFromTextureCopy(GrContext*, const GrBackendTextureDesc&,
                                       SkAlphaType = kPremul_SkAlphaType);
    static SkImage* NewFromYUVTexturesCopy(GrContext*, SkYUVColorSpace,
                                           const GrBackendObject yuvTextureHandles[3],
                                           const SkISize yuvSizes[3],
                                           GrSurfaceOrigin);
    static SkImage* NewFromPicture(const SkPicture*, const SkISize& dimensions,
                                   const SkMatrix*, const SkPaint*);
    static SkImage* NewTextureFromPixmap(GrContext*, const SkPixmap&, SkBudgeted budgeted);
    static SkImage* NewFromDeferredTextureImageData(GrContext*, const void*, SkBudgeted);

    SkImage* newSubset(const SkIRect& subset) const { return this->makeSubset(subset).release(); }
    SkImage* newTextureImage(GrContext* ctx) const { return this->makeTextureImage(ctx).release(); }
#endif

protected:
    SkImage(int width, int height, uint32_t uniqueID);

private:
    static sk_sp<SkImage> MakeTextureFromMipMap(GrContext*, const SkImageInfo&,
                                                const GrMipLevel* texels, int mipLevelCount,
                                                SkBudgeted);

    const int       fWidth;
    const int       fHeight;
    const uint32_t  fUniqueID;

    typedef SkRefCnt INHERITED;
};

#endif
