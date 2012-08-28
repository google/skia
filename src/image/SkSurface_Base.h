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
     *  If the surface is about to change, we call this so that our subclass
     *  can optionally fork their backend (copy-on-write) in case it was
     *  being shared with the cachedImage.
     *
     *  The default implementation does nothing.
     */
    virtual void onCopyOnWrite(SkImage* cachedImage, SkCanvas*);

    inline SkCanvas* getCachedCanvas();
    inline SkImage* getCachedImage();

    // called by SkSurface to compute a new genID
    uint32_t newGenerationID();

private:
    SkCanvas*   fCachedCanvas;
    SkImage*    fCachedImage;

    void aboutToDraw(SkCanvas*);
    friend class SkCanvas;
    friend class SkSurface;

    inline void installIntoCanvasForDirtyNotification();

    typedef SkSurface INHERITED;
};

#endif

