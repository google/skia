/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageEncoderPriv.h"

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)

#include "SkBitmap.h"
#include "SkCGUtils.h"
#include "SkColorPriv.h"
#include "SkData.h"
#include "SkStream.h"
#include "SkStreamPriv.h"
#include "SkTemplates.h"
#include "SkUnPreMultiply.h"

#ifdef SK_BUILD_FOR_MAC
#include <ApplicationServices/ApplicationServices.h>
#endif

#ifdef SK_BUILD_FOR_IOS
#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>
#include <MobileCoreServices/MobileCoreServices.h>
#endif

static size_t consumer_put(void* info, const void* buffer, size_t count) {
    SkWStream* stream = reinterpret_cast<SkWStream*>(info);
    return stream->write(buffer, count) ? count : 0;
}

static void consumer_release(void* info) {
    // we do nothing, since by design we don't "own" the stream (i.e. info)
}

static CGDataConsumerRef SkStreamToCGDataConsumer(SkWStream* stream) {
    CGDataConsumerCallbacks procs;
    procs.putBytes = consumer_put;
    procs.releaseConsumer = consumer_release;
    // we don't own/reference the stream, so it our consumer must not live
    // longer that our caller's ownership of the stream
    return CGDataConsumerCreate(stream, &procs);
}

static CGImageDestinationRef SkStreamToImageDestination(SkWStream* stream,
                                                        CFStringRef type) {
    CGDataConsumerRef consumer = SkStreamToCGDataConsumer(stream);
    if (nullptr == consumer) {
        return nullptr;
    }
    SkAutoTCallVProc<const void, CFRelease> arconsumer(consumer);

    return CGImageDestinationCreateWithDataConsumer(consumer, type, 1, nullptr);
}

/*  Encode bitmaps via CGImageDestination. We setup a DataConsumer which writes
    to our SkWStream. Since we don't reference/own the SkWStream, our consumer
    must only live for the duration of the onEncode() method.
 */
bool SkEncodeImageWithCG(SkWStream* stream, const SkPixmap& pixmap, SkEncodedImageFormat format) {
    SkBitmap bm;
    if (!bm.installPixels(pixmap)) {
        return false;
    }
    bm.setImmutable();

    CFStringRef type;
    switch (format) {
        case SkEncodedImageFormat::kICO:
            type = kUTTypeICO;
            break;
        case SkEncodedImageFormat::kBMP:
            type = kUTTypeBMP;
            break;
        case SkEncodedImageFormat::kGIF:
            type = kUTTypeGIF;
            break;
        case SkEncodedImageFormat::kJPEG:
            type = kUTTypeJPEG;
            break;
        case SkEncodedImageFormat::kPNG:
            // PNG encoding an ARGB_4444 bitmap gives the following errors in GM:
            // <Error>: CGImageDestinationAddImage image could not be converted to destination
            // format.
            // <Error>: CGImageDestinationFinalize image destination does not have enough images
            // So instead we copy to 8888.
            if (bm.colorType() == kARGB_4444_SkColorType) {
                SkBitmap bitmapN32;
                bitmapN32.allocPixels(bm.info().makeColorType(kN32_SkColorType));
                bm.readPixels(bitmapN32.info(), bitmapN32.getPixels(), bitmapN32.rowBytes(), 0, 0);
                bm.swap(bitmapN32);
            }
            type = kUTTypePNG;
            break;
        default:
            return false;
    }

    CGImageDestinationRef dst = SkStreamToImageDestination(stream, type);
    if (nullptr == dst) {
        return false;
    }
    SkAutoTCallVProc<const void, CFRelease> ardst(dst);

    CGImageRef image = SkCreateCGImageRef(bm);
    if (nullptr == image) {
        return false;
    }
    SkAutoTCallVProc<CGImage, CGImageRelease> agimage(image);

    CGImageDestinationAddImage(dst, image, nullptr);
    return CGImageDestinationFinalize(dst);
}

#endif//defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
