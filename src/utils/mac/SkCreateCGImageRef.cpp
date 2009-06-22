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
            // try to match our rgba ordering in SkColorPriv, but take into
            // account that the data layout could have been overridden in
            // SkUserConfig.
#define HAS_ARGB_SHIFTS(a, r, g, b) \
            (SK_A32_SHIFT == (a) && SK_R32_SHIFT == (r) \
             && SK_G32_SHIFT == (g) && SK_B32_SHIFT == (b))
#if defined(SK_CPU_LENDIAN) && HAS_ARGB_SHIFTS(24, 0, 8, 16) \
 || defined(SK_CPU_BENDIAN) && HAS_ARGB_SHIFTS(0, 24, 16, 8)
            // The default rgba ordering from SkColorPriv.h
            *info = kCGBitmapByteOrder32Big |
                    kCGImageAlphaPremultipliedLast;
#elif defined(SK_CPU_LENDIAN) && HAS_ARGB_SHIFTS(24, 16, 8, 0) \
   || defined(SK_CPU_BENDIAN) && HAS_ARGB_SHIFTS(24, 16, 8, 0)
            // Matches the CGBitmapInfo that Apple recommends for best
            // performance, used by google chrome.
            *info = kCGBitmapByteOrder32Host |
                    kCGImageAlphaPremultipliedFirst;
#else
// ...add more formats as required...
#warning Cannot convert SkBitmap to CGImageRef with these shiftmasks. \
            This will probably not work.
            // Legacy behavior. Perhaps turn this into an error at some
            // point.
            *info = kCGBitmapByteOrder32Big |
                    kCGImageAlphaPremultipliedLast;
#endif
#undef HAS_ARGB_SHIFTS
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


