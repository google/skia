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

    SkImage_Gpu(GrTexture*);
    virtual ~SkImage_Gpu();

    virtual void onDraw(SkCanvas*, SkScalar x, SkScalar y, const SkPaint*) SK_OVERRIDE;

    GrTexture* getTexture() { return fTexture; }

    void setTexture(GrTexture* texture);

private:
    GrTexture*  fTexture;
    SkBitmap    fBitmap;

    typedef SkImage_Base INHERITED;
};

SK_DEFINE_INST_COUNT(SkImage_Gpu)

///////////////////////////////////////////////////////////////////////////////

SkImage_Gpu::SkImage_Gpu(GrTexture* texture)
    : INHERITED(texture->width(), texture->height())
    , fTexture(texture) {

    SkASSERT(NULL != fTexture);
    fTexture->ref();
    fBitmap.setConfig(SkBitmap::kARGB_8888_Config, fTexture->width(), fTexture->height());
    fBitmap.setPixelRef(new SkGrPixelRef(fTexture))->unref();
}

SkImage_Gpu::~SkImage_Gpu() {
    SkSafeUnref(fTexture);
}

void SkImage_Gpu::onDraw(SkCanvas* canvas, SkScalar x, SkScalar y,
                         const SkPaint* paint) {
    canvas->drawBitmap(fBitmap, x, y, paint);
}

void SkImage_Gpu::setTexture(GrTexture* texture) {

    if (texture == fTexture) {
        return;
    }

    SkRefCnt_SafeAssign(fTexture, texture);
    fBitmap.setPixelRef(new SkGrPixelRef(texture))->unref();
}

///////////////////////////////////////////////////////////////////////////////

SkImage* SkImage::NewTexture(GrTexture* texture) {
    if (NULL == texture) {
        return NULL;
    }

    return SkNEW_ARGS(SkImage_Gpu, (texture));
}

GrTexture* SkTextureImageGetTexture(SkImage* image) {
    return ((SkImage_Gpu*)image)->getTexture();
}

void SkTextureImageSetTexture(SkImage* image, GrTexture* texture) {
    ((SkImage_Gpu*)image)->setTexture(texture);
}
