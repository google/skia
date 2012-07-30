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

#endif

