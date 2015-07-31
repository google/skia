/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGr.h"

#include "GrCaps.h"
#include "GrDrawContext.h"
#include "GrXferProcessor.h"
#include "SkColorFilter.h"
#include "SkConfig8888.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkErrorInternals.h"
#include "SkGrPixelRef.h"
#include "SkMessageBus.h"
#include "SkPixelRef.h"
#include "SkResourceCache.h"
#include "SkTextureCompressor.h"
#include "SkYUVPlanesCache.h"
#include "effects/GrDitherEffect.h"
#include "effects/GrPorterDuffXferProcessor.h"
#include "effects/GrYUVtoRGBEffect.h"

#ifndef SK_IGNORE_ETC1_SUPPORT
#  include "ktx.h"
#  include "etc1.h"
#endif

/*  Fill out buffer with the compressed format Ganesh expects from a colortable
 based bitmap. [palette (colortable) + indices].

 At the moment Ganesh only supports 8bit version. If Ganesh allowed we others
 we could detect that the colortable.count is <= 16, and then repack the
 indices as nibbles to save RAM, but it would take more time (i.e. a lot
 slower than memcpy), so skipping that for now.

 Ganesh wants a full 256 palette entry, even though Skia's ctable is only as big
 as the colortable.count says it is.
 */
