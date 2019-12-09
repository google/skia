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

#include <climits>

template <typename T, void (*F)(T)>
struct SkUniqueCFRef_Release { void operator()(T v) { F(v); } };

template <typename T, void (*F)(T)>
using SkUniqueCFRef = std::unique_ptr<typename std::remove_pointer<T>::type,
                                      SkUniqueCFRef_Release<T, F>>;
namespace {
struct BitmapInfo {
    size_t bitsPerComponent = 0;
    CGBitmapInfo info = 0;
    bool good = false;
};
}  // namespace

static BitmapInfo get_bitmap_info(SkColorType skColorType, SkAlphaType skAlphaType) {
    BitmapInfo r;
    switch (skColorType) {
        case kRGBA_8888_SkColorType:
            r.bitsPerComponent = 8;
            r.info = kCGBitmapByteOrder32Big;
            switch (skAlphaType) {
                case kUnknown_SkAlphaType:                                            break;
                case kOpaque_SkAlphaType:   r.info |= kCGImageAlphaNoneSkipLast;      break;
                case kPremul_SkAlphaType:   r.info |= kCGImageAlphaPremultipliedLast; break;
                case kUnpremul_SkAlphaType: r.info |= kCGImageAlphaLast;              break;
            }
            r.good = true;
            break;
        case kBGRA_8888_SkColorType:
            r.bitsPerComponent = 8;
            r.info = kCGBitmapByteOrder32Little;
            switch (skAlphaType) {
                case kUnknown_SkAlphaType:                                             break;
                case kOpaque_SkAlphaType:   r.info |= kCGImageAlphaNoneSkipFirst;      break;
                case kPremul_SkAlphaType:   r.info |= kCGImageAlphaPremultipliedFirst; break;
                case kUnpremul_SkAlphaType: r.info |= kCGImageAlphaFirst;              break;
            }
            r.good = true;
            break;
        case kARGB_4444_SkColorType:
            r.bitsPerComponent = 4;
            r.info = kCGBitmapByteOrder16Little;
            switch(skAlphaType) {
                case kOpaque_SkAlphaType:   r.info |= kCGImageAlphaNoneSkipLast;      break;
                default:                    r.info |= kCGImageAlphaPremultipliedLast; break;
            }
            r.good = true;
            break;
        default:
            // Unsupported SkColorType
            r.good = false;
    }
    return r;
}

CGImageRef SkCreateCGImageRefWithColorspace(const SkBitmap& bm,
                                            CGColorSpaceRef colorSpace) {
    if (bm.drawsNothing()) {
        return nullptr;
    }
    BitmapInfo bitmapInfo = get_bitmap_info(bm.colorType(), bm.alphaType());
    SkBitmap* bitmap = nullptr;
    if (bitmapInfo.good) {
        bitmap = new SkBitmap(bm);
    } else {
        bitmap = new SkBitmap;
        bitmap->allocPixels(bm.info().makeColorType(kN32_SkColorType));
        bm.readPixels(bitmap->info(), bitmap->getPixels(), bitmap->rowBytes(), 0, 0);
    }
    const int w = bitmap->width();
    const int h = bitmap->height();
    const size_t bytesPerPixel = bitmap->bytesPerPixel();
    const size_t rowBytes = bitmap->rowBytes();
    void* const pixels = bitmap->getPixels();
    const size_t byteSize = bitmap->computeByteSize();
    // our provider "owns" the bitmap*, and will take care of deleting it
    SkUniqueCFRef<CGDataProviderRef, CGDataProviderRelease> dataRef(
            CGDataProviderCreateWithData(
                bitmap, pixels, byteSize, [](void* info, const void*, size_t) {
                    delete reinterpret_cast<SkBitmap*>(info);
                }));
    SkUniqueCFRef<CGColorSpaceRef, CGColorSpaceRelease> rgb;
    if (nullptr == colorSpace) {
        rgb.reset(CGColorSpaceCreateDeviceRGB());
        colorSpace = rgb.get();
    }
    CGImageRef ref = CGImageCreate(w, h, bitmapInfo.bitsPerComponent,
                                   bytesPerPixel * CHAR_BIT,
                                   rowBytes, colorSpace, bitmapInfo.info, dataRef.get(),
                                   nullptr, false, kCGRenderingIntentDefault);
    return ref;
}

