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
class GrBackendTexture;
class GrContext;
class GrContextThreadSafeProxy;
class GrTexture;

struct AHardwareBuffer;

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
    static sk_sp<SkImage> MakeFromGenerator(std::unique_ptr<SkImageGenerator>,
                                            const SkIRect* subset = nullptr);

    /**
     *  Construct a new SkImage based on the specified encoded data. Returns NULL on failure,
     *  which can mean that the format of the encoded data was not recognized/supported.
     *
     *  If a subset is specified, it must be contained within the encoded data's bounds.
     */
    static sk_sp<SkImage> MakeFromEncoded(sk_sp<SkData> encoded, const SkIRect* subset = nullptr);

    typedef void (*TextureReleaseProc)(ReleaseContext);

    /**
     *  Create a new image from the specified descriptor. Note - the caller is responsible for
     *  managing the lifetime of the underlying platform texture.
     *
     *  Will return NULL if the specified backend texture is unsupported.
     */
    static sk_sp<SkImage> MakeFromTexture(GrContext* ctx,
                                          const GrBackendTexture& tex, GrSurfaceOrigin origin,
                                          SkAlphaType at, sk_sp<SkColorSpace> cs) {
        return MakeFromTexture(ctx, tex, origin, at, cs, nullptr, nullptr);
    }

    /**
     *  Create a new image from the GrBackendTexture. The underlying platform texture must stay
     *  valid and unaltered until the specified release-proc is invoked, indicating that Skia
     *  no longer is holding a reference to it.
     *
     *  Will return NULL if the specified backend texture is unsupported.
     */
    static sk_sp<SkImage> MakeFromTexture(GrContext*,
                                          const GrBackendTexture&, GrSurfaceOrigin origin,
                                          SkAlphaType, sk_sp<SkColorSpace>,
                                          TextureReleaseProc, ReleaseContext);

    /**
     *  Decodes and uploads the encoded data to a GPU backed image using the supplied GrContext.
     *  That image can be safely used by other GrContexts, across thread boundaries. The GrContext
     *  used here, and the ones used to draw this image later must be in the same GL share group,
     *  or otherwise be able to share resources.
     *
     *  When the image's ref count reaches zero, the original GrContext will destroy the texture,
     *  asynchronously.
     *
     *  The texture will be decoded and uploaded to be suitable for use with surfaces that have the
     *  supplied destination color space. The color space of the image itself will be determined
     *  from the encoded data.
     */
    static sk_sp<SkImage> MakeCrossContextFromEncoded(GrContext*, sk_sp<SkData>, bool buildMips,
                                                      SkColorSpace* dstColorSpace);

    /**
     *  Create a new image from the specified descriptor. Note - Skia will delete or recycle the
     *  texture when the image is released.
     *
     *  Will return NULL if the specified backend texture is unsupported.
     */
    static sk_sp<SkImage> MakeFromAdoptedTexture(GrContext*,
                                                 const GrBackendTexture&, GrSurfaceOrigin,
                                                 SkAlphaType = kPremul_SkAlphaType,
                                                 sk_sp<SkColorSpace> = nullptr);

    /**
     *  Create a new image by copying the pixels from the specified y, u, v textures. The data
     *  from the textures is immediately ingested into the image and the textures can be modified or
     *  deleted after the function returns. The image will have the dimensions of the y texture.
     */
    static sk_sp<SkImage> MakeFromYUVTexturesCopy(GrContext*, SkYUVColorSpace,
                                                  const GrBackendObject yuvTextureHandles[3],
                                                  const SkISize yuvSizes[3],
                                                  GrSurfaceOrigin,
                                                  sk_sp<SkColorSpace> = nullptr);

    /**
     *  Create a new image by copying the pixels from the specified y and uv textures. The data
     *  from the textures is immediately ingested into the image and the textures can be modified or
     *  deleted after the function returns. The image will have the dimensions of the y texture.
     */
    static sk_sp<SkImage> MakeFromNV12TexturesCopy(GrContext*, SkYUVColorSpace,
                                                   const GrBackendObject nv12TextureHandles[2],
                                                   const SkISize nv12Sizes[2], GrSurfaceOrigin,
                                                   sk_sp<SkColorSpace> = nullptr);

    enum class BitDepth {
        kU8,
        kF16,
    };

    /**
     *  Create a new image from the specified picture.
     *  On creation of the SkImage, snap the SkPicture to a particular BitDepth and SkColorSpace.
     */
    static sk_sp<SkImage> MakeFromPicture(sk_sp<SkPicture>, const SkISize& dimensions,
                                          const SkMatrix*, const SkPaint*, BitDepth,
                                          sk_sp<SkColorSpace>);

#if defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26
    /**
     *  Create a new image from the an Android hardware buffer.
     *  The new image takes a reference on the buffer.
     */
    static sk_sp<SkImage> MakeFromAHardwareBuffer(AHardwareBuffer*,
                                                 SkAlphaType = kPremul_SkAlphaType,
                                                 sk_sp<SkColorSpace> = nullptr);
