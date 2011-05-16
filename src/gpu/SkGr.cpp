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

    GrTextureDesc desc = {
        kNone_GrTextureFlags,
        kNone_GrAALevel,
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
                                kGrColorTableSize;
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

GrSetOp SkGrClipIterator::getOp() const {
    // we skipped to the last "replace" op
    // when this iter was reset.
    // GrClip doesn't allow replace, so treat it as
    // intersect.
    GrSetOp skToGrOps[] = {
        kDifference_SetOp,         // kDifference_Op
        kIntersect_SetOp,          // kIntersect_Op
        kUnion_SetOp,              // kUnion_Op
        kXor_SetOp,                // kXOR_Op
        kReverseDifference_SetOp,  // kReverseDifference_Op
        kIntersect_SetOp           // kReplace_op
    };
    GR_STATIC_ASSERT(0 == SkRegion::kDifference_Op);
    GR_STATIC_ASSERT(1 == SkRegion::kIntersect_Op);
    GR_STATIC_ASSERT(2 == SkRegion::kUnion_Op);
    GR_STATIC_ASSERT(3 == SkRegion::kXOR_Op);
    GR_STATIC_ASSERT(4 == SkRegion::kReverseDifference_Op);
    GR_STATIC_ASSERT(5 == SkRegion::kReplace_Op);
    return skToGrOps[fCurr->fOp];
}

GrPathFill SkGrClipIterator::getPathFill() const {
    switch (fCurr->fPath->getFillType()) {
        case SkPath::kWinding_FillType:
            return kWinding_PathFill;
        case SkPath::kEvenOdd_FillType:
            return  kEvenOdd_PathFill;
        case SkPath::kInverseWinding_FillType:
            return kInverseWinding_PathFill;
        case SkPath::kInverseEvenOdd_FillType:
            return kInverseEvenOdd_PathFill;
        default:
            GrCrash("Unsupported path fill in clip.");
            return kWinding_PathFill; // suppress warning
    }
}

///////////////////////////////////////////////////////////////////////////////

GrPixelConfig SkGr::BitmapConfig2PixelConfig(SkBitmap::Config config,
                                                    bool isOpaque) {
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
            if (isOpaque) {
                return kRGBX_8888_GrPixelConfig;
            } else {
                return kRGBA_8888_GrPixelConfig;
            }
        default:
            return kUnknown_GrPixelConfig;
    }
}

