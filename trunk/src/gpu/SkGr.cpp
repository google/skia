
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

static GrTexture* sk_gr_create_bitmap_texture(GrContext* ctx,
                                              uint64_t key,
                                              const GrTextureParams* params,
                                              const SkBitmap& origBitmap) {
    SkAutoLockPixels alp(origBitmap);

    if (!origBitmap.readyToDraw()) {
        return NULL;
    }

    SkBitmap tmpBitmap;

    const SkBitmap* bitmap = &origBitmap;

    GrTextureDesc desc;
    desc.fWidth = bitmap->width();
    desc.fHeight = bitmap->height();
    desc.fConfig = SkBitmapConfig2GrPixelConfig(bitmap->config());

    GrCacheData cacheData(key);

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

            if (GrCacheData::kScratch_CacheID != key) {
                return ctx->createTexture(params, desc, cacheData,
                                          storage.get(),
                                          bitmap->width());
            } else {
                GrTexture* result = ctx->lockScratchTexture(desc,
                                          GrContext::kExact_ScratchTexMatch);
                result->writePixels(0, 0, bitmap->width(),
                                    bitmap->height(), desc.fConfig,
                                    storage.get());
                return result;
            }

        } else {
            origBitmap.copyTo(&tmpBitmap, SkBitmap::kARGB_8888_Config);
            // now bitmap points to our temp, which has been promoted to 32bits
            bitmap = &tmpBitmap;
        }
    }

    desc.fConfig = SkBitmapConfig2GrPixelConfig(bitmap->config());
    if (GrCacheData::kScratch_CacheID != key) {
        // This texture is likely to be used again so leave it in the cache
        // but locked.
        return ctx->createTexture(params, desc, cacheData,
                                  bitmap->getPixels(),
                                  bitmap->rowBytes());
    } else {
        // This texture is unlikely to be used again (in its present form) so
        // just use a scratch texture. This will remove the texture from the
        // cache so no one else can find it. Additionally, once unlocked, the
        // scratch texture will go to the end of the list for purging so will
        // likely be available for this volatile bitmap the next time around.
        GrTexture* result = ctx->lockScratchTexture(desc,
                                         GrContext::kExact_ScratchTexMatch);
        result->writePixels(0, 0,
                            bitmap->width(), bitmap->height(),
                            desc.fConfig,
                            bitmap->getPixels(),
                            bitmap->rowBytes());
        return result;
    }
}

///////////////////////////////////////////////////////////////////////////////

GrTexture* GrLockCachedBitmapTexture(GrContext* ctx,
                                     const SkBitmap& bitmap,
                                     const GrTextureParams* params) {
    GrTexture* result = NULL;

    if (!bitmap.isVolatile()) {
        // If the bitmap isn't changing try to find a cached copy first
        uint64_t key = bitmap.getGenerationID();
        key |= ((uint64_t) bitmap.pixelRefOffset()) << 32;

        GrTextureDesc desc;
        desc.fWidth = bitmap.width();
        desc.fHeight = bitmap.height();
        desc.fConfig = SkBitmapConfig2GrPixelConfig(bitmap.config());

        GrCacheData cacheData(key);

        result = ctx->findTexture(desc, cacheData, params);
        if (NULL == result) {
            // didn't find a cached copy so create one
            result = sk_gr_create_bitmap_texture(ctx, key, params, bitmap);
        }
    } else {
        result = sk_gr_create_bitmap_texture(ctx, GrCacheData::kScratch_CacheID, params, bitmap);
    }
    if (NULL == result) {
        GrPrintf("---- failed to create texture for cache [%d %d]\n",
                    bitmap.width(), bitmap.height());
    }
    return result;
}

void GrUnlockCachedBitmapTexture(GrTexture* texture) {
    GrAssert(NULL != texture->getContext());

    texture->getContext()->unlockScratchTexture(texture);
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

