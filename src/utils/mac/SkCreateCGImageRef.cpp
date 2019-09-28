/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)

#include "include/core/SkBitmap.h"
#include "include/private/SkColorData.h"
#include "include/private/SkMacros.h"
#include "include/private/SkTo.h"
#include "include/utils/mac/SkCGUtils.h"

static CGBitmapInfo ComputeCGAlphaInfo_RGBA(SkAlphaType at) {
    CGBitmapInfo info = kCGBitmapByteOrder32Big;
    switch (at) {
        case kUnknown_SkAlphaType:
            break;
        case kOpaque_SkAlphaType:
            info |= kCGImageAlphaNoneSkipLast;
            break;
        case kPremul_SkAlphaType:
            info |= kCGImageAlphaPremultipliedLast;
            break;
        case kUnpremul_SkAlphaType:
            info |= kCGImageAlphaLast;
            break;
    }
    return info;
}

static CGBitmapInfo ComputeCGAlphaInfo_BGRA(SkAlphaType at) {
    CGBitmapInfo info = kCGBitmapByteOrder32Little;
    switch (at) {
        case kUnknown_SkAlphaType:
            break;
        case kOpaque_SkAlphaType:
            info |= kCGImageAlphaNoneSkipFirst;
            break;
        case kPremul_SkAlphaType:
            info |= kCGImageAlphaPremultipliedFirst;
            break;
        case kUnpremul_SkAlphaType:
            info |= kCGImageAlphaFirst;
            break;
    }
    return info;
}

static void SkBitmap_ReleaseInfo(void* info, const void* pixelData, size_t size) {
    SkBitmap* bitmap = reinterpret_cast<SkBitmap*>(info);
    delete bitmap;
}

static bool getBitmapInfo(const SkBitmap& bm,
                          size_t* bitsPerComponent,
                          CGBitmapInfo* info,
                          bool* upscaleTo32) {
    if (upscaleTo32) {
        *upscaleTo32 = false;
    }

    switch (bm.colorType()) {
        case kRGB_565_SkColorType:
#if 0
            // doesn't see quite right. Are they thinking 1555?
            *bitsPerComponent = 5;
            *info = kCGBitmapByteOrder16Little | kCGImageAlphaNone;
#else
            if (upscaleTo32) {
                *upscaleTo32 = true;
            }
            // now treat like RGBA
            *bitsPerComponent = 8;
            *info = ComputeCGAlphaInfo_RGBA(kOpaque_SkAlphaType);
#endif
            break;
        case kRGBA_8888_SkColorType:
            *bitsPerComponent = 8;
            *info = ComputeCGAlphaInfo_RGBA(bm.alphaType());
            break;
        case kBGRA_8888_SkColorType:
            *bitsPerComponent = 8;
            *info = ComputeCGAlphaInfo_BGRA(bm.alphaType());
            break;
        case kARGB_4444_SkColorType:
            *bitsPerComponent = 4;
            *info = kCGBitmapByteOrder16Little;
            if (bm.isOpaque()) {
                *info |= kCGImageAlphaNoneSkipLast;
            } else {
                *info |= kCGImageAlphaPremultipliedLast;
            }
            break;
        default:
            return false;
    }
    return true;
}

static SkBitmap* prepareForImageRef(const SkBitmap& bm,
                                    size_t* bitsPerComponent,
                                    CGBitmapInfo* info) {
    bool upscaleTo32;
    if (!getBitmapInfo(bm, bitsPerComponent, info, &upscaleTo32)) {
        return nullptr;
    }

    SkBitmap* copy;
    if (upscaleTo32) {
        copy = new SkBitmap;
        // here we make a deep copy of the pixels, since CG won't take our
        // 565 directly
        copy->allocPixels(bm.info().makeColorType(kN32_SkColorType));
        bm.readPixels(copy->info(), copy->getPixels(), copy->rowBytes(), 0, 0);
    } else {
        copy = new SkBitmap(bm);
    }
    return copy;
}

CGImageRef SkCreateCGImageRefWithColorspace(const SkBitmap& bm,
                                            CGColorSpaceRef colorSpace) {
    size_t bitsPerComponent SK_INIT_TO_AVOID_WARNING;
    CGBitmapInfo info       SK_INIT_TO_AVOID_WARNING;

    SkBitmap* bitmap = prepareForImageRef(bm, &bitsPerComponent, &info);
    if (nullptr == bitmap) {
        return nullptr;
    }

    const int w = bitmap->width();
    const int h = bitmap->height();
    const size_t s = bitmap->computeByteSize();

    // our provider "owns" the bitmap*, and will take care of deleting it
    CGDataProviderRef dataRef = CGDataProviderCreateWithData(bitmap, bitmap->getPixels(), s,
                                                             SkBitmap_ReleaseInfo);

    bool releaseColorSpace = false;
    if (nullptr == colorSpace) {
        colorSpace = CGColorSpaceCreateDeviceRGB();
        releaseColorSpace = true;
    }

    CGImageRef ref = CGImageCreate(w, h, bitsPerComponent,
                                   bitmap->bytesPerPixel() * 8,
                                   bitmap->rowBytes(), colorSpace, info, dataRef,
                                   nullptr, false, kCGRenderingIntentDefault);

    if (releaseColorSpace) {
        CGColorSpaceRelease(colorSpace);
    }
    CGDataProviderRelease(dataRef);
    return ref;
}

