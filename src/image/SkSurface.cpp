/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSurface.h"
#include "SkImagePriv.h"
#include "SkCanvas.h"

///////////////////////////////////////////////////////////////////////////////

class SkSurface_Base : public SkSurface {
public:
    SkSurface_Base(int width, int height) : INHERITED(width, height) {}

    virtual SkCanvas* onNewCanvas() = 0;
    virtual SkSurface* onNewSurface(const SkImage::Info&, SkColorSpace*) = 0;
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

private:
    typedef SkSurface INHERITED;
};

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

SkCanvas* SkSurface::newCanvas() {
    return asSB(this)->onNewCanvas();
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

