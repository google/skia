
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#include "SkGrTexturePixelRef.h"
#include "GrTexture.h"
#include "SkRect.h"

// since we call lockPixels recursively on fBitmap, we need a distinct mutex,
// to avoid deadlock with the default one provided by SkPixelRef.
static SkMutex  gROLockPixelsPixelRefMutex;

SkROLockPixelsPixelRef::SkROLockPixelsPixelRef() : INHERITED(&gROLockPixelsPixelRefMutex) {
}

SkROLockPixelsPixelRef::~SkROLockPixelsPixelRef() {
}

void* SkROLockPixelsPixelRef::onLockPixels(SkColorTable** ctable) {
    if (ctable) {
        *ctable = NULL;
    }
    fBitmap.reset();
//    SkDebugf("---------- calling readpixels in support of lockpixels\n");
    if (!this->onReadPixels(&fBitmap, NULL)) {
        SkDebugf("SkROLockPixelsPixelRef::onLockPixels failed!\n");
        return NULL;
    }
    fBitmap.lockPixels();
    return fBitmap.getPixels();
}

void SkROLockPixelsPixelRef::onUnlockPixels() {
    fBitmap.unlockPixels();
}

bool SkROLockPixelsPixelRef::onLockPixelsAreWritable() const {
    return false;
}

///////////////////////////////////////////////////////////////////////////////

SkGrTexturePixelRef::SkGrTexturePixelRef(GrTexture* tex) {
    fTexture = tex;
    GrSafeRef(tex);
}

SkGrTexturePixelRef::~SkGrTexturePixelRef() {
    GrSafeUnref(fTexture);
}

SkGpuTexture* SkGrTexturePixelRef::getTexture() {
    return (SkGpuTexture*)fTexture;
}

bool SkGrTexturePixelRef::onReadPixels(SkBitmap* dst, const SkIRect* subset) {
    if (NULL != fTexture && fTexture->isValid()) {
        int left, top, width, height;
        if (NULL != subset) {
            left = subset->fLeft;
            width = subset->width();
            top = subset->fTop;
            height = subset->height();
        } else {
            left = 0;
            width = fTexture->width();
            top = 0;
            height = fTexture->height();
        }
        dst->setConfig(SkBitmap::kARGB_8888_Config, width, height);
        dst->allocPixels();
        SkAutoLockPixels al(*dst);
        void* buffer = dst->getPixels();
        return fTexture->readPixels(left, top, width, height,
                                    kRGBA_8888_GrPixelConfig,
                                    buffer);
    } else {
        return false;
    }
}

///////////////////////////////////////////////////////////////////////////////

SkGrRenderTargetPixelRef::SkGrRenderTargetPixelRef(GrRenderTarget* rt) {
    fRenderTarget = rt;
    GrSafeRef(fRenderTarget);
}

SkGrRenderTargetPixelRef::~SkGrRenderTargetPixelRef() {
    GrSafeUnref(fRenderTarget);
}

SkGpuTexture* SkGrRenderTargetPixelRef::getTexture() { 
    if (NULL != fRenderTarget) {
        return (SkGpuTexture*) fRenderTarget->asTexture();
    }
    return NULL;
}

bool SkGrRenderTargetPixelRef::onReadPixels(SkBitmap* dst, const SkIRect* subset) {
    if (NULL != fRenderTarget && fRenderTarget->isValid()) {
        int left, top, width, height;
        if (NULL != subset) {
            left = subset->fLeft;
            width = subset->width();
            top = subset->fTop;
            height = subset->height();
        } else {
            left = 0;
            width = fRenderTarget->width();
            top = 0;
            height = fRenderTarget->height();
        }
        dst->setConfig(SkBitmap::kARGB_8888_Config, width, height);
        dst->allocPixels();
        SkAutoLockPixels al(*dst);
        void* buffer = dst->getPixels();
        return fRenderTarget->readPixels(left, top, width, height,
                                         kRGBA_8888_GrPixelConfig,
                                         buffer);
    } else {
        return false;
    }
}

