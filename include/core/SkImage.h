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
class SkImageGenerator;
class SkPaint;
class SkString;
class SkSurface;
class SkSurfaceProps;
class GrContext;
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
    SK_DECLARE_INST_COUNT(SkImage)

    typedef SkImageInfo Info;

    static SkImage* NewRasterCopy(const Info&, const void* pixels, size_t rowBytes);
    static SkImage* NewRasterData(const Info&, SkData* pixels, size_t rowBytes);

    /**
     *  Construct a new SkImage based on the given ImageGenerator.
     *  This function will always take ownership of the passed
     *  ImageGenerator.  Returns NULL on error.
     */
    static SkImage* NewFromGenerator(SkImageGenerator*);

    /**
     *  Construct a new SkImage based on the specified encoded data. Returns NULL on failure,
     *  which can mean that the format of the encoded data was not recognized/supported.
     *
     *  Regardless of success or failure, the caller is responsible for managing their ownership
     *  of the data.
     */
    static SkImage* NewFromData(SkData* data);

    /**
     *  Create a new image from the specified descriptor. Note - the caller is responsible for
     *  managing the lifetime of the underlying platform texture.
     *
     *  Will return NULL if the specified descriptor is unsupported.
     */
    static SkImage* NewFromTexture(GrContext*, const GrBackendTextureDesc&,
                                   SkAlphaType = kPremul_SkAlphaType);

    /**
     *  Create a new image by copying the pixels from the specified descriptor. No reference is
     *  kept to the original platform texture.
     *
     *  Will return NULL if the specified descriptor is unsupported.
     */
    static SkImage* NewFromTextureCopy(GrContext*, const GrBackendTextureDesc&,
                                       SkAlphaType = kPremul_SkAlphaType);

    int width() const { return fWidth; }
    int height() const { return fHeight; }
    uint32_t uniqueID() const { return fUniqueID; }
    virtual bool isOpaque() const { return false; }

    /**
     * Return the GrTexture that stores the image pixels. Calling getTexture
     * does not affect the reference count of the GrTexture object.
     * Will return NULL if the image does not use a texture.
     */
    GrTexture* getTexture() const;

    virtual SkShader* newShader(SkShader::TileMode,
                                SkShader::TileMode,
                                const SkMatrix* localMatrix = NULL) const;

    /**
     *  If the image has direct access to its pixels (i.e. they are in local
     *  RAM) return the (const) address of those pixels, and if not null, return
     *  the ImageInfo and rowBytes. The returned address is only valid while
     *  the image object is in scope.
     *
     *  On failure, returns NULL and the info and rowBytes parameters are
     *  ignored.
     */
    const void* peekPixels(SkImageInfo* info, size_t* rowBytes) const;

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
                    int srcX, int srcY) const;

    /**
     *  Encode the image's pixels and return the result as a new SkData, which
     *  the caller must manage (i.e. call unref() when they are done).
     *
     *  If the image type cannot be encoded, or the requested encoder type is
     *  not supported, this will return NULL.
     */
    SkData* encode(SkImageEncoder::Type t = SkImageEncoder::kPNG_Type,
                   int quality = 80) const;

    /**
     *  Return a new surface that is compatible with this image's internal representation
     *  (e.g. raster or gpu).
     *
     *  If no surfaceprops are specified, the image will attempt to match the props of when it
     *  was created (if it came from a surface).
     */
    SkSurface* newSurface(const SkImageInfo&, const SkSurfaceProps* = NULL) const;

    const char* toString(SkString*) const;

    /**
     *  Return an image that is a rescale of this image (using newWidth, newHeight).
     *
     *  If subset is NULL, then the entire original image is used as the src for the scaling.
     *  If subset is not NULL, then it specifies subset of src-pixels used for scaling. If
     *  subset extends beyond the bounds of the original image, then NULL is returned.
     *
     *  Notes:
     *  - newWidth and newHeight must be > 0 or NULL will be returned.
     *
     *  - it is legal for the returned image to be the same instance as the src image
     *    (if the new dimensions == the src dimensions and subset is NULL or == src dimensions).
     *
     *  - it is legal for the "scaled" image to have changed its SkAlphaType from unpremul
     *    to premul (as required by the impl). The image should draw (nearly) identically,
     *    since during drawing we will "apply the alpha" to the pixels. Future optimizations
     *    may take away this caveat, preserving unpremul.
     */
    SkImage* newImage(int newWidth, int newHeight, const SkIRect* subset = NULL,
                      SkFilterQuality = kNone_SkFilterQuality) const;

protected:
    SkImage(int width, int height) :
        fWidth(width),
        fHeight(height),
        fUniqueID(NextUniqueID()) {

        SkASSERT(width > 0);
        SkASSERT(height > 0);
    }

private:
    const int       fWidth;
    const int       fHeight;
    const uint32_t  fUniqueID;

    static uint32_t NextUniqueID();

    typedef SkRefCnt INHERITED;
};

#endif
