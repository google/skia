/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

//  c++ --std=c++11 coreGraphicsPdf2png.cpp -o coreGraphicsPdf2png  -framework ApplicationServices

#include <cstdio>
#include <memory>

#include <ApplicationServices/ApplicationServices.h>

#define ASSERT(x)                                \
    do {                                         \
        if (!(x)) {                              \
            fprintf(stderr, "ERROR: " __FILE__   \
                    ":%d (%s)\n", __LINE__, #x); \
            return 1;                            \
        }                                        \
    } while (false)                              \

const int PAGE = 1;

int main(int argc, char** argv) {
    if (argc <= 1 || !*(argv[1]) || 0 == strcmp(argv[1], "-")) {
        fprintf(stderr, "usage:\n\t%s INPUT_PDF_FILE_PATH [OUTPUT_PNG_PATH]\n", argv[0]);
        return 1;
    }
    const char* output = argc > 2 ? argv[2] : nullptr;
    CGDataProviderRef data = CGDataProviderCreateWithFilename(argv[1]);
    ASSERT(data);
    CGPDFDocumentRef pdf = CGPDFDocumentCreateWithProvider(data);
    CGDataProviderRelease(data);
    ASSERT(pdf);
    CGPDFPageRef page = CGPDFDocumentGetPage(pdf, PAGE);
    ASSERT(page);
    CGRect bounds = CGPDFPageGetBoxRect(page, kCGPDFMediaBox);
    int w = (int)CGRectGetWidth(bounds);
    int h = (int)CGRectGetHeight(bounds);
    CGBitmapInfo info = kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedLast;
    CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
    ASSERT(cs);
    std::unique_ptr<uint32_t[]> bitmap(new uint32_t[w * h]);
    memset(bitmap.get(), 0xFF, 4 * w * h);
    CGContextRef ctx = CGBitmapContextCreate(bitmap.get(), w, h, 8, w * 4, cs, info);
    ASSERT(ctx);
    CGContextDrawPDFPage(ctx, page);
    CGPDFDocumentRelease(pdf);
    CGImageRef image = CGBitmapContextCreateImage(ctx);
    ASSERT(image);
    CGDataConsumerCallbacks procs;
    procs.putBytes = [](void* f, const void* buf, size_t s) {
        return fwrite(buf, 1, s, (FILE*)f);
    };
    procs.releaseConsumer = [](void* info) { fclose((FILE*)info); };
    FILE* ofile = (!output || !output[0] || 0 == strcmp(output, "-"))
        ? stdout : fopen(output, "wb");
    ASSERT(ofile);
    CGDataConsumerRef consumer = CGDataConsumerCreate(ofile, &procs);
    ASSERT(consumer);
    CGImageDestinationRef dst =
        CGImageDestinationCreateWithDataConsumer(consumer, kUTTypePNG, 1, nullptr);
    CFRelease(consumer);
    ASSERT(dst);
    CGImageDestinationAddImage(dst, image, nullptr);
    ASSERT(CGImageDestinationFinalize(dst));
    CFRelease(dst);
    CGImageRelease(image);
    CGColorSpaceRelease(cs);
    CGContextRelease(ctx);
    return 0;
}