static void build_index8_data(void* buffer, const SkBitmap& bitmap) {
    SkASSERT(kIndex_8_SkColorType == bitmap.colorType());

    SkAutoLockPixels alp(bitmap);
    if (!bitmap.readyToDraw()) {
        SkDEBUGFAIL("bitmap not ready to draw!");
        return;
    }

    SkColorTable* ctable = bitmap.getColorTable();
    char* dst = (char*)buffer;

    const int count = ctable->count();

    SkDstPixelInfo dstPI;
    dstPI.fColorType = kRGBA_8888_SkColorType;
    dstPI.fAlphaType = kPremul_SkAlphaType;
    dstPI.fPixels = buffer;
    dstPI.fRowBytes = count * sizeof(SkPMColor);

    SkSrcPixelInfo srcPI;
    srcPI.fColorType = kN32_SkColorType;
    srcPI.fAlphaType = kPremul_SkAlphaType;
    srcPI.fPixels = ctable->readColors();
    srcPI.fRowBytes = count * sizeof(SkPMColor);

    srcPI.convertPixelsTo(&dstPI, count, 1);

    // always skip a full 256 number of entries, even if we memcpy'd fewer
    dst += 256 * sizeof(GrColor);

    if ((unsigned)bitmap.width() == bitmap.rowBytes()) {
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

struct Stretch {
    enum Type {
        kNone_Type,
        kBilerp_Type,
        kNearest_Type
    } fType;
    int fWidth;
    int fHeight;
};

static void get_stretch(const GrContext* ctx, int width, int height,
                        const GrTextureParams* params, Stretch* stretch) {
    stretch->fType = Stretch::kNone_Type;
    bool doStretch = false;
    if (params && params->isTiled() && !ctx->caps()->npotTextureTileSupport() &&
        (!SkIsPow2(width) || !SkIsPow2(height))) {
        doStretch = true;
        stretch->fWidth  = GrNextPow2(SkTMax(width, ctx->caps()->minTextureSize()));
        stretch->fHeight = GrNextPow2(SkTMax(height, ctx->caps()->minTextureSize()));
    } else if (width < ctx->caps()->minTextureSize() || height < ctx->caps()->minTextureSize()) {
        // The small texture issues appear to be with tiling. Hence it seems ok to scale them
        // up using the GPU. If issues persist we may need to CPU-stretch.
        doStretch = true;
        stretch->fWidth = SkTMax(width, ctx->caps()->minTextureSize());
        stretch->fHeight = SkTMax(height, ctx->caps()->minTextureSize());
    }
    if (doStretch) {
        if (params) {
            switch(params->filterMode()) {
                case GrTextureParams::kNone_FilterMode:
                    stretch->fType = Stretch::kNearest_Type;
                    break;
                case GrTextureParams::kBilerp_FilterMode:
                case GrTextureParams::kMipMap_FilterMode:
                    stretch->fType = Stretch::kBilerp_Type;
                    break;
            }
        } else {
            stretch->fType = Stretch::kBilerp_Type;
        }
    } else {
        stretch->fWidth  = -1;
        stretch->fHeight = -1;
        stretch->fType = Stretch::kNone_Type;
    }
}

static bool make_stretched_key(const GrUniqueKey& origKey, const Stretch& stretch,
                               GrUniqueKey* stretchedKey) {
    if (origKey.isValid() && Stretch::kNone_Type != stretch.fType) {
        uint32_t width = SkToU16(stretch.fWidth);
        uint32_t height = SkToU16(stretch.fHeight);
        static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
        GrUniqueKey::Builder builder(stretchedKey, origKey, kDomain, 3);
        builder[0] = stretch.fType;
        builder[1] = width | (height << 16);
        builder.finish();
        return true;
    }
    SkASSERT(!stretchedKey->isValid());
    return false;
}

static void make_unstretched_key(const SkBitmap& bitmap, GrUniqueKey* key) {
    // Our id includes the offset, width, and height so that bitmaps created by extractSubset()
    // are unique.
    uint32_t genID = bitmap.getGenerationID();
    SkIPoint origin = bitmap.pixelRefOrigin();
    uint32_t width = SkToU16(bitmap.width());
    uint32_t height = SkToU16(bitmap.height());

    static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
    GrUniqueKey::Builder builder(key, kDomain, 4);
    builder[0] = genID;
    builder[1] = origin.fX;
    builder[2] = origin.fY;
    builder[3] = width | (height << 16);
}

static void make_bitmap_keys(const SkBitmap& bitmap,
                             const Stretch& stretch,
                             GrUniqueKey* key,
                             GrUniqueKey* stretchedKey) {
    make_unstretched_key(bitmap, key);
    if (Stretch::kNone_Type != stretch.fType) {
        make_stretched_key(*key, stretch, stretchedKey);
    }
}

static void generate_bitmap_texture_desc(const SkBitmap& bitmap, GrSurfaceDesc* desc) {
    desc->fFlags = kNone_GrSurfaceFlags;
    desc->fWidth = bitmap.width();
    desc->fHeight = bitmap.height();
    desc->fConfig = SkImageInfo2GrPixelConfig(bitmap.info());
    desc->fSampleCnt = 0;
}

namespace {

// When the SkPixelRef genID changes, invalidate a corresponding GrResource described by key.
class BitmapInvalidator : public SkPixelRef::GenIDChangeListener {
public:
    explicit BitmapInvalidator(const GrUniqueKey& key) : fMsg(key) {}
private:
    GrUniqueKeyInvalidatedMessage fMsg;

    void onChange() override {
        SkMessageBus<GrUniqueKeyInvalidatedMessage>::Post(fMsg);
    }
};

}  // namespace


static GrTexture* create_texture_for_bmp(GrContext* ctx,
                                         const GrUniqueKey& optionalKey,
                                         GrSurfaceDesc desc,
                                         SkPixelRef* pixelRefForInvalidationNotification,
                                         const void* pixels,
                                         size_t rowBytes) {
    GrTexture* result = ctx->textureProvider()->createTexture(desc, true, pixels, rowBytes);
    if (result && optionalKey.isValid()) {
        BitmapInvalidator* listener = SkNEW_ARGS(BitmapInvalidator, (optionalKey));
        pixelRefForInvalidationNotification->addGenIDChangeListener(listener);
        ctx->textureProvider()->assignUniqueKeyToTexture(optionalKey, result);
    }
    return result;
}

// creates a new texture that is the input texture scaled up. If optionalKey is valid it will be
// set on the new texture. stretch controls whether the scaling is done using nearest or bilerp
// filtering and the size to stretch the texture to.
GrTexture* stretch_texture(GrTexture* inputTexture, const Stretch& stretch,
                           SkPixelRef* pixelRef,
                           const GrUniqueKey& optionalKey) {
    SkASSERT(Stretch::kNone_Type != stretch.fType);

    GrContext* context = inputTexture->getContext();
    SkASSERT(context);
    const GrCaps* caps = context->caps();

    // Either it's a cache miss or the original wasn't cached to begin with.
    GrSurfaceDesc rtDesc = inputTexture->desc();
    rtDesc.fFlags =  rtDesc.fFlags | kRenderTarget_GrSurfaceFlag;
    rtDesc.fWidth  = stretch.fWidth;
    rtDesc.fHeight = stretch.fHeight;
    rtDesc.fConfig = GrMakePixelConfigUncompressed(rtDesc.fConfig);

    // If the config isn't renderable try converting to either A8 or an 32 bit config. Otherwise,
    // fail.
    if (!caps->isConfigRenderable(rtDesc.fConfig, false)) {
        if (GrPixelConfigIsAlphaOnly(rtDesc.fConfig)) {
            if (caps->isConfigRenderable(kAlpha_8_GrPixelConfig, false)) {
                rtDesc.fConfig = kAlpha_8_GrPixelConfig;
            } else if (caps->isConfigRenderable(kSkia8888_GrPixelConfig, false)) {
                rtDesc.fConfig = kSkia8888_GrPixelConfig;
            } else {
                return NULL;
            }
        } else if (kRGB_GrColorComponentFlags ==
                   (kRGB_GrColorComponentFlags & GrPixelConfigComponentMask(rtDesc.fConfig))) {
            if (caps->isConfigRenderable(kSkia8888_GrPixelConfig, false)) {
                rtDesc.fConfig = kSkia8888_GrPixelConfig;
            } else {
                return NULL;
            }
        } else {
            return NULL;
        }
    }

    GrTexture* stretched = create_texture_for_bmp(context, optionalKey, rtDesc, pixelRef, NULL, 0);

    if (!stretched) {
        return NULL;
    }
    GrPaint paint;

    // If filtering is not desired then we want to ensure all texels in the resampled image are
    // copies of texels from the original.
    GrTextureParams params(SkShader::kClamp_TileMode,
                           Stretch::kBilerp_Type == stretch.fType ?
                              GrTextureParams::kBilerp_FilterMode :
                              GrTextureParams::kNone_FilterMode);
    paint.addColorTextureProcessor(inputTexture, SkMatrix::I(), params);

    SkRect rect = SkRect::MakeWH(SkIntToScalar(rtDesc.fWidth), SkIntToScalar(rtDesc.fHeight));
    SkRect localRect = SkRect::MakeWH(1.f, 1.f);

    GrDrawContext* drawContext = context->drawContext();
    if (!drawContext) {
        return NULL;
    }

    drawContext->drawNonAARectToRect(stretched->asRenderTarget(), GrClip::WideOpen(), paint,
                                     SkMatrix::I(), rect, localRect);

    return stretched;
}

#ifndef SK_IGNORE_ETC1_SUPPORT
static GrTexture *load_etc1_texture(GrContext* ctx, const GrUniqueKey& optionalKey,
                                    const SkBitmap &bm, GrSurfaceDesc desc) {
    SkAutoTUnref<SkData> data(bm.pixelRef()->refEncodedData());

    // Is this even encoded data?
    if (NULL == data) {
        return NULL;
    }

    // Is this a valid PKM encoded data?
    const uint8_t *bytes = data->bytes();
    if (etc1_pkm_is_valid(bytes)) {
        uint32_t encodedWidth = etc1_pkm_get_width(bytes);
        uint32_t encodedHeight = etc1_pkm_get_height(bytes);

        // Does the data match the dimensions of the bitmap? If not,
        // then we don't know how to scale the image to match it...
        if (encodedWidth != static_cast<uint32_t>(bm.width()) ||
            encodedHeight != static_cast<uint32_t>(bm.height())) {
            return NULL;
        }

        // Everything seems good... skip ahead to the data.
        bytes += ETC_PKM_HEADER_SIZE;
        desc.fConfig = kETC1_GrPixelConfig;
    } else if (SkKTXFile::is_ktx(bytes)) {
        SkKTXFile ktx(data);

        // Is it actually an ETC1 texture?
        if (!ktx.isCompressedFormat(SkTextureCompressor::kETC1_Format)) {
            return NULL;
        }

        // Does the data match the dimensions of the bitmap? If not,
        // then we don't know how to scale the image to match it...
        if (ktx.width() != bm.width() || ktx.height() != bm.height()) {
            return NULL;
        }

        bytes = ktx.pixelData();
        desc.fConfig = kETC1_GrPixelConfig;
    } else {
        return NULL;
    }

    return create_texture_for_bmp(ctx, optionalKey, desc, bm.pixelRef(), bytes, 0);
}
#endif   // SK_IGNORE_ETC1_SUPPORT

static GrTexture* load_yuv_texture(GrContext* ctx, const GrUniqueKey& optionalKey,
                                   const SkBitmap& bm, const GrSurfaceDesc& desc) {
    // Subsets are not supported, the whole pixelRef is loaded when using YUV decoding
    SkPixelRef* pixelRef = bm.pixelRef();
    if ((NULL == pixelRef) || 
        (pixelRef->info().width()  != bm.info().width()) ||
        (pixelRef->info().height() != bm.info().height())) {
        return NULL;
    }

    const bool useCache = optionalKey.isValid();
    SkYUVPlanesCache::Info yuvInfo;
    SkAutoTUnref<SkCachedData> cachedData;
    SkAutoMalloc storage;
    if (useCache) {
        cachedData.reset(SkYUVPlanesCache::FindAndRef(pixelRef->getGenerationID(), &yuvInfo));
    }

    void* planes[3];
    if (cachedData.get()) {
        planes[0] = (void*)cachedData->data();
        planes[1] = (uint8_t*)planes[0] + yuvInfo.fSizeInMemory[0];
        planes[2] = (uint8_t*)planes[1] + yuvInfo.fSizeInMemory[1];
    } else {
        // Fetch yuv plane sizes for memory allocation. Here, width and height can be
        // rounded up to JPEG block size and be larger than the image's width and height.
        if (!pixelRef->getYUV8Planes(yuvInfo.fSize, NULL, NULL, NULL)) {
            return NULL;
        }

        // Allocate the memory for YUV
        size_t totalSize(0);
        for (int i = 0; i < 3; ++i) {
            yuvInfo.fRowBytes[i] = yuvInfo.fSize[i].fWidth;
            yuvInfo.fSizeInMemory[i] = yuvInfo.fRowBytes[i] * yuvInfo.fSize[i].fHeight;
            totalSize += yuvInfo.fSizeInMemory[i];
        }
        if (useCache) {
            cachedData.reset(SkResourceCache::NewCachedData(totalSize));
            planes[0] = cachedData->writable_data();
        } else {
            storage.reset(totalSize);
            planes[0] = storage.get();
        }
        planes[1] = (uint8_t*)planes[0] + yuvInfo.fSizeInMemory[0];
        planes[2] = (uint8_t*)planes[1] + yuvInfo.fSizeInMemory[1];

        // Get the YUV planes and update plane sizes to actual image size
        if (!pixelRef->getYUV8Planes(yuvInfo.fSize, planes, yuvInfo.fRowBytes,
                                     &yuvInfo.fColorSpace)) {
            return NULL;
        }

        if (useCache) {
            // Decoding is done, cache the resulting YUV planes
            SkYUVPlanesCache::Add(pixelRef->getGenerationID(), cachedData, &yuvInfo);
        }
    }

    GrSurfaceDesc yuvDesc;
    yuvDesc.fConfig = kAlpha_8_GrPixelConfig;
    SkAutoTUnref<GrTexture> yuvTextures[3];
    for (int i = 0; i < 3; ++i) {
        yuvDesc.fWidth  = yuvInfo.fSize[i].fWidth;
        yuvDesc.fHeight = yuvInfo.fSize[i].fHeight;
        bool needsExactTexture =
            (yuvDesc.fWidth  != yuvInfo.fSize[0].fWidth) ||
            (yuvDesc.fHeight != yuvInfo.fSize[0].fHeight);
        if (needsExactTexture) {
            yuvTextures[i].reset(ctx->textureProvider()->createTexture(yuvDesc, true));
        } else {
            yuvTextures[i].reset(ctx->textureProvider()->createApproxTexture(yuvDesc));
        }
        if (!yuvTextures[i] ||
            !yuvTextures[i]->writePixels(0, 0, yuvDesc.fWidth, yuvDesc.fHeight,
                                         yuvDesc.fConfig, planes[i], yuvInfo.fRowBytes[i])) {
            return NULL;
        }
    }

    GrSurfaceDesc rtDesc = desc;
    rtDesc.fFlags = rtDesc.fFlags | kRenderTarget_GrSurfaceFlag;

    GrTexture* result = create_texture_for_bmp(ctx, optionalKey, rtDesc, pixelRef, NULL, 0);
    if (!result) {
        return NULL;
    }

    GrRenderTarget* renderTarget = result->asRenderTarget();
    SkASSERT(renderTarget);

    GrPaint paint;
    SkAutoTUnref<GrFragmentProcessor>
        yuvToRgbProcessor(GrYUVtoRGBEffect::Create(paint.getProcessorDataManager(), yuvTextures[0],
                                                   yuvTextures[1], yuvTextures[2],
                                                   yuvInfo.fSize, yuvInfo.fColorSpace));
    paint.addColorProcessor(yuvToRgbProcessor);
    SkRect r = SkRect::MakeWH(SkIntToScalar(yuvInfo.fSize[0].fWidth),
                              SkIntToScalar(yuvInfo.fSize[0].fHeight));

    GrDrawContext* drawContext = ctx->drawContext();
    if (!drawContext) {
        return NULL;
    }

    drawContext->drawRect(renderTarget, GrClip::WideOpen(), paint, SkMatrix::I(), r);

    return result;
}

static GrTexture* create_unstretched_bitmap_texture(GrContext* ctx,
                                                    const SkBitmap& origBitmap,
                                                    const GrUniqueKey& optionalKey) {
    if (origBitmap.width() < ctx->caps()->minTextureSize() ||
        origBitmap.height() < ctx->caps()->minTextureSize()) {
        return NULL;
    }
    SkBitmap tmpBitmap;

    const SkBitmap* bitmap = &origBitmap;

    GrSurfaceDesc desc;
    generate_bitmap_texture_desc(*bitmap, &desc);
    const GrCaps* caps = ctx->caps();

    if (kIndex_8_SkColorType == bitmap->colorType()) {
        if (caps->isConfigTexturable(kIndex_8_GrPixelConfig)) {
            size_t imageSize = GrCompressedFormatDataSize(kIndex_8_GrPixelConfig,
                                                          bitmap->width(), bitmap->height());
            SkAutoMalloc storage(imageSize);
            build_index8_data(storage.get(), origBitmap);

            // our compressed data will be trimmed, so pass width() for its
            // "rowBytes", since they are the same now.
            return create_texture_for_bmp(ctx, optionalKey, desc, origBitmap.pixelRef(),
                                          storage.get(), bitmap->width());
        } else {
            origBitmap.copyTo(&tmpBitmap, kN32_SkColorType);
            // now bitmap points to our temp, which has been promoted to 32bits
            bitmap = &tmpBitmap;
            desc.fConfig = SkImageInfo2GrPixelConfig(bitmap->info());
        }
    }

    // Is this an ETC1 encoded texture?
#ifndef SK_IGNORE_ETC1_SUPPORT
    // Make sure that the underlying device supports ETC1 textures before we go ahead
    // and check the data.
    else if (caps->isConfigTexturable(kETC1_GrPixelConfig)
            // If the bitmap had compressed data and was then uncompressed, it'll still return
            // compressed data on 'refEncodedData' and upload it. Probably not good, since if
            // the bitmap has available pixels, then they might not be what the decompressed
            // data is.
            && !(bitmap->readyToDraw())) {
        GrTexture *texture = load_etc1_texture(ctx, optionalKey, *bitmap, desc);
        if (texture) {
            return texture;
        }
    }
#endif   // SK_IGNORE_ETC1_SUPPORT

    GrTexture *texture = load_yuv_texture(ctx, optionalKey, *bitmap, desc);
    if (texture) {
        return texture;
    }

    SkAutoLockPixels alp(*bitmap);
    if (!bitmap->readyToDraw()) {
        return NULL;
    }

    return create_texture_for_bmp(ctx, optionalKey, desc, origBitmap.pixelRef(),
                                  bitmap->getPixels(), bitmap->rowBytes());
}

static SkBitmap stretch_on_cpu(const SkBitmap& bmp, const Stretch& stretch) {
    SkBitmap stretched;
    stretched.allocN32Pixels(stretch.fWidth, stretch.fHeight);
    SkCanvas canvas(stretched);
    SkPaint paint;
    switch (stretch.fType) {
        case Stretch::kNearest_Type:
            paint.setFilterQuality(kNone_SkFilterQuality);
            break;
        case Stretch::kBilerp_Type:
            paint.setFilterQuality(kLow_SkFilterQuality);
            break;
        case Stretch::kNone_Type:
            SkDEBUGFAIL("Shouldn't get here.");
            break;
    }
    SkRect dstRect = SkRect::MakeWH(SkIntToScalar(stretch.fWidth), SkIntToScalar(stretch.fHeight));
    canvas.drawBitmapRect(bmp, dstRect, &paint);
    return stretched;
}

static GrTexture* create_bitmap_texture(GrContext* ctx,
                                        const SkBitmap& bmp,
                                        const Stretch& stretch,
                                        const GrUniqueKey& unstretchedKey,
                                        const GrUniqueKey& stretchedKey) {
    if (Stretch::kNone_Type != stretch.fType) {
        SkAutoTUnref<GrTexture> unstretched;
        // Check if we have the unstretched version in the cache, if not create it.
        if (unstretchedKey.isValid()) {
            unstretched.reset(ctx->textureProvider()->findAndRefTextureByUniqueKey(unstretchedKey));
        }
        if (!unstretched) {
            unstretched.reset(create_unstretched_bitmap_texture(ctx, bmp, unstretchedKey));
            if (!unstretched) {
                // We might not have been able to create a unstrecthed texture because it is smaller
                // than the min texture size. In that case do cpu stretching.
                SkBitmap stretchedBmp = stretch_on_cpu(bmp, stretch);
                return create_unstretched_bitmap_texture(ctx, stretchedBmp, stretchedKey);
            }
        }
        return stretch_texture(unstretched, stretch, bmp.pixelRef(), stretchedKey);
    }
    return create_unstretched_bitmap_texture(ctx, bmp, unstretchedKey);
}

bool GrIsBitmapInCache(const GrContext* ctx,
                       const SkBitmap& bitmap,
                       const GrTextureParams* params) {
    Stretch stretch;
    get_stretch(ctx, bitmap.width(), bitmap.height(), params, &stretch);

    // Handle the case where the bitmap is explicitly texture backed.
    GrTexture* texture = bitmap.getTexture();
    if (texture) {
        if (Stretch::kNone_Type == stretch.fType) {
            return true;
        }
        // No keys for volatile bitmaps.
        if (bitmap.isVolatile()) {
            return false;
        }
        const GrUniqueKey& key = texture->getUniqueKey();
        if (!key.isValid()) {
            return false;
        }
        GrUniqueKey stretchedKey;
        make_stretched_key(key, stretch, &stretchedKey);
        return ctx->textureProvider()->existsTextureWithUniqueKey(stretchedKey);
    }

    // We don't cache volatile bitmaps
    if (bitmap.isVolatile()) {
        return false;
    }

    GrUniqueKey key, stretchedKey;
    make_bitmap_keys(bitmap, stretch, &key, &stretchedKey);
    return ctx->textureProvider()->existsTextureWithUniqueKey(
        (Stretch::kNone_Type == stretch.fType) ? key : stretchedKey);
}

GrTexture* GrRefCachedBitmapTexture(GrContext* ctx,
                                    const SkBitmap& bitmap,
                                    const GrTextureParams* params) {

    Stretch stretch;
    get_stretch(ctx, bitmap.width(), bitmap.height(), params, &stretch);

    GrTexture* result = bitmap.getTexture();
    if (result) {
        if (Stretch::kNone_Type == stretch.fType) {
            return SkRef(result);
        }
        GrUniqueKey stretchedKey;
        // Don't create a key for the resized version if the bmp is volatile.
        if (!bitmap.isVolatile()) {
            const GrUniqueKey& key = result->getUniqueKey();
            if (key.isValid()) {
                make_stretched_key(key, stretch, &stretchedKey);
                GrTexture* stretched =
                    ctx->textureProvider()->findAndRefTextureByUniqueKey(stretchedKey);
                if (stretched) {
                    return stretched;
                }
            }
        }
        return stretch_texture(result, stretch, bitmap.pixelRef(), stretchedKey);
    }

    GrUniqueKey key, resizedKey;

    if (!bitmap.isVolatile()) {
        // If the bitmap isn't changing try to find a cached copy first.
        make_bitmap_keys(bitmap, stretch, &key, &resizedKey);

        result = ctx->textureProvider()->findAndRefTextureByUniqueKey(
            resizedKey.isValid() ? resizedKey : key);
        if (result) {
            return result;
        }
    }

    result = create_bitmap_texture(ctx, bitmap, stretch, key, resizedKey);
    if (result) {
        return result;
    }

    SkErrorInternals::SetError( kInternalError_SkError,
                                "---- failed to create texture for cache [%d %d]\n",
                                bitmap.width(), bitmap.height());

    return NULL;
}
///////////////////////////////////////////////////////////////////////////////

// alphatype is ignore for now, but if GrPixelConfig is expanded to encompass
// alpha info, that will be considered.
GrPixelConfig SkImageInfo2GrPixelConfig(SkColorType ct, SkAlphaType, SkColorProfileType pt) {
    switch (ct) {
        case kUnknown_SkColorType:
            return kUnknown_GrPixelConfig;
        case kAlpha_8_SkColorType:
            return kAlpha_8_GrPixelConfig;
        case kRGB_565_SkColorType:
            return kRGB_565_GrPixelConfig;
        case kARGB_4444_SkColorType:
            return kRGBA_4444_GrPixelConfig;
        case kRGBA_8888_SkColorType:
            //if (kSRGB_SkColorProfileType == pt) {
            //    return kSRGBA_8888_GrPixelConfig;
            //}
            return kRGBA_8888_GrPixelConfig;
        case kBGRA_8888_SkColorType:
            return kBGRA_8888_GrPixelConfig;
        case kIndex_8_SkColorType:
            return kIndex_8_GrPixelConfig;
        case kGray_8_SkColorType:
            return kAlpha_8_GrPixelConfig; // TODO: gray8 support on gpu
    }
    SkASSERT(0);    // shouldn't get here
    return kUnknown_GrPixelConfig;
}

bool GrPixelConfig2ColorAndProfileType(GrPixelConfig config, SkColorType* ctOut,
                                       SkColorProfileType* ptOut) {
    SkColorType ct;
    SkColorProfileType pt = kLinear_SkColorProfileType;
    switch (config) {
        case kAlpha_8_GrPixelConfig:
            ct = kAlpha_8_SkColorType;
            break;
        case kIndex_8_GrPixelConfig:
            ct = kIndex_8_SkColorType;
            break;
        case kRGB_565_GrPixelConfig:
            ct = kRGB_565_SkColorType;
            break;
        case kRGBA_4444_GrPixelConfig:
            ct = kARGB_4444_SkColorType;
            break;
        case kRGBA_8888_GrPixelConfig:
            ct = kRGBA_8888_SkColorType;
            break;
        case kBGRA_8888_GrPixelConfig:
            ct = kBGRA_8888_SkColorType;
            break;
        case kSRGBA_8888_GrPixelConfig:
            ct = kRGBA_8888_SkColorType;
            pt = kSRGB_SkColorProfileType;
            break;
        default:
            return false;
    }
    if (ctOut) {
        *ctOut = ct;
    }
    if (ptOut) {
        *ptOut = pt;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool SkPaint2GrPaintNoShader(GrContext* context, GrRenderTarget* rt, const SkPaint& skPaint,
                             GrColor paintColor, bool constantColor, GrPaint* grPaint) {

    grPaint->setDither(skPaint.isDither());
    grPaint->setAntiAlias(skPaint.isAntiAlias());

    SkXfermode* mode = skPaint.getXfermode();
    GrXPFactory* xpFactory = NULL;
    if (!SkXfermode::AsXPFactory(mode, &xpFactory)) {
        // Fall back to src-over
        // return false here?
        xpFactory = GrPorterDuffXPFactory::Create(SkXfermode::kSrcOver_Mode);
    }
    SkASSERT(xpFactory);
    grPaint->setXPFactory(xpFactory)->unref();

    //set the color of the paint to the one of the parameter
    grPaint->setColor(paintColor);

    SkColorFilter* colorFilter = skPaint.getColorFilter();
    if (colorFilter) {
        // if the source color is a constant then apply the filter here once rather than per pixel
        // in a shader.
        if (constantColor) {
            SkColor filtered = colorFilter->filterColor(skPaint.getColor());
            grPaint->setColor(SkColor2GrColor(filtered));
        } else {
            SkTDArray<GrFragmentProcessor*> array;
            // return false if failed?
            if (colorFilter->asFragmentProcessors(context, grPaint->getProcessorDataManager(),
                                                  &array)) {
                for (int i = 0; i < array.count(); ++i) {
                    grPaint->addColorProcessor(array[i]);
                    array[i]->unref();
                }
            }
        }
    }

#ifndef SK_IGNORE_GPU_DITHER
    // If the dither flag is set, then we need to see if the underlying context
    // supports it. If not, then install a dither effect.
    if (skPaint.isDither() && grPaint->numColorStages() > 0) {
        // What are we rendering into?
        SkASSERT(rt);

        // Suspect the dithering flag has no effect on these configs, otherwise
        // fall back on setting the appropriate state.
        if (GrPixelConfigIs8888(rt->config()) ||
            GrPixelConfigIs8888(rt->config())) {
            // The dither flag is set and the target is likely
            // not going to be dithered by the GPU.
            SkAutoTUnref<GrFragmentProcessor> fp(GrDitherEffect::Create());
            if (fp.get()) {
                grPaint->addColorProcessor(fp);
                grPaint->setDither(false);
            }
        }
    }
#endif
    return true;
}

bool SkPaint2GrPaint(GrContext* context, GrRenderTarget* rt, const SkPaint& skPaint,
                     const SkMatrix& viewM, bool constantColor, GrPaint* grPaint) {
    SkShader* shader = skPaint.getShader();
    if (NULL == shader) {
        return SkPaint2GrPaintNoShader(context, rt, skPaint, SkColor2GrColor(skPaint.getColor()),
                                       constantColor, grPaint);
    }

    GrColor paintColor = SkColor2GrColor(skPaint.getColor());

    // Start a new block here in order to preserve our context state after calling
    // asFragmentProcessor(). Since these calls get passed back to the client, we don't really
    // want them messing around with the context.
    {
        // Allow the shader to modify paintColor and also create an effect to be installed as
        // the first color effect on the GrPaint.
        GrFragmentProcessor* fp = NULL;
        if (!shader->asFragmentProcessor(context, skPaint, viewM, NULL, &paintColor,
                                         grPaint->getProcessorDataManager(), &fp)) {
            return false;
        }
        if (fp) {
            grPaint->addColorProcessor(fp)->unref();
            constantColor = false;
        }
    }

    // The grcolor is automatically set when calling asFragmentProcessor.
    // If the shader can be seen as an effect it returns true and adds its effect to the grpaint.
    return SkPaint2GrPaintNoShader(context, rt, skPaint, paintColor, constantColor, grPaint);
}

SkImageInfo GrMakeInfoFromTexture(GrTexture* tex, int w, int h, bool isOpaque) {
#ifdef SK_DEBUG
    const GrSurfaceDesc& desc = tex->desc();
    SkASSERT(w <= desc.fWidth);
    SkASSERT(h <= desc.fHeight);
#endif
    const GrPixelConfig config = tex->config();
    SkColorType ct;
    SkAlphaType at = isOpaque ? kOpaque_SkAlphaType : kPremul_SkAlphaType;
    if (!GrPixelConfig2ColorAndProfileType(config, &ct, NULL)) {
        ct = kUnknown_SkColorType;
    }
    return SkImageInfo::Make(w, h, ct, at);
}


void GrWrapTextureInBitmap(GrTexture* src, int w, int h, bool isOpaque, SkBitmap* dst) {
    const SkImageInfo info = GrMakeInfoFromTexture(src, w, h, isOpaque);
    dst->setInfo(info);
    dst->setPixelRef(SkNEW_ARGS(SkGrPixelRef, (info, src)))->unref();
}
