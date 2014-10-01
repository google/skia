
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#include "SkGrPixelRef.h"

#include "GrContext.h"
#include "GrTexture.h"
#include "SkBitmapCache.h"
#include "SkGr.h"
#include "SkRect.h"

// since we call lockPixels recursively on fBitmap, we need a distinct mutex,
// to avoid deadlock with the default one provided by SkPixelRef.
SK_DECLARE_STATIC_MUTEX(gROLockPixelsPixelRefMutex);

SkROLockPixelsPixelRef::SkROLockPixelsPixelRef(const SkImageInfo& info)
    : INHERITED(info, &gROLockPixelsPixelRefMutex) {}

SkROLockPixelsPixelRef::~SkROLockPixelsPixelRef() {}

bool SkROLockPixelsPixelRef::onNewLockPixels(LockRec* rec) {
    fBitmap.reset();
//    SkDebugf("---------- calling readpixels in support of lockpixels\n");
    if (!this->onReadPixels(&fBitmap, NULL)) {
        SkDebugf("SkROLockPixelsPixelRef::onLockPixels failed!\n");
        return false;
    }
    fBitmap.lockPixels();
    if (NULL == fBitmap.getPixels()) {
        return false;
    }

    rec->fPixels = fBitmap.getPixels();
    rec->fColorTable = NULL;
    rec->fRowBytes = fBitmap.rowBytes();
    return true;
}

void SkROLockPixelsPixelRef::onUnlockPixels() {
    fBitmap.unlockPixels();
}

bool SkROLockPixelsPixelRef::onLockPixelsAreWritable() const {
    return false;
}

///////////////////////////////////////////////////////////////////////////////

static SkGrPixelRef* copy_to_new_texture_pixelref(GrTexture* texture, SkColorType dstCT,
                                           const SkIRect* subset) {
    if (NULL == texture || kUnknown_SkColorType == dstCT) {
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
    desc.fConfig = SkImageInfo2GrPixelConfig(dstCT, kPremul_SkAlphaType);

    GrTexture* dst = context->createUncachedTexture(desc, NULL, 0);
    if (NULL == dst) {
        return NULL;
    }

    context->copyTexture(texture, dst->asRenderTarget(), topLeft);

    // Blink is relying on the above copy being sent to GL immediately in the case when the source
    // is a WebGL canvas backing store. We could have a TODO to remove this flush, but we have a
    // larger TODO to remove SkGrPixelRef entirely.
    context->flush();

    SkImageInfo info = SkImageInfo::Make(desc.fWidth, desc.fHeight, dstCT, kPremul_SkAlphaType);
    SkGrPixelRef* pixelRef = SkNEW_ARGS(SkGrPixelRef, (info, dst));
    SkSafeUnref(dst);
    return pixelRef;
}

///////////////////////////////////////////////////////////////////////////////

SkGrPixelRef::SkGrPixelRef(const SkImageInfo& info, GrSurface* surface,
                           bool transferCacheLock) : INHERITED(info) {
    // For surfaces that are both textures and render targets, the texture owns the
    // render target but not vice versa. So we ref the texture to keep both alive for
    // the lifetime of this pixel ref.
    fSurface = SkSafeRef(surface->asTexture());
    if (NULL == fSurface) {
        fSurface = SkSafeRef(surface);
    }
    fUnlock = transferCacheLock;

    if (fSurface) {
        SkASSERT(info.width() <= fSurface->width());
        SkASSERT(info.height() <= fSurface->height());
    }
}

SkGrPixelRef::~SkGrPixelRef() {
    if (fUnlock) {
        GrContext* context = fSurface->getContext();
        GrTexture* texture = fSurface->asTexture();
        if (context && texture) {
            context->unlockScratchTexture(texture);
        }
    }
    SkSafeUnref(fSurface);
}

GrTexture* SkGrPixelRef::getTexture() {
    if (fSurface) {
        return fSurface->asTexture();
    }
    return NULL;
}

SkPixelRef* SkGrPixelRef::deepCopy(SkColorType dstCT, const SkIRect* subset) {
    if (NULL == fSurface) {
        return NULL;
    }

    // Note that when copying a render-target-backed pixel ref, we
    // return a texture-backed pixel ref instead.  This is because
    // render-target pixel refs are usually created in conjunction with
    // a GrTexture owned elsewhere (e.g., SkGpuDevice), and cannot live
    // independently of that texture.  Texture-backed pixel refs, on the other
    // hand, own their GrTextures, and are thus self-contained.
    return copy_to_new_texture_pixelref(fSurface->asTexture(), dstCT, subset);
}

static bool tryAllocBitmapPixels(SkBitmap* bitmap) {
    SkBitmap::Allocator* allocator = SkBitmapCache::GetAllocator();
    if (NULL != allocator) {
        return allocator->allocPixelRef(bitmap, 0);
    } else {
        // DiscardableMemory is not available, fallback to default allocator
        return bitmap->tryAllocPixels();
    }
}

bool SkGrPixelRef::onReadPixels(SkBitmap* dst, const SkIRect* subset) {
    if (NULL == fSurface || fSurface->wasDestroyed()) {
        return false;
    }

    SkIRect bounds;
    if (subset) {
        bounds = *subset;
    } else {
        bounds = SkIRect::MakeWH(this->info().width(), this->info().height());
    }

    //Check the cache
    if(!SkBitmapCache::Find(this->getGenerationID(), bounds, dst)) {
        //Cache miss

        SkBitmap cachedBitmap;
        cachedBitmap.setInfo(this->info().makeWH(bounds.width(), bounds.height()));

        // If we can't alloc the pixels, then fail
        if (!tryAllocBitmapPixels(&cachedBitmap)) {
            return false;
        }

        // Try to read the pixels from the surface
        void* buffer = cachedBitmap.getPixels();
        bool readPixelsOk = fSurface->readPixels(bounds.fLeft, bounds.fTop,
                                bounds.width(), bounds.height(),
                                kSkia8888_GrPixelConfig,
                                buffer, cachedBitmap.rowBytes());

        if (!readPixelsOk) {
            return false;
        }

        // If we are here, pixels were read correctly from the surface.
        cachedBitmap.setImmutable();
        //Add to the cache
        SkBitmapCache::Add(this->getGenerationID(), bounds, cachedBitmap);

        dst->swap(cachedBitmap);
    }

    return true;

}
