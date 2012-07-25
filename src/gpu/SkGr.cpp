
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#include "SkGr.h"

/*  Fill out buffer with the compressed format Ganesh expects from a colortable
 based bitmap. [palette (colortable) + indices].

 At the moment Ganesh only supports 8bit version. If Ganesh allowed we others
 we could detect that the colortable.count is <= 16, and then repack the
 indices as nibbles to save RAM, but it would take more time (i.e. a lot
 slower than memcpy), so skipping that for now.

 Ganesh wants a full 256 palette entry, even though Skia's ctable is only as big
 as the colortable.count says it is.
 */
static void build_compressed_data(void* buffer, const SkBitmap& bitmap) {
    SkASSERT(SkBitmap::kIndex8_Config == bitmap.config());

    SkAutoLockPixels apl(bitmap);
    if (!bitmap.readyToDraw()) {
        SkDEBUGFAIL("bitmap not ready to draw!");
        return;
    }

    SkColorTable* ctable = bitmap.getColorTable();
    char* dst = (char*)buffer;

    memcpy(dst, ctable->lockColors(), ctable->count() * sizeof(SkPMColor));
    ctable->unlockColors(false);

    // always skip a full 256 number of entries, even if we memcpy'd fewer
    dst += kGrColorTableSize;

    if (bitmap.width() == bitmap.rowBytes()) {
        memcpy(dst, bitmap.getPixels(), bitmap.getSize());
    } else {
        // need to trim off the extra bytes per row
        size_t width = bitmap.width();
        size_t rowBytes = bitmap.rowBytes();
        const char* src = (const char*)bitmap.getPixels();
        for (int y = 0; y < bitmap.height(); y++) {
            memcpy(dst, src, width);
            src += rowBytes;
            dst += width;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

static GrContext::TextureCacheEntry sk_gr_create_bitmap_texture(GrContext* ctx,
                                                uint64_t key,
                                                const GrTextureParams* params,
                                                const SkBitmap& origBitmap) {
    SkAutoLockPixels alp(origBitmap);
    GrContext::TextureCacheEntry entry;

    if (!origBitmap.readyToDraw()) {
        return entry;
    }

    SkBitmap tmpBitmap;

    const SkBitmap* bitmap = &origBitmap;

    GrTextureDesc desc;
    desc.fWidth = bitmap->width();
    desc.fHeight = bitmap->height();
    desc.fConfig = SkBitmapConfig2GrPixelConfig(bitmap->config());
    desc.fClientCacheID = key;

    if (SkBitmap::kIndex8_Config == bitmap->config()) {
        // build_compressed_data doesn't do npot->pot expansion
        // and paletted textures can't be sub-updated
        if (ctx->supportsIndex8PixelConfig(params,
                                           bitmap->width(), bitmap->height())) {
            size_t imagesize = bitmap->width() * bitmap->height() +
                                kGrColorTableSize;
            SkAutoMalloc storage(imagesize);

            build_compressed_data(storage.get(), origBitmap);

            // our compressed data will be trimmed, so pass width() for its
            // "rowBytes", since they are the same now.
            
            if (kUncached_CacheID != key) {
                return ctx->createAndLockTexture(params, desc, storage.get(),
                                                 bitmap->width());
            } else {
                entry = ctx->lockScratchTexture(desc,
                                        GrContext::kExact_ScratchTexMatch);
                entry.texture()->writePixels(0, 0, bitmap->width(), 
                                             bitmap->height(), desc.fConfig,
                                             storage.get(), 0);
                return entry;
            }

        } else {
            origBitmap.copyTo(&tmpBitmap, SkBitmap::kARGB_8888_Config);
            // now bitmap points to our temp, which has been promoted to 32bits
            bitmap = &tmpBitmap;
        }
    }

    desc.fConfig = SkBitmapConfig2GrPixelConfig(bitmap->config());
    if (kUncached_CacheID != key) {
        return ctx->createAndLockTexture(params, desc,
                                         bitmap->getPixels(),
                                         bitmap->rowBytes());
    } else {
        entry = ctx->lockScratchTexture(desc,
                                        GrContext::kExact_ScratchTexMatch);
        entry.texture()->writePixels(0, 0,
                                     bitmap->width(), bitmap->height(),
                                     desc.fConfig,
                                     bitmap->getPixels(),
                                     bitmap->rowBytes());
        return entry;
    }
}

///////////////////////////////////////////////////////////////////////////////

GrContext::TextureCacheEntry GrLockCachedBitmapTexture(GrContext* ctx, 
                                                       const SkBitmap& bitmap,
                                                       const GrTextureParams* params) {
    GrContext::TextureCacheEntry entry;

    if (!bitmap.isVolatile()) {
        uint64_t key = bitmap.getGenerationID();
        key |= ((uint64_t) bitmap.pixelRefOffset()) << 32;

        GrTextureDesc desc;
        desc.fWidth = bitmap.width();
        desc.fHeight = bitmap.height();
        desc.fConfig = SkBitmapConfig2GrPixelConfig(bitmap.config());
        desc.fClientCacheID = key;

        entry = ctx->findAndLockTexture(desc, params);
        if (NULL == entry.texture()) {
            entry = sk_gr_create_bitmap_texture(ctx, key, params, bitmap);
        }
    } else {
        entry = sk_gr_create_bitmap_texture(ctx, kUncached_CacheID, params, bitmap);
    }
    if (NULL == entry.texture()) {
        GrPrintf("---- failed to create texture for cache [%d %d]\n",
                    bitmap.width(), bitmap.height());
    }
    return entry;
}

void GrUnlockCachedBitmapTexture(GrContext* ctx, GrContext::TextureCacheEntry cache) {
    ctx->unlockTexture(cache);
}

///////////////////////////////////////////////////////////////////////////////

void SkGrClipIterator::reset(const SkClipStack& clipStack) {
    fClipStack = &clipStack;
    fIter.reset(clipStack);
    // Gr has no notion of replace, skip to the
    // last replace in the clip stack.
    int lastReplace = 0;
    int curr = 0;
    while (NULL != (fCurr = fIter.next())) {
        if (SkRegion::kReplace_Op == fCurr->fOp) {
            lastReplace = curr;
        }
        ++curr;
    }
    fIter.reset(clipStack);
    for (int i = 0; i < lastReplace+1; ++i) {
        fCurr = fIter.next();
    }
}

GrClipType SkGrClipIterator::getType() const {
    GrAssert(!this->isDone());
    if (NULL == fCurr->fPath) {
        return kRect_ClipType;
    } else {
        return kPath_ClipType;
    }
}

SkRegion::Op SkGrClipIterator::getOp() const {
    // we skipped to the last "replace" op
    // when this iter was reset.
    // GrClip doesn't allow replace, so treat it as
    // intersect.
    if (SkRegion::kReplace_Op == fCurr->fOp) {
        return SkRegion::kIntersect_Op;
    }

    return fCurr->fOp;

}

bool SkGrClipIterator::getDoAA() const {
    return fCurr->fDoAA;
}

GrPathFill SkGrClipIterator::getPathFill() const {
    switch (fCurr->fPath->getFillType()) {
        case SkPath::kWinding_FillType:
            return kWinding_GrPathFill;
        case SkPath::kEvenOdd_FillType:
            return  kEvenOdd_GrPathFill;
        case SkPath::kInverseWinding_FillType:
            return kInverseWinding_GrPathFill;
        case SkPath::kInverseEvenOdd_FillType:
            return kInverseEvenOdd_GrPathFill;
        default:
            GrCrash("Unsupported path fill in clip.");
            return kWinding_GrPathFill; // suppress warning
    }
}

///////////////////////////////////////////////////////////////////////////////

GrPixelConfig SkBitmapConfig2GrPixelConfig(SkBitmap::Config config) {
    switch (config) {
        case SkBitmap::kA8_Config:
            return kAlpha_8_GrPixelConfig;
        case SkBitmap::kIndex8_Config:
            return kIndex_8_GrPixelConfig;
        case SkBitmap::kRGB_565_Config:
            return kRGB_565_GrPixelConfig;
        case SkBitmap::kARGB_4444_Config:
            return kRGBA_4444_GrPixelConfig;
        case SkBitmap::kARGB_8888_Config:
            return kSkia8888_PM_GrPixelConfig;
        default:
            // kNo_Config, kA1_Config missing, and kRLE_Index8_Config
            return kUnknown_GrPixelConfig;
    }
}

