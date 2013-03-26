/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSurface_Base.h"
#include "SkCanvas.h"
#include "SkImagePriv.h"
#include "SkPicture.h"

/**
 *  What does it mean to ask for more than one canvas from a picture?
 *  How do we return an Image and then "continue" recording?
 */
class SkSurface_Picture : public SkSurface_Base {
public:
    SkSurface_Picture(int width, int height);
    virtual ~SkSurface_Picture();

    virtual SkCanvas* onNewCanvas() SK_OVERRIDE;
    virtual SkSurface* onNewSurface(const SkImage::Info&) SK_OVERRIDE;
    virtual SkImage* onNewImageShapshot() SK_OVERRIDE;
    virtual void onDraw(SkCanvas*, SkScalar x, SkScalar y,
                        const SkPaint*) SK_OVERRIDE;
    virtual void onCopyOnWrite(SkImage*, SkCanvas*) SK_OVERRIDE;

private:
    SkPicture*  fPicture;

    typedef SkSurface_Base INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

SkSurface_Picture::SkSurface_Picture(int width, int height) : INHERITED(width, height) {
    fPicture = NULL;
}

SkSurface_Picture::~SkSurface_Picture() {
    SkSafeUnref(fPicture);
}

SkCanvas* SkSurface_Picture::onNewCanvas() {
    if (!fPicture) {
        fPicture = SkNEW(SkPicture);
    }
    SkCanvas* canvas = fPicture->beginRecording(this->width(), this->height());
    canvas->ref();  // our caller will call unref()
    return canvas;
}

SkSurface* SkSurface_Picture::onNewSurface(const SkImage::Info& info) {
    return SkSurface::NewPicture(info.fWidth, info.fHeight);
}

SkImage* SkSurface_Picture::onNewImageShapshot() {
    if (fPicture) {
        return SkNewImageFromPicture(fPicture);
    } else {
        SkImage::Info info;
        info.fWidth = info.fHeight = 0;
        info.fColorType = SkImage::kPMColor_ColorType;
        info.fAlphaType = SkImage::kOpaque_AlphaType;
        return SkImage::NewRasterCopy(info, NULL, 0);
    }
}

void SkSurface_Picture::onDraw(SkCanvas* canvas, SkScalar x, SkScalar y,
                               const SkPaint* paint) {
    if (!fPicture) {
        return;
    }
    SkImagePrivDrawPicture(canvas, fPicture, x, y, paint);
}

void SkSurface_Picture::onCopyOnWrite(SkImage* cachedImage, SkCanvas*) {
    // We always spawn a copy of the recording picture when we
    // are asked for a snapshot, so we never need to do anything here.
}

///////////////////////////////////////////////////////////////////////////////


SkSurface* SkSurface::NewPicture(int width, int height) {
    if ((width | height) < 0) {
        return NULL;
    }

    return SkNEW_ARGS(SkSurface_Picture, (width, height));
}