#endif

    ///////////////////////////////////////////////////////////////////////////////////////////////

    int width() const { return fWidth; }
    int height() const { return fHeight; }
    SkISize dimensions() const { return SkISize::Make(fWidth, fHeight); }
    SkIRect bounds() const { return SkIRect::MakeWH(fWidth, fHeight); }
    uint32_t uniqueID() const { return fUniqueID; }
    SkAlphaType alphaType() const;

    /**
     *  Returns the color space of the SkImage.
     *
     *  This is the color space that was supplied on creation of the SkImage or a color
     *  space that was parsed from encoded data.  This color space is not guaranteed to be
     *  renderable.  Can return nullptr if the SkImage was created without a color space.
     */
    SkColorSpace* colorSpace() const;
    sk_sp<SkColorSpace> refColorSpace() const;

    /**
     *  Returns true fi the image will be drawn as a mask, with no intrinsic color of its own.
     */
    bool isAlphaOnly() const;
    bool isOpaque() const { return SkAlphaTypeIsOpaque(this->alphaType()); }

    sk_sp<SkShader> makeShader(SkShader::TileMode, SkShader::TileMode,
                               const SkMatrix* localMatrix = nullptr) const;
    /**
     *  Helper version of makeShader() that specifies Clamp tilemode.
     */
    sk_sp<SkShader> makeShader(const SkMatrix* localMatrix = nullptr) const {
        return this->makeShader(SkShader::kClamp_TileMode, SkShader::kClamp_TileMode, localMatrix);
    }

    /**
     *  If the image has direct access to its pixels (i.e. they are in local RAM)
     *  return true, and if not null, return in the pixmap parameter the info about the
     *  images pixels.
     *
     *  On failure, return false and ignore the pixmap parameter.
     */
    bool peekPixels(SkPixmap* pixmap) const;

    // DEPRECATED - currently used by Canvas2DLayerBridge in Chromium.
    GrTexture* getTexture() const;

    /**
     *  Returns true if the image is texture backed.
     */
    bool isTextureBacked() const;

    /**
     *  Returns true if the image is able to be drawn to a particular type of device. If context
     *  is nullptr, tests for drawability to CPU devices. Otherwise, tests for drawability to a GPU
     *  device backed by context.
     *
     *  Texture-backed images may become invalid if their underlying GrContext is abandoned. Some
     *  generator-backed images may be invalid for CPU and/or GPU.
     */
    bool isValid(GrContext* context) const;

    /**
     *  Retrieves the backend API handle of the texture. If flushPendingGrContextIO then the
     *  GrContext will issue to the backend API any deferred IO operations on the texture before
     *  returning.
     *  If 'origin' is supplied it will be filled in with the origin of the content drawn
     *  into the image.
     */
    GrBackendObject getTextureHandle(bool flushPendingGrContextIO,
                                     GrSurfaceOrigin* origin = nullptr) const;

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
     *  Encode the image's pixels and return the result as SkData.
     *
     *  If the image type cannot be encoded, or the requested encoder format is
     *  not supported, this will return NULL.
     */
    sk_sp<SkData> encodeToData(SkEncodedImageFormat, int quality) const;

    /**
     *  Encode the image and return the result as SkData.  This will attempt to reuse existing
     *  encoded data (as returned by refEncodedData).
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
    sk_sp<SkData> encodeToData(SkPixelSerializer* = nullptr) const;

    /**
     *  If the image already has its contents in encoded form (e.g. PNG or JPEG), return that
     *  as SkData. If the image does not already has its contents in encoded form, return NULL.
     *
     *  Note: to force the image to return its contents as encoded data, use encodeToData(...).
     */
    sk_sp<SkData> refEncodedData() const;

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
     *  Ensures that an image is backed by a texture (when GrContext is non-null), suitable for use
     *  with surfaces that have the supplied destination color space. If no transformation is
     *  required, the returned image may be the same as this image. If this image is from a
     *  different GrContext, this will fail.
     */
    sk_sp<SkImage> makeTextureImage(GrContext*, SkColorSpace* dstColorSpace) const;

    /**
     * If the image is texture-backed this will make a raster copy of it (or nullptr if reading back
     * the pixels fails). Otherwise, it returns the original image.
     */
    sk_sp<SkImage> makeNonTextureImage() const;
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
     *
     * dstColorSpace is the color space of the surface where this texture will ultimately be used.
     * If the method determines that mip-maps are needed, this helps determine the correct strategy
     * for building them (gamma-correct or not).
     *
     * dstColorType is the color type of the surface where this texture will ultimately be used.
     * This determines the format with which the image will be uploaded to the GPU. If dstColorType
     * does not support color spaces (low bit depth types such as ARGB_4444), then dstColorSpace
     * must be null.
     */
    size_t getDeferredTextureImageData(const GrContextThreadSafeProxy&,
                                       const DeferredTextureImageUsageParams[],
                                       int paramCnt,
                                       void* buffer,
                                       SkColorSpace* dstColorSpace = nullptr,
                                       SkColorType dstColorType = kN32_SkColorType) const;

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

    /**
     *  If |target| is supported, returns an SkImage in the |target| color space.
     *  Otherwise, returns nullptr.
     *
     *  This will leave the image as is if it already in the |target| color space.
     *  Otherwise, it will convert the pixels from the src color space to the |target|
     *  color space.  If this->colorSpace() is nullptr, the src color space will be
     *  treated as sRGB.
     *
     *  If |premulBehavior| is kIgnore, any premultiplication or unpremultiplication will
     *  be performed in the gamma encoded space.  If it is kRespect, premultiplication is
     *  assumed to be linear.
     */
    sk_sp<SkImage> makeColorSpace(sk_sp<SkColorSpace> target,
                                  SkTransferFunctionBehavior premulBehavior) const;

private:
    SkImage(int width, int height, uint32_t uniqueID);
    friend class SkImage_Base;

    static sk_sp<SkImage> MakeTextureFromMipMap(GrContext*, const SkImageInfo&,
                                                const GrMipLevel texels[], int mipLevelCount,
                                                SkBudgeted, SkDestinationSurfaceColorMode);

    const int       fWidth;
    const int       fHeight;
    const uint32_t  fUniqueID;

    typedef SkRefCnt INHERITED;
};

#endif
