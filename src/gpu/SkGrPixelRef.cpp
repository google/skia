
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#include "SkGrPixelRef.h"
#include "GrContext.h"
#include "GrTexture.h"
#include "SkGr.h"
#include "SkRect.h"

// since we call lockPixels recursively on fBitmap, we need a distinct mutex,
// to avoid deadlock with the default one provided by SkPixelRef.
SK_DECLARE_STATIC_MUTEX(gROLockPixelsPixelRefMutex);

SkROLockPixelsPixelRef::SkROLockPixelsPixelRef(const SkImageInfo& info)
    : INHERITED(info, &gROLockPixelsPixelRefMutex) {}

SkROLockPixelsPixelRef::~SkROLockPixelsPixelRef() {}

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

static SkGrPixelRef* copyToTexturePixelRef(GrTexture* texture, SkBitmap::Config dstConfig,
                                           const SkIRect* subset) {
    if (NULL == texture) {
        return NULL;
    }
    GrContext* context = texture->getContext();
    if (NULL == context) {
        return NULL;
    }
    GrTextureDesc desc;

    SkIPoint pointStorage;
    SkIPoint* topLeft;
    if (subset != NULL) {
        SkASSERT(SkIRect::MakeWH(texture->width(), texture->height()).contains(*subset));
        // Create a new texture that is the size of subset.
        desc.fWidth = subset->width();
        desc.fHeight = subset->height();
        pointStorage.set(subset->x(), subset->y());
        topLeft = &pointStorage;
    } else {
        desc.fWidth  = texture->width();
        desc.fHeight = texture->height();
        topLeft = NULL;
    }
    desc.fFlags = kRenderTarget_GrTextureFlagBit | kNoStencil_GrTextureFlagBit;
    desc.fConfig = SkBitmapConfig2GrPixelConfig(dstConfig);

    SkImageInfo info;
    if (!GrPixelConfig2ColorType(desc.fConfig, &info.fColorType)) {
        return NULL;
    }
    info.fWidth = desc.fWidth;
    info.fHeight = desc.fHeight;
    info.fAlphaType = kPremul_SkAlphaType;
    
    GrTexture* dst = context->createUncachedTexture(desc, NULL, 0);
    if (NULL == dst) {
        return NULL;
    }

    context->copyTexture(texture, dst->asRenderTarget(), topLeft);

    // TODO: figure out if this is responsible for Chrome canvas errors
#if 0
    // The render texture we have created (to perform the copy) isn't fully
    // functional (since it doesn't have a stencil buffer). Release it here
    // so the caller doesn't try to render to it.
    // TODO: we can undo this release when dynamic stencil buffer attach/
    // detach has been implemented
    dst->releaseRenderTarget();
#endif

    SkGrPixelRef* pixelRef = SkNEW_ARGS(SkGrPixelRef, (info, dst));
    SkSafeUnref(dst);
    return pixelRef;
}

///////////////////////////////////////////////////////////////////////////////

SkGrPixelRef::SkGrPixelRef(const SkImageInfo& info, GrSurface* surface,
                           bool transferCacheLock) : INHERITED(info) {
    // TODO: figure out if this is responsible for Chrome canvas errors
#if 0
    // The GrTexture has a ref to the GrRenderTarget but not vice versa.
    // If the GrTexture exists take a ref to that (rather than the render
    // target)
    fSurface = surface->asTexture();
#else
    fSurface = NULL;
#endif
    if (NULL == fSurface) {
        fSurface = surface;
    }
    fUnlock = transferCacheLock;
    SkSafeRef(surface);
}

SkGrPixelRef::~SkGrPixelRef() {
    if (fUnlock) {
        GrContext* context = fSurface->getContext();
        GrTexture* texture = fSurface->asTexture();
        if (NULL != context && NULL != texture) {
            context->unlockScratchTexture(texture);
        }
    }
    SkSafeUnref(fSurface);
}

GrTexture* SkGrPixelRef::getTexture() {
    if (NULL != fSurface) {
        return fSurface->asTexture();
    }
    return NULL;
}

SkPixelRef* SkGrPixelRef::deepCopy(SkBitmap::Config dstConfig, const SkIRect* subset) {
    if (NULL == fSurface) {
        return NULL;
    }

    // Note that when copying a render-target-backed pixel ref, we
    // return a texture-backed pixel ref instead.  This is because
    // render-target pixel refs are usually created in conjunction with
    // a GrTexture owned elsewhere (e.g., SkGpuDevice), and cannot live
    // independently of that texture.  Texture-backed pixel refs, on the other
    // hand, own their GrTextures, and are thus self-contained.
    return copyToTexturePixelRef(fSurface->asTexture(), dstConfig, subset);
}

bool SkGrPixelRef::onReadPixels(SkBitmap* dst, const SkIRect* subset) {
    if (NULL == fSurface || !fSurface->isValid()) {
        return false;
    }

    int left, top, width, height;
    if (NULL != subset) {
        left = subset->fLeft;
        width = subset->width();
        top = subset->fTop;
        height = subset->height();
    } else {
        left = 0;
        width = fSurface->width();
        top = 0;
        height = fSurface->height();
    }
    dst->setConfig(SkBitmap::kARGB_8888_Config, width, height);
    if (!dst->allocPixels()) {
        SkDebugf("SkGrPixelRef::onReadPixels failed to alloc bitmap for result!\n");
        return false;
    }
    SkAutoLockPixels al(*dst);
    void* buffer = dst->getPixels();
    return fSurface->readPixels(left, top, width, height,
                                kSkia8888_GrPixelConfig,
                                buffer, dst->rowBytes());
}
