/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageFilter_DEFINED
#define SkImageFilter_DEFINED

#include "SkFlattenable.h"

class SkBitmap;
class SkDevice;
class SkMatrix;
struct SkIPoint;
struct SkIRect;
struct SkRect;
class GrCustomStage;
class GrTexture;

/**
 *  Experimental.
 *
 *  Base class for image filters. If one is installed in the paint, then
 *  all drawing occurs as usual, but it is as if the drawing happened into an
 *  offscreen (before the xfermode is applied). This offscreen bitmap will
 *  then be handed to the imagefilter, who in turn creates a new bitmap which
 *  is what will finally be drawn to the device (using the original xfermode).
 *
 *  THIS SIGNATURE IS TEMPORARY
 *
 *  There are several weaknesses in this function signature:
 *  1. Does not expose the destination/target device, so filters that can draw
 *     directly to it are unable to take advantage of that optimization.
 *  2. Does not expose a way to create a "compabitible" image (i.e. gpu -> gpu)
 *  3. As with #1, the filter is unable to "read" the dest (which would be slow)
 *
 *  Therefore, we should not create any real dependencies on this API yet -- it
 *  is being checked in as a check-point so we can explore these and other
 *  considerations.
 */
class SK_API SkImageFilter : public SkFlattenable {
public:
    SK_DECLARE_INST_COUNT(SkImageFilter)

    class Proxy {
    public:
        virtual ~Proxy() {};

        virtual SkDevice* createDevice(int width, int height) = 0;
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
     *  GrCustomStage, used to process this filter on the GPU, or false if
     *  not.
     *
     *  If stage is non-NULL, a new GrCustomStage instance is stored
     *  in it.  The caller assumes ownership of the stage, and it is up to the
     *  caller to unref it.
     */
    virtual bool asNewCustomStage(GrCustomStage** stage, GrTexture*) const;

    /**
     *  Returns true if the filter can be processed on the GPU.  This is most
     *  often used for multi-pass effects, where intermediate results must be
     *  rendered to textures.  For single-pass effects, use asNewCustomStage().
     *  The default implementation returns false.
     */
    virtual bool canFilterImageGPU() const;

    /**
     *  Process this image filter on the GPU.  texture is the source texture
     *  for processing, and rect is the effect region to process.  The
     *  function must allocate a new texture of at least rect width/height
     *  size, and return it to the caller.  The default implementation returns
     *  NULL.
     */
    virtual GrTexture* onFilterImageGPU(GrTexture* texture, const SkRect& rect);

protected:
    SkImageFilter() {}
    explicit SkImageFilter(SkFlattenableReadBuffer& rb) : INHERITED(rb) {}

    // Default impl returns false
    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const SkMatrix&,
                               SkBitmap* result, SkIPoint* offset);
    // Default impl copies src into dst and returns true
    virtual bool onFilterBounds(const SkIRect&, const SkMatrix&, SkIRect*);

private:
    typedef SkFlattenable INHERITED;
};

#endif
