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
#include "src/utils/mac/SkUniqueCFRef.h"

#include <climits>

static CGBitmapInfo compute_cgalpha_info_rgba(SkAlphaType at) {
    CGBitmapInfo info = kCGBitmapByteOrder32Big;
    switch (at) {
        case kUnknown_SkAlphaType:                                          break;
        case kOpaque_SkAlphaType:   info |= kCGImageAlphaNoneSkipLast;      break;
        case kPremul_SkAlphaType:   info |= kCGImageAlphaPremultipliedLast; break;
        case kUnpremul_SkAlphaType: info |= kCGImageAlphaLast;              break;
    }
    return info;
}

static CGBitmapInfo compute_cgalpha_info_bgra(SkAlphaType at) {
    CGBitmapInfo info = kCGBitmapByteOrder32Little;
    switch (at) {
        case kUnknown_SkAlphaType:                                           break;
        case kOpaque_SkAlphaType:   info |= kCGImageAlphaNoneSkipFirst;      break;
        case kPremul_SkAlphaType:   info |= kCGImageAlphaPremultipliedFirst; break;
        case kUnpremul_SkAlphaType: info |= kCGImageAlphaFirst;              break;
    }
    return info;
}
static CGBitmapInfo compute_cgalpha_info_4444(SkAlphaType at) {
    CGBitmapInfo info = kCGBitmapByteOrder16Little;
    switch (at) {
        case kOpaque_SkAlphaType: info |= kCGImageAlphaNoneSkipLast;      break;
        default:                  info |= kCGImageAlphaPremultipliedLast; break;
    }
    return info;
}

CGImageRef SkCreateCGImageRefWithColorspace(const SkBitmap& bm,
                                            CGColorSpaceRef colorSpace) {
    if (bm.drawsNothing()) {
        return nullptr;
    }
    size_t bitsPerComponent = 0;
    CGBitmapInfo info = 0;
    std::unique_ptr<SkBitmap> bitmap;

    switch (bm.colorType()) {
        case kRGB_565_SkColorType:
            // TODO: Support other SkColorType cases?
            bitmap.reset(new SkBitmap);
            // here we make a deep copy of the pixels, since CG won't take our
            // 565 directly
            bitmap->allocPixels(bm.info().makeColorType(kRGBA_8888_SkColorType));
            bm.readPixels(bitmap->info(), bitmap->getPixels(), bitmap->rowBytes(), 0, 0);
            bitsPerComponent = 8;
            info = compute_cgalpha_info_rgba(kOpaque_SkAlphaType);
            break;
        case kRGBA_8888_SkColorType:
            bitsPerComponent = 8;
            info = compute_cgalpha_info_rgba(bm.alphaType());
            break;
        case kBGRA_8888_SkColorType:
            bitsPerComponent = 8;
            info = compute_cgalpha_info_bgra(bm.alphaType());
            break;
        case kARGB_4444_SkColorType:
            bitsPerComponent = 4;
            info = compute_cgalpha_info_4444(bm.alphaType());
            break;
        default:
            return nullptr;
    }
    if (!bitmap) {
        bitmap.reset(new SkBitmap(bm));  // Shallow copy of bitmap.
    }

    SkPixmap pm = bitmap->pixmap();  // Copy bitmap info before releasing it.
    const size_t s = bitmap->computeByteSize();
    void* pixels = bitmap->getPixels();

    // our provider "owns" the bitmap*, and will take care of deleting it
    SkUniqueCFRef<CGDataProviderRef> dataRef(CGDataProviderCreateWithData(
            bitmap.release(), pixels, s,
            [](void* p, const void*, size_t) { delete reinterpret_cast<SkBitmap*>(p); }));

    SkUniqueCFRef<CGColorSpaceRef> rgb;
    if (nullptr == colorSpace) {
        rgb.reset(CGColorSpaceCreateDeviceRGB());
        colorSpace = rgb.get();
    }
    return CGImageCreate(pm.width(), pm.height(), bitsPerComponent,
                         pm.info().bytesPerPixel() * CHAR_BIT, pm.rowBytes(), colorSpace,
                         info, dataRef.get(), nullptr, false, kCGRenderingIntentDefault);
}

void SkCGDrawBitmap(CGContextRef cg, const SkBitmap& bm, float x, float y) {
    SkUniqueCFRef<CGImageRef> img(SkCreateCGImageRef(bm));

    if (img) {
        CGRect r = CGRectMake(0, 0, bm.width(), bm.height());

        CGContextSaveGState(cg);
        CGContextTranslateCTM(cg, x, r.size.height + y);
        CGContextScaleCTM(cg, 1, -1);

        CGContextDrawImage(cg, r, img.get());

        CGContextRestoreGState(cg);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

CGContextRef SkCreateCGContext(const SkPixmap& pmap) {
    CGBitmapInfo cg_bitmap_info = 0;
    size_t bitsPerComponent = 0;
    switch (pmap.colorType()) {
        case kRGBA_8888_SkColorType:
            bitsPerComponent = 8;
            cg_bitmap_info = compute_cgalpha_info_rgba(pmap.alphaType());
            break;
        case kBGRA_8888_SkColorType:
            bitsPerComponent = 8;
            cg_bitmap_info = compute_cgalpha_info_bgra(pmap.alphaType());
            break;
        default:
            return nullptr;   // no other colortypes are supported (for now)
    }

    size_t rb = pmap.addr() ? pmap.rowBytes() : 0;
    SkUniqueCFRef<CGColorSpaceRef> cs(CGColorSpaceCreateDeviceRGB());
    CGContextRef cg = CGBitmapContextCreate(pmap.writable_addr(), pmap.width(), pmap.height(),
                                            bitsPerComponent, rb, cs.get(), cg_bitmap_info);
    return cg;
}

bool SkCopyPixelsFromCGImage(const SkImageInfo& info, size_t rowBytes, void* pixels,
                             CGImageRef image) {
    CGBitmapInfo cg_bitmap_info = 0;
    size_t bitsPerComponent = 0;
    switch (info.colorType()) {
        case kRGBA_8888_SkColorType:
            bitsPerComponent = 8;
            cg_bitmap_info = compute_cgalpha_info_rgba(info.alphaType());
            break;
        case kBGRA_8888_SkColorType:
            bitsPerComponent = 8;
            cg_bitmap_info = compute_cgalpha_info_bgra(info.alphaType());
            break;
        default:
            return false;   // no other colortypes are supported (for now)
    }

    SkUniqueCFRef<CGColorSpaceRef> cs(CGColorSpaceCreateDeviceRGB());
    SkUniqueCFRef<CGContextRef> cg(CGBitmapContextCreate(
                pixels, info.width(), info.height(), bitsPerComponent,
                rowBytes, cs.get(), cg_bitmap_info));
    if (!cg) {
        return false;
    }

    // use this blend mode, to avoid having to erase the pixels first, and to avoid CG performing
    // any blending (which could introduce errors and be slower).
    CGContextSetBlendMode(cg.get(), kCGBlendModeCopy);

    CGContextDrawImage(cg.get(), CGRectMake(0, 0, info.width(), info.height()), image);
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
