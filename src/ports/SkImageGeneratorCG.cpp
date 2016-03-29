/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageGeneratorCG.h"

#ifdef SK_BUILD_FOR_MAC
#include <ApplicationServices/ApplicationServices.h>
#endif

#ifdef SK_BUILD_FOR_IOS
#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>
#include <MobileCoreServices/MobileCoreServices.h>
#endif

static CGImageSourceRef data_to_CGImageSrc(SkData* data) {
    CGDataProviderRef cgData = CGDataProviderCreateWithData(data, data->data(), data->size(),
            nullptr);
    if (!cgData) {
        return nullptr;
    }
    CGImageSourceRef imageSrc = CGImageSourceCreateWithDataProvider(cgData, 0);
    CGDataProviderRelease(cgData);
    return imageSrc;
}

SkImageGenerator* SkImageGeneratorCG::NewFromEncodedCG(SkData* data) {
    CGImageSourceRef imageSrc = data_to_CGImageSrc(data);
    if (!imageSrc) {
        return nullptr;
    }

    // Make sure we call CFRelease to free the imageSrc.  Since CFRelease actually takes
    // a const void*, we must cast the imageSrc to a const void*.
    SkAutoTCallVProc<const void, CFRelease> autoImageSrc(imageSrc);

    CFDictionaryRef properties = CGImageSourceCopyPropertiesAtIndex(imageSrc, 0, nullptr);
    if (!properties) {
        return nullptr;
    }

    CFNumberRef widthRef = (CFNumberRef) (CFDictionaryGetValue(properties,
            kCGImagePropertyPixelWidth));
    CFNumberRef heightRef = (CFNumberRef) (CFDictionaryGetValue(properties,
            kCGImagePropertyPixelHeight));
    if (nullptr == widthRef || nullptr == heightRef) {
        return nullptr;
    }
    bool hasAlpha = (bool) (CFDictionaryGetValue(properties,
            kCGImagePropertyHasAlpha));

    int width, height;
    if (!CFNumberGetValue(widthRef, kCFNumberIntType, &width) ||
            !CFNumberGetValue(heightRef, kCFNumberIntType, &height)) {
        return nullptr;
    }

    SkAlphaType alphaType = hasAlpha ? kPremul_SkAlphaType : kOpaque_SkAlphaType;
    SkImageInfo info = SkImageInfo::Make(width, height, kN32_SkColorType, alphaType);

    // FIXME: We have the opportunity to extract color space information here,
    //        though I think it makes sense to wait until we understand how
    //        we want to communicate it to the generator.

    return new SkImageGeneratorCG(info, autoImageSrc.release(), data);
}

SkImageGeneratorCG::SkImageGeneratorCG(const SkImageInfo& info, const void* imageSrc, SkData* data)
    : INHERITED(info)
    , fImageSrc(imageSrc)
    , fData(SkRef(data))
{}

SkData* SkImageGeneratorCG::onRefEncodedData(SK_REFENCODEDDATA_CTXPARAM) {
    return SkRef(fData.get());
}

bool SkImageGeneratorCG::onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
        SkPMColor ctable[], int* ctableCount) {
    if (kN32_SkColorType != info.colorType()) {
        // FIXME: Support other colorTypes.
        return false;
    }

    switch (info.alphaType()) {
        case kOpaque_SkAlphaType:
            if (kOpaque_SkAlphaType != this->getInfo().alphaType()) {
                return false;
            }
            break;
        case kPremul_SkAlphaType:
            break;
        default:
            return false;
    }

    CGImageRef image = CGImageSourceCreateImageAtIndex((CGImageSourceRef) fImageSrc.get(), 0,
            nullptr);
    if (!image) {
        return false;
    }
    SkAutoTCallVProc<CGImage, CGImageRelease> autoImage(image);

    // FIXME: Using this function (as opposed to swizzling ourselves) greatly
    //        restricts the color and alpha types that we support.  If we
    //        swizzle ourselves, we can add support for:
    //            kUnpremul_SkAlphaType
    //            16-bit per component RGBA
    //            kGray_8_SkColorType
    //            kIndex_8_SkColorType
    //        Additionally, it would be interesting to compare the performance
    //        of SkSwizzler with CG's built in swizzler.
    if (!SkCopyPixelsFromCGImage(info, rowBytes, pixels, image)) {
        return false;
    }

    return true;
}
