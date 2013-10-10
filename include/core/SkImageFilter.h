/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageFilter_DEFINED
#define SkImageFilter_DEFINED

#include "SkFlattenable.h"
#include "SkRect.h"

class SkBitmap;
class SkColorFilter;
class SkBaseDevice;
class SkMatrix;
struct SkIPoint;
class SkShader;
class GrEffectRef;
class GrTexture;

/**
 *  Base class for image filters. If one is installed in the paint, then
 *  all drawing occurs as usual, but it is as if the drawing happened into an
 *  offscreen (before the xfermode is applied). This offscreen bitmap will
 *  then be handed to the imagefilter, who in turn creates a new bitmap which
 *  is what will finally be drawn to the device (using the original xfermode).
 */
class SK_API SkImageFilter : public SkFlattenable {
public:
    SK_DECLARE_INST_COUNT(SkImageFilter)

#ifdef SK_CROP_RECT_IS_INT
    typedef SkIRect CropRect;
#else
    struct CropRect {
        SkRect fRect;
        uint32_t fFlags;
        enum CropEdge {
            kHasLeft_CropEdge   = 0x01,
            kHasTop_CropEdge    = 0x02,
            kHasRight_CropEdge  = 0x04,
            kHasBottom_CropEdge = 0x08,
            kHasAll_CropEdge    = 0x0F,
        };
        CropRect() {}
        explicit CropRect(const SkRect& rect, uint32_t flags = kHasAll_CropEdge) : fRect(rect), fFlags(flags) {}
        bool isSet() const
        {
            return fFlags != 0x0;
        }
    };
#endif

    class Proxy {
    public:
        virtual ~Proxy() {};

        virtual SkBaseDevice* createDevice(int width, int height) = 0;
        // returns true if the proxy can handle this filter natively
        virtual bool canHandleImageFilter(SkImageFilter*) = 0;
        // returns true if the proxy handled the filter itself. if this returns
        // false then the filter's code will be called.
        virtual bool filterImage(SkImageFilter*, const SkBitmap& src,
                                 const SkMatrix& ctm,
                                 SkBitmap* result, SkIPoint* offset) = 0;
    };

    /**
     *  Request a new (result) image to be created from the src image.
     *  If the src has no pixels (isNull()) then the request just wants to
     *  receive the config and width/height of the result.
     *
     *  The matrix is the current matrix on the canvas.
     *
     *  Offset is the amount to translate the resulting image relative to the
     *  src when it is drawn.
     *
     *  If the result image cannot be created, return false, in which case both
     *  the result and offset parameters will be ignored by the caller.
     */
    bool filterImage(Proxy*, const SkBitmap& src, const SkMatrix& ctm,
                     SkBitmap* result, SkIPoint* offset);

    /**
     *  Given the src bounds of an image, this returns the bounds of the result
     *  image after the filter has been applied.
     */
    bool filterBounds(const SkIRect& src, const SkMatrix& ctm, SkIRect* dst);

    /**
     *  Returns true if the filter can be expressed a single-pass
     *  GrEffect, used to process this filter on the GPU, or false if
     *  not.
     *
     *  If effect is non-NULL, a new GrEffect instance is stored
     *  in it.  The caller assumes ownership of the stage, and it is up to the
     *  caller to unref it.
     *
     *  The effect can assume its vertexCoords space maps 1-to-1 with texels
     *  in the texture.  "matrix" is a transformation to apply to filter
     *  parameters before they are used in the effect. Note that this function
     *  will be called with (NULL, NULL, SkMatrix::I()) to query for support,
     *  so returning "true" indicates support for all possible matrices.
     */
    virtual bool asNewEffect(GrEffectRef** effect, GrTexture*, const SkMatrix& matrix) const;

    /**
     *  Returns true if the filter can be processed on the GPU.  This is most
     *  often used for multi-pass effects, where intermediate results must be
     *  rendered to textures.  For single-pass effects, use asNewEffect().
     *  The default implementation returns asNewEffect(NULL, NULL, SkMatrix::I()).
     */
    virtual bool canFilterImageGPU() const;

    /**
     *  Process this image filter on the GPU.  This is most often used for
     *  multi-pass effects, where intermediate results must be rendered to
     *  textures.  For single-pass effects, use asNewEffect().  src is the
     *  source image for processing, as a texture-backed bitmap.  result is
     *  the destination bitmap, which should contain a texture-backed pixelref
     *  on success.  offset is the amount to translate the resulting image
     *  relative to the src when it is drawn. The default implementation does
     *  single-pass processing using asNewEffect().
     */
    virtual bool filterImageGPU(Proxy*, const SkBitmap& src, const SkMatrix& ctm,
                                SkBitmap* result, SkIPoint* offset);

    /**
     *  Returns whether this image filter is a color filter and puts the color filter into the
     *  "filterPtr" parameter if it can. Does nothing otherwise.
     *  If this returns false, then the filterPtr is unchanged.
     *  If this returns true, then if filterPtr is not null, it must be set to a ref'd colorfitler
     *  (i.e. it may not be set to NULL).
     */
    virtual bool asColorFilter(SkColorFilter** filterPtr) const;

    /**
     *  Returns the number of inputs this filter will accept (some inputs can
     *  be NULL).
     */
    int countInputs() const { return fInputCount; }

    /**
     *  Returns the input filter at a given index, or NULL if no input is
     *  connected.  The indices used are filter-specific.
     */
    SkImageFilter* getInput(int i) const {
        SkASSERT(i < fInputCount);
        return fInputs[i];
    }

    /**
     *  Returns the crop rectangle of this filter. This is set at construction
     *  time, and determines which pixels from the input image will
     *  be processed. The size of this rectangle should be used as the size
     *  of the destination image. The origin of this rect should be used to
     *  offset access to the input images, and should also be added to the
     *  "offset" parameter in onFilterImage and filterImageGPU(). (The latter
     *  ensures that the resulting buffer is drawn in the correct location.)
     */
#ifdef SK_CROP_RECT_IS_INT
    bool cropRectIsSet() const { return !fCropRect.isLargest(); }
#else
    bool cropRectIsSet() const { return fCropRect.isSet(); }
#endif

protected:
    SkImageFilter(int inputCount, SkImageFilter** inputs, const CropRect* cropRect = NULL);

    // Convenience constructor for 1-input filters.
    explicit SkImageFilter(SkImageFilter* input, const CropRect* cropRect = NULL);

    // Convenience constructor for 2-input filters.
    SkImageFilter(SkImageFilter* input1, SkImageFilter* input2, const CropRect* cropRect = NULL);

    virtual ~SkImageFilter();

    explicit SkImageFilter(SkFlattenableReadBuffer& rb);

    virtual void flatten(SkFlattenableWriteBuffer& wb) const SK_OVERRIDE;

    // Default impl returns false
    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const SkMatrix&,
                               SkBitmap* result, SkIPoint* offset);
    // Default impl copies src into dst and returns true
    virtual bool onFilterBounds(const SkIRect&, const SkMatrix&, SkIRect*);

    // Applies "matrix" to the crop rect, and sets "rect" to the intersection of
    // "rect" and the transformed crop rect. If there is no overlap, returns
    // false and leaves "rect" unchanged.
    bool applyCropRect(SkIRect* rect, const SkMatrix& matrix) const;

private:
    typedef SkFlattenable INHERITED;
    int fInputCount;
    SkImageFilter** fInputs;
    CropRect fCropRect;
};

#endif
