#include "SkCGUtils.h"
#include "SkBitmap.h"

extern CGImageRef SkCreateCGImageRef(const SkBitmap&);

static void SkBitmap_ReleaseInfo(void* info, const void* pixelData, size_t size) {
    SkBitmap* bitmap = reinterpret_cast<SkBitmap*>(info);
    delete bitmap;
}

static SkBitmap* prepareForImageRef(const SkBitmap& bm,
                                    size_t* bitsPerComponent,
                                    CGBitmapInfo* info) {
    switch (bm.config()) {
        case SkBitmap::kARGB_8888_Config:
            *bitsPerComponent = 8;
            // try to match our argb ordering in SkColorPriv
            *info = kCGBitmapByteOrder32Big |
                    kCGImageAlphaPremultipliedLast;
            break;
        case SkBitmap::kRGB_565_Config:
            // doesn't see quite right. Are they thinking 1555?
            *bitsPerComponent = 5;
            *info = kCGBitmapByteOrder16Little;
            break;
        case SkBitmap::kARGB_4444_Config:
            *bitsPerComponent = 4;
            *info = kCGBitmapByteOrder16Little | kCGImageAlphaPremultipliedLast;
            break;
        default:
            return NULL;
    }

    return new SkBitmap(bm);
}

CGImageRef SkCreateCGImageRef(const SkBitmap& bm) {
    size_t bitsPerComponent;
    CGBitmapInfo info;

    SkBitmap* bitmap = prepareForImageRef(bm, &bitsPerComponent, &info);
    if (NULL == bitmap) {
        return NULL;
    }

    const int w = bitmap->width();
    const int h = bitmap->height();
    const size_t s = bitmap->getSize();

    // our provider "owns" the bitmap*, and will take care of deleting it
	// we initially lock it, so we can access the pixels. The bitmap will be deleted in the release
	// proc, which will in turn unlock the pixels
	bitmap->lockPixels();
    CGDataProviderRef dataRef = CGDataProviderCreateWithData(bitmap, bitmap->getPixels(), s,
															 SkBitmap_ReleaseInfo);

    CGColorSpaceRef space = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
    CGImageRef ref = CGImageCreate(w, h, bitsPerComponent,
                                   bitmap->bytesPerPixel() * 8,
                                   bitmap->rowBytes(), space, info, dataRef,
                                   NULL, false, kCGRenderingIntentDefault);
    CGColorSpaceRelease(space);
    CGDataProviderRelease(dataRef);
    return ref;
}


