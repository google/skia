/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImage_Base.h"
#include "SkImagePriv.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "GrContext.h"
#include "GrTexture.h"
#include "SkGrPixelRef.h"

class SkImage_Gpu : public SkImage_Base {
public:
    SK_DECLARE_INST_COUNT(SkImage_Gpu)

    explicit SkImage_Gpu(const SkBitmap&);
    virtual ~SkImage_Gpu();

    virtual void onDraw(SkCanvas*, SkScalar x, SkScalar y, const SkPaint*) SK_OVERRIDE;
    virtual void onDrawRectToRect(SkCanvas*, const SkRect* src, const SkRect& dst, const SkPaint*) SK_OVERRIDE;
    virtual GrTexture* onGetTexture() SK_OVERRIDE;
    virtual bool getROPixels(SkBitmap*) const SK_OVERRIDE {
        // TODO
        return false;
    }

    GrTexture* getTexture() { return fBitmap.getTexture(); }

private:
    SkBitmap    fBitmap;

    typedef SkImage_Base INHERITED;
};

SK_DEFINE_INST_COUNT(SkImage_Gpu)

///////////////////////////////////////////////////////////////////////////////

SkImage_Gpu::SkImage_Gpu(const SkBitmap& bitmap)
    : INHERITED(bitmap.width(), bitmap.height())
    , fBitmap(bitmap) {
    SkASSERT(NULL != fBitmap.getTexture());
}

SkImage_Gpu::~SkImage_Gpu() {
}

void SkImage_Gpu::onDraw(SkCanvas* canvas, SkScalar x, SkScalar y,
                         const SkPaint* paint) {
    canvas->drawBitmap(fBitmap, x, y, paint);
}

void SkImage_Gpu::onDrawRectToRect(SkCanvas* canvas, const SkRect* src, const SkRect& dst,
                         const SkPaint* paint) {
    canvas->drawBitmapRectToRect(fBitmap, src, dst, paint);
}

GrTexture* SkImage_Gpu::onGetTexture() {
    return fBitmap.getTexture();
}

///////////////////////////////////////////////////////////////////////////////

SkImage* SkImage::NewTexture(const SkBitmap& bitmap) {
    if (NULL == bitmap.getTexture()) {
        return NULL;
    }

    return SkNEW_ARGS(SkImage_Gpu, (bitmap));
}

GrTexture* SkTextureImageGetTexture(SkImage* image) {
    return ((SkImage_Gpu*)image)->getTexture();
}
