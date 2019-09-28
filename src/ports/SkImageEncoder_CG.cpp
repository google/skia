/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/images/SkImageEncoderPriv.h"

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)

#include "include/core/SkBitmap.h"
#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "include/core/SkUnPreMultiply.h"
#include "include/private/SkColorData.h"
#include "include/private/SkTemplates.h"
#include "include/utils/mac/SkCGUtils.h"
#include "src/core/SkStreamPriv.h"
#include "src/utils/mac/SkUniqueCFRef.h"

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

static SkUniqueCFRef<CGDataConsumerRef> SkStreamToCGDataConsumer(SkWStream* stream) {
    CGDataConsumerCallbacks procs;
    procs.putBytes = consumer_put;
    procs.releaseConsumer = consumer_release;
    // we don't own/reference the stream, so it our consumer must not live
    // longer that our caller's ownership of the stream
    return SkUniqueCFRef<CGDataConsumerRef>(CGDataConsumerCreate(stream, &procs));
}

static SkUniqueCFRef<CGImageDestinationRef> SkStreamToImageDestination(SkWStream* stream,
                                                                       CFStringRef type) {
    SkUniqueCFRef<CGDataConsumerRef> consumer = SkStreamToCGDataConsumer(stream);
    if (nullptr == consumer) {
        return nullptr;
    }

    return SkUniqueCFRef<CGImageDestinationRef>(
            CGImageDestinationCreateWithDataConsumer(consumer.get(), type, 1, nullptr));
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

    SkUniqueCFRef<CGImageDestinationRef> dst = SkStreamToImageDestination(stream, type);
    if (nullptr == dst) {
        return false;
    }

    SkUniqueCFRef<CGImageRef> image(SkCreateCGImageRef(bm));
    if (nullptr == image) {
        return false;
    }

    CGImageDestinationAddImage(dst.get(), image.get(), nullptr);
    return CGImageDestinationFinalize(dst.get());
}

#endif//defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
