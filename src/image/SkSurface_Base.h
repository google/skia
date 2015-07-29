/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSurface_Base_DEFINED
#define SkSurface_Base_DEFINED

#include "SkCanvas.h"
#include "SkSurface.h"
#include "SkSurfacePriv.h"

class SkSurface_Base : public SkSurface {
public:
    SkSurface_Base(int width, int height, const SkSurfaceProps*);
    SkSurface_Base(const SkImageInfo&, const SkSurfaceProps*);
    virtual ~SkSurface_Base();

    virtual GrBackendObject onGetTextureHandle(BackendHandleAccess) {
        return 0;
    }

    virtual bool onGetRenderTargetHandle(GrBackendObject*, BackendHandleAccess) {
        return false;
    }

    /**
     *  Allocate a canvas that will draw into this surface. We will cache this
     *  canvas, to return the same object to the caller multiple times. We
     *  take ownership, and will call unref() on the canvas when we go out of
     *  scope.
     */
    virtual SkCanvas* onNewCanvas() = 0;

    virtual SkSurface* onNewSurface(const SkImageInfo&) = 0;

    /**
     *  Allocate an SkImage that represents the current contents of the surface.
     *  This needs to be able to outlive the surface itself (if need be), and
     *  must faithfully represent the current contents, even if the surface
     *  is changed after this called (e.g. it is drawn to via its canvas).
     */
    virtual SkImage* onNewImageSnapshot(Budgeted) = 0;

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
     * Called as a performance hint when the Surface is allowed to make it's contents
     * undefined.
     */
    virtual void onDiscard() {}

    /**
     *  If the surface is about to change, we call this so that our subclass
     *  can optionally fork their backend (copy-on-write) in case it was
     *  being shared with the cachedImage.
     */
    virtual void onCopyOnWrite(ContentChangeMode) = 0;

    /**
     *  Signal the surface to remind its backing store that it's mutable again.
     *  Called only when we _didn't_ copy-on-write; we assume the copies start mutable.
     */
    virtual void onRestoreBackingMutability() {}

    inline SkCanvas* getCachedCanvas();
    inline SkImage* getCachedImage(Budgeted);

    bool hasCachedImage() const { return fCachedImage != NULL; }

    // called by SkSurface to compute a new genID
    uint32_t newGenerationID();

private:
    SkCanvas*   fCachedCanvas;
    SkImage*    fCachedImage;

    void aboutToDraw(ContentChangeMode mode);

    // Returns true if there is an outstanding image-snapshot, indicating that a call to aboutToDraw
    // would trigger a copy-on-write.
    bool outstandingImageSnapshot() const;

    friend class SkCanvas;
    friend class SkSurface;

    typedef SkSurface INHERITED;
};

SkCanvas* SkSurface_Base::getCachedCanvas() {
    if (NULL == fCachedCanvas) {
        fCachedCanvas = this->onNewCanvas();
        if (fCachedCanvas) {
            fCachedCanvas->setSurfaceBase(this);
        }
    }
    return fCachedCanvas;
}

SkImage* SkSurface_Base::getCachedImage(Budgeted budgeted) {
    if (NULL == fCachedImage) {
        fCachedImage = this->onNewImageSnapshot(budgeted);
        SkASSERT(!fCachedCanvas || fCachedCanvas->getSurfaceBase() == this);
    }
    return fCachedImage;
}

#endif
