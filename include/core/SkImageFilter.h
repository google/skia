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
class SkMatrix;
struct SkPoint;

/**
 *  Experimental.
 *
 *  Base class for image filters. If one is installed in the paint, then
 *  all drawing occurs as usual, but it is as if the drawing happened into an
 *  offscreen (before the xfermode is applied). This offscreen bitmap will
 *  then be handed to the imagefilter, who in turn creates a new bitmap which
 *  is what will finally be drawn to the device (using the original xfermode).
 *
 *  If the imagefilter returns false, nothing is drawn.
 */
class SK_API SkImageFilter : public SkFlattenable {
public:

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
    bool filterImage(const SkBitmap& src, const SkMatrix&,
                     SkBitmap* result, SkIPoint* offset);

    /**
     *  Experimental.
     *
     *  If the filter can be expressed as a gaussian-blur, return true and
     *  set the sigma to the values for horizontal and vertical.
     */
    virtual bool asABlur(SkSize* sigma) const;

protected:
    SkImageFilter() {}
    explicit SkImageFilter(SkFlattenableReadBuffer& rb) : INHERITED(rb) {}
    virtual bool onFilterImage(const SkBitmap& src, const SkMatrix&,
                               SkBitmap* result, SkIPoint* offset);

private:
    typedef SkFlattenable INHERITED;
};

#endif
