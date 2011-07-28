
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkCGUtils.h"
#include "SkBitmap.h"
#include "SkColorPriv.h"

static void SkBitmap_ReleaseInfo(void* info, const void* pixelData, size_t size) {
    SkBitmap* bitmap = reinterpret_cast<SkBitmap*>(info);
    delete bitmap;
}

#define HAS_ARGB_SHIFTS(a, r, g, b) \
    (SK_A32_SHIFT == (a) && SK_R32_SHIFT == (r) \
    && SK_G32_SHIFT == (g) && SK_B32_SHIFT == (b))

static bool getBitmapInfo(const SkBitmap& bm, 
                          size_t* bitsPerComponent,
                          CGBitmapInfo* info,
                          bool* upscaleTo32) {
    if (upscaleTo32) {
        *upscaleTo32 = false;
    }
    
    switch (bm.config()) {
        case SkBitmap::kRGB_565_Config:
            if (upscaleTo32) {
                *upscaleTo32 = true;
            }
            // fall through
        case SkBitmap::kARGB_8888_Config:
            *bitsPerComponent = 8;
#if defined(SK_CPU_LENDIAN) && HAS_ARGB_SHIFTS(24, 0, 8, 16) \
|| defined(SK_CPU_BENDIAN) && HAS_ARGB_SHIFTS(0, 24, 16, 8)
            *info = kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedLast;
#elif defined(SK_CPU_LENDIAN) && HAS_ARGB_SHIFTS(24, 16, 8, 0) \
|| defined(SK_CPU_BENDIAN) && HAS_ARGB_SHIFTS(24, 16, 8, 0)
            // Matches the CGBitmapInfo that Apple recommends for best
            // performance, used by google chrome.
            *info = kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst;
#else
            // ...add more formats as required...
#warning Cannot convert SkBitmap to CGImageRef with these shiftmasks. \
This will probably not work.
            // Legacy behavior. Perhaps turn this into an error at some
            // point.
            *info = kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedLast;
#endif
            break;
#if 0
        case SkBitmap::kRGB_565_Config:
            // doesn't see quite right. Are they thinking 1555?
            *bitsPerComponent = 5;
            *info = kCGBitmapByteOrder16Little;
            break;
#endif
        case SkBitmap::kARGB_4444_Config:
            *bitsPerComponent = 4;
            *info = kCGBitmapByteOrder16Little | kCGImageAlphaPremultipliedLast;
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
        return NULL;
    }

    SkBitmap* copy;
    if (upscaleTo32) {
        copy = new SkBitmap;
        // here we make a ceep copy of the pixels, since CG won't take our
        // 565 directly
        bm.copyTo(copy, SkBitmap::kARGB_8888_Config);
    } else {
        copy = new SkBitmap(bm);
    }
    return copy;
}

#undef HAS_ARGB_SHIFTS

CGImageRef SkCreateCGImageRefWithColorspace(const SkBitmap& bm,
                                            CGColorSpaceRef colorSpace) {
    size_t bitsPerComponent SK_INIT_TO_AVOID_WARNING;
    CGBitmapInfo info       SK_INIT_TO_AVOID_WARNING;

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

    bool releaseColorSpace = false;
    if (NULL == colorSpace) {
        colorSpace = CGColorSpaceCreateDeviceRGB();
        releaseColorSpace = true;
    }

    CGImageRef ref = CGImageCreate(w, h, bitsPerComponent,
                                   bitmap->bytesPerPixel() * 8,
                                   bitmap->rowBytes(), colorSpace, info, dataRef,
                                   NULL, false, kCGRenderingIntentDefault);

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

///////////////////////////////////////////////////////////////////////////////

#include "SkStream.h"

class SkAutoPDFRelease {
public:
    SkAutoPDFRelease(CGPDFDocumentRef doc) : fDoc(doc) {}
    ~SkAutoPDFRelease() {
        if (fDoc) {
            CGPDFDocumentRelease(fDoc);
        }
    }
private:
    CGPDFDocumentRef fDoc;
};

static void CGDataProviderReleaseData_FromMalloc(void*, const void* data,
                                                 size_t size) {
    sk_free((void*)data);
}

bool SkPDFDocumentToBitmap(SkStream* stream, SkBitmap* output) {
    size_t size = stream->getLength();
    void* ptr = sk_malloc_throw(size);
    stream->read(ptr, size);
    CGDataProviderRef data = CGDataProviderCreateWithData(NULL, ptr, size,
                                          CGDataProviderReleaseData_FromMalloc);
    if (NULL == data) {
        return false;
    }
    
    CGPDFDocumentRef pdf = CGPDFDocumentCreateWithProvider(data);
    CGDataProviderRelease(data);
    if (NULL == pdf) {
        return false;
    }
    SkAutoPDFRelease releaseMe(pdf);

    CGPDFPageRef page = CGPDFDocumentGetPage(pdf, 1);
    if (NULL == page) {
        return false;
    }
    
    CGRect bounds = CGPDFPageGetBoxRect(page, kCGPDFMediaBox);
    
    int w = (int)CGRectGetWidth(bounds);
    int h = (int)CGRectGetHeight(bounds);
        
    SkBitmap bitmap;
    bitmap.setConfig(SkBitmap::kARGB_8888_Config, w, h);
    bitmap.allocPixels();
    bitmap.eraseColor(SK_ColorWHITE);

    size_t bitsPerComponent;
    CGBitmapInfo info;
    getBitmapInfo(bitmap, &bitsPerComponent, &info, NULL); 

    CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
    CGContextRef ctx = CGBitmapContextCreate(bitmap.getPixels(), w, h,
                                             bitsPerComponent, bitmap.rowBytes(),
                                             cs, info);
    CGColorSpaceRelease(cs);

    if (ctx) {
        CGContextDrawPDFPage(ctx, page);
        CGContextRelease(ctx);
    }

    output->swap(bitmap);
    return true;
}