void SkCGDrawBitmap(CGContextRef cg, const SkBitmap& bm, float x, float y) {
    CGImageRef img = SkCreateCGImageRef(bm);

    if (img) {
        CGRect r = CGRectMake(0, 0, bm.width(), bm.height());

        CGContextSaveGState(cg);
        CGContextTranslateCTM(cg, x, r.size.height + y);
        CGContextScaleCTM(cg, 1, -1);

        CGContextDrawImage(cg, r, img);

        CGContextRestoreGState(cg);

        CGImageRelease(img);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

CGContextRef SkCreateCGContext(const SkPixmap& pmap) {
    CGBitmapInfo cg_bitmap_info = 0;
    size_t bitsPerComponent = 0;
    switch (pmap.colorType()) {
        case kRGBA_8888_SkColorType:
            bitsPerComponent = 8;
            cg_bitmap_info = ComputeCGAlphaInfo_RGBA(pmap.alphaType());
            break;
        case kBGRA_8888_SkColorType:
            bitsPerComponent = 8;
            cg_bitmap_info = ComputeCGAlphaInfo_BGRA(pmap.alphaType());
            break;
        default:
            return nullptr;   // no other colortypes are supported (for now)
    }

    size_t rb = pmap.addr() ? pmap.rowBytes() : 0;
    CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
    CGContextRef cg = CGBitmapContextCreate(pmap.writable_addr(), pmap.width(), pmap.height(),
                                            bitsPerComponent, rb, cs, cg_bitmap_info);
    CFRelease(cs);
    return cg;
}

bool SkCopyPixelsFromCGImage(const SkImageInfo& info, size_t rowBytes, void* pixels,
                             CGImageRef image) {
    CGBitmapInfo cg_bitmap_info = 0;
    size_t bitsPerComponent = 0;
    switch (info.colorType()) {
        case kRGBA_8888_SkColorType:
            bitsPerComponent = 8;
            cg_bitmap_info = ComputeCGAlphaInfo_RGBA(info.alphaType());
            break;
        case kBGRA_8888_SkColorType:
            bitsPerComponent = 8;
            cg_bitmap_info = ComputeCGAlphaInfo_BGRA(info.alphaType());
            break;
        default:
            return false;   // no other colortypes are supported (for now)
    }

    CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
    CGContextRef cg = CGBitmapContextCreate(pixels, info.width(), info.height(), bitsPerComponent,
                                            rowBytes, cs, cg_bitmap_info);
    CFRelease(cs);
    if (nullptr == cg) {
        return false;
    }

    // use this blend mode, to avoid having to erase the pixels first, and to avoid CG performing
    // any blending (which could introduce errors and be slower).
    CGContextSetBlendMode(cg, kCGBlendModeCopy);

    CGContextDrawImage(cg, CGRectMake(0, 0, info.width(), info.height()), image);
    CGContextRelease(cg);
    return true;
}

bool SkCreateBitmapFromCGImage(SkBitmap* dst, CGImageRef image) {
    const int width = SkToInt(CGImageGetWidth(image));
    const int height = SkToInt(CGImageGetHeight(image));
    SkImageInfo info = SkImageInfo::MakeN32Premul(width, height);

    SkBitmap tmp;
    if (!tmp.tryAllocPixels(info)) {
        return false;
    }

    if (!SkCopyPixelsFromCGImage(tmp.info(), tmp.rowBytes(), tmp.getPixels(), image)) {
        return false;
    }

    CGImageAlphaInfo cgInfo = CGImageGetAlphaInfo(image);
    switch (cgInfo) {
        case kCGImageAlphaNone:
        case kCGImageAlphaNoneSkipLast:
        case kCGImageAlphaNoneSkipFirst:
            SkASSERT(SkBitmap::ComputeIsOpaque(tmp));
            tmp.setAlphaType(kOpaque_SkAlphaType);
            break;
        default:
            // we don't know if we're opaque or not, so compute it.
            if (SkBitmap::ComputeIsOpaque(tmp)) {
                tmp.setAlphaType(kOpaque_SkAlphaType);
            }
    }

    *dst = tmp;
    return true;
}

sk_sp<SkImage> SkMakeImageFromCGImage(CGImageRef src) {
    SkBitmap bm;
    if (!SkCreateBitmapFromCGImage(&bm, src)) {
        return nullptr;
    }

    bm.setImmutable();
    return SkImage::MakeFromBitmap(bm);
}

#endif//defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