// Caller *must* promise that the ImageRef will not outlive the pixamp.
static CGImageRef create_cgimageref(const SkPixmap& pixmap) {
    if (pixmap.info().isEmpty() || !pixmap.addr()) {
        return nullptr;
    }
    BitmapInfo bitmapInfo = get_bitmap_info(pixmap.colorType(), pixmap.alphaType());
    if (!bitmapInfo.good) {
        return nullptr;
    }
    SkUniqueCFRef<CGColorSpaceRef, CGColorSpaceRelease> rgb(CGColorSpaceCreateDeviceRGB());
    SkUniqueCFRef<CGContextRef, CGContextRelease> cgBitmapContext(
            CGBitmapContextCreate(pixmap.writable_addr(),
                                  pixmap.width(),
                                  pixmap.height(),
                                  bitmapInfo.bitsPerComponent,
                                  pixmap.rowBytes(),
                                  rgb.get(),
                                  bitmapInfo.info));
    return CGBitmapContextCreateImage(cgBitmapContext.get());
}

static void draw_cgimage(CGContextRef cg, CGImageRef img, SkISize s, float x, float y) {
    SkASSERT(img);
    CGRect r = {{0, 0}, {(float)s.width(), (float)s.height()}};
    CGContextSaveGState(cg);
    CGContextTranslateCTM(cg, x, r.size.height + y);
    CGContextScaleCTM(cg, 1, -1);
    CGContextDrawImage(cg, r, img);
    CGContextRestoreGState(cg);
}

void SkCGDrawBitmap(CGContextRef cg, const SkBitmap& bm, float x, float y) {
    SkUniqueCFRef<CGImageRef, CGImageRelease> img(SkCreateCGImageRef(bm));
    if (img) {
        draw_cgimage(cg, img.get(), bm.dimensions(), x, y);
    }
}

void SkCGDrawPixmap(CGContextRef cg, const SkPixmap& pm, float x, float y) {
    SkUniqueCFRef<CGImageRef, CGImageRelease> img(create_cgimageref(pm));
    if (img) {
        draw_cgimage(cg, img.get(), pm.dimensions(), x, y);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

CGContextRef SkCreateCGContext(const SkPixmap& pmap) {
    BitmapInfo bitmapInfo = get_bitmap_info(pmap.colorType(), pmap.alphaType());
    if (!bitmapInfo.good || 8 != bitmapInfo.bitsPerComponent) {
        return nullptr;
    }
    size_t rb = pmap.addr() ? pmap.rowBytes() : 0;
    SkUniqueCFRef<CGColorSpaceRef, CGColorSpaceRelease> rgb(CGColorSpaceCreateDeviceRGB());
    return CGBitmapContextCreate(pmap.writable_addr(), pmap.width(), pmap.height(),
                                 bitmapInfo.bitsPerComponent, rb, rgb.get(), bitmapInfo.info);
}

bool SkCopyPixelsFromCGImage(const SkImageInfo& info, size_t rowBytes, void* pixels,
                             CGImageRef image) {
    BitmapInfo bitmapInfo = get_bitmap_info(info.colorType(), info.alphaType());
    if (!bitmapInfo.good || 8 != bitmapInfo.bitsPerComponent) {
        return false;
    }

    SkUniqueCFRef<CGColorSpaceRef, CGColorSpaceRelease> rgb(CGColorSpaceCreateDeviceRGB());
    SkUniqueCFRef<CGContextRef, CGContextRelease> cg(
            CGBitmapContextCreate(pixels, info.width(), info.height(),
                                  bitmapInfo.bitsPerComponent, rowBytes, rgb.get(),
                                  bitmapInfo.info));
    if (!cg) {
        return false;
    }

    // use this blend mode, to avoid having to erase the pixels first, and to avoid CG performing
    // any blending (which could introduce errors and be slower).
    CGContextSetBlendMode(cg.get(), kCGBlendModeCopy);

    CGRect rect = {{0, 0}, {(float)info.width(), (float)info.height()}};
    CGContextDrawImage(cg.get(), rect, image);
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
