/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSurface_Base_DEFINED
#define SkSurface_Base_DEFINED

#include "SkSurface.h"

class SkSurface_Base : public SkSurface {
public:
    SkSurface_Base(int width, int height);
    virtual ~SkSurface_Base();

    /**
     *  Allocate a canvas that will draw into this surface. We will cache this
     *  canvas, to return the same object to the caller multiple times. We
     *  take ownership, and will call unref() on the canvas when we go out of
     *  scope.
     */
    virtual SkCanvas* onNewCanvas() = 0;

    virtual SkSurface* onNewSurface(const SkImage::Info&, SkColorSpace*) = 0;

    /**
     *  Allocate an SkImage that represents the current contents of the surface.
     *  This needs to be able to outlive the surface itself (if need be), and
     *  must faithfully represent the current contents, even if the surface
     *  is chaged after this calle (e.g. it is drawn to via its canvas).
     */
    virtual SkImage* onNewImageShapshot() = 0;

    /**
     *  Default implementation:
     *
     *  image = this->newImageSnapshot();
     *  if (image) {
     *      image->draw(canvas, ...);
     *      image->unref();
     *  }
     */
    virtual void onDraw(SkCanvas*, SkScalar x, SkScalar y, const SkPaint*);

    /**
     *  Returns a the result of onNewCanvas(), but caches it so that only one
     *  canvas never ever be created.
     */
    SkCanvas* getCachedCanvas() {
        if (NULL == fCachedCanvas) {
            fCachedCanvas = this->onNewCanvas();
        }
        return fCachedCanvas;
    }

private:
    SkCanvas*   fCachedCanvas;

    typedef SkSurface INHERITED;
};

#endif

