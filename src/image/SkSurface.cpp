/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSurface_Base.h"
#include "SkImagePriv.h"
#include "SkCanvas.h"

SK_DEFINE_INST_COUNT(SkSurface)

///////////////////////////////////////////////////////////////////////////////

SkSurface_Base::SkSurface_Base(int width, int height) : INHERITED(width, height) {
    fCachedCanvas = NULL;
}

SkSurface_Base::~SkSurface_Base() {
    SkSafeUnref(fCachedCanvas);
}

void SkSurface_Base::onDraw(SkCanvas* canvas, SkScalar x, SkScalar y,
                            const SkPaint* paint) {
    SkImage* image = this->newImageShapshot();
    if (image) {
        image->draw(canvas, x, y, paint);
        image->unref();
    }
}

static SkSurface_Base* asSB(SkSurface* surface) {
    return static_cast<SkSurface_Base*>(surface);
}

///////////////////////////////////////////////////////////////////////////////

SkSurface::SkSurface(int width, int height) : fWidth(width), fHeight(height) {
    SkASSERT(width >= 0);
    SkASSERT(height >= 0);
    fGenerationID = 0;
}

SkCanvas* SkSurface::getCanvas() {
    return asSB(this)->getCachedCanvas();
}

SkSurface* SkSurface::newSurface(const SkImage::Info& info, SkColorSpace* cs) {
    return asSB(this)->onNewSurface(info, cs);
}

SkImage* SkSurface::newImageShapshot() {
    return asSB(this)->onNewImageShapshot();
}

void SkSurface::draw(SkCanvas* canvas, SkScalar x, SkScalar y,
                     const SkPaint* paint) {
    return asSB(this)->onDraw(canvas, x, y, paint);
}

