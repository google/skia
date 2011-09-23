/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageFilter_DEFINED
#define SkImageFilter_DEFINED

#include "SkFlattenable.h"

class SkImageFilter : public SkFlattenable {
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
                     SkBitmap* result, SkPoint* offset);

protected:
    virtual bool onFilterImage(const SkBitmap& src, const SkMatrix&
                               SkBitmap* result, SkPoint* offset) = 0;

private:
    typedef SkFlattenable INHERITED;
};

#endif
