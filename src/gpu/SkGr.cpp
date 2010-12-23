/*
    Copyright 2010 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
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
        SkASSERT(!"bitmap not ready to draw!");
        return;
    }

    SkColorTable* ctable = bitmap.getColorTable();
    char* dst = (char*)buffer;
    
    memcpy(dst, ctable->lockColors(), ctable->count() * sizeof(SkPMColor));
    ctable->unlockColors(false);
    
    // always skip a full 256 number of entries, even if we memcpy'd fewer
    dst += GrGpu::kColorTableSize;

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

GrTextureEntry* sk_gr_create_bitmap_texture(GrContext* ctx, 
                                            GrTextureKey* key,
                                            const GrSamplerState& sampler,
                                            const SkBitmap& origBitmap) {
    SkAutoLockPixels alp(origBitmap);
    if (!origBitmap.readyToDraw()) {
        return NULL;
    }

    SkBitmap tmpBitmap;

    const SkBitmap* bitmap = &origBitmap;
    
    GrGpu::TextureDesc desc = {
        0,
        GrGpu::kNone_AALevel,
        bitmap->width(),
        bitmap->height(),
        SkGr::Bitmap2PixelConfig(*bitmap)
    };
    
    if (SkBitmap::kIndex8_Config == bitmap->config()) {
        // build_compressed_data doesn't do npot->pot expansion
        // and paletted textures can't be sub-updated
        if (ctx->supportsIndex8PixelConfig(sampler,
                                           bitmap->width(), bitmap->height())) {
            size_t imagesize = bitmap->width() * bitmap->height() + 
                                GrGpu::kColorTableSize;
            SkAutoMalloc storage(imagesize);
        
            build_compressed_data(storage.get(), origBitmap);

            // our compressed data will be trimmed, so pass width() for its
            // "rowBytes", since they are the same now.
            return ctx->createAndLockTexture(key, sampler, desc, storage.get(),
                                             bitmap->width());

        } else {
            origBitmap.copyTo(&tmpBitmap, SkBitmap::kARGB_8888_Config);
            // now bitmap points to our temp, which has been promoted to 32bits
            bitmap = &tmpBitmap;
        }
    } 

    desc.fFormat = SkGr::Bitmap2PixelConfig(*bitmap);
    return ctx->createAndLockTexture(key, sampler, desc, bitmap->getPixels(),
                                     bitmap->rowBytes());
}

////////////////////////////////////////////////////////////////////////////////
  
void sk_gr_set_paint(GrContext* ctx, const SkPaint& paint, bool justAlpha) {
    ctx->setDither(paint.isDither());
    ctx->setAntiAlias(paint.isAntiAlias());

    if (justAlpha) {
        ctx->setAlpha(paint.getAlpha());
    } else {
        ctx->setColor(SkGr::SkColor2GrColor(paint.getColor()));
    }
    
    SkXfermode::Coeff sm = SkXfermode::kOne_Coeff;
    SkXfermode::Coeff dm = SkXfermode::kISA_Coeff;
    
    SkXfermode* mode = paint.getXfermode();
    if (mode) {
        mode->asCoeff(&sm, &dm);
    }   
    ctx->setBlendFunc(sk_blend_to_grblend(sm), sk_blend_to_grblend(dm));
}

////////////////////////////////////////////////////////////////////////////////

SkGrPathIter::Command SkGrPathIter::next(GrPoint pts[]) {
    GrAssert(NULL != pts);
#if SK_SCALAR_IS_GR_SCALAR
    return sk_path_verb_to_gr_path_command(fIter.next((SkPoint*)pts));
#else
    Command cmd = sk_path_verb_to_gr_path_command(fIter.next(fPoints));
    int n = NumCommandPoints(cmd);
    for (int i = 0; i < n; ++i) {
        pts[i].fX = SkScalarToGrScalar(fPoints[i].fX);
        pts[i].fY = SkScalarToGrScalar(fPoints[i].fY);
    }
    return cmd;
#endif
}

SkGrPathIter::Command SkGrPathIter::next() {
    return sk_path_verb_to_gr_path_command(fIter.next(NULL));
}

void SkGrPathIter::rewind() {
    fIter.setPath(fPath, false);
}

GrPathIter::ConvexHint SkGrPathIter::hint() const {
    return fPath.isConvex() ? GrPathIter::kConvex_ConvexHint : 
                              GrPathIter::kNone_ConvexHint;
}

///////////////////////////////////////////////////////////////////////////////

void SkGrClipIterator::computeBounds(GrIRect* bounds) {
    const SkRegion* rgn = fIter.rgn();
    if (rgn) {
        SkGr::SetIRect(bounds, rgn->getBounds());
    } else {
        bounds->setEmpty();
    }
}

///////////////////////////////////////////////////////////////////////////////

GrTexture::PixelConfig SkGr::BitmapConfig2PixelConfig(SkBitmap::Config config,
                                                    bool isOpaque) {
    switch (config) {
        case SkBitmap::kA8_Config:
            return GrTexture::kAlpha_8_PixelConfig;
        case SkBitmap::kIndex8_Config:
            return GrTexture::kIndex_8_PixelConfig;
        case SkBitmap::kRGB_565_Config:
            return GrTexture::kRGB_565_PixelConfig;
        case SkBitmap::kARGB_4444_Config:
            return GrTexture::kRGBA_4444_PixelConfig;
        case SkBitmap::kARGB_8888_Config:
            if (isOpaque) {
                return GrTexture::kRGBX_8888_PixelConfig;
            } else {
                return GrTexture::kRGBA_8888_PixelConfig;
            }
        default:
            return GrTexture::kUnknown_PixelConfig;
    }
}

void SkGr::AbandonAllTextures(GrContext* ctx) {
    ctx->abandonAllTextures();
}


