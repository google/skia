/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSurface_Base.h"
#include "SkImagePriv.h"
#include "SkCanvas.h"

///////////////////////////////////////////////////////////////////////////////

SkSurface_Base::SkSurface_Base(int width, int height) : INHERITED(width, height) {
    fCachedCanvas = NULL;
    fCachedImage = NULL;
}

SkSurface_Base::SkSurface_Base(const SkImageInfo& info) : INHERITED(info) {
    fCachedCanvas = NULL;
    fCachedImage = NULL;
}

SkSurface_Base::~SkSurface_Base() {
    // in case the canvas outsurvives us, we null the callback
    if (fCachedCanvas) {
        fCachedCanvas->setSurfaceBase(NULL);
    }

    SkSafeUnref(fCachedImage);
    SkSafeUnref(fCachedCanvas);
}

void SkSurface_Base::onDraw(SkCanvas* canvas, SkScalar x, SkScalar y,
                            const SkPaint* paint) {
    SkImage* image = this->newImageSnapshot();
    if (image) {
        image->draw(canvas, x, y, paint);
        image->unref();
    }
}

void SkSurface_Base::aboutToDraw(ContentChangeMode mode) {
    this->dirtyGenerationID();

    SkASSERT(!fCachedCanvas || fCachedCanvas->getSurfaceBase() == this);

    if (NULL != fCachedImage) {
        // the surface may need to fork its backend, if its sharing it with
        // the cached image. Note: we only call if there is an outstanding owner
        // on the image (besides us).
        if (!fCachedImage->unique()) {
            this->onCopyOnWrite(mode);
        }

        // regardless of copy-on-write, we must drop our cached image now, so
        // that the next request will get our new contents.
        fCachedImage->unref();
        fCachedImage = NULL;
    } else if (kDiscard_ContentChangeMode == mode) {
        this->onDiscard();
    }
}

uint32_t SkSurface_Base::newGenerationID() {
    SkASSERT(!fCachedCanvas || fCachedCanvas->getSurfaceBase() == this);
    static int32_t gID;
    return sk_atomic_inc(&gID) + 1;
}

static SkSurface_Base* asSB(SkSurface* surface) {
    return static_cast<SkSurface_Base*>(surface);
}

///////////////////////////////////////////////////////////////////////////////

SkSurface::SkSurface(int width, int height) : fWidth(width), fHeight(height) {
    SkASSERT(fWidth >= 0);
    SkASSERT(fHeight >= 0);
    fGenerationID = 0;
}

SkSurface::SkSurface(const SkImageInfo& info)
    : fWidth(info.fWidth)
    , fHeight(info.fHeight)
{
    SkASSERT(fWidth >= 0);
    SkASSERT(fHeight >= 0);
    fGenerationID = 0;
}

uint32_t SkSurface::generationID() {
    if (0 == fGenerationID) {
        fGenerationID = asSB(this)->newGenerationID();
    }
    return fGenerationID;
}

void SkSurface::notifyContentWillChange(ContentChangeMode mode) {
    asSB(this)->aboutToDraw(mode);
}

SkCanvas* SkSurface::getCanvas() {
    return asSB(this)->getCachedCanvas();
}

SkImage* SkSurface::newImageSnapshot() {
    SkImage* image = asSB(this)->getCachedImage();
    SkSafeRef(image);   // the caller will call unref() to balance this
    return image;
}

SkSurface* SkSurface::newSurface(const SkImageInfo& info) {
    return asSB(this)->onNewSurface(info);
}

void SkSurface::draw(SkCanvas* canvas, SkScalar x, SkScalar y,
                     const SkPaint* paint) {
    return asSB(this)->onDraw(canvas, x, y, paint);
}

const void* SkSurface::peekPixels(SkImageInfo* info, size_t* rowBytes) {
    return this->getCanvas()->peekPixels(info, rowBytes);
}
